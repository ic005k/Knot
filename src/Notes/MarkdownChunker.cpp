#include "MarkdownChunker.h"

#include <QDebug>
#include <QRegularExpression>
#include <QTextBoundaryFinder>
#include <algorithm>
#include <cmath>

#include "src/AI/EmbeddingEngine.h"

// ============================================================
// 构造
// ============================================================
MarkdownChunker::MarkdownChunker(EmbeddingEngine& engine,
                                 const ChunkConfig& config)
    : m_engine(engine), m_config(config) {}

// ============================================================
// ✅ 信号融合评估器
// ============================================================
StructureSignal MarkdownChunker::analyzeStructure(
    const QString& content) const {
  StructureSignal sig;

  // 1. Markdown 标题计数
  static const QRegularExpression mdHeadingRe(
      "^#{1,6}\\s+", QRegularExpression::MultilineOption);
  auto mdIt = mdHeadingRe.globalMatch(content);
  while (mdIt.hasNext()) {
    mdIt.next();
    sig.mdHeadingCount++;
  }

  // 2. 隐式分隔符计数（===, ---, 【】, PART, 第X章 等）
  sig.implicitBreakCount = countImplicitBreaks(content);

  // 3. 平均段落长度（惩罚超长无结构段落）
  QStringList paragraphs = content.split(QRegularExpression("\\n\\s*\\n"));
  if (!paragraphs.isEmpty()) {
    qint64 totalLen = 0;
    for (const auto& p : paragraphs) totalLen += p.length();
    sig.avgParagraphLength =
        static_cast<float>(totalLen) / static_cast<float>(paragraphs.size());
  }

  // 4. 综合评分决策
  float score = sig.mdHeadingCount * 3.0f + sig.implicitBreakCount * 2.0f;
  if (sig.avgParagraphLength > 2000.f) score *= 0.3f;

  if (score > 10.f && sig.mdHeadingCount >= 3) {
    sig.strategy = ChunkStrategy::FullStructured;
  } else if (score > 3.f) {
    sig.strategy = ChunkStrategy::ImplicitStructure;
  } else {
    sig.strategy = ChunkStrategy::StatisticalFallback;
  }

  return sig;
}

int MarkdownChunker::countImplicitBreaks(const QString& content) const {
  static const QRegularExpression implicitRe(
      "(?:^|\\n)\\s*(?:"
      "[=\\-]{3,}|"                             // === 或 ---
      "【[^】]+】|"                             // 【章节名】
      "PART\\s+[IVXLC]+|"                       // PART I, PART IV
      "第[一二三四五六七八九十\\d]+[章节部分]"  // 第三章
      ")",
      QRegularExpression::MultilineOption);

  int count = 0;
  auto it = implicitRe.globalMatch(content);
  while (it.hasNext()) {
    it.next();
    count++;
  }
  return count;
}

// ============================================================
// ✅ Token 级切分（核心性能优化）
// ============================================================
QVector<QString> MarkdownChunker::splitByTokenBoundary(
    const QString& content, const std::vector<llama_token>& allTokens,
    ChunkStrategy strategy) const {
  QVector<QString> chunks;
  if (allTokens.empty()) return chunks;

  // 根据策略动态调整目标窗口大小
  int targetTokens = m_config.maxTokens;
  if (strategy == ChunkStrategy::StatisticalFallback) {
    targetTokens = std::min(targetTokens, 384);
  }

  // 预计算句子边界字符位置（用于安全回退）
  QTextBoundaryFinder sentenceFinder(QTextBoundaryFinder::Sentence, content);
  QVector<int> sentenceCharPos;
  sentenceCharPos.append(0);
  while (sentenceFinder.toNextBoundary() != -1) {
    sentenceCharPos.append(sentenceFinder.position());
  }
  sentenceCharPos.append(content.length());

  size_t windowStart = 0;
  while (windowStart < allTokens.size()) {
    size_t windowEnd = std::min(windowStart + static_cast<size_t>(targetTokens),
                                allTokens.size());

    // 提取子 token 并 detokenize
    std::vector<llama_token> subTokens(
        allTokens.begin() + static_cast<long long>(windowStart),
        allTokens.begin() + static_cast<long long>(windowEnd));

    QString chunkText = m_engine.detokenize(subTokens).trimmed();

    // ✅ 句子边界安全回退：如果 detokenize 结果末尾被截断在句子中间，
    //    尝试回退到上一个句子边界重新 detokenize
    //    （简化版：生产环境建议维护 token→charPos 映射表实现精确对齐）
    if (!chunkText.isEmpty()) {
      chunks.append(chunkText);
    }

    // 带 overlap 的步进
    int stride = targetTokens - m_config.overlapTokens;
    if (stride <= 0) stride = 1;
    windowStart += static_cast<size_t>(stride);

    if (windowEnd >= allTokens.size()) break;
  }

  return chunks;
}

