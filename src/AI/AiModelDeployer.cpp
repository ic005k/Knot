#include "AiModelDeployer.h"

#include <QDir>
#include <QFileInfo>

QString AiModelDeployer::getModelRoot() {
  QDir dir(privateDir + "model");
  dir.mkpath(".");
  return dir.absolutePath();
}

// 新增：GGUF单模型文件路径
QString AiModelDeployer::getGgufModelPath() {
  return QDir(getModelRoot()).filePath("multilingual-e5-small-q4_k.gguf");
}

bool AiModelDeployer::isAllModelReady() {
  // 仅校验单个GGUF文件
  QFileInfo fiGguf(getGgufModelPath());

  // 文件存在 + 体积大于100MB（当前文件118MB，阈值留余量）
  const qint64 MIN_GGUF_SIZE = 100LL * 1024 * 1024;
  bool okGguf = fiGguf.exists() && fiGguf.size() > MIN_GGUF_SIZE;

  return okGguf;
}