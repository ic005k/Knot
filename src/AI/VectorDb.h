#ifdef VECTOR_SEARCH
#ifndef VECTORDATABASE_H
#define VECTORDATABASE_H

#include <QList>
#include <QMutex>
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

// 向量搜索结果项（与 VectorSearchService 解耦的纯数据结构）
struct VectorHit {
  QString noteId;
  int chunkIndex;
  float score;
  QString content;  // ✅ 直接从DB取出原文，避免回查
};

class VectorDb {
 public:
  VectorDb();
  ~VectorDb();

  bool open(const QString& dbPath);
  void close();
  bool isOpen() const;
  QVector<VectorHit> searchWithContent(const QVector<float>& queryVec, int topN,
                                       float threshold = 0.3f);

  // ✅ 事务控制（多chunk写入必须原子化）
  bool beginTransaction();
  bool commit();
  bool rollback();

  // ✅ 批量写入替代单条 upsert
  bool insertChunk(const QString& noteId, int chunkIndex,
                   const QString& content, const QVector<float>& vec384);

  // ✅ 按 noteId 删除所有 chunk
  bool deleteChunksByNoteId(const QString& noteId);

  // ✅ 返回 (noteId, score)，内部已做 GROUP BY MAX
  QList<QPair<QString, float>> querySimilar(const QVector<float>& queryVec,
                                            int topN = 20);

  // 保留接口签名，内部逻辑需调用方适配分块
  void fillMissingVec(BaseEmbeddingEngine* engine,
                      const QList<QString>& allNoteIdList);

  int countChunks() const;

  /// 清空所有向量与元数据（保留表结构），用于全量重建索引
  bool clearAll();

 private:
  bool initTable();
  sqlite3* m_db = nullptr;
  mutable QMutex m_mutex;
};

#endif  // VECTORDATABASE_H
#endif  // VECTOR_SEARCH