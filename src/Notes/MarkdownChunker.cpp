#include "MarkdownChunker.h"

#include <QFile>
#include <QRegularExpression>
#include <algorithm>

MarkdownChunker::MarkdownChunker(EmbeddingEngine& engine,
                                 const ChunkConfig& config)
    : m_engine(engine), m_config(config) {}

QVector<NoteChunk> MarkdownChunker::processFile(
    const QString& mdFilePath) const {
  QFile file(mdFilePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return {};
  }
  QString content = QString::fromUtf8(file.readAll());
  file.close();
  return processText(content);
}

QVector<NoteChunk> MarkdownChunker::processText(
    const QString& mdContent) const {
  QVector<NoteChunk> results;
  auto segments = splitByStructure(mdContent);

  int chunkIdx = 0;
  for (const QString& seg : segments) {
    if (seg.trimmed().isEmpty()) continue;

    auto tokens = m_engine.tokenizeText(seg);

    // 短段落：直接编码，零额外开销
    if (static_cast<int>(tokens.size()) <= m_config.maxTokens) {
      NoteChunk chunk;
      chunk.chunkIndex = chunkIdx++;
      chunk.content = seg;
      chunk.vector = m_engine.encodeTokens(tokens);
      results.append(chunk);
      continue;
    }

    // 长段落：Token 级滑动窗口
    int stride = m_config.maxTokens - m_config.overlapTokens;
    if (stride <= 0) stride = 1;  // 防御性保护

    for (size_t start = 0; start < tokens.size(); start += stride) {
      size_t end = std::min(start + static_cast<size_t>(m_config.maxTokens),
                            tokens.size());

      std::vector<llama_token> subTokens(
          tokens.begin() + static_cast<long long>(start),
          tokens.begin() + static_cast<long long>(end));

      NoteChunk chunk;
      chunk.chunkIndex = chunkIdx++;
      chunk.content = m_engine.detokenize(subTokens);
      chunk.vector = m_engine.encodeTokens(subTokens);
      results.append(chunk);

      if (end >= tokens.size()) break;
    }
  }
  return results;
}

QVector<QString> MarkdownChunker::splitByStructure(
    const QString& mdContent) const {
  QVector<QString> segments;
  QStringList lines = mdContent.split('\n');

  QString currentSegment;
  bool inCodeBlock = false;
  static const QRegularExpression headingRe("^#{1,6}\\s+");

  for (const QString& line : lines) {
    QString trimmedLine = line.trimmed();

    // 代码块边界检测（支持 ```lang 和 ~~~ 两种语法）
    if (trimmedLine.startsWith("```") || trimmedLine.startsWith("~~~")) {
      inCodeBlock = !inCodeBlock;
    }

    // 仅在非代码块内，以标题行作为自然分割点
    bool isHeading = !inCodeBlock && headingRe.match(trimmedLine).hasMatch();

    if (isHeading && !currentSegment.trimmed().isEmpty()) {
      segments.append(currentSegment.trimmed());
      currentSegment.clear();
    }

    currentSegment += line + "\n";
  }

  if (!currentSegment.trimmed().isEmpty()) {
    segments.append(currentSegment.trimmed());
  }

  return segments;
}