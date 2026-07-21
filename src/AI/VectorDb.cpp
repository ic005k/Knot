#ifdef VECTOR_SEARCH

#define SQLITE_CORE

#include "VectorDb.h"

#include <sqlite3.h>

#include <QByteArray>
#include <QCryptographicHash>
#include <QSet>
#include <QString>
#include <QVariant>
#include <cmath>

#include "sqlite-vec.h"
#include "sqlite3ext.h"

bool VectorDb::isOpen() const { return m_db != nullptr; }

VectorDb::VectorDb(int embeddingDim) : m_embeddingDim(embeddingDim) {
  Q_ASSERT_X(embeddingDim > 0, "VectorDb",
             "embeddingDim must be > 0, check engine initialization order");

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

  // 元数据表：(note_id, chunk_index) 联合主键（保持不变）
  const char* sqlMeta = R"(
        CREATE TABLE IF NOT EXISTS note_chunks (
            note_id     TEXT NOT NULL,
            chunk_index INT NOT NULL,
            content     TEXT,
            content_hash TEXT NOT NULL DEFAULT '',
            note_hash    TEXT NOT NULL DEFAULT '',
            PRIMARY KEY (note_id, chunk_index)
        );
    )";
  int rc = sqlite3_exec(m_db, sqlMeta, nullptr, nullptr, &err);
  if (rc != SQLITE_OK) {
    qCritical() << "建note_chunks失败:" << (err ? err : "无错误信息");
    if (err) sqlite3_free(err);
    return false;
  }

  // ✅ 核心改造：动态维度替代硬编码 384
  // ⚠️ 注意：vec0 虚拟表不支持 ALTER，若旧库已存在 FLOAT[384] 的 vec_index，
  //    CREATE IF NOT EXISTS 会静默跳过，必须先删除旧库或手动 DROP TABLE
  QString sqlVecStr = QString(R"(
        CREATE VIRTUAL TABLE IF NOT EXISTS vec_index USING vec0(
            vec FLOAT[%1] distance_metric=cosine
        );
    )")
                          .arg(m_embeddingDim);

  QByteArray sqlVecUtf8 = sqlVecStr.toUtf8();
  rc = sqlite3_exec(m_db, sqlVecUtf8.constData(), nullptr, nullptr, &err);
  if (rc != SQLITE_OK) {
    qCritical() << "创建vec0虚拟表失败:" << (err ? err : "无错误信息")
                << ", 目标维度:" << m_embeddingDim;
    if (err) sqlite3_free(err);
    return false;
  }

  // 加速按 note_id 删除（保持不变）
  const char* sqlIdx =
      "CREATE INDEX IF NOT EXISTS idx_chunks_note ON note_chunks(note_id);";
  rc = sqlite3_exec(m_db, sqlIdx, nullptr, nullptr, &err);
  if (rc != SQLITE_OK) {
    qCritical() << "创建索引失败:" << (err ? err : "无错误信息");
    if (err) sqlite3_free(err);
    return false;
  }

  if (err) sqlite3_free(err);

  qDebug() << "[VectorDb] ✅ 初始化成功, 向量维度:" << m_embeddingDim;
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
bool VectorDb::prepareInsertStmts() {
  QMutexLocker locker(&m_mutex);
  if (!m_db) return false;

  // 如果已经准备过，直接返回成功（支持重入）
  if (m_stmtMeta && m_stmtVec) return true;

  const char* sqlMeta =
      "INSERT OR REPLACE INTO note_chunks(note_id, chunk_index, content, "
      "content_hash, note_hash) "
      "VALUES(?, ?, ?, ?, ?);";
  const char* sqlVec =
      "INSERT OR REPLACE INTO vec_index(rowid, vec) VALUES(?, ?);";

  int rc1 = sqlite3_prepare_v2(m_db, sqlMeta, -1, &m_stmtMeta, nullptr);
  int rc2 = sqlite3_prepare_v2(m_db, sqlVec, -1, &m_stmtVec, nullptr);

  if (rc1 != SQLITE_OK || rc2 != SQLITE_OK) {
    qWarning() << "[VectorDb] prepareInsertStmts失败:" << sqlite3_errmsg(m_db);
    if (m_stmtMeta) {
      sqlite3_finalize(m_stmtMeta);
      m_stmtMeta = nullptr;
    }
    if (m_stmtVec) {
      sqlite3_finalize(m_stmtVec);
      m_stmtVec = nullptr;
    }
    return false;
  }
  return true;
}

