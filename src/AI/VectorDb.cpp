#ifdef VECTOR_SEARCH

#define SQLITE_CORE
#include "VectorDb.h"

#include <sqlite3.h>

#include <QByteArray>
#include <QSet>
#include <QVariant>
#include <cmath>

#include "sqlite-vec.h"
#include "sqlite3ext.h"

bool VectorDb::isOpen() const { return m_db != nullptr; }

VectorDb::VectorDb() {
  sqlite3_auto_extension((void (*)(void))sqlite3_vec_init);
  m_db = nullptr;
}

VectorDb::~VectorDb() { close(); }

bool VectorDb::open(const QString& dbPath) {
  int rc = sqlite3_open_v2(dbPath.toUtf8().data(), &m_db,
                           SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
  if (rc != SQLITE_OK) {
    qCritical() << "sqlite文件打开失败:" << sqlite3_errmsg(m_db);
    return false;
  }
  sqlite3_enable_load_extension(m_db, 1);

  // 性能优化：WAL模式 + 同步降级（本地笔记场景安全且快数倍）
  sqlite3_exec(m_db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
  sqlite3_exec(m_db, "PRAGMA synchronous=NORMAL;", nullptr, nullptr, nullptr);

  if (!initTable()) {
    qCritical() << "建表失败";
    return false;
  }
  return true;
}

void VectorDb::close() {
  if (m_db) sqlite3_close(m_db);
  m_db = nullptr;
}

// ✅ 核心改造：双表设计
// metadata 存原文+索引，vec_index 纯向量检索
bool VectorDb::initTable() {
  char* err = nullptr;

  // 元数据表：(note_id, chunk_index) 联合主键
  const char* sqlMeta = R"(
        CREATE TABLE IF NOT EXISTS note_chunks (
            note_id     TEXT NOT NULL,
            chunk_index INT NOT NULL,
            content     TEXT,
            PRIMARY KEY (note_id, chunk_index)
        );
    )";
  int rc = sqlite3_exec(m_db, sqlMeta, nullptr, nullptr, &err);
  if (rc != SQLITE_OK) {
    qCritical() << "建note_chunks失败:" << (err ? err : "无错误信息");
    if (err) sqlite3_free(err);
    return false;
  }

  // 向量虚拟表：rowid 隐式对应 metadata 的 rowid
  // ⚠️ sqlite-vec 的 vec0 不支持 TEXT PRIMARY KEY，
  //    改用默认整数 rowid，通过 JOIN 关联元数据
  const char* sqlVec = R"(
        CREATE VIRTUAL TABLE IF NOT EXISTS vec_index USING vec0(
            vec FLOAT[384] distance_metric=cosine
        );
    )";
  rc = sqlite3_exec(m_db, sqlVec, nullptr, nullptr, &err);
  if (rc != SQLITE_OK) {
    qCritical() << "创建vec0虚拟表失败:" << (err ? err : "无错误信息");
    if (err) sqlite3_free(err);
    return false;
  }

  // 加速按 note_id 删除
  const char* sqlIdx =
      "CREATE INDEX IF NOT EXISTS idx_chunks_note ON note_chunks(note_id);";
  rc = sqlite3_exec(m_db, sqlIdx, nullptr, nullptr, &err);
  if (rc != SQLITE_OK) {
    qCritical() << "创建索引失败:" << (err ? err : "无错误信息");
    if (err) sqlite3_free(err);
    return false;
  }

  if (err) sqlite3_free(err);
  return true;
}

// ==================== 事务控制 ====================

bool VectorDb::beginTransaction() {
  if (!m_db) return false;
  return sqlite3_exec(m_db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr) ==
         SQLITE_OK;
}

bool VectorDb::commit() {
  if (!m_db) return false;
  return sqlite3_exec(m_db, "COMMIT;", nullptr, nullptr, nullptr) == SQLITE_OK;
}

bool VectorDb::rollback() {
  if (!m_db) return false;
  return sqlite3_exec(m_db, "ROLLBACK;", nullptr, nullptr, nullptr) ==
         SQLITE_OK;
}

