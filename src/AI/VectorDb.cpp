#ifdef VECTOR_SEARCH

#define SQLITE_CORE

#include "VectorDb.h"

#include <sqlite3.h>

#include <QCryptographicHash>
#include <QDebug>

#include "sqlite-vec.h"
#include "sqlite3ext.h"

// ==================== 生命周期 ====================

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
    qCritical() << "[VectorDb] sqlite文件打开失败:" << sqlite3_errmsg(m_db);
    return false;
  }
  sqlite3_enable_load_extension(m_db, 1);

  // WAL + NORMAL：本地笔记场景安全且快数倍
  sqlite3_exec(m_db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
  sqlite3_exec(m_db, "PRAGMA synchronous=NORMAL;", nullptr, nullptr, nullptr);

  if (!initTable()) {
    qCritical() << "[VectorDb] 建表失败";
    return false;
  }
  return true;
}

void VectorDb::close() {
  finalizeInsertStmts();
  if (m_db) {
    sqlite3_close(m_db);
    m_db = nullptr;
  }
}

// ==================== Schema 初始化 ====================

bool VectorDb::initTable() {
  char* err = nullptr;

  // ✅ 新 Schema：坐标为一等公民，content 降级为可选 fallback
  // content_hash 使用 BLOB(32) 存储原始 SHA-256 字节
  const char* sqlMeta = R"(
        CREATE TABLE IF NOT EXISTS note_chunks (
            note_id      TEXT    NOT NULL,
            chunk_index  INTEGER NOT NULL,
            char_start   INTEGER NOT NULL,
            char_end     INTEGER NOT NULL,
            section_path TEXT    NOT NULL DEFAULT '',
            content      TEXT,
            content_hash BLOB    NOT NULL,
            note_hash    TEXT    NOT NULL DEFAULT '',
            PRIMARY KEY (note_id, chunk_index)
        );
    )";

  int rc = sqlite3_exec(m_db, sqlMeta, nullptr, nullptr, &err);
  if (rc != SQLITE_OK) {
    qCritical() << "[VectorDb] 建note_chunks失败:"
                << (err ? err : "无错误信息");
    if (err) sqlite3_free(err);
    return false;
  }

  // 动态维度向量表
  QString sqlVecStr = QString(R"(
        CREATE VIRTUAL TABLE IF NOT EXISTS vec_index USING vec0(
            vec FLOAT[%1] distance_metric=cosine
        );
    )")
                          .arg(m_embeddingDim);

  QByteArray sqlVecUtf8 = sqlVecStr.toUtf8();
  rc = sqlite3_exec(m_db, sqlVecUtf8.constData(), nullptr, nullptr, &err);
  if (rc != SQLITE_OK) {
    qCritical() << "[VectorDb] 创建vec0虚拟表失败:"
                << (err ? err : "无错误信息")
                << ", 目标维度:" << m_embeddingDim;
    if (err) sqlite3_free(err);
    return false;
  }

  // 加速按 note_id 删除/查询
  const char* sqlIdx =
      "CREATE INDEX IF NOT EXISTS idx_chunks_note ON note_chunks(note_id);";
  rc = sqlite3_exec(m_db, sqlIdx, nullptr, nullptr, &err);
  if (rc != SQLITE_OK) {
    qCritical() << "[VectorDb] 创建索引失败:" << (err ? err : "无错误信息");
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
  if (m_stmtMeta && m_stmtVec) return true;

  const char* sqlMeta =
      "INSERT OR REPLACE INTO note_chunks("
      "  note_id, chunk_index, char_start, char_end, section_path,"
      "  content, content_hash, note_hash"
      ") VALUES(?, ?, ?, ?, ?, ?, ?, ?);";

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
                             qint64 charStart, qint64 charEnd,
                             const QString& sectionPath, const QString& content,
                             const QByteArray& contentHash,
                             const QString& noteHash,
                             const QVector<float>& vec) {
  QMutexLocker locker(&m_mutex);
  if (!m_stmtMeta || !m_stmtVec) {
    qWarning() << "[VectorDb] executeInsert失败: stmt未准备";
    return false;
  }
  if (vec.size() != m_embeddingDim) {
    qWarning() << "[VectorDb] executeInsert失败: 维度不匹配" << vec.size()
               << "vs" << m_embeddingDim;
    return false;
  }
  if (contentHash.size() != 32) {
    qWarning()
        << "[VectorDb] executeInsert失败: contentHash必须为32字节原始SHA-256";
    return false;
  }

  // Step 1: Meta
  sqlite3_reset(m_stmtMeta);
  sqlite3_clear_bindings(m_stmtMeta);

  QByteArray noteIdUtf8 = noteId.toUtf8();
  QByteArray sectionPathUtf8 = sectionPath.toUtf8();
  QByteArray contentUtf8 = content.toUtf8();
  QByteArray noteHashUtf8 = noteHash.toUtf8();

  sqlite3_bind_text(m_stmtMeta, 1, noteIdUtf8.constData(), -1,
                    SQLITE_TRANSIENT);
  sqlite3_bind_int(m_stmtMeta, 2, chunkIndex);
  sqlite3_bind_int64(m_stmtMeta, 3, charStart);
  sqlite3_bind_int64(m_stmtMeta, 4, charEnd);
  sqlite3_bind_text(m_stmtMeta, 5, sectionPathUtf8.constData(), -1,
                    SQLITE_TRANSIENT);
  sqlite3_bind_text(m_stmtMeta, 6, contentUtf8.constData(), -1,
                    SQLITE_TRANSIENT);
  sqlite3_bind_blob(m_stmtMeta, 7, contentHash.constData(), contentHash.size(),
                    SQLITE_TRANSIENT);
  sqlite3_bind_text(m_stmtMeta, 8, noteHashUtf8.constData(), -1,
                    SQLITE_TRANSIENT);

  if (sqlite3_step(m_stmtMeta) != SQLITE_DONE) {
    qWarning() << "[VectorDb] insert meta失败:" << sqlite3_errmsg(m_db);
    return false;
  }

  sqlite_int64 rowid = sqlite3_last_insert_rowid(m_db);

  // Step 2: Vec
  QByteArray vecBin(reinterpret_cast<const char*>(vec.constData()),
                    vec.size() * sizeof(float));

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

bool VectorDb::insertChunk(const QString& noteId, int chunkIndex,
                           qint64 charStart, qint64 charEnd,
                           const QString& sectionPath, const QString& content,
                           const QByteArray& contentHash,
                           const QString& noteHash, const QVector<float>& vec) {
  if (!prepareInsertStmts()) return false;
  bool ok = executeInsert(noteId, chunkIndex, charStart, charEnd, sectionPath,
                          content, contentHash, noteHash, vec);
  finalizeInsertStmts();
  return ok;
}

// ==================== 删除操作 ====================

bool VectorDb::deleteChunksByNoteId(const QString& noteId) {
  if (!m_db) return false;
  QMutexLocker locker(&m_mutex);

  QByteArray idUtf8 = noteId.toUtf8();

  // Step 1: 删除向量（通过子查询获取 rowid）
  sqlite3_stmt* stmt1 = nullptr;
  const char* sql1 =
      "DELETE FROM vec_index WHERE rowid IN ("
      "  SELECT rowid FROM note_chunks WHERE note_id = ?"
      ");";
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

QVector<VectorHit> VectorDb::searchWithContent(const QVector<float>& queryVec,
                                               int topN, float threshold) {
  QVector<VectorHit> results;
  if (!m_db || queryVec.size() != m_embeddingDim || topN <= 0) return results;

  QByteArray vecBin(reinterpret_cast<const char*>(queryVec.constData()),
                    queryVec.size() * sizeof(float));
  sqlite3_stmt* stmt = nullptr;

  const char* sql = R"(
        SELECT c.note_id, c.chunk_index, v.distance,
               c.char_start, c.char_end, c.section_path,
               c.content_hash, c.content
        FROM vec_index v
        JOIN note_chunks c ON c.rowid = v.rowid
        WHERE v.vec MATCH ? AND k = ?
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
  sqlite3_bind_int(stmt, 2, topN);

  float distThreshold = 1.0f - threshold;

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    float dist = static_cast<float>(sqlite3_column_double(stmt, 2));
    if (dist > distThreshold) continue;

    VectorHit hit;
    hit.noteId = QString::fromUtf8(
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
    hit.chunkIndex = sqlite3_column_int(stmt, 1);
    hit.score = 1.0f - dist;

    hit.charStart = sqlite3_column_int64(stmt, 3);
    hit.charEnd = sqlite3_column_int64(stmt, 4);

    const char* sp =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
    hit.sectionPath = sp ? QString::fromUtf8(sp) : QString();

    const void* hashBlob = sqlite3_column_blob(stmt, 6);
    int hashLen = sqlite3_column_bytes(stmt, 6);
    if (hashBlob && hashLen == 32) {
      hit.contentHash =
          QByteArray(reinterpret_cast<const char*>(hashBlob), hashLen);
    }

    const char* text =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
    hit.content = text ? QString::fromUtf8(text) : QString();

    results.append(hit);
  }

  sqlite3_finalize(stmt);
  return results;
}

QList<QPair<QString, float>> VectorDb::querySimilar(
    const QVector<float>& queryVec, int topN) {
  QList<QPair<QString, float>> out;
  if (!m_db) return out;

  QByteArray vecBin(reinterpret_cast<const char*>(queryVec.constData()),
                    queryVec.size() * sizeof(float));
  sqlite3_stmt* stmt = nullptr;

  const char* sql = R"(
        SELECT c.note_id, MIN(v.distance) AS min_dist
        FROM vec_index v
        JOIN note_chunks c ON c.rowid = v.rowid
        WHERE v.vec MATCH ?
        GROUP BY c.note_id
        ORDER BY min_dist ASC
        LIMIT ?;
    )";

  int rc = sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    qWarning() << "[VectorDb] querySimilar prepare失败:"
               << sqlite3_errmsg(m_db);
    return out;
  }

  sqlite3_bind_blob(stmt, 1, vecBin.constData(), vecBin.size(),
                    SQLITE_TRANSIENT);
  sqlite3_bind_int(stmt, 2, topN);

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    QString id = QString::fromUtf8(
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
    float dist = static_cast<float>(sqlite3_column_double(stmt, 1));
    out.append({id, 1.0f - dist});
  }

  sqlite3_finalize(stmt);
  return out;
}

