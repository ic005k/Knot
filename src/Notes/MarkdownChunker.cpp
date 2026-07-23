#include "MarkdownChunker.h"

#include <QCryptographicHash>  // ✅ 修复缺失的头文件
#include <QDebug>
#include <QRegularExpression>
#include <QTextBoundaryFinder>
#include <algorithm>

#include "src/AI/EmbeddingEngine.h"

// ============================================================
// 构造
// ============================================================
MarkdownChunker::MarkdownChunker(EmbeddingEngine& engine,
                                 const ChunkConfig& config)
    : m_engine(engine), m_config(config) {}

// ============================================================
// Token → Char 映射构建 (全程 qsizetype)
// ============================================================
QVector<TokenCharSpan> MarkdownChunker::buildTokenCharMap(
    const QString& content, const std::vector<llama_token>& allTokens) const {
  QVector<TokenCharSpan> map;
  map.reserve(static_cast<int>(allTokens.size()));

  qsizetype searchPos = 0;  // ✅ 修复窄化转换

  for (size_t i = 0; i < allTokens.size(); ++i) {
    std::vector<llama_token> single = {allTokens[i]};
    QString piece = m_engine.detokenize(single);

    qsizetype foundPos = content.indexOf(piece, searchPos);  // ✅ 修复窄化转换

    TokenCharSpan span;
    if (foundPos >= 0) {
      span.charStart = foundPos;
      span.charEnd = foundPos + piece.length();
      searchPos = span.charEnd;
    } else {
      span.charStart = searchPos;
      span.charEnd = searchPos;
    }
    map.append(span);
  }
  return map;
}

// ============================================================
// 标题预计算 & 章节路径构建
// ============================================================
QVector<MarkdownChunker::HeadingInfo> MarkdownChunker::extractAllHeadings(
    const QString& content) const {
  static const QRegularExpression headingRe(
      "^(#{1,6})\\s+(.+?)$", QRegularExpression::MultilineOption);

  QVector<HeadingInfo> result;
  auto it = headingRe.globalMatch(content);
  while (it.hasNext()) {
    auto match = it.next();
    HeadingInfo info;
    info.charPos = match.capturedStart();  // ✅ qsizetype
    info.level =
        static_cast<int>(match.captured(1).length());  // ✅ 显式转换防警告
    info.title = match.captured(2).trimmed();
    result.append(info);
  }
  return result;
}

QString MarkdownChunker::buildSectionPath(const QVector<HeadingInfo>& headings,
                                          qsizetype charPos) const {
  struct Entry {
    int level;
    QString title;
  };
  QVector<Entry> stack;

  for (const auto& h : headings) {
    if (h.charPos >= charPos) break;
    while (!stack.isEmpty() && stack.last().level >= h.level) {
      stack.pop_back();
    }
    stack.append({h.level, h.title});
  }

  QStringList path;
  for (const auto& e : stack) path.append(e.title);
  return path.join(" > ");
}

// ============================================================
// 信号融合评估器
// ============================================================
StructureSignal MarkdownChunker::analyzeStructure(
    const QString& content) const {
  StructureSignal sig;

  static const QRegularExpression mdHeadingRe(
      "^#{1,6}\\s+", QRegularExpression::MultilineOption);
  auto mdIt = mdHeadingRe.globalMatch(content);
  while (mdIt.hasNext()) {
    mdIt.next();
    sig.mdHeadingCount++;
  }

  sig.implicitBreakCount = countImplicitBreaks(content);

  QStringList paragraphs = content.split(QRegularExpression("\\n\\s*\\n"));
  if (!paragraphs.isEmpty()) {
    qint64 totalLen = 0;
    for (const auto& p : paragraphs) totalLen += p.length();
    sig.avgParagraphLength =
        static_cast<float>(totalLen) / static_cast<float>(paragraphs.size());
  }

  float score = sig.mdHeadingCount * 3.0f + sig.implicitBreakCount * 2.0f;
  if (sig.avgParagraphLength > 2000.f) score *= 0.3f;

  if (score > 10.f && sig.mdHeadingCount >= 3)
    sig.strategy = ChunkStrategy::FullStructured;
  else if (score > 3.f)
    sig.strategy = ChunkStrategy::ImplicitStructure;
  else
    sig.strategy = ChunkStrategy::StatisticalFallback;

  return sig;
}

