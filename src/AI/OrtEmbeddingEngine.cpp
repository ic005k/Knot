// OrtEmbeddingEngine.h
#ifdef USE_ONNX_RUNTIME
#include <QString>
#include <QVector>
#include <memory>

#include "BaseEmbeddingEngine.h"
namespace Ort {
class Session;
class Env;
}  // namespace Ort
namespace tokenizers {
class Tokenizer;
}
class OrtEmbeddingEngine : public BaseEmbeddingEngine {
 public:
  OrtEmbeddingEngine(const QString& tokJsonPath, const QString& spModelPath,
                     const QString& onnxPath);
  ~OrtEmbeddingEngine() override;
  QVector<float> encode(const QString& text) override;
  bool isValid() const override;

 private:
  QVector<QString> splitTextChunk(const QString& text, int maxToken = 512);
  QVector<float> averageNormalizeVec(const QVector<QVector<float>>& vecList);
  bool m_valid = false;
  std::unique_ptr<Ort::Env> m_ortEnv;
  std::unique_ptr<Ort::Session> m_session;
  std::unique_ptr<tokenizers::Tokenizer> m_tokenizer;
};
#endif