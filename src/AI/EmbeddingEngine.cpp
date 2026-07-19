#include "EmbeddingEngine.h"

#include <QByteArray>
#include <QDebug>
#include <QElapsedTimer>
#include <cmath>
#include <cstring>
#include <vector>

#include "lib/llama.cpp/ggml/include/ggml-backend.h"
#include "lib/llama.cpp/ggml/include/gguf.h"
#include "lib/llama.cpp/include/llama.h"

EmbeddingEngine::EmbeddingEngine(const QString& ggufPath) {
  std::string path = ggufPath.toUtf8().toStdString();
  llama_model_params model_params = llama_model_default_params();
  m_model = llama_load_model_from_file(path.c_str(), model_params);
  if (!m_model) return;

  llama_context_params ctx_params = llama_context_default_params();
  ctx_params.n_threads = 4;
  ctx_params.embeddings = true;
  ctx_params.n_ctx = 512;
  m_ctx = llama_new_context_with_model(m_model, ctx_params);
}

EmbeddingEngine::~EmbeddingEngine() {
  if (m_ctx) llama_free(m_ctx);
  if (m_model) llama_free_model(m_model);
}

bool EmbeddingEngine::isValid() const {
  return m_model != nullptr && m_ctx != nullptr;
}

QVector<float> EmbeddingEngine::encode(const QString& text) {
  QMutexLocker locker(&m_mutex);

  QVector<float> result;
  if (!isValid()) return result;

  std::string input = text.toUtf8().toStdString();
  const llama_vocab* vocab = llama_model_get_vocab(m_model);

  // 1. 预查询所需 token 数量
  int needed = llama_tokenize(vocab, input.c_str(), input.size(), nullptr, 0,
                              true, true);
  if (needed == 0) {
    qWarning() << "[ENCODE] tokenize 结果为空";
    return result;
  }
  // 负数 = 需要的空间大小；正数 = 已填入的数量（仅当 buffer 足够时）
  int total_tokens = (needed < 0) ? -needed : needed;

  // 2. 分配完整缓冲区，一次性完成 tokenize
  std::vector<llama_token> all_tokens(total_tokens);
  int actual = llama_tokenize(vocab, input.c_str(), input.size(),
                              all_tokens.data(), total_tokens, true, true);
  if (actual <= 0) {
    qWarning() << "[ENCODE] 正式 tokenize 失败, ret:" << actual;
    return result;
  }
  // actual 此时应为正数且 == total_tokens
  all_tokens.resize(actual);

  // 3. ✅ 在 token 层面截断（而非在 tokenize 之前截断数字）
  const int max_tokens = 510;  // e5-small n_ctx=512, 预留 BOS/EOS
  if (static_cast<int>(all_tokens.size()) > max_tokens) {
    qDebug() << "[ENCODE] ⚠️ 截断 tokens:" << all_tokens.size() << "→"
             << max_tokens;
    all_tokens.resize(max_tokens);
  }

  int n_tokens = static_cast<int>(all_tokens.size());

  // 4. 构建 batch（使用截断后的 tokens）
  int dim = llama_n_embd(m_model);
  llama_batch batch = llama_batch_init(n_tokens, 0, 1);

  for (int i = 0; i < n_tokens; i++) {
    batch.token[i] = all_tokens[i];
    batch.pos[i] = i;
    batch.n_seq_id[i] = 1;
    batch.seq_id[i][0] = 0;
    // batch.logits[i] = (i == n_tokens - 1);
    batch.logits[i] = false;
  }
  batch.n_tokens = n_tokens;

  // 5. decode + pooling + normalize（与你原有逻辑相同）
  QElapsedTimer timer;
  timer.start();

  // ❌ 生成API，会触发警告
  // int rc = llama_decode(m_ctx, batch);
  // ✅ 嵌入专用API，无警告
  int rc = llama_encode(m_ctx, batch);

  qDebug() << "[ENCODE] decode rc:" << rc << ", tokens:" << n_tokens
           << ", 耗时:" << timer.elapsed() << "ms";

  if (rc != 0) {
    qWarning() << "[ENCODE] llama_decode 失败! rc:" << rc;
    llama_batch_free(batch);
    return result;
  }

  const float* emb = llama_get_embeddings_seq(m_ctx, 0);
  if (!emb) {
    qWarning() << "[ENCODE] embeddings_seq 返回空指针!";
    llama_batch_free(batch);
    return result;
  }

  result.resize(dim);
  std::memcpy(result.data(), emb, dim * sizeof(float));

  // L2 归一化
  float norm = 0.0f;
  for (int i = 0; i < dim; ++i) norm += result[i] * result[i];
  norm = std::sqrt(norm);
  if (norm > 1e-6f) {
    for (int i = 0; i < dim; ++i) result[i] /= norm;
  }

  llama_batch_free(batch);
  return result;
}

