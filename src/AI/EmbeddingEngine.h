#ifndef EMBEDDINGENGINE_H
#define EMBEDDINGENGINE_H

#include <QMutex>
#include <QString>
#include <QVector>
#include <vector>

#include "BaseEmbeddingEngine.h"
#include "llama.h"

// 前向声明llama底层类型，不用引入头文件到.h，隔离依赖
struct llama_model;
struct llama_context;

class EmbeddingEngine : public BaseEmbeddingEngine {
 public:
  explicit EmbeddingEngine(const QString& ggufPath);
  ~EmbeddingEngine() override;

  QVector<float> encode(const QString& text) override;
  bool isValid() const override;

  /// 仅分词，不推理。返回 token 序列供外部做滑动窗口
  std::vector<llama_token> tokenizeText(const QString& text) const;

  /// 对已有 token 序列做推理+归一化，跳过 tokenize 步骤
  QVector<float> encodeTokens(const std::vector<llama_token>& tokens);

  /// 将 token 序列还原为文本（用于存储 chunk 原文）
  QString detokenize(const std::vector<llama_token>& tokens) const;

 private:
  struct llama_model* m_model = nullptr;
  struct llama_context* m_ctx = nullptr;

  QMutex m_mutex;
};

#endif