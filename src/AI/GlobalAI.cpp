#include "GlobalAI.h"

#include <QDir>
#include <memory>

#include "AiModelDeployer.h"
#include "EmbeddingEngine.h"
#include "VectorDb.h"
#include "lib/llama.cpp/ggml/include/ggml-backend.h"
#include "lib/llama.cpp/include/llama.h"

// 全局向量引擎，统一基类无需改动
std::unique_ptr<BaseEmbeddingEngine> g_embEngine;

static bool g_llama_ggml_inited = false;

#ifdef VECTOR_SEARCH

bool initGlobalAiEngine() {
  // ========= 全局后端、llama一次性初始化 =========
  if (!g_llama_ggml_inited) {
    // 多线程上下文隔离兜底，任意平台子线程加载模型前刷新后端注册表
    // ggml_backend_load_all();

    qDebug() << "子线程执行ggml_backend_load_all() 后端扫描完成";

    g_llama_ggml_inited = true;
  }
  // =====================================================

  QString ggufPath = AiModelDeployer::getGgufModelPath();
  QFileInfo fiGguf(ggufPath);
  const qint64 MIN_GGUF_SIZE = 1LL * 1024 * 1024;

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

  // 第二层前置校验GGUF魔数，不交给llama.cpp
  QFile ggufFile(ggufPath);
  if (!ggufFile.open(QIODevice::ReadOnly)) {
    qWarning() << "无法打开GGUF文件：" << ggufPath;
    return false;
  }
  char magic[4];
  if (ggufFile.read(magic, 4) != 4 || memcmp(magic, "GGUF", 4) != 0) {
    qCritical() << "GGUF文件格式损坏/bad magic，无法加载模型：" << ggufPath;
    ggufFile.close();
    return false;
  }
  ggufFile.close();

  // 第三层：尝试加载模型，校验文件二进制合法性
  auto tmpEngine = std::make_unique<EmbeddingEngine>(ggufPath);
  if (!tmpEngine->isValid()) {
    qCritical() << "GGUF模型加载失败（不支持的架构/量化类型）：" << ggufPath;
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

// 新增全局释放函数，程序退出调用
void releaseGlobalAiEngine() {
  if (g_embEngine) {
    g_embEngine.reset();
  }
  if (g_llama_ggml_inited) {
    llama_backend_free();
    g_llama_ggml_inited = false;
    qDebug() << "llama/ggml 全局资源释放完成";
  }
}
#endif
