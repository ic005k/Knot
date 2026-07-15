#ifdef VECTOR_SEARCH
#ifndef VECTORDATABASE_H
#define VECTORDATABASE_H

#include <QList>
#include <QPair>
#include <QString>
#include <QVector>

#include "BaseEmbeddingEngine.h"

struct sqlite3;

class VectorDb {
 public:
  VectorDb();
  ~VectorDb();

  bool open(const QString& dbPath);
  void close();
  bool upsertNoteVec(const QString& noteId, const QVector<float>& vec384);
  QList<QPair<QString, float>> querySimilar(const QVector<float>& queryVec,
                                            int topN = 20);
  bool deleteNoteVec(const QString& noteId);
  void fillMissingVec(BaseEmbeddingEngine* engine,
                      const QList<QString>& allNoteIdList);

 private:
  bool initTable();
  sqlite3* m_db = nullptr;
};
#endif  // VECTORDATABASE_H
#endif  // VECTOR_SEARCH