// ==================== 写入操作 ====================

bool VectorDb::insertChunk(const QString& noteId, int chunkIndex,
                           const QString& content,
                           const QVector<float>& vec384) {
  if (!m_db) {
    qWarning() << "[VectorDb] insertChunk失败: 数据库未打开";
    return false;
  }
  if (vec384.size() != 384) {
    qWarning() << "[VectorDb] insertChunk失败: 向量维度不是384, 实际:"
               << vec384.size();
    return false;
  }

  // Step 1: 插入元数据并获取 rowid
  sqlite3_stmt* stmtMeta = nullptr;
  const char* sqlMeta =
      "INSERT OR REPLACE INTO note_chunks(note_id, chunk_index, content) "
      "VALUES(?, ?, ?);";
  int rc = sqlite3_prepare_v2(m_db, sqlMeta, -1, &stmtMeta, nullptr);
  if (rc != SQLITE_OK) {
    qWarning() << "[VectorDb] prepare meta失败:" << sqlite3_errmsg(m_db);
    return false;
  }
  sqlite3_bind_text(stmtMeta, 1, noteId.toUtf8().constData(), -1,
                    SQLITE_TRANSIENT);
  sqlite3_bind_int(stmtMeta, 2, chunkIndex);
  sqlite3_bind_text(stmtMeta, 3, content.toUtf8().constData(), -1,
                    SQLITE_TRANSIENT);

  rc = sqlite3_step(stmtMeta);
  sqlite_int64 rowid = sqlite3_last_insert_rowid(m_db);
  sqlite3_finalize(stmtMeta);

  if (rc != SQLITE_DONE) {
    qWarning() << "[VectorDb] insert meta失败:" << sqlite3_errmsg(m_db);
    return false;
  }

  // Step 2: 用相同 rowid 插入向量
  QByteArray vecBin((const char*)vec384.constData(),
                    vec384.size() * sizeof(float));
  sqlite3_stmt* stmtVec = nullptr;
  const char* sqlVec =
      "INSERT OR REPLACE INTO vec_index(rowid, vec) VALUES(?, ?);";
  rc = sqlite3_prepare_v2(m_db, sqlVec, -1, &stmtVec, nullptr);
  if (rc != SQLITE_OK) {
    qWarning() << "[VectorDb] prepare vec失败:" << sqlite3_errmsg(m_db);
    return false;
  }
  sqlite3_bind_int64(stmtVec, 1, rowid);
  sqlite3_bind_blob(stmtVec, 2, vecBin.constData(), vecBin.size(),
                    SQLITE_TRANSIENT);

  rc = sqlite3_step(stmtVec);
  sqlite3_finalize(stmtVec);

  if (rc != SQLITE_DONE) {
    qWarning() << "[VectorDb] insert vec失败:" << sqlite3_errmsg(m_db);
    return false;
  }
  return true;
}

bool VectorDb::deleteChunksByNoteId(const QString& noteId) {
  if (!m_db) return false;

  // ⚠️ 必须先删 vec_index 再删 metadata
  // 因为 vec_index 依赖 metadata 的 rowid
  // 使用子查询确保一致性
  const char* sqlDelVec =
      "DELETE FROM vec_index WHERE rowid IN "
      "(SELECT rowid FROM note_chunks WHERE note_id = ?);";
  sqlite3_stmt* stmt = nullptr;
  sqlite3_prepare_v2(m_db, sqlDelVec, -1, &stmt, nullptr);
  sqlite3_bind_text(stmt, 1, noteId.toUtf8().constData(), -1, SQLITE_TRANSIENT);
  int rc = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  if (rc != SQLITE_DONE) {
    qWarning() << "[VectorDb] 删除vec失败:" << sqlite3_errmsg(m_db);
    return false;
  }

  const char* sqlDelMeta = "DELETE FROM note_chunks WHERE note_id = ?;";
  sqlite3_prepare_v2(m_db, sqlDelMeta, -1, &stmt, nullptr);
  sqlite3_bind_text(stmt, 1, noteId.toUtf8().constData(), -1, SQLITE_TRANSIENT);
  rc = sqlite3_step(stmt);
  sqlite3_finalize(stmt);

  return rc == SQLITE_DONE;
}