bool VectorDb::executeInsert(const QString& noteId, int chunkIndex,
                             const QString& content, const QString& noteHash,
                             const QVector<float>& vec) {
  QMutexLocker locker(&m_mutex);
  if (!m_stmtMeta || !m_stmtVec) {
    qWarning() << "[VectorDb] executeInsert失败: stmt未准备";
    return false;
  }
  if (vec.size() != m_embeddingDim) {
    qWarning() << "[VectorDb] executeInsert失败: 维度不匹配";
    return false;
  }

  QByteArray contentHash =
      QCryptographicHash::hash(content.toUtf8(), QCryptographicHash::Sha256)
          .toHex();

  // Step 1: Meta
  sqlite3_reset(m_stmtMeta);
  sqlite3_clear_bindings(m_stmtMeta);
  sqlite3_bind_text(m_stmtMeta, 1, noteId.toUtf8().constData(), -1,
                    SQLITE_TRANSIENT);
  sqlite3_bind_int(m_stmtMeta, 2, chunkIndex);
  sqlite3_bind_text(m_stmtMeta, 3, content.toUtf8().constData(), -1,
                    SQLITE_TRANSIENT);
  sqlite3_bind_text(m_stmtMeta, 4, contentHash.constData(), -1,
                    SQLITE_TRANSIENT);
  sqlite3_bind_text(m_stmtMeta, 5, noteHash.toUtf8().constData(), -1,
                    SQLITE_TRANSIENT);

  if (sqlite3_step(m_stmtMeta) != SQLITE_DONE) {
    qWarning() << "[VectorDb] insert meta失败:" << sqlite3_errmsg(m_db);
    return false;
  }
  sqlite_int64 rowid = sqlite3_last_insert_rowid(m_db);

  // Step 2: Vec
  QByteArray vecBin((const char*)vec.constData(), vec.size() * sizeof(float));
  sqlite3_reset(m_stmtVec);
  sqlite3_clear_bindings(m_stmtVec);
  sqlite3_bind_int64(m_stmtVec, 1, rowid);
  sqlite3_bind_blob(m_stmtVec, 2, vecBin.constData(), vecBin.size(),
                    SQLITE_TRANSIENT);

  if (sqlite3_step(m_stmtVec) != SQLITE_DONE) {
    qWarning() << "[VectorDb] insert vec失败:" << sqlite3_errmsg(m_db);
    return false;
  }
  return true;
}

void VectorDb::finalizeInsertStmts() {
  QMutexLocker locker(&m_mutex);
  if (m_stmtMeta) {
    sqlite3_finalize(m_stmtMeta);
    m_stmtMeta = nullptr;
  }
  if (m_stmtVec) {
    sqlite3_finalize(m_stmtVec);
    m_stmtVec = nullptr;
  }
}

// 兼容单条写入：自动 prepare → execute → finalize
bool VectorDb::insertChunk(const QString& noteId, int chunkIndex,
                           const QString& content, const QString& noteHash,
                           const QVector<float>& vec) {
  if (!prepareInsertStmts()) return false;
  bool ok = executeInsert(noteId, chunkIndex, content, noteHash, vec);
  finalizeInsertStmts();
  return ok;
}

bool VectorDb::deleteChunksByNoteId(const QString& noteId) {
  if (!m_db) return false;
  QMutexLocker locker(&m_mutex);

  QByteArray idUtf8 = noteId.toUtf8();

  // Step 1: 删除向量
  sqlite3_stmt* stmt1 = nullptr;
  const char* sql1 =
      "DELETE FROM vec_index WHERE rowid IN (SELECT rowid FROM note_chunks "
      "WHERE note_id = ?);";
  if (sqlite3_prepare_v2(m_db, sql1, -1, &stmt1, nullptr) != SQLITE_OK) {
    qWarning() << "[VectorDb] delete vec prepare失败:" << sqlite3_errmsg(m_db);
    return false;
  }
  sqlite3_bind_text(stmt1, 1, idUtf8.constData(), -1, SQLITE_TRANSIENT);
  int rc = sqlite3_step(stmt1);
  sqlite3_finalize(stmt1);
  if (rc != SQLITE_DONE) {
    qWarning() << "[VectorDb] delete vec失败:" << sqlite3_errmsg(m_db);
    return false;
  }

  // Step 2: 删除元数据
  sqlite3_stmt* stmt2 = nullptr;
  const char* sql2 = "DELETE FROM note_chunks WHERE note_id = ?;";
  if (sqlite3_prepare_v2(m_db, sql2, -1, &stmt2, nullptr) != SQLITE_OK) {
    qWarning() << "[VectorDb] delete meta prepare失败:" << sqlite3_errmsg(m_db);
    return false;
  }
  sqlite3_bind_text(stmt2, 1, idUtf8.constData(), -1, SQLITE_TRANSIENT);
  rc = sqlite3_step(stmt2);
  sqlite3_finalize(stmt2);
  if (rc != SQLITE_DONE) {
    qWarning() << "[VectorDb] delete meta失败:" << sqlite3_errmsg(m_db);
    return false;
  }

  return true;
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
void VectorDb::fillMissingVec(EmbeddingEngine* engine,
                              const QList<QString>& allNoteIdList) {
  Q_UNUSED(engine);
  Q_UNUSED(allNoteIdList);
  // TODO: 遍历 allNoteIdList，对缺失的 noteId 调用
  //       MarkdownChunker::processText + beginTransaction/insertChunk/commit
}

QVector<VectorHit> VectorDb::searchWithContent(const QVector<float>& queryVec,
                                               int topN, float threshold) {
  QVector<VectorHit> results;
  if (!m_db || queryVec.size() != m_embeddingDim || topN <= 0) return results;

  QByteArray vecBin((const char*)queryVec.constData(),
                    queryVec.size() * sizeof(float));
  sqlite3_stmt* stmt = nullptr;

  // ✅ 使用 k = ? 代替 LIMIT，兼容性最好
  const char* sql = R"(
        SELECT c.note_id, c.chunk_index, v.distance, c.content
        FROM vec_index v
        JOIN note_chunks c ON c.rowid = v.rowid
        WHERE v.vec MATCH ?
          AND k = ?
        ORDER BY v.distance ASC
    )";

  int rc = sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    qWarning() << "[VectorDb] searchWithContent prepare失败:"
               << sqlite3_errmsg(m_db);
    return results;
  }

  sqlite3_bind_blob(stmt, 1, vecBin.constData(), vecBin.size(),
                    SQLITE_TRANSIENT);
  sqlite3_bind_int(stmt, 2, topN);  // ✅ 绑定 k 值

  float distThreshold = 1.0f - threshold;

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    float dist = (float)sqlite3_column_double(stmt, 2);
    if (dist > distThreshold) continue;

    VectorHit hit;
    hit.noteId = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 0));
    hit.chunkIndex = sqlite3_column_int(stmt, 1);
    hit.score = 1.0f - dist;

    const char* text = (const char*)sqlite3_column_text(stmt, 3);
    hit.content = text ? QString::fromUtf8(text) : QString();

    results.append(hit);
  }

  sqlite3_finalize(stmt);
  return results;
}

