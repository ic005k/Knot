#ifdef VECTOR_SEARCH
#ifndef VECTORDATABASE_H
#define VECTORDATABASE_H

#include <QList>
#include <QPair>
#include <QString>
#include <QVector>

#include "BaseEmbeddingEngine.h"

struct sqlite3;

// 分块数据结构（与 MarkdownChunker 保持一致）
struct NoteChunkRecord {
  QString noteId;
  int chunkIndex;
  QString content;
  QVector<float> vector;
};

class VectorDb {
 public:
  VectorDb();
  ~VectorDb();

  bool open(const QString& dbPath);
  void close();
  bool isOpen() const;

  // ✅ 新增：事务控制（多chunk写入必须原子化）
  bool beginTransaction();
  bool commit();
  bool rollback();

  // ✅ 改造：批量写入替代单条 upsert
  bool insertChunk(const QString& noteId, int chunkIndex,
                   const QString& content, const QVector<float>& vec384);

  // ✅ 改造：按 noteId 删除所有 chunk
  bool deleteChunksByNoteId(const QString& noteId);

  // ✅ 改造：返回 (noteId, score)，内部已做 GROUP BY MAX
  QList<QPair<QString, float>> querySimilar(const QVector<float>& queryVec,
                                            int topN = 20);

  // 保留接口签名，内部逻辑需调用方适配分块
  void fillMissingVec(BaseEmbeddingEngine* engine,
                      const QList<QString>& allNoteIdList);

 private:
  bool initTable();
  sqlite3* m_db = nullptr;
};

#endif  // VECTORDATABASE_H
#endif  // VECTOR_SEARCH