// ==================== 维护操作 ====================

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

  sqlite3_stmt* stmt = nullptr;
  const char* sql = "SELECT 1 FROM note_chunks WHERE note_id = ? LIMIT 1;";

  int rc = sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    qWarning() << "[VectorDb] hasNoteChunks prepare失败:"
               << sqlite3_errmsg(m_db);
    return false;
  }

  QByteArray idUtf8 = noteId.toUtf8();
  sqlite3_bind_text(stmt, 1, idUtf8.constData(), -1, SQLITE_TRANSIENT);

  bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
  sqlite3_finalize(stmt);
  return exists;
}

bool VectorDb::isNoteContentChanged(const QString& noteId,
                                    const QString& newNoteHash) const {
  if (!m_db) return true;

  sqlite3_stmt* stmt = nullptr;
  const char* sql =
      "SELECT note_hash FROM note_chunks WHERE note_id = ? LIMIT 1;";

  int rc = sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) return true;

  QByteArray idUtf8 = noteId.toUtf8();
  sqlite3_bind_text(stmt, 1, idUtf8.constData(), -1, SQLITE_TRANSIENT);

  bool changed = true;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    const char* stored =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    changed = !stored || QString::fromUtf8(stored) != newNoteHash;
  }
  sqlite3_finalize(stmt);
  return changed;
}

