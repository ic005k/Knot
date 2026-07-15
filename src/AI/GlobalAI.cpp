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
  // 向量库与AI模型统一存放至 privateDir/model
  QString vecDir = QDir(privateDir).filePath("model");
  QDir dir;
  // 自动创建model目录，首次运行不存在也能生成
  dir.mkpath(vecDir);
  QString vecDbPath = QDir(vecDir).filePath("note_vector.sqlite");
  bool vecDbOk = g_vecDb.open(vecDbPath);
  if (!vecDbOk) {
    qCritical() << "向量数据库打开失败，路径：" << vecDbPath;
    return false;
  }
  qDebug() << "向量数据库初始化成功，路径：" << vecDbPath;
#endif

  return true;
}