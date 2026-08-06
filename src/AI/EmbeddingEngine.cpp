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
  // ⭐ b10041: 在任何 llama API 调用前禁用 fused ops（静态初始化更安全）
  static const bool s_envInit = []() {
    qputenv("LLAMA_NO_FUSED_OPS", "1");
    return true;
  }();
  Q_UNUSED(s_envInit);

  std::string path = ggufPath.toUtf8().toStdString();

  // ========== 模型参数 ==========
  llama_model_params model_params = llama_model_default_params();
  model_params.n_gpu_layers = 99;
  // ⭐ 新增：新版默认启用mmap，降低 RSS峰值
  // model_params.use_mmap = true;
  model_params.check_tensors = true;  // ⭐ 新增：防止损坏模型导致后续崩溃

  m_model = llama_load_model_from_file(path.c_str(), model_params);
  if (!m_model) {
    qCritical() << "[EmbeddingEngine] Failed to load model:" << ggufPath;
    return;
  }

  // ========== 上下文参数 ==========
  llama_context_params ctx_params = llama_context_default_params();
  ctx_params.embeddings = true;
  ctx_params.n_threads = 4;

  // ⭐ 核心：限制最大上下文，而非盲目使用训练长度
  // Embedding 推理不需要完整训练上下文，8192 已覆盖绝大多数场景
  const uint32_t trainCtx = llama_n_ctx_train(m_model);
  const uint32_t maxReasonableCtx = 8192;
  ctx_params.n_ctx = std::min(trainCtx, maxReasonableCtx);

  // ⭐ 核心：ubatch 绑定模型训练上下文长度
  // BERT/Sentence-Transformer 的位置编码是固定长度的，
  // ubatch 超过 trainCtx 会导致 GGML_ASSERT 越界崩溃。
  // 对于 LLM-based embedding 模型（如 bge-m3），trainCtx 本身较大，同样适用。
  ctx_params.n_ubatch =
      trainCtx;  // std::min(ctx_params.n_ctx, (uint32_t)2048);

  // ⭐ 核心：seq_max 按需设置，64 太大
  // encodeBatch 的 maxBatchSize 通常 <= 16，这里留余量即可
  ctx_params.n_seq_max = 16;

  m_ctx = llama_new_context_with_model(m_model, ctx_params);
  if (!m_ctx) {
    qCritical() << "[EmbeddingEngine] Failed to create context";
    llama_free_model(m_model);
    m_model = nullptr;
    return;
  }

  m_maxTokens = static_cast<int>(ctx_params.n_ctx) - 2;

  qDebug() << "[EmbeddingEngine] ✅ Initialized"
           << "| train_ctx:" << trainCtx << "| actual_ctx:" << ctx_params.n_ctx
           << "| n_ubatch:" << ctx_params.n_ubatch
           << "| n_seq_max:" << ctx_params.n_seq_max
           << "| maxTokens:" << m_maxTokens;
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
                              false, true);
  if (needed == 0) {
    qWarning() << "[ENCODE] tokenize 结果为空";
    return result;
  }
  // 负数 = 需要的空间大小；正数 = 已填入的数量（仅当 buffer 足够时）
  int total_tokens = (needed < 0) ? -needed : needed;

  // 2. 分配完整缓冲区，一次性完成 tokenize
  std::vector<llama_token> all_tokens(total_tokens);
  int actual = llama_tokenize(vocab, input.c_str(), input.size(),
                              all_tokens.data(), total_tokens, false, true);
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
    batch.logits[i] = true;
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
                              false, true);
  if (needed == 0) return result;

  int total_tokens = (needed < 0) ? -needed : needed;
  result.resize(total_tokens);

  int actual = llama_tokenize(vocab, input.c_str(), input.size(), result.data(),
                              total_tokens, false, true);
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
    batch.logits[i] = true;
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

int EmbeddingEngine::maxTokens() const { return m_maxTokens; }

