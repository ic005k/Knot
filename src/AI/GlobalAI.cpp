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
  QString ggufPath = AiModelDeployer::getGgufModelPath();
  QFileInfo fiGguf(ggufPath);
  const qint64 MIN_GGUF_SIZE = 100LL * 1024 * 1024;

  // 第一层：文件物理存在+大小校验
  if (!fiGguf.exists()) {
    qWarning() << "GGUF模型文件不存在：" << ggufPath;
    return false;
  }
  if (fiGguf.size() <= MIN_GGUF_SIZE) {
    qWarning() << "GGUF模型文件不完整，大小过小：" << fiGguf.size() << " 路径："
               << ggufPath;
    return false;
  }

  // 第二层：尝试加载模型，校验文件二进制合法性
  auto tmpEngine = std::make_unique<EmbeddingEngine>(ggufPath);
  if (!tmpEngine->isValid()) {
    qCritical() << "GGUF文件格式损坏/bad magic，无法加载模型：" << ggufPath;
    return false;
  }

  // 加载成功再转移所有权
  BaseEmbeddingEngine* rawPtr =
      static_cast<BaseEmbeddingEngine*>(tmpEngine.release());
  g_embEngine.reset(rawPtr);
  qDebug() << "GGUF向量模型加载完成";

  // ========== 向量数据库逻辑不变 ==========
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