void VectorDb::fillMissingVec(EmbeddingEngine* engine,
                              const QList<QString>& allNoteIdList) {
  Q_UNUSED(engine);
  Q_UNUSED(allNoteIdList);
  // TODO: 遍历 allNoteIdList，对缺失的 noteId 调用
  //       MarkdownChunker::processText → beginTransaction → executeInsert × N →
  //       commit
}

int VectorDb::purgeOrphanedNotes(const QStringList& validNoteIds) {
  if (!m_db) return -1;
  QMutexLocker locker(&m_mutex);

  // ✅ 使用事务保证原子性
  if (sqlite3_exec(m_db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr) !=
      SQLITE_OK) {
    qWarning() << "[VectorDb] purgeOrphanedNotes BEGIN失败:"
               << sqlite3_errmsg(m_db);
    return -1;
  }

  // Step 1: 创建临时表存放有效ID（避免超长 NOT IN 参数绑定问题）
  const char* createTempSql =
      "CREATE TEMP TABLE IF NOT EXISTS _valid_notes(note_id TEXT PRIMARY KEY);";
  if (sqlite3_exec(m_db, createTempSql, nullptr, nullptr, nullptr) !=
      SQLITE_OK) {
    qWarning() << "[VectorDb] 创建临时表失败:" << sqlite3_errmsg(m_db);
    sqlite3_exec(m_db, "ROLLBACK;", nullptr, nullptr, nullptr);
    return -1;
  }

  // 清空临时表（防止上次残留）
  sqlite3_exec(m_db, "DELETE FROM _valid_notes;", nullptr, nullptr, nullptr);

  // Step 2: 批量插入有效ID到临时表
  sqlite3_stmt* insertStmt = nullptr;
  const char* insertSql = "INSERT INTO _valid_notes(note_id) VALUES(?);";
  if (sqlite3_prepare_v2(m_db, insertSql, -1, &insertStmt, nullptr) !=
      SQLITE_OK) {
    qWarning() << "[VectorDb] 准备临时表插入失败:" << sqlite3_errmsg(m_db);
    sqlite3_exec(m_db, "ROLLBACK;", nullptr, nullptr, nullptr);
    return -1;
  }

  for (const QString& id : validNoteIds) {
    QByteArray idUtf8 = id.toUtf8();
    sqlite3_reset(insertStmt);
    sqlite3_bind_text(insertStmt, 1, idUtf8.constData(), -1, SQLITE_TRANSIENT);
    sqlite3_step(insertStmt);  // 忽略单条插入错误，继续执行
  }
  sqlite3_finalize(insertStmt);

  // Step 3: 删除不在有效列表中的孤儿数据（先删向量，再删元数据）
  const char* deleteVecSql = R"(
        DELETE FROM vec_index WHERE rowid IN (
            SELECT rowid FROM note_chunks
            WHERE note_id NOT IN (SELECT note_id FROM _valid_notes)
        );
    )";

  const char* deleteMetaSql = R"(
        DELETE FROM note_chunks
        WHERE note_id NOT IN (SELECT note_id FROM _valid_notes);
    )";

  int rc1 = sqlite3_exec(m_db, deleteVecSql, nullptr, nullptr, nullptr);
  int rc2 = sqlite3_exec(m_db, deleteMetaSql, nullptr, nullptr, nullptr);

  // Step 4: 获取受影响的行数（用于日志/反馈）
  int deletedCount = sqlite3_changes(m_db);

  // 清理临时表
  sqlite3_exec(m_db, "DROP TABLE IF EXISTS _valid_notes;", nullptr, nullptr,
               nullptr);

  if (rc1 != SQLITE_OK || rc2 != SQLITE_OK) {
    qCritical() << "[VectorDb] purgeOrphanedNotes 删除失败:"
                << sqlite3_errmsg(m_db);
    sqlite3_exec(m_db, "ROLLBACK;", nullptr, nullptr, nullptr);
    return -1;
  }

  if (sqlite3_exec(m_db, "COMMIT;", nullptr, nullptr, nullptr) != SQLITE_OK) {
    qCritical() << "[VectorDb] purgeOrphanedNotes COMMIT失败:"
                << sqlite3_errmsg(m_db);
    return -1;
  }

  qDebug() << "[VectorDb] ✅ 孤立笔记清理完成, 移除无效笔记数:" << deletedCount;
  return deletedCount;
}

#endif  // VECTOR_SEARCH