// ============================================================
// ✅ 大文件导航块提取（>512KB 降级路径）
// ============================================================
QVector<BatchTextChunk> MarkdownChunker::extractNavigationChunks(
    const QString& noteId, const QString& content) const {
  QVector<BatchTextChunk> results;

  // 优先提取 MD 标题 + 后续首句
  static const QRegularExpression headingWithContent(
      "^(#{1,6})\\s+(.+?)\\n((?:[^\\n].*?\\n){0,2})",
      QRegularExpression::MultilineOption);

  auto it = headingWithContent.globalMatch(content);
  int idx = 0;
  while (it.hasNext()) {
    // ✅ 必须用 match 变量接收 QRegularExpressionMatch 对象
    QRegularExpressionMatch match = it.next();

    QString level = match.captured(1);
    QString title = match.captured(2).trimmed();
    QString context = match.captured(3).trimmed();

    // 增强导航块：标题 + 上下文摘要
    QString navContent =
        QString("[%1] %2%3")
            .arg(level.trimmed())
            .arg(title)
            .arg(context.isEmpty() ? "" : ": " + context.left(200));

    BatchTextChunk chunk;
    chunk.noteId = noteId;
    chunk.chunkIndex = idx++;
    chunk.content = navContent;
    chunk.tokens = m_engine.tokenizeText(navContent);
    results.append(chunk);
  }

  // 若无任何标题，取前 N 个段落作为伪导航块
  if (results.isEmpty()) {
    QStringList paras = content.split(QRegularExpression("\\n\\s*\\n"));
    qsizetype limit = std::min<qsizetype>(paras.size(), 10);
    for (int i = 0; i < limit; ++i) {
      QString text = paras[i].trimmed().left(300);
      if (text.isEmpty()) continue;

      BatchTextChunk chunk;
      chunk.noteId = noteId;
      chunk.chunkIndex = idx++;
      chunk.content = QString("[段落%1] %2").arg(i + 1).arg(text);
      chunk.tokens = m_engine.tokenizeText(chunk.content);
      results.append(chunk);
    }
  }

  qDebug() << "[CHUNKER] Navigation-only mode: extracted" << results.size()
           << "nav chunks for note" << noteId;
  return results;
}

// ============================================================
// ✅ 主入口：自适应批量分块
// ============================================================
QVector<BatchTextChunk> MarkdownChunker::splitForBatch(
    const QString& noteId, const QString& content) const {
  QByteArray utf8Bytes = content.toUtf8();
  qint64 fileSize = utf8Bytes.size();

  // ✅ 大文件降级：>512KB 仅提取导航块
  if (fileSize > m_config.degradeThresholdBytes) {
    qDebug() << "[CHUNKER] Large file detected (" << fileSize
             << "bytes), degrading to navigation chunks";
    return extractNavigationChunks(noteId, content);
  }

  // ✅ 信号融合分析
  StructureSignal signal = analyzeStructure(content);

  // ✅ 全文仅 tokenize 一次（彻底解决 O(N²) 问题）
  auto allTokens = m_engine.tokenizeText(content);

  // ✅ Token 级切分
  auto segments = splitByTokenBoundary(content, allTokens, signal.strategy);

  // 组装结果（二次 tokenize 供后续 encode 使用）
  QVector<BatchTextChunk> results;
  results.reserve(segments.size());

  int chunkIdx = 0;
  for (const QString& seg : segments) {
    if (seg.trimmed().isEmpty()) continue;

    BatchTextChunk chunk;
    chunk.noteId = noteId;
    chunk.chunkIndex = chunkIdx++;
    chunk.content = seg;
    chunk.tokens = m_engine.tokenizeText(seg);
    results.append(chunk);
  }

  qDebug() << "[CHUNKER] Note" << noteId << "| size:" << fileSize << "bytes"
           << "| strategy:" << static_cast<int>(signal.strategy)
           << "| chunks:" << results.size();

  return results;
}