#include "src/AI/EmbeddingEngine.h"
#include "src/AI/GlobalAI.h"
#include "src/AI/VectorDb.h"
#include "src/Notes/Notes.h"

#ifdef VECTOR_SEARCH
bool Notes::syncNoteVectorToDb(const QString& mdFilePath) {
  qDebug() << "[DEBUG] isLocalAIModel:" << isLocalAIModel
           << " g_embEngine有效指针:" << (g_embEngine != nullptr);
  if (g_embEngine) qDebug() << "[DEBUG] emb isValid:" << g_embEngine->isValid();

  if (!isLocalAIModel || !g_embEngine || !g_embEngine->isValid()) {
    qWarning() << "AI引擎未就绪：" << mdFilePath;
    return false;
  }

  if (!isLocalAIModel || !g_embEngine || !g_embEngine->isValid()) {
    qWarning() << "AI引擎未就绪：" << mdFilePath;
    return false;
  }
  // 每个线程单独新建连接，无跨线程冲突
  VectorDb localVecDb;
  QString vecDir = QDir(privateDir).filePath("model");
  QString vecDbPath = QDir(vecDir).filePath("note_vector.sqlite");
  if (!localVecDb.open(vecDbPath)) {
    qWarning() << "向量库打开失败：" << vecDbPath;
    return false;
  }
  QString noteId = getNoteIdFromFilePath(mdFilePath);
  QString fullContent = loadNoteFullText(mdFilePath);
  if (fullContent.trimmed().isEmpty()) return false;
  QVector<float> vec384 = g_embEngine->encode(fullContent);
  bool ok = localVecDb.upsertNoteVec(noteId, vec384);
  if (ok) qDebug() << "向量更新成功 noteId:" << noteId;
  return ok;
}

bool Notes::removeNoteVector(const QString& noteId) {
  // 没有AI模型直接跳过
  if (!isLocalAIModel) return false;

  // 线程内独立临时连接
  VectorDb localVecDb;
  QString vecDir = QDir(privateDir).filePath("model");
  QDir dir;
  dir.mkpath(vecDir);
  QString vecDbPath = QDir(vecDir).filePath("note_vector.sqlite");

  if (!localVecDb.open(vecDbPath)) {
    qWarning() << "删除向量：向量库打开失败 " << vecDbPath;
    return false;
  }

  bool ok = localVecDb.deleteNoteVec(noteId);
  if (ok)
    qDebug() << "删除笔记向量 noteId:" << noteId;
  else
    qWarning() << "删除笔记向量失败 noteId:" << noteId;
  return ok;
}

QString Notes::getNoteIdFromFilePath(const QString& mdPath) {
  QFileInfo fi(mdPath);
  return fi.baseName();
}

QString Notes::loadNoteFullText(const QString& mdPath) {
  QFile f(mdPath);
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return "";
  QString text = f.readAll();
  f.close();
  return text;
}
#endif