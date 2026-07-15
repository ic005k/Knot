#ifndef AIMODELDEPLOYER_H
#define AIMODELDEPLOYER_H

#include <QString>

extern QString privateDir;

class AiModelDeployer {
 public:
  // AI根目录 privateDir + "model"
  static QString getModelRoot();
  // 新增GGUF接口
  static QString getGgufModelPath();

  // 校验整套AI文件是否齐全、完整可用
  static bool isAllModelReady();
};

#endif  // AIMODELDEPLOYER_H