#include "AiModelDeployer.h"

#include <QDir>
#include <QFileInfo>

QString AiModelDeployer::getModelRoot() {
  QDir dir(privateDir + "model");
  dir.mkpath(".");
  return dir.absolutePath();
}

QString AiModelDeployer::getOnnxModelPath() {
  return QDir(getModelRoot()).filePath("model_quantized.onnx");
}

QString AiModelDeployer::getTokenizerJsonPath() {
  return QDir(getModelRoot()).filePath("tokenizer.json");
}

QString AiModelDeployer::getTokenizerConfigPath() {
  return QDir(getModelRoot()).filePath("tokenizer_config.json");
}

QString AiModelDeployer::getSentencePiecePath() {
  return QDir(getModelRoot()).filePath("sentencepiece.bpe.model");
}

bool AiModelDeployer::isAllModelReady() {
  // 逐个校验全部4个文件存在且体积达标
  QFileInfo fiOnnx(getOnnxModelPath());
  QFileInfo fiTokJson(getTokenizerJsonPath());
  QFileInfo fiTokCfg(getTokenizerConfigPath());
  QFileInfo fiSp(getSentencePiecePath());

  // ONNX必须大于500MB，分词文件大于1KB
  bool okOnnx = fiOnnx.exists() && fiOnnx.size() > 500LL * 1024 * 1024;
  bool okTokJson = fiTokJson.exists() && fiTokJson.size() > 1024;
  bool okTokCfg = fiTokCfg.exists() && fiTokCfg.size() > 100;
  bool okSp = fiSp.exists() && fiSp.size() > 1024;

  return okOnnx && okTokJson && okTokCfg && okSp;
}