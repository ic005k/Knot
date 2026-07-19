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
  QVector<QString> chunks;
  if (mdContent.isEmpty()) return chunks;

  // ⚠️ 注意：这里必须使用你配置中的 maxTokens 作为目标块大小
  // 如果你的 ChunkConfig 中有类似 targetTokens 或
  // maxTokens，请替换下面的硬编码值 通常 RAG 推荐的目标块大小为 256 ~ 512
  // tokens
  const int TARGET_CHUNK_TOKENS = 512;
  const int MIN_CHUNK_TOKENS = 50;  // 防止在极短内容上死循环

  QTextBoundaryFinder sentenceFinder(QTextBoundaryFinder::Sentence, mdContent);
  static const QRegularExpression headingRe("^#{1,6}\\s+");

  bool inCodeBlock = false;
  int chunkStartPos = 0;
  int lastSafeBoundary = 0;  // 记录上一个安全的句子/段落边界

  // 预计算所有安全边界的位置，避免在循环中反复调用 ICU
  QVector<int> safeBoundaries;
  safeBoundaries.append(0);
  while (sentenceFinder.toNextBoundary() != -1) {
    safeBoundaries.append(sentenceFinder.position());
  }
  safeBoundaries.append(mdContent.length());

  int currentBoundaryIdx = 0;

  while (chunkStartPos < mdContent.length()) {
    // 1. 估算当前累积文本的 token 数
    // 💡 优化建议：如果 m_engine.tokenizeText 较慢，可先用 (字符数 / 3)
    // 做粗略估算， 接近 TARGET_CHUNK_TOKENS 时再精确计算
    QString currentText =
        mdContent.mid(chunkStartPos, lastSafeBoundary - chunkStartPos);

    // 当累积文本达到目标 token 数，或者已经到达文档末尾时，尝试切块
    auto tokens = m_engine.tokenizeText(currentText);
    bool reachedEnd = (lastSafeBoundary >= mdContent.length());

    if (static_cast<int>(tokens.size()) >= TARGET_CHUNK_TOKENS || reachedEnd) {
      // 2. 检查代码块状态：如果当前切点落在代码块内，强制延伸到代码块结束
      QString segmentToCheck =
          mdContent.mid(chunkStartPos, lastSafeBoundary - chunkStartPos);
      int fenceCount =
          segmentToCheck.count("```") + segmentToCheck.count("~~~");
      if (fenceCount % 2 != 0) {
        // 处于未闭合的代码块中，继续向后寻找下一个安全边界
        if (!reachedEnd && currentBoundaryIdx < safeBoundaries.size() - 1) {
          currentBoundaryIdx++;
          lastSafeBoundary = safeBoundaries[currentBoundaryIdx];
          continue;
        }
      }

      // 3. 提交当前块
      QString finalChunk =
          mdContent.mid(chunkStartPos, lastSafeBoundary - chunkStartPos)
              .trimmed();
      if (!finalChunk.isEmpty()) {
        chunks.append(finalChunk);
      }

      // 4. 移动窗口起点
      chunkStartPos = lastSafeBoundary;

      // 防御性保护：如果切出的块太小且未到文末，说明遇到了超长单句，强制推进
      if (static_cast<int>(tokens.size()) < MIN_CHUNK_TOKENS && !reachedEnd) {
        currentBoundaryIdx++;
        if (currentBoundaryIdx < safeBoundaries.size()) {
          lastSafeBoundary = safeBoundaries[currentBoundaryIdx];
        }
      }
      continue;
    }

    // 5. 未达上限，继续向后扩展安全边界
    if (currentBoundaryIdx < safeBoundaries.size() - 1) {
      currentBoundaryIdx++;
      lastSafeBoundary = safeBoundaries[currentBoundaryIdx];
    } else {
      break;  // 已无更多边界
    }
  }

  return chunks;
}