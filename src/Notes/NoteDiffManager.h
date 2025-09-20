#ifndef NOTEDIFFMANAGER_H
#define NOTEDIFFMANAGER_H

#include <QList>
#include <QString>
#include <QVector>

#include "lib/diff/diff_match_patch.h"

class NoteDiffManager {
 private:
  diff_match_patch dmp;  // 内部 diff-match-patch 实例

     QList<Diff> mergeAdjacentDiffs(const QList<Diff> &diffs);
  QList<Diff> splitDiffsByLines(const QList<Diff> &diffs);
  public:
  // 构造函数：初始化默认参数
  NoteDiffManager();

  // 计算两个文本的差异列表
  QList<Diff> computeDiffs(const QString &oldText, const QString &newText);

  // 根据差异列表生成补丁字符串（可存储）
  QString generatePatch(const QString &oldText, const QList<Diff> &diffs);

  // 从补丁字符串解析出补丁对象
  QList<Patch> parsePatch(const QString &patchStr);

  // 简化接口：直接从新旧文本生成补丁
  QString createPatchFromTexts(const QString &oldText, const QString &newText);

  // 将差异转换为 HTML（含样式，用于界面展示）
  QString diffsToHtml(const QList<Diff> &diffs);

  // 检查补丁是否能有效应用到旧文本
  bool isPatchValid(const QString &oldText, const QString &patchStr);

  // 参数配置接口
  void setDiffTimeout(float timeout);       // 设置差异计算超时时间（秒）
  void setMatchThreshold(float threshold);  // 设置匹配阈值（0.0-1.0）

  // 应用补丁到旧文本，返回新文本及每个补丁的应用结果
  QString applyPatch(const QString &oldText, QList<Patch> &patches,
                     QVector<bool> &success);
  QList<Diff> filterDiffsForDisplay(const QList<Diff> &originalDiffs, int contextLines);
  QString revertPatchToOldText(const QString &newText, const QString &patchStr, QVector<bool> &success);
};

#endif  // NOTEDIFFMANAGER_H
