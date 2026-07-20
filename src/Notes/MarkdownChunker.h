#ifndef MARKDOWN_CHUNKER_H
#define MARKDOWN_CHUNKER_H

#include <QRegularExpression>
#include <QString>
#include <QTextBoundaryFinder>
#include <QVector>

#include "src/AI/EmbeddingEngine.h"

// ✅ 专用于批处理的中间结构
struct BatchTextChunk {
  QString noteId;
  int chunkIndex;
  QString content;
  std::vector<llama_token>
      tokens;  // 保留tokens，避免batch encode时重复tokenize
};

struct NoteChunk {
  int chunkIndex;
  QString content;
  QVector<float> vector;
};

struct ChunkConfig {
  int maxTokens = 450;
  int overlapTokens = 80;
};

class MarkdownChunker {
 public:
  explicit MarkdownChunker(EmbeddingEngine& engine,
                           const ChunkConfig& config = ChunkConfig());

  /**
   * @brief 读取 MD 文件并返回带向量的分块结果
   * @param mdFilePath Markdown 文件路径
   * @return 分块结果，失败返回空向量
   */
  QVector<NoteChunk> processFile(const QString& mdFilePath) const;

  /**
   * @brief 直接对内存中的 MD 文本进行分块（不读文件）
   */
  QVector<NoteChunk> processText(const QString& mdContent) const;

  // ✅ 专为批处理设计的纯分块接口（不做任何推理）
  QVector<BatchTextChunk> splitForBatch(const QString& noteId,
                                        const QString& mdContent) const;

 private:
  EmbeddingEngine& m_engine;
  ChunkConfig m_config;

  QVector<QString> splitByStructure(const QString& mdContent) const;
};

#endif  // MARKDOWN_CHUNKER_H