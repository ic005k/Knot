#ifndef MARKDOWNCHUNKER_H
#define MARKDOWNCHUNKER_H

#include <QByteArray>
#include <QString>
#include <QVector>
#include <vector>

#include "llama.h"

class EmbeddingEngine;

// ============================================================
// 配置与数据结构
// ============================================================

/// Token 到原文 UTF-16 字符区间的映射 (Qt6 安全类型)
struct TokenCharSpan {
  qsizetype charStart;
  qsizetype charEnd;
};

struct ChunkConfig {
  int maxTokens = 512;
  int overlapTokens = 64;
  qint64 fastPathThresholdBytes = 128 * 1024;  // 128KB
  qint64 degradeThresholdBytes = 512 * 1024;   // 512KB: 降级为导航块
};

enum class ChunkStrategy {
  FullStructured,      // 标准 MD 结构分块
  ImplicitStructure,   // 纯文本隐式结构
  StatisticalFallback  // 统计学分块兜底
};

struct StructureSignal {
  int mdHeadingCount = 0;
  int implicitBreakCount = 0;
  float avgParagraphLength = 0.f;
  ChunkStrategy strategy = ChunkStrategy::FullStructured;
};

/// 批量分块结果，携带完整的回源定位元数据
struct BatchTextChunk {
  QString noteId;
  int chunkIndex = 0;
  QString content;
  QVector<float> vector;
  std::vector<llama_token> tokens;

  // 回源定位字段
  qsizetype charStart = -1;
  qsizetype charEnd = -1;
  QString sectionPath;
  QByteArray contentHash;
};

// ============================================================
// 核心分块器
// ============================================================
class MarkdownChunker {
 public:
  explicit MarkdownChunker(EmbeddingEngine& engine,
                           const ChunkConfig& config = {});

  /// 主入口：自适应批量分块
  QVector<BatchTextChunk> splitForBatch(const QString& noteId,
                                        const QString& content) const;

 private:
  StructureSignal analyzeStructure(const QString& content) const;

  QVector<TokenCharSpan> buildTokenCharMap(
      const QString& content, const std::vector<llama_token>& allTokens) const;

  QVector<BatchTextChunk> splitByTokenBoundary(
      const QString& noteId, const QString& content,
      const std::vector<llama_token>& allTokens,
      const QVector<TokenCharSpan>& charMap,
      const QVector<qsizetype>& sentenceBounds, ChunkStrategy strategy) const;

  QVector<BatchTextChunk> extractNavigationChunks(const QString& noteId,
                                                  const QString& content) const;

  struct HeadingInfo {
    qsizetype charPos;  // ✅ Qt6 安全类型
    int level;
    QString title;
  };

  QVector<HeadingInfo> extractAllHeadings(const QString& content) const;
  QString buildSectionPath(const QVector<HeadingInfo>& headings,
                           qsizetype charPos) const;
  int countImplicitBreaks(const QString& content) const;

  EmbeddingEngine& m_engine;
  ChunkConfig m_config;
};

#endif  // MARKDOWNCHUNKER_H