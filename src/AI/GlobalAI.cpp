#include "GlobalAI.h"

#include <QDir>

#include "AiModelDeployer.h"
#ifdef USE_ONNX_RUNTIME
#include "OrtEmbeddingEngine.h"
#endif

std::unique_ptr<BaseEmbeddingEngine> g_embEngine;
#ifdef VECTOR_SEARCH
VectorDb g_vecDb;
#endif

bool initGlobalAiEngine() {
  // 校验onnx分词模型文件
  if (!AiModelDeployer::isAllModelReady()) return false;

#ifdef USE_ONNX_RUNTIME
  QString tokJson = AiModelDeployer::getTokenizerJsonPath();
  QString spModel = AiModelDeployer::getSentencePiecePath();
  QString onnxFile = AiModelDeployer::getOnnxModelPath();
  g_embEngine =
      std::make_unique<OrtEmbeddingEngine>(tokJson, spModel, onnxFile);
  if (!g_embEngine->isValid()) {
    g_embEngine.reset();
    return false;
  }
#endif

#ifdef VECTOR_SEARCH
  // 仅开启向量检索时才打开向量库
  QString vecDbPath = QDir(privateDir).filePath("note_vector.sqlite");
  g_vecDb.open(vecDbPath);
#endif
  return true;
}