int MarkdownChunker::countImplicitBreaks(const QString& content) const {
  static const QRegularExpression implicitRe(
      "(?:^|\\n)\\s*(?:"
      "[=\\-]{3,}|"
      "【[^】]+】|"
      "PART\\s+[IVXLC]+|"
      "第[一二三四五六七八九十\\d]+[章节部分]"
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
// Token 级切分（带坐标追踪 + 句子边界安全回退）
// ============================================================
QVector<BatchTextChunk> MarkdownChunker::splitByTokenBoundary(
    const QString& noteId, const QString& content,
    const std::vector<llama_token>& allTokens,
    const QVector<TokenCharSpan>& charMap,
    const QVector<qsizetype>& sentenceBounds, ChunkStrategy strategy) const {
  QVector<BatchTextChunk> chunks;
  if (allTokens.empty()) return chunks;

  auto headings = extractAllHeadings(content);
  int targetTokens = m_config.maxTokens;
  if (strategy == ChunkStrategy::StatisticalFallback)
    targetTokens = std::min(targetTokens, 384);

  size_t windowStart = 0;
  int chunkIdx = 0;

  while (windowStart < allTokens.size()) {
    size_t windowEnd = std::min(windowStart + static_cast<size_t>(targetTokens),
                                allTokens.size());

    qsizetype cStart = charMap[static_cast<int>(windowStart)].charStart;
    qsizetype cEnd = charMap[static_cast<int>(windowEnd - 1)].charEnd;

    // 句子边界安全回退
    auto lb =
        std::lower_bound(sentenceBounds.begin(), sentenceBounds.end(), cEnd);
    if (lb != sentenceBounds.end() && *lb > cEnd &&
        lb != sentenceBounds.begin()) {
      --lb;
      if ((*lb - cStart) >= (cEnd - cStart) / 2) cEnd = *lb;
    }

    QString chunkText = content.mid(cStart, cEnd - cStart).trimmed();
    if (chunkText.isEmpty()) {
      windowStart += static_cast<size_t>(targetTokens);
      continue;
    }

    BatchTextChunk chunk;
    chunk.noteId = noteId;
    chunk.chunkIndex = chunkIdx++;
    chunk.content = chunkText;
    chunk.charStart = cStart;
    chunk.charEnd = cEnd;
    chunk.sectionPath = buildSectionPath(headings, cStart);
    chunk.contentHash = QCryptographicHash::hash(  // ✅ 已包含头文件
        chunkText.toUtf8(), QCryptographicHash::Sha256);
    chunk.tokens = m_engine.tokenizeText(chunkText);
    chunks.append(chunk);

    int stride = targetTokens - m_config.overlapTokens;
    if (stride <= 0) stride = 1;
    windowStart += static_cast<size_t>(stride);
    if (windowEnd >= allTokens.size()) break;
  }
  return chunks;
}

// ============================================================
// 大文件导航块提取（>512KB 降级路径）
// ============================================================
QVector<BatchTextChunk> MarkdownChunker::extractNavigationChunks(
    const QString& noteId, const QString& content) const {
  QVector<BatchTextChunk> results;
  auto headings = extractAllHeadings(content);

  static const QRegularExpression headingWithContent(
      "^(#{1,6})\\s+(.+?)\\n((?:[^\\n].*?\\n){0,2})",
      QRegularExpression::MultilineOption);

  auto it = headingWithContent.globalMatch(content);
  int idx = 0;
  while (it.hasNext()) {
    auto match = it.next();
    QString navContent =
        QString("[%1] %2%3")
            .arg(match.captured(1).trimmed())
            .arg(match.captured(2).trimmed())
            .arg(match.captured(3).trimmed().isEmpty()
                     ? ""
                     : ": " + match.captured(3).trimmed().left(200));

    BatchTextChunk chunk;
    chunk.noteId = noteId;
    chunk.chunkIndex = idx++;
    chunk.content = navContent;
    chunk.charStart = match.capturedStart();
    chunk.charEnd = match.capturedEnd();
    chunk.sectionPath = buildSectionPath(headings, match.capturedStart());
    chunk.contentHash = QCryptographicHash::hash(navContent.toUtf8(),
                                                 QCryptographicHash::Sha256);
    chunk.tokens = m_engine.tokenizeText(navContent);
    results.append(chunk);
  }

  if (results.isEmpty()) {
    QStringList paras = content.split(QRegularExpression("\\n\\s*\\n"));
    qsizetype limit = std::min<qsizetype>(paras.size(), 10);
    qsizetype posAccum = 0;
    for (qsizetype i = 0; i < limit; ++i) {
      QString text = paras[i].trimmed().left(300);
      if (text.isEmpty()) continue;

      qsizetype paraStart = content.indexOf(paras[i], posAccum);
      if (paraStart < 0) paraStart = posAccum;

      BatchTextChunk chunk;
      chunk.noteId = noteId;
      chunk.chunkIndex = idx++;
      chunk.content = QString("[段落%1] %2").arg(i + 1).arg(text);
      chunk.charStart = paraStart;
      chunk.charEnd = paraStart + paras[i].length();
      chunk.sectionPath = buildSectionPath(headings, paraStart);
      chunk.contentHash = QCryptographicHash::hash(chunk.content.toUtf8(),
                                                   QCryptographicHash::Sha256);
      chunk.tokens = m_engine.tokenizeText(chunk.content);
      results.append(chunk);
      posAccum = paraStart + paras[i].length();
    }
  }

  qDebug() << "[CHUNKER] Navigation-only mode: extracted" << results.size()
           << "nav chunks for note" << noteId;
  return results;
}

// ============================================================
// 主入口：自适应批量分块
// ============================================================
QVector<BatchTextChunk> MarkdownChunker::splitForBatch(
    const QString& noteId, const QString& content) const {
  QByteArray utf8Bytes = content.toUtf8();
  qint64 fileSize = utf8Bytes.size();

  // ✅ 大文件降级策略完整保留
  if (fileSize > m_config.degradeThresholdBytes) {
    qDebug() << "[CHUNKER] Large file detected (" << fileSize
             << "bytes), degrading to navigation chunks";
    return extractNavigationChunks(noteId, content);
  }

  StructureSignal signal = analyzeStructure(content);
  auto allTokens = m_engine.tokenizeText(content);
  auto charMap = buildTokenCharMap(content, allTokens);

  QTextBoundaryFinder sentenceFinder(QTextBoundaryFinder::Sentence, content);
  QVector<qsizetype> sentenceBounds;  // ✅ 统一使用 qsizetype
  sentenceBounds.append(0);
  while (sentenceFinder.toNextBoundary() != -1)
    sentenceBounds.append(sentenceFinder.position());
  sentenceBounds.append(content.length());

  auto results = splitByTokenBoundary(noteId, content, allTokens, charMap,
                                      sentenceBounds, signal.strategy);

  qDebug() << "[CHUNKER] Note" << noteId << "| size:" << fileSize << "bytes"
           << "| strategy:" << static_cast<int>(signal.strategy)
           << "| chunks:" << results.size();
  return results;
}