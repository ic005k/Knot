#ifndef AIMODELDEPLOYER_H
#define AIMODELDEPLOYER_H

#include <QString>

extern QString privateDir;

class AiModelDeployer {
 public:
  // AI根目录 privateDir + "model"
  static QString getModelRoot();
  // ONNX主模型完整路径
  static QString getOnnxModelPath();
  // 分词三个文件路径
  static QString getTokenizerJsonPath();
  static QString getTokenizerConfigPath();
  static QString getSentencePiecePath();
  // 校验整套AI文件是否齐全、完整可用
  static bool isAllModelReady();
};

#endif  // AIMODELDEPLOYER_H