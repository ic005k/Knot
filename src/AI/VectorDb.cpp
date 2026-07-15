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

static int sqlCallback(void* res, int colCnt, char** colData, char** colName) {
  auto result = (QList<QPair<QString, float>>*)res;
  QString noteId = colData[0];
  float score = std::atof(colData[1]);
  result->append({noteId, score});
  return 0;
}

VectorDb::VectorDb() {
  // 进程全局注册sqlite-vec扩展，只需要执行一次
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
  bool tableOk = initTable();
  if (!tableOk) {
    qCritical() << "建表失败";
    return false;
  }
  return true;
}

void VectorDb::close() {
  if (m_db) sqlite3_close(m_db);
  m_db = nullptr;
}

bool VectorDb::initTable() {
  char* err = nullptr;
  // 1、先建普通表
  const char* sql1 =
      "CREATE TABLE IF NOT EXISTS note_vec (note_id TEXT PRIMARY KEY);";
  int rc = sqlite3_exec(m_db, sql1, nullptr, nullptr, &err);
  if (rc != SQLITE_OK) {
    qCritical() << "建note_vec失败:" << (err ? err : "无错误信息");
    if (err) sqlite3_free(err);
    return false;
  }

  // 2、修正官方标准语法 distance_metric=cosine
  const char* sql2 = R"(
    CREATE VIRTUAL TABLE IF NOT EXISTS vec_index USING vec0(
        note_id TEXT PRIMARY KEY,
        vec FLOAT[384] distance_metric=cosine
    );
  )";
  rc = sqlite3_exec(m_db, sql2, nullptr, nullptr, &err);
  if (rc != SQLITE_OK) {
    qCritical() << "创建vec0虚拟表失败:" << (err ? err : "无错误信息");
    if (err) sqlite3_free(err);
    return false;
  }
  sqlite3_free(err);
  return true;
}

bool VectorDb::upsertNoteVec(const QString& noteId,
                             const QVector<float>& vec384) {
  if (!m_db) return false;
  QByteArray vecBin((char*)vec384.data(), vec384.size() * sizeof(float));
  sqlite3_stmt* stmt;
  const char* sql =
      "INSERT OR REPLACE INTO vec_index(note_id, vec) VALUES(?, ?);";
  int rc = sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) return false;

  sqlite3_bind_text(stmt, 1, noteId.toUtf8().data(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_blob(stmt, 2, vecBin.data(), vecBin.size(), SQLITE_TRANSIENT);
  rc = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  return rc == SQLITE_DONE;
}

QList<QPair<QString, float>> VectorDb::querySimilar(
    const QVector<float>& queryVec, int topN) {
  QList<QPair<QString, float>> out;
  if (!m_db) return out;
  QByteArray vecBin((char*)queryVec.data(), queryVec.size() * sizeof(float));
  sqlite3_stmt* stmt;
  const char* sql = R"(
        SELECT note_id, distance FROM vec_index
        WHERE vec MATCH ? ORDER BY distance LIMIT ?;
    )";
  int rc = sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) return out;

  sqlite3_bind_blob(stmt, 1, vecBin.data(), vecBin.size(), SQLITE_TRANSIENT);
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

bool VectorDb::deleteNoteVec(const QString& noteId) {
  if (!m_db) return false;
  sqlite3_stmt* stmt;
  const char* sql = "DELETE FROM vec_index WHERE note_id = ?;";
  sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
  sqlite3_bind_text(stmt, 1, noteId.toUtf8().data(), -1, SQLITE_TRANSIENT);
  int rc = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  return rc == SQLITE_DONE;
}

void VectorDb::fillMissingVec(BaseEmbeddingEngine* engine,
                              const QList<QString>& allNoteIdList) {
  if (!m_db || !engine || !engine->isValid()) return;

  QList<QString> existIds;
  sqlite3_stmt* stmtSel;
  const char* sqlSel = "SELECT note_id FROM vec_index;";
  sqlite3_prepare_v2(m_db, sqlSel, -1, &stmtSel, nullptr);
  while (sqlite3_step(stmtSel) == SQLITE_ROW) {
    existIds.append((const char*)sqlite3_column_text(stmtSel, 0));
  }
  sqlite3_finalize(stmtSel);

  QSet<QString> existSet(existIds.begin(), existIds.end());
  for (const QString& nid : allNoteIdList) {
    if (existSet.contains(nid)) continue;
    // QString fullText = NoteManager::getNoteFullText(nid);
    // QVector<float> vec = engine->encode(fullText);
    // upsertNoteVec(nid, vec);
  }
}
#endif