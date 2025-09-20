#include "NoteDiffManager.h"

#include <QStringList>
#include <QVector>

// 构造函数：初始化 diff-match-patch 实例并配置默认参数
NoteDiffManager::NoteDiffManager() {
  // 配置适合文本编辑场景的参数
  dmp.Diff_Timeout = 1.0f;           // 差异计算超时时间（秒）
  dmp.Match_Threshold = 0.6f;        // 匹配阈值（0.6 平衡精度与容错）
  dmp.Patch_Margin = 4;              // 补丁上下文长度（增强容错性）
  dmp.Patch_DeleteThreshold = 0.5f;  // 删除操作的匹配宽容度
}

// 计算两个文本的差异列表
QList<Diff> NoteDiffManager::computeDiffs(const QString &oldText,
                                          const QString &newText) {
  if (oldText.isEmpty() && newText.isEmpty()) {
    return QList<Diff>();  // 空文本无差异
  }

  // 计算原始差异（启用行级预检查提升大文本性能）
  QList<Diff> diffs = dmp.diff_main(oldText, newText, true);
  // 语义化清理差异结果（合并相邻同类操作，优化可读性）
  dmp.diff_cleanupSemantic(diffs);
  return diffs;
}

// 根据差异列表生成可存储的补丁字符串
QString NoteDiffManager::generatePatch(const QString &oldText,
                                       const QList<Diff> &diffs) {
  if (diffs.isEmpty()) {
    return "";  // 无差异则返回空补丁
  }

  QList<Patch> patches = dmp.patch_make(oldText, diffs);
  return dmp.patch_toText(patches);
}

// 从补丁字符串解析出补丁对象列表
QList<Patch> NoteDiffManager::parsePatch(const QString &patchStr) {
  if (patchStr.isEmpty()) {
    return QList<Patch>();  // 空字符串返回空补丁列表
  }

  return dmp.patch_fromText(patchStr);
}

// 应用补丁到旧文本，返回新文本及应用结果
QString NoteDiffManager::applyPatch(const QString &oldText,
                                    QList<Patch> &patches,
                                    QVector<bool> &success) {
  if (patches.isEmpty()) {
    success.clear();
    return oldText;  // 无补丁则直接返回旧文本
  }

  auto result = dmp.patch_apply(patches, oldText);  // 现在参数类型匹配
  success = QVector<bool>::fromList(
      result.second);  // 注意：如果 result.second 是 QList<bool>，需要转换为
                       // QVector
  return result.first;
}

// 简化接口：直接从新旧文本生成补丁字符串
QString NoteDiffManager::createPatchFromTexts(const QString &oldText,
                                              const QString &newText) {
  QList<Diff> diffs = computeDiffs(oldText, newText);
  return generatePatch(oldText, diffs);
}

// 将差异列表转换为 HTML 格式（用于界面展示）
QString NoteDiffManager::diffsToHtml(const QList<Diff> &diffs) {
  if (diffs.isEmpty()) {
    return "<p>No differences.</p>";
  }

  return dmp.diff_prettyHtml(diffs);
}

// 检查补丁是否能成功应用到旧文本
bool NoteDiffManager::isPatchValid(const QString &oldText,
                                   const QString &patchStr) {
  QList<Patch> patches = parsePatch(patchStr);
  if (patches.isEmpty()) {
    return false;  // 空补丁视为无效
  }

  QVector<bool> success;
  applyPatch(oldText, patches, success);
  // 所有补丁片段都成功应用才算有效
  return !success.contains(false);
}

// 设置差异计算超时时间（秒）
void NoteDiffManager::setDiffTimeout(float timeout) {
  if (timeout >= 0) {
    dmp.Diff_Timeout = timeout;
  }
}

// 设置匹配阈值（0.0-1.0，值越高匹配越宽松）
void NoteDiffManager::setMatchThreshold(float threshold) {
  if (threshold >= 0.0f && threshold <= 1.0f) {
    dmp.Match_Threshold = threshold;
  }
}

QList<Diff> NoteDiffManager::filterDiffsForDisplay(
    const QList<Diff> &originalDiffs, int contextLines) {
  QList<Diff> filteredDiffs;
  if (originalDiffs.isEmpty()) {
    return filteredDiffs;
  }

  // 第一步：将差异按行拆分，便于按行处理上下文
  QList<Diff> lineDiffs = splitDiffsByLines(originalDiffs);

  // 第二步：标记所有包含变化的行位置
  QList<int> changeLineIndices;
  for (int i = 0; i < lineDiffs.size(); ++i) {
    const Diff &diff = lineDiffs[i];
    if (diff.operation != EQUAL_OP) {
      changeLineIndices.append(i);
    }
  }

  if (changeLineIndices.isEmpty()) {
    // 没有变化，返回空
    return filteredDiffs;
  }

  // 第三步：确定需要保留的行范围
  QSet<int> keepLines;
  foreach (int changeIndex, changeLineIndices) {
    // 保留变化行及其上下文
    for (int i = qMax(0, changeIndex - contextLines);
         i <= qMin(lineDiffs.size() - 1, changeIndex + contextLines); ++i) {
      keepLines.insert(i);
    }
  }

  // 第四步：重建差异列表，添加省略号分隔符
  bool inSkippedSection = false;
  for (int i = 0; i < lineDiffs.size(); ++i) {
    if (keepLines.contains(i)) {
      // 如果之前有跳过的部分，添加省略号
      if (inSkippedSection) {
        filteredDiffs.append(Diff(EQUAL_OP, "...\n"));
        inSkippedSection = false;
      }
      filteredDiffs.append(lineDiffs[i]);
    } else {
      inSkippedSection = true;
    }
  }

  // 第五步：合并相邻的相同类型差异（优化显示）
  return mergeAdjacentDiffs(filteredDiffs);
}

