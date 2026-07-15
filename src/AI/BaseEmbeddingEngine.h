#ifndef BASEEMBEDDINGENGINE_H
#define BASEEMBEDDINGENGINE_H

#include <QString>
#include <QVector>

class BaseEmbeddingEngine {
 public:
  virtual ~BaseEmbeddingEngine() = default;
  virtual QVector<float> encode(const QString& text) = 0;
  virtual bool isValid() const = 0;
};

#endif  // BASEEMBEDDINGENGINE_H