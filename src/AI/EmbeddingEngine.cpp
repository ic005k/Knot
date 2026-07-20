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

/*EmbeddingEngine::EmbeddingEngine(const QString& ggufPath) {
  std::string path = ggufPath.toUtf8().toStdString();
  llama_model_params model_params = llama_model_default_params();
  m_model = llama_load_model_from_file(path.c_str(), model_params);
  if (!m_model) return;

  llama_context_params ctx_params = llama_context_default_params();
  ctx_params.n_threads = 4;
  ctx_params.embeddings = true;

  ctx_params.n_ctx = llama_n_ctx_train(m_model);  // 获取模型训练时的最大上下文
  // 可选：设置一个合理上限，防止内存爆炸
  // ctx_params.n_ctx = std::min(ctx_params.n_ctx, (uint32_t)8192);

  // ✅ 设置物理批处理上限
  // 建议设为 n_ctx 或一个安全值（如 2048/4096），取决于显存大小
  ctx_params.n_ubatch = std::min(ctx_params.n_ctx, (uint32_t)4096);

  m_ctx = llama_new_context_with_model(m_model, ctx_params);

  // ✅ 保存到成员变量，预留 BOS/EOS
  m_maxTokens = static_cast<int>(ctx_params.n_ctx) - 2;
}*/

EmbeddingEngine::EmbeddingEngine(const QString& ggufPath) {
  std::string path = ggufPath.toUtf8().toStdString();
  llama_model_params model_params = llama_model_default_params();
  model_params.n_gpu_layers = 99;
  m_model = llama_load_model_from_file(path.c_str(), model_params);
  if (!m_model) return;

  llama_context_params ctx_params = llama_context_default_params();
  ctx_params.n_threads = 4;
  ctx_params.embeddings = true;
  ctx_params.n_seq_max = 64;

  // ✅ 直接使用模型原生训练上下文，不做人为截断
  ctx_params.n_ctx = llama_n_ctx_train(m_model);
  // ✅ Encoder 模式下 n_ubatch 必须 == n_ctx
  ctx_params.n_ubatch = ctx_params.n_ctx;

  m_ctx = llama_new_context_with_model(m_model, ctx_params);
  // ✅ maxTokens 也要同步使用实际 n_ctx
  m_maxTokens = static_cast<int>(ctx_params.n_ctx) - 2;

  qDebug() << "[EmbeddingEngine] n_ctx:" << ctx_params.n_ctx
           << ", n_ubatch:" << ctx_params.n_ubatch
           << ", n_seq_max:" << ctx_params.n_seq_max
           << ", maxTokens:" << m_maxTokens;
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
  if (static_cast<int>(all_tokens.size()) > m_maxTokens) {
    qDebug() << "[ENCODE] ⚠ 截断 tokens:" << all_tokens.size() << "→"
             << m_maxTokens;
    all_tokens.resize(m_maxTokens);
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

int EmbeddingEngine::embeddingDimension() const {
  if (!m_model) return 0;
  return llama_n_embd(m_model);
}

std::vector<QVector<float>> EmbeddingEngine::encodeBatch(
    const QStringList& texts, int maxBatchSize) {
  QMutexLocker locker(&m_mutex);
  std::vector<QVector<float>> results(texts.size());
  if (!isValid() || texts.isEmpty()) return results;

  const int dim = llama_n_embd(m_model);
  const llama_vocab* vocab = llama_model_get_vocab(m_model);
  const uint32_t n_ubatch = llama_n_ubatch(m_ctx);

  // ✅ 预 tokenize 所有文本，获取精确 token 数
  struct TextInfo {
    int originalIdx;
    std::vector<llama_token> tokens;
  };
  std::vector<TextInfo> allInfos;
  allInfos.reserve(texts.size());

  for (int i = 0; i < texts.size(); ++i) {
    std::string input = texts[i].toUtf8().toStdString();
    int needed = llama_tokenize(vocab, input.c_str(), input.size(), nullptr, 0,
                                true, true);
    int tokCount = (needed < 0) ? -needed : needed;

    TextInfo info{i, {}};
    if (tokCount > 0) {
      info.tokens.resize(tokCount);
      int actual = llama_tokenize(vocab, input.c_str(), input.size(),
                                  info.tokens.data(), tokCount, true, true);
      if (actual > 0) {
        info.tokens.resize(actual);
        // ✅ 单条超过 n_ubatch 时截断到安全范围
        if (info.tokens.size() > n_ubatch) {
          info.tokens.resize(n_ubatch);
        }
      }
    }
    allInfos.push_back(std::move(info));
  }

  // ✅ 按 token 预算动态分组，而非固定条数
  int offset = 0;
  while (offset < (int)allInfos.size()) {
    uint32_t batchTokens = 0;
    int batchEnd = offset;

    // 贪心填充：在不超过 n_ubatch 的前提下尽可能多塞序列
    while (batchEnd < (int)allInfos.size() &&
           batchEnd - offset < maxBatchSize) {
      uint32_t nextTokens =
          static_cast<uint32_t>(allInfos[batchEnd].tokens.size());
      if (batchTokens + nextTokens > n_ubatch) break;
      batchTokens += nextTokens;
      batchEnd++;
    }

    // 如果一条都塞不进（单条就超了），至少处理一条（已截断）
    if (batchEnd == offset) batchEnd = offset + 1;

    int curBatch = batchEnd - offset;

    // === 构建 Batch ===
    llama_batch batch = llama_batch_init(batchTokens, 0, curBatch);
    int seqIdx = 0;
    for (int i = offset; i < batchEnd; ++i) {
      const auto& toks = allInfos[i].tokens;
      for (int t = 0; t < (int)toks.size(); ++t) {
        int idx = batch.n_tokens;
        batch.token[idx] = toks[t];
        batch.pos[idx] = t;
        batch.n_seq_id[idx] = 1;
        batch.seq_id[idx][0] = seqIdx;
        batch.logits[idx] = false;
        batch.n_tokens++;
      }
      seqIdx++;
    }

    // === Encode ===
    auto mem = llama_get_memory(m_ctx);
    if (mem) llama_memory_clear(mem, true);

    QElapsedTimer timer;
    timer.start();
    int rc = llama_encode(m_ctx, batch);

    if (rc == 0) {
      int outSeqIdx = 0;
      for (int i = offset; i < batchEnd; ++i) {
        const float* emb = llama_get_embeddings_seq(m_ctx, outSeqIdx);
        if (emb) {
          QVector<float> vec(dim);
          std::memcpy(vec.data(), emb, dim * sizeof(float));
          float norm = 0.0f;
          for (int d = 0; d < dim; ++d) norm += vec[d] * vec[d];
          norm = std::sqrt(norm);
          if (norm > 1e-6f)
            for (int d = 0; d < dim; ++d) vec[d] /= norm;
          results[allInfos[i].originalIdx] = std::move(vec);
        }
        outSeqIdx++;
      }
    } else {
      qWarning() << "[BATCH_ENCODE] 失败 rc:" << rc << ", seqs:" << curBatch
                 << ", tokens:" << batch.n_tokens;
    }

    qDebug() << "[BATCH_ENCODE] seqs:" << curBatch
             << ", tokens:" << batch.n_tokens << ", 耗时:" << timer.elapsed()
             << "ms";

    llama_batch_free(batch);
    offset = batchEnd;
  }

  return results;
}