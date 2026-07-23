#ifndef MARKDOWNCHUNKER_H
#define MARKDOWNCHUNKER_H

#include <QString>
#include <QVector>
#include <vector>

#include "llama.h"

class EmbeddingEngine;

// ============================================================
// 配置与数据结构
// ============================================================
struct ChunkConfig {
  int maxTokens = 512;
  int overlapTokens = 64;
  qint64 fastPathThresholdBytes = 128 * 1024;  // 128KB: 快速路径上限
  qint64 degradeThresholdBytes = 512 * 1024;   // 512KB: 降级为导航块
};

enum class ChunkStrategy {
  FullStructured,      // 标准 MD 结构分块
  ImplicitStructure,   // 纯文本隐式结构（民间分隔符）
  StatisticalFallback  // 统计学分块兜底（流水账/无结构）
};

struct StructureSignal {
  int mdHeadingCount = 0;
  int implicitBreakCount = 0;
  float avgParagraphLength = 0.f;
  ChunkStrategy strategy = ChunkStrategy::FullStructured;
};

struct BatchTextChunk {
  QString noteId;
  int chunkIndex = 0;
  QString content;
  std::vector<llama_token> tokens;
};

// ============================================================
// 核心分块器
// ============================================================
class MarkdownChunker {
 public:
  explicit MarkdownChunker(EmbeddingEngine& engine,
                           const ChunkConfig& config = {});

  /// 主入口：自适应批量分块（供 syncNoteVectorsBatchToDb 调用）
  QVector<BatchTextChunk> splitForBatch(const QString& noteId,
                                        const QString& content) const;

 private:
  // ✅ 信号融合评估：决定使用哪种分块策略
  StructureSignal analyzeStructure(const QString& content) const;

  // ✅ Token 级切分：全文仅 tokenize 一次，O(N) 复杂度
  QVector<QString> splitByTokenBoundary(
      const QString& content, const std::vector<llama_token>& allTokens,
      ChunkStrategy strategy) const;

  // ✅ 大文件降级：提取导航块（标题+首句摘要）
  QVector<BatchTextChunk> extractNavigationChunks(const QString& noteId,
                                                  const QString& content) const;

  // 归一化民间分隔符位置（仅用于分析，不修改原文）
  int countImplicitBreaks(const QString& content) const;

  EmbeddingEngine& m_engine;
  ChunkConfig m_config;
};

#endif  // MARKDOWNCHUNKER_H