// 辅助函数：将差异按行拆分
QList<Diff> NoteDiffManager::splitDiffsByLines(const QList<Diff> &diffs) {
  QList<Diff> lineDiffs;
  foreach (const Diff &diff, diffs) {
    QString text = diff.text;
    int start = 0;
    int pos = text.indexOf('\n');

    while (pos != -1) {
      // 提取一行（包含换行符）
      QString line = text.mid(start, pos - start + 1);
      lineDiffs.append(Diff(diff.operation, line));

      start = pos + 1;
      pos = text.indexOf('\n', start);
    }

    // 处理最后一行（可能没有换行符）
    if (start < text.length()) {
      lineDiffs.append(Diff(diff.operation, text.mid(start)));
    }
  }
  return lineDiffs;
}

// 辅助函数：合并相邻的相同类型差异
QList<Diff> NoteDiffManager::mergeAdjacentDiffs(const QList<Diff> &diffs) {
  if (diffs.isEmpty()) {
    return diffs;
  }

  QList<Diff> merged;
  Diff current = diffs[0];

  for (int i = 1; i < diffs.size(); ++i) {
    const Diff &next = diffs[i];
    if (current.operation == next.operation) {
      // 合并相同类型的差异
      current.text += next.text;
    } else {
      merged.append(current);
      current = next;
    }
  }

  merged.append(current);
  return merged;
}

// 核心函数：通过“新文本 + 补丁”反推“旧文本”
// 参数：
//   newText: 目标新文本
//   patchStr: 从旧文本生成新文本的补丁字符串
//   success: 输出参数，记录每个补丁片段的应用成功状态（true=成功，false=失败）
// 返回值：反推得到的旧文本
QString NoteDiffManager::revertPatchToOldText(const QString &newText,
                                              const QString &patchStr,
                                              QVector<bool> &success) {
  // 1. 边界处理：空补丁→新旧文本相同，直接返回新文本
  if (patchStr.isEmpty()) {
    success.clear();
    return newText;
  }

  // 2. 解析原始补丁（旧→新的补丁）
  QList<Patch> originalPatches = parsePatch(patchStr);
  if (originalPatches.isEmpty()) {
    success.clear();
    return newText;
  }

  // 3. 生成“反向补丁”（新→旧的补丁）：反转每个补丁的操作和位置参数
  QList<Patch> reversedPatches;
  foreach (const Patch &origPatch, originalPatches) {
    Patch reversedPatch;

    // 3.1 反转补丁的位置/长度参数（新旧文本角色互换）
    // 原patch：start1=旧文本位置，start2=新文本位置；length1=旧文本影响长度，length2=新文本影响长度
    // 反向patch：start1=新文本位置（原start2），start2=旧文本位置（原start1）
    reversedPatch.start1 = origPatch.start2;
    reversedPatch.start2 = origPatch.start1;
    reversedPatch.length1 = origPatch.length2;
    reversedPatch.length2 = origPatch.length1;

    // 3.2 反转每个Diff的操作类型
    foreach (const Diff &origDiff, origPatch.diffs) {
      Diff reversedDiff;
      reversedDiff.text = origDiff.text;  // 文本内容不变，仅反转操作

      switch (origDiff.operation) {
        case INSERT_OP:
          // 原操作：旧文本→新文本 新增内容→反向需从新文本删除→DELETE_OP
          reversedDiff.operation = DELETE_OP;
          break;
        case DELETE_OP:
          // 原操作：旧文本→新文本 删除内容→反向需加回旧文本→INSERT_OP
          reversedDiff.operation = INSERT_OP;
          break;
        case EQUAL_OP:
          // 原操作：新旧文本相同→反向仍相同→保持EQUAL_OP
          reversedDiff.operation = EQUAL_OP;
          break;
      }

      reversedPatch.diffs.append(reversedDiff);
    }

    reversedPatches.append(reversedPatch);
  }

  // 4. 应用“反向补丁”到新文本→得到旧文本（复用现有applyPatch逻辑）
  // 此时逻辑：新文本 + 反向补丁 → 旧文本
  return applyPatch(newText, reversedPatches, success);
}
