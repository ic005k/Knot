#ifdef VECTOR_SEARCH
#ifndef VECTORDATABASE_H
#define VECTORDATABASE_H

#include <QByteArray>
#include <QList>
#include <QMutex>
#include <QPair>
#include <QString>
#include <QVector>

struct sqlite3;
struct sqlite3_stmt;

class EmbeddingEngine;

/// 向量搜索结果项 —— 纯定位数据结构，不承载展示逻辑
struct VectorHit {
  QString noteId;
  int chunkIndex = 0;
  float score = 0.0f;

  // 回源定位字段
  qint64 charStart = -1;
  qint64 charEnd = -1;
  QString sectionPath;
  QByteArray contentHash;  // 原始 SHA-256 (32 bytes)

  // Fallback：仅在回源失败时使用
  QString content;
};

class VectorDb {
 public:
  explicit VectorDb(int embeddingDim);
  ~VectorDb();

  bool open(const QString& dbPath);
  void close();
  bool isOpen() const;

  // ==================== 事务控制 ====================
  bool beginTransaction();
  bool commit();
  bool rollback();

  // ==================== 批量写入 ====================
  bool prepareInsertStmts();
  bool executeInsert(const QString& noteId, int chunkIndex, qint64 charStart,
                     qint64 charEnd, const QString& sectionPath,
                     const QString& content,         // 可选 fallback
                     const QByteArray& contentHash,  // 原始32字节SHA-256
                     const QString& noteHash, const QVector<float>& vec);
  void finalizeInsertStmts();

  /// 便捷单条写入（自动 prepare → execute → finalize）
  bool insertChunk(const QString& noteId, int chunkIndex, qint64 charStart,
                   qint64 charEnd, const QString& sectionPath,
                   const QString& content, const QByteArray& contentHash,
                   const QString& noteHash, const QVector<float>& vec);

  // ==================== 删除 ====================
  bool deleteChunksByNoteId(const QString& noteId);

  // ==================== 检索 ====================
  /// 语义搜索，返回带定位信息的命中结果
  QVector<VectorHit> searchWithContent(const QVector<float>& queryVec, int topN,
                                       float threshold = 0.3f);

  /// 笔记级去重排序，返回 (noteId, bestScore)
  QList<QPair<QString, float>> querySimilar(const QVector<float>& queryVec,
                                            int topN = 20);

  // ==================== 维护 ====================
  int countChunks() const;
  bool clearAll();
  bool hasNoteChunks(const QString& noteId) const;
  bool isNoteContentChanged(const QString& noteId,
                            const QString& newNoteHash) const;

  /// 占位：调用方适配分块器后实现
  void fillMissingVec(EmbeddingEngine* engine,
                      const QList<QString>& allNoteIdList);

 private:
  bool initTable();

  sqlite3* m_db = nullptr;
  mutable QMutex m_mutex;
  int m_embeddingDim;

  sqlite3_stmt* m_stmtMeta = nullptr;
  sqlite3_stmt* m_stmtVec = nullptr;
};

#endif  // VECTORDATABASE_H
#endif  // VECTOR_SEARCH