std::vector<llama_token> EmbeddingEngine::tokenizeText(
    const QString& text) const {
  std::vector<llama_token> result;
  if (!isValid()) return result;

  std::string input = text.toUtf8().toStdString();
  const llama_vocab* vocab = llama_model_get_vocab(m_model);

  // 预查询所需空间
  int needed = llama_tokenize(vocab, input.c_str(), input.size(), nullptr, 0,
                              true, true);
  if (needed == 0) return result;

  int total_tokens = (needed < 0) ? -needed : needed;
  result.resize(total_tokens);

  int actual = llama_tokenize(vocab, input.c_str(), input.size(), result.data(),
                              total_tokens, true, true);
  if (actual <= 0) {
    qWarning() << "[TOKENIZE] 失败, ret:" << actual;
    return {};
  }
  result.resize(actual);
  return result;
}

QVector<float> EmbeddingEngine::encodeTokens(
    const std::vector<llama_token>& tokens) {
  QMutexLocker locker(&m_mutex);  // ✅ 与 encode() 互斥

  QVector<float> result;
  if (!isValid() || tokens.empty()) return result;

  int n_tokens = static_cast<int>(tokens.size());
  int dim = llama_n_embd(m_model);

  // 构建 batch
  llama_batch batch = llama_batch_init(n_tokens, 0, 1);
  for (int i = 0; i < n_tokens; i++) {
    batch.token[i] = tokens[i];
    batch.pos[i] = i;
    batch.n_seq_id[i] = 1;
    batch.seq_id[i][0] = 0;
    // batch.logits[i] = (i == n_tokens - 1);
    batch.logits[i] = false;
  }
  batch.n_tokens = n_tokens;

  // decode
  // ❌ 生成API，会触发警告
  // int rc = llama_decode(m_ctx, batch);
  // ✅ 嵌入专用API，无警告
  int rc = llama_encode(m_ctx, batch);

  if (rc != 0) {
    qWarning() << "[ENCODE_TOKENS] llama_decode 失败! rc:" << rc;
    llama_batch_free(batch);
    return result;
  }

  // pooling + normalize（与 encode() 完全一致）
  const float* emb = llama_get_embeddings_seq(m_ctx, 0);
  if (!emb) {
    qWarning() << "[ENCODE_TOKENS] embeddings_seq 返回空指针!";
    llama_batch_free(batch);
    return result;
  }

  result.resize(dim);
  std::memcpy(result.data(), emb, dim * sizeof(float));

  float norm = 0.0f;
  for (int i = 0; i < dim; ++i) norm += result[i] * result[i];
  norm = std::sqrt(norm);
  if (norm > 1e-6f) {
    for (int i = 0; i < dim; ++i) result[i] /= norm;
  }

  llama_batch_free(batch);
  return result;
}

QString EmbeddingEngine::detokenize(
    const std::vector<llama_token>& tokens) const {
  if (!isValid() || tokens.empty()) return {};

  const llama_vocab* vocab = llama_model_get_vocab(m_model);
  std::string result;
  result.reserve(tokens.size() * 4);  // 预估 UTF-8 长度

  for (llama_token tok : tokens) {
    // llama_token_to_piece 返回当前 token 对应的字节片段
    char buf[256];
    int len = llama_token_to_piece(vocab, tok, buf, sizeof(buf), 0, true);
    if (len > 0) {
      result.append(buf, len);
    }
  }
  return QString::fromUtf8(result.c_str(), static_cast<int>(result.size()));
}