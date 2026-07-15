#include "GlobalAI.h"

#include <QDir>
#include <memory>

#include "AiModelDeployer.h"
#include "EmbeddingEngine.h"
#include "VectorDb.h"

// 全局向量引擎，统一基类无需改动
std::unique_ptr<BaseEmbeddingEngine> g_embEngine;

#ifdef VECTOR_SEARCH
bool initGlobalAiEngine() {
  // 校验GGUF单模型文件
  if (!AiModelDeployer::isAllModelReady()) return false;

  // 仅需GGUF文件路径，不再需要分词json/sp/onnx
  QString ggufPath = AiModelDeployer::getGgufModelPath();

  auto tmpEngine = std::make_unique<EmbeddingEngine>(ggufPath);
  // 强制向上转型：EmbeddingEngine* → BaseEmbeddingEngine*
  BaseEmbeddingEngine* rawPtr =
      static_cast<BaseEmbeddingEngine*>(tmpEngine.release());
  g_embEngine.reset(rawPtr);

  // 校验推理引擎是否加载成功
  if (!g_embEngine->isValid()) {
    g_embEngine.reset();
    return false;
  }

  // ========== 向量数据库逻辑完全保留无需修改 ==========
  QString vecDir = QDir(privateDir).filePath("model");
  QDir dir;
  dir.mkpath(vecDir);
  QString vecDbPath = QDir(vecDir).filePath("note_vector.sqlite");
  VectorDb tempInitDb;
  bool vecDbOk = tempInitDb.open(vecDbPath);
  if (!vecDbOk) {
    qCritical() << "向量数据库打开失败，路径：" << vecDbPath;
    return false;
  }
  qDebug() << "向量数据库初始化成功，路径：" << vecDbPath;

  return true;
}
#endif