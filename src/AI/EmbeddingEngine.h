#ifndef EMBEDDINGENGINE_H
#define EMBEDDINGENGINE_H

#include <QString>
#include <QVector>
#include <vector>

#include "BaseEmbeddingEngine.h"

// 前向声明llama底层类型，不用引入头文件到.h，隔离依赖
struct llama_model;
struct llama_context;

class EmbeddingEngine : public BaseEmbeddingEngine {
 public:
  explicit EmbeddingEngine(const QString& ggufPath);
  ~EmbeddingEngine() override;

  QVector<float> encode(const QString& text) override;
  bool isValid() const override;

 private:
  struct llama_model* m_model = nullptr;
  struct llama_context* m_ctx = nullptr;
};

#endif