// ==================== 检索操作 ====================

// ✅ 核心改造：JOIN + GROUP BY MAX 实现笔记级去重排序
QList<QPair<QString, float>> VectorDb::querySimilar(
    const QVector<float>& queryVec, int topN) {
  QList<QPair<QString, float>> out;
  if (!m_db) return out;

  QByteArray vecBin((const char*)queryVec.constData(),
                    queryVec.size() * sizeof(float));
  sqlite3_stmt* stmt = nullptr;

  // distance → score = 1 - distance (cosine)
  // GROUP BY note_id 取每个笔记的最佳匹配块
  const char* sql = R"(
        SELECT c.note_id, MIN(v.distance) as min_dist
        FROM vec_index v
        JOIN note_chunks c ON c.rowid = v.rowid
        WHERE v.vec MATCH ?
        ORDER BY min_dist ASC
        LIMIT ?;
    )";

  int rc = sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    qWarning() << "[VectorDb] query prepare失败:" << sqlite3_errmsg(m_db);
    return out;
  }

  sqlite3_bind_blob(stmt, 1, vecBin.constData(), vecBin.size(),
                    SQLITE_TRANSIENT);
  sqlite3_bind_int(stmt, 2, topN);

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    QString id = (const char*)sqlite3_column_text(stmt, 0);
    float dist = (float)sqlite3_column_double(stmt, 1);
    float score = 1.0f - dist;
    out.append({id, score});
  }
  sqlite3_finalize(stmt);
  return out;
}

// 占位：调用方需改为传入 MarkdownChunker 处理后的结果
void VectorDb::fillMissingVec(BaseEmbeddingEngine* engine,
                              const QList<QString>& allNoteIdList) {
  Q_UNUSED(engine);
  Q_UNUSED(allNoteIdList);
  // TODO: 遍历 allNoteIdList，对缺失的 noteId 调用
  //       MarkdownChunker::processText + beginTransaction/insertChunk/commit
}

QVector<VectorHit> VectorDb::searchWithContent(const QVector<float>& queryVec,
                                               int topN, float threshold) {
  QVector<VectorHit> results;
  if (!m_db || queryVec.size() != 384) return results;

  QByteArray vecBin((const char*)queryVec.constData(),
                    queryVec.size() * sizeof(float));
  sqlite3_stmt* stmt = nullptr;

  // ✅ 关键改进：不再 GROUP BY note_id，直接返回 Top-K 个最佳 chunk
  // 笔记级去重交给 VectorSearchService 在内存中处理（更灵活）
  const char* sql = R"(
        SELECT c.note_id, c.chunk_index, v.distance, c.content
        FROM vec_index v
        JOIN note_chunks c ON c.rowid = v.rowid
        WHERE v.vec MATCH ?
          AND v.distance <= ?
        ORDER BY v.distance ASC
        LIMIT ?;
    )";

  int rc = sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    qWarning() << "[VectorDb] searchWithContent prepare失败:"
               << sqlite3_errmsg(m_db);
    return results;
  }

  // cosine distance 阈值 = 1 - similarity threshold
  float distThreshold = 1.0f - threshold;
  sqlite3_bind_blob(stmt, 1, vecBin.constData(), vecBin.size(),
                    SQLITE_TRANSIENT);
  sqlite3_bind_double(stmt, 2, distThreshold);
  sqlite3_bind_int(stmt, 3, topN);

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    VectorHit hit;
    hit.noteId = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 0));
    hit.chunkIndex = sqlite3_column_int(stmt, 1);
    float dist = (float)sqlite3_column_double(stmt, 2);
    hit.score = 1.0f - dist;

    const char* text = (const char*)sqlite3_column_text(stmt, 3);
    hit.content = text ? QString::fromUtf8(text) : QString();

    results.append(hit);
  }
  sqlite3_finalize(stmt);
  return results;
}

#endif  // VECTOR_SEARCH