std::vector<QVector<float>> EmbeddingEngine::encodeBatch(
    const QStringList& texts, int maxBatchSize) {
  QMutexLocker locker(&m_mutex);
  std::vector<QVector<float>> results(texts.size());
  if (!isValid() || texts.isEmpty()) return results;

  const int dim = llama_n_embd(m_model);
  const llama_vocab* vocab = llama_model_get_vocab(m_model);
  const uint32_t n_ubatch = llama_n_ubatch(m_ctx);

  // ========================================================================
  // 1. 预 tokenize 所有文本，获取精确 token 数并截断到安全范围
  // ========================================================================
  struct TextInfo {
    int originalIdx;  // 映射回原始输入索引
    std::vector<llama_token> tokens;
  };

  std::vector<TextInfo> allInfos;
  allInfos.reserve(texts.size());

  for (int i = 0; i < texts.size(); ++i) {
    std::string input = texts[i].toUtf8().toStdString();

    TextInfo info{i, {}};
    int needed = llama_tokenize(vocab, input.c_str(), input.size(), nullptr, 0,
                                false, true);
    int tokCount = (needed < 0) ? -needed : needed;

    if (tokCount > 0) {
      info.tokens.resize(tokCount);
      int actual = llama_tokenize(vocab, input.c_str(), input.size(),
                                  info.tokens.data(), tokCount, false, true);
      if (actual > 0) {
        info.tokens.resize(actual);
        // ✅ 单条超过 n_ubatch 时截断，防止 GGML_ASSERT 崩溃
        if (static_cast<uint32_t>(info.tokens.size()) > n_ubatch) {
          qDebug() << "[BATCH_ENCODE] ⚠ 文本" << i << "tokens("
                   << info.tokens.size() << ") > n_ubatch(" << n_ubatch
                   << "), 截断";
          info.tokens.resize(n_ubatch);
        }
      }
    }
    allInfos.push_back(std::move(info));
  }

  // ========================================================================
  // 2. 按 token 数降序排序（双指针装箱前提）
  // ========================================================================
  std::sort(allInfos.begin(), allInfos.end(),
            [](const TextInfo& a, const TextInfo& b) {
              return a.tokens.size() > b.tokens.size();
            });

  // ========================================================================
  // 3. 双指针装箱 + Encode 循环
  // ========================================================================
  // 用 visited 标记已处理的元素，避免 erase 带来的 O(n) 开销
  std::vector<bool> visited(allInfos.size(), false);
  int processed = 0;
  const int total = static_cast<int>(allInfos.size());

  while (processed < total) {
    uint32_t batchTokens = 0;
    std::vector<int> batchIndices;  // 本批次选中的 allInfos 下标
    batchIndices.reserve(maxBatchSize);

    int left = -1;
    int right = total - 1;

    // 找到第一个未访问的元素作为左指针起点
    for (int k = 0; k < total; ++k) {
      if (!visited[k]) {
        left = k;
        break;
      }
    }
    if (left < 0) break;

    // 右指针从末尾向前找第一个未访问的元素
    while (right >= left && visited[right]) --right;

    // ✅ 双指针贪心装箱：长文本优先，短文本填充剩余空间
    while (left <= right &&
           static_cast<int>(batchIndices.size()) < maxBatchSize) {
      uint32_t longTokens = static_cast<uint32_t>(allInfos[left].tokens.size());

      if (batchTokens + longTokens <= n_ubatch) {
        batchTokens += longTokens;
        batchIndices.push_back(left);
        visited[left] = true;
        ++processed;
        ++left;

        // 跳过已访问的左指针
        while (left <= right && visited[left]) ++left;

        // ✅ 尝试用最短的未访问文本填充剩余空间
        while (right >= left &&
               static_cast<int>(batchIndices.size()) < maxBatchSize &&
               !visited[right]) {
          uint32_t shortTokens =
              static_cast<uint32_t>(allInfos[right].tokens.size());
          if (batchTokens + shortTokens <= n_ubatch) {
            batchTokens += shortTokens;
            batchIndices.push_back(right);
            visited[right] = true;
            ++processed;
            --right;
            // 跳过已访问的右指针
            while (right >= left && visited[right]) --right;
          } else {
            break;  // 最短的都放不下，停止填充
          }
        }
      } else {
        break;  // 当前最长的都放不下，结束本批
      }
    }

    // 防御：如果一轮什么都没选中（不应发生），强制取一条
    if (batchIndices.empty()) {
      for (int k = 0; k < total; ++k) {
        if (!visited[k]) {
          batchIndices.push_back(k);
          visited[k] = true;
          ++processed;
          batchTokens = static_cast<uint32_t>(allInfos[k].tokens.size());
          break;
        }
      }
      if (batchIndices.empty()) break;
    }

    int curBatch = static_cast<int>(batchIndices.size());

    // ================================================================
    // 构建 Batch
    // ================================================================
    llama_batch batch = llama_batch_init(batchTokens, 0, curBatch);

    int seqIdx = 0;
    for (int infoIdx : batchIndices) {
      const auto& toks = allInfos[infoIdx].tokens;
      for (int t = 0; t < static_cast<int>(toks.size()); ++t) {
        int idx = batch.n_tokens;
        batch.token[idx] = toks[t];
        batch.pos[idx] = t;
        batch.n_seq_id[idx] = 1;
        batch.seq_id[idx][0] = seqIdx;
        // ✅ 【关键修复】Encoder 模式下所有 token 标记为需要输出
        // 消除 "embeddings required but some input tokens were not
        // marked as outputs -> overriding" 警告
        batch.logits[idx] = true;
        batch.n_tokens++;
      }
      seqIdx++;
    }

    // ================================================================
    // Encode
    // ================================================================
    // ✅ 每次 encode 前清理 memory，防止跨批次污染
    auto* mem = llama_get_memory(m_ctx);
    if (mem) llama_memory_clear(mem, true);

    QElapsedTimer timer;
    timer.start();
    int rc = llama_encode(m_ctx, batch);
    qint64 elapsed = timer.elapsed();

    if (rc == 0) {
      // ============================================================
      // 提取结果并映射回原始索引
      // ============================================================
      int outSeqIdx = 0;
      for (int infoIdx : batchIndices) {
        const float* emb = llama_get_embeddings_seq(m_ctx, outSeqIdx);
        if (emb) {
          QVector<float> vec(dim);
          std::memcpy(vec.data(), emb, dim * sizeof(float));

          // L2 归一化
          float norm = 0.0f;
          for (int d = 0; d < dim; ++d) norm += vec[d] * vec[d];
          norm = std::sqrt(norm);
          if (norm > 1e-6f)
            for (int d = 0; d < dim; ++d) vec[d] /= norm;

          results[allInfos[infoIdx].originalIdx] = std::move(vec);
        }
        outSeqIdx++;
      }
    } else {
      qWarning() << "[BATCH_ENCODE] ❌ llama_encode 失败 rc:" << rc
                 << "| seqs:" << curBatch << "| tokens:" << batch.n_tokens;
    }

    qDebug() << "[BATCH_ENCODE] seqs:" << curBatch
             << "| tokens:" << batch.n_tokens << "| 耗时:" << elapsed << "ms";

    llama_batch_free(batch);
  }

  return results;
}