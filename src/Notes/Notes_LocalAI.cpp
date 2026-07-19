#include "src/AI/EmbeddingEngine.h"
#include "src/AI/GlobalAI.h"
#include "src/AI/VectorDb.h"
#include "src/Notes/MarkdownChunker.h"
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

  VectorDb localVecDb;
  QString vecDir = QDir(privateDir).filePath("model");
  QString vecDbPath = QDir(vecDir).filePath("note_vector.sqlite");
  if (!localVecDb.open(vecDbPath)) {
    qWarning() << "向量库打开失败：" << vecDbPath;
    return false;
  }

  QString noteId = getNoteIdFromFilePath(mdFilePath);
  QString fullContent = loadNoteFullText(mdFilePath);
  if (fullContent.trimmed().isEmpty()) {
    qDebug() << "[DEBUG] 笔记内容为空，跳过向量化:" << noteId;
    return false;
  }

  // 🔵 新增诊断点：分块开始
  qDebug() << "[DEBUG] 开始分块+encode, noteId:" << noteId
           << ", 文本长度:" << fullContent.length();

  // ✅ 核心改造：使用 MarkdownChunker 替代单次 encode
  // ✅ 先从 unique_ptr 取出原始指针，再 dynamic_cast
  auto* embEngine = dynamic_cast<EmbeddingEngine*>(g_embEngine.get());
  if (!embEngine) {
    qWarning() << "❌ g_embEngine 不是 EmbeddingEngine 类型，无法分块:"
               << noteId;
    return false;
  }

  ChunkConfig config;
  MarkdownChunker chunker(*embEngine, config);
  QVector<NoteChunk> chunks = chunker.processText(fullContent);

  // 🔵 新增诊断点：分块结果
  qDebug() << "[DEBUG] 分块完成, noteId:" << noteId
           << ", 总块数:" << chunks.size();

  if (chunks.isEmpty()) {
    qWarning() << "分块结果为空, noteId:" << noteId;
    return false;
  }

  // ✅ 核心改造：事务内原子替换（先删旧块，再插新块）
  bool ok = localVecDb.beginTransaction();
  if (!ok) {
    qWarning() << "❌ 开启事务失败 noteId:" << noteId;
    return false;
  }

  // 删除该笔记的所有旧 chunk
  ok = localVecDb.deleteChunksByNoteId(noteId);
  if (!ok) {
    qWarning() << "❌ 删除旧chunk失败 noteId:" << noteId;
    localVecDb.rollback();
    return false;
  }

  // 批量插入新 chunk
  for (const auto& chunk : chunks) {
    if (chunk.vector.size() != 384) {
      qWarning() << "⚠️ chunk向量维度异常:" << chunk.vector.size()
                 << "noteId:" << noteId << "chunkIndex:" << chunk.chunkIndex;
      localVecDb.rollback();
      return false;
    }
    ok = localVecDb.insertChunk(noteId, chunk.chunkIndex, chunk.content,
                                chunk.vector);
    if (!ok) {
      qWarning() << "❌ chunk写入失败 noteId:" << noteId
                 << "chunkIndex:" << chunk.chunkIndex;
      localVecDb.rollback();
      return false;
    }
  }

  ok = localVecDb.commit();
  if (ok) {
    qDebug() << "✅ 向量更新成功 noteId:" << noteId << ", 共" << chunks.size()
             << "个chunk";
  } else {
    qWarning() << "❌ 提交事务失败 noteId:" << noteId
               << ", vecDbPath:" << vecDbPath;
    localVecDb.rollback();
  }
  return ok;
}

bool Notes::removeNoteVector(const QString& noteId) {
  if (!isLocalAIModel) return false;

  VectorDb localVecDb;
  QString vecDir = QDir(privateDir).filePath("model");
  QDir dir;
  dir.mkpath(vecDir);
  QString vecDbPath = QDir(vecDir).filePath("note_vector.sqlite");

  if (!localVecDb.open(vecDbPath)) {
    qWarning() << "删除向量：向量库打开失败 " << vecDbPath;
    return false;
  }

  // ✅ 事务包裹双表删除，保证原子性
  bool ok = localVecDb.beginTransaction();
  if (!ok) {
    qWarning() << "❌ 删除向量开启事务失败 noteId:" << noteId;
    return false;
  }

  ok = localVecDb.deleteChunksByNoteId(noteId);
  if (ok) {
    localVecDb.commit();
    qDebug() << "✅ 删除笔记向量成功 noteId:" << noteId;
  } else {
    localVecDb.rollback();
    qWarning() << "❌ 删除笔记向量失败 noteId:" << noteId;
  }
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