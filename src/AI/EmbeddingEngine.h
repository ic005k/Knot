#ifndef EMBEDDINGENGINE_H
#define EMBEDDINGENGINE_H

#include <QString>
#include <QVector>
#include <vector>
// 关键：引入基类头文件，让编译器识别继承关系
#include "BaseEmbeddingEngine.h"

extern "C" {
#include "bert.h"
}

// 必须 public 公有继承
class EmbeddingEngine : public BaseEmbeddingEngine {
 public:
  explicit EmbeddingEngine(const QString& ggufPath);
  ~EmbeddingEngine() override;

  // 重写基类纯虚函数，加override校验
  QVector<float> encode(const QString& text) override;
  bool isValid() const override;

 private:
  struct bert_ctx* m_ctx = nullptr;
};

#endif  // EMBEDDINGENGINE_H