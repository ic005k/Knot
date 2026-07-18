#include "EmbeddingEngine.h"

#include <QByteArray>
#include <QDebug>
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
  QVector<float> result;
  if (!isValid()) return result;
  std::string input = text.toUtf8().toStdString();
  const char* str = input.c_str();
  int str_len = static_cast<int>(input.size());

  const llama_vocab* vocab = llama_model_get_vocab(m_model);

  int token_count = llama_tokenize(vocab, str, str_len, nullptr, 0, true, true);
  std::vector<llama_token> tokens(token_count);
  llama_tokenize(vocab, str, str_len, tokens.data(), token_count, true, true);
  int n_tokens = tokens.size();

  llama_batch batch = llama_batch_init(n_tokens, 0, 1);
  for (int i = 0; i < n_tokens; i++) {
    batch.token[i] = tokens[i];
    batch.pos[i] = i;

    batch.n_seq_id[i] = 1;
    batch.seq_id[i][0] = 0;
    batch.logits[i] = false;
  }
  batch.n_tokens = n_tokens;

  llama_decode(m_ctx, batch);
  int dim = llama_n_embd(m_model);  // e5-small = 384
  const float* emb = llama_get_embeddings_seq(m_ctx, 0);
  result.resize(dim);
  std::memcpy(result.data(), emb, dim * sizeof(float));

  // ✅✅✅ 核心：E5 必须 L2 归一化 ✅✅✅
  float norm = 0.0f;
  for (int i = 0; i < dim; ++i) {
    norm += result[i] * result[i];
  }
  norm = std::sqrt(norm);
  if (norm > 1e-6f) {
    for (int i = 0; i < dim; ++i) {
      result[i] /= norm;
    }
  }

  llama_batch_free(batch);
  return result;
}