int VectorDb::countChunks() const {
  if (!m_db) return 0;
  sqlite3_stmt* stmt = nullptr;
  int rc = sqlite3_prepare_v2(m_db, "SELECT COUNT(*) FROM note_chunks;", -1,
                              &stmt, nullptr);
  if (rc != SQLITE_OK) return 0;

  int count = 0;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    count = sqlite3_column_int(stmt, 0);
  }
  sqlite3_finalize(stmt);
  return count;
}

bool VectorDb::clearAll() {
  if (!m_db) return false;

  QMutexLocker locker(&m_mutex);

  // ⚠必须在事务中执行，保证原子性
  // 先删 vec_index（依赖 rowid），再删 note_chunks
  const char* sql = R"(
        BEGIN TRANSACTION;
        DELETE FROM vec_index;
        DELETE FROM note_chunks;
        COMMIT;
    )";

  char* err = nullptr;
  int rc = sqlite3_exec(m_db, sql, nullptr, nullptr, &err);

  if (rc != SQLITE_OK) {
    qCritical() << "[VectorDb] clearAll失败:" << (err ? err : "未知错误");
    if (err) sqlite3_free(err);
    sqlite3_exec(m_db, "ROLLBACK;", nullptr, nullptr, nullptr);
    return false;
  }

  qDebug() << "[VectorDb] ✅ 向量库已清空，准备全量重建";
  return true;
}

bool VectorDb::hasNoteChunks(const QString& noteId) const {
  if (!m_db) return false;

  // 注意：此函数为只读查询，不需要加 m_mutex
  // （SQLite WAL模式下读不阻塞写，且const方法不应持有非mutable锁）
  sqlite3_stmt* stmt = nullptr;
  const char* sql = "SELECT 1 FROM note_chunks WHERE note_id = ? LIMIT 1;";

  int rc = sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    qWarning() << "[VectorDb] hasNoteChunks prepare失败:"
               << sqlite3_errmsg(m_db);
    return false;
  }

  sqlite3_bind_text(stmt, 1, noteId.toUtf8().constData(), -1, SQLITE_TRANSIENT);

  bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
  sqlite3_finalize(stmt);

  return exists;
}

// isNoteContentChanged 改为比对 note_hash
bool VectorDb::isNoteContentChanged(const QString& noteId,
                                    const QString& newNoteHash) const {
  if (!m_db) return true;
  sqlite3_stmt* stmt = nullptr;
  // ✅ 只需取任意一条记录的 note_hash 即可（同一笔记所有chunk的note_hash相同）
  const char* sql =
      "SELECT note_hash FROM note_chunks WHERE note_id = ? LIMIT 1;";
  int rc = sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) return true;
  sqlite3_bind_text(stmt, 1, noteId.toUtf8().constData(), -1, SQLITE_TRANSIENT);

  bool changed = true;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    const char* stored = (const char*)sqlite3_column_text(stmt, 0);
    changed = !stored || QString(stored) != newNoteHash;
  }
  sqlite3_finalize(stmt);
  return changed;
}

#endif  // VECTOR_SEARCH