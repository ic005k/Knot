#ifndef EMBEDDINGENGINE_H
#define EMBEDDINGENGINE_H

#include <QMutex>
#include <QString>
#include <QVector>
#include <vector>

#include "llama.h"

struct llama_model;
struct llama_context;

class EmbeddingEngine {
 public:
  explicit EmbeddingEngine(const QString& ggufPath);
  ~EmbeddingEngine();

  QVector<float> encode(const QString& text);
  bool isValid() const;

  std::vector<llama_token> tokenizeText(const QString& text) const;
  QVector<float> encodeTokens(const std::vector<llama_token>& tokens);
  QString detokenize(const std::vector<llama_token>& tokens) const;

  int embeddingDimension() const;

  std::vector<QVector<float>> encodeBatch(const QStringList& texts,
                                          int maxBatchSize = 32);

  // ✅ 获取模型安全的最大用户文本 token 数（已扣除特殊 token）
  int maxTokens() const;

 private:
  struct llama_model* m_model = nullptr;
  struct llama_context* m_ctx = nullptr;
  int m_maxTokens = 510;
  QMutex m_mutex;
};

#endif