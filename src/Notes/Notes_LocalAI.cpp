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

  if (!g_vectorDb) {
    qWarning() << "向量数据库打开失败：";
    return false;
  }

  QString noteId = mdFilePath;
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
  bool ok = g_vectorDb->beginTransaction();
  if (!ok) {
    qWarning() << "❌ 开启事务失败 noteId:" << noteId;
    return false;
  }

  // 删除该笔记的所有旧 chunk
  ok = g_vectorDb->deleteChunksByNoteId(noteId);
  if (!ok) {
    qWarning() << "❌ 删除旧chunk失败 noteId:" << noteId;
    g_vectorDb->rollback();
    return false;
  }

  // 批量插入新 chunk
  for (const auto& chunk : chunks) {
    if (chunk.vector.size() != embEngine->embeddingDimension()) {
      qWarning() << "⚠️ chunk向量维度异常:" << chunk.vector.size()
                 << "noteId:" << noteId << "chunkIndex:" << chunk.chunkIndex;
      g_vectorDb->rollback();
      return false;
    }
    ok = g_vectorDb->insertChunk(noteId, chunk.chunkIndex, chunk.content,
                                 chunk.vector);
    if (!ok) {
      qWarning() << "❌ chunk写入失败 noteId:" << noteId
                 << "chunkIndex:" << chunk.chunkIndex;
      g_vectorDb->rollback();
      return false;
    }
  }

  ok = g_vectorDb->commit();
  if (ok) {
    qDebug() << "✅ 向量更新成功 noteId:" << noteId << ", 共" << chunks.size()
             << "个chunk";
  } else {
    qWarning() << "❌ 提交事务失败 noteId:" << noteId;

    g_vectorDb->rollback();
  }
  return ok;
}

// 非加锁实现
/*bool Notes::syncNoteVectorsBatchToDb(const QString& mdFilePath) {
  QString mdContent = loadNoteFullText(mdFilePath);
  if (mdContent.trimmed().isEmpty()) return true;

  if (!isLocalAIModel || !g_embEngine || !g_embEngine->isValid()) return false;
  auto* embEngine = dynamic_cast<EmbeddingEngine*>(g_embEngine.get());
  if (!embEngine || !g_vectorDb) return false;

  ChunkConfig config;
  MarkdownChunker chunker(*embEngine, config);

  // 1. 纯分块（零推理）
  auto chunks = chunker.splitForBatch(mdFilePath, mdContent);
  if (chunks.isEmpty()) return true;

  // ✅ 2. 提取文本列表，调用真正的 Batch Encode
  QStringList texts;
  texts.reserve(chunks.size());
  for (const auto& c : chunks) texts.append(c.content);

  QElapsedTimer timer;
  timer.start();
  // maxBatchSize 建议设为 32~64，防止超出 n_ctx
  auto vectors = embEngine->encodeBatch(texts, 64);

  qDebug() << "[BATCH] encode耗时:" << timer.elapsed()
           << "ms, chunks:" << chunks.size();

  if ((int)vectors.size() != chunks.size()) {
    qWarning() << "[BATCH] encode数量不匹配";
    return false;
  }

  // ✅ 3. 单事务写入 DB
  timer.restart();

  if (!g_vectorDb->beginTransaction()) return false;

  if (!g_vectorDb->deleteChunksByNoteId(mdFilePath)) {
    g_vectorDb->rollback();
    return false;
  }

  const int dim = embEngine->embeddingDimension();
  for (int i = 0; i < chunks.size(); ++i) {
    if (vectors[i].size() != dim ||
        !g_vectorDb->insertChunk(mdFilePath, chunks[i].chunkIndex,
                                 chunks[i].content, vectors[i])) {
      g_vectorDb->rollback();
      return false;
    }
  }

  if (!g_vectorDb->commit()) {
    g_vectorDb->rollback();
    return false;
  }
  qDebug() << "[BATCH] DB写入耗时:" << timer.elapsed() << "ms";

  return true;
}*/

// 加锁实现
bool Notes::syncNoteVectorsBatchToDb(const QString& mdFilePath) {
  QString mdContent = loadNoteFullText(mdFilePath);
  if (mdContent.trimmed().isEmpty()) return true;

  if (!isLocalAIModel || !g_embEngine || !g_embEngine->isValid()) return false;
  auto* embEngine = dynamic_cast<EmbeddingEngine*>(g_embEngine.get());
  if (!embEngine || !g_vectorDb) return false;

  ChunkConfig config;
  MarkdownChunker chunker(*embEngine, config);
  auto chunks = chunker.splitForBatch(mdFilePath, mdContent);
  if (chunks.isEmpty()) return true;

  QStringList texts;
  texts.reserve(chunks.size());
  for (const auto& c : chunks) texts.append(c.content);

  QElapsedTimer timer;
  timer.start();
  // 修正类型：和encodeBatch返回对齐 std::vector<QList<float>>
  std::vector<QList<float>> vectors;
  int embDim = 0;

  // 推理临界区上锁：同时读取维度，避免无锁读共享对象
  {
    QMutexLocker embLock(&s_embMutex);
    vectors = embEngine->encodeBatch(texts, 64);
    embDim = embEngine->embeddingDimension();
  }

  qDebug() << "[BATCH] encode耗时:" << timer.elapsed()
           << "ms, chunks:" << chunks.size();

  if ((int)vectors.size() != chunks.size()) {
    qWarning() << "[BATCH] encode数量不匹配";
    return false;
  }

  timer.restart();
  bool dbSuccess = false;
  // DB事务整体上锁
  {
    QMutexLocker dbLock(&s_vecDbMutex);
    // 变量提到goto之前，解决C2362跳过初始化报错
    bool insertAllOk = true;

    if (!g_vectorDb->beginTransaction()) {
      goto dbEnd;
    }

    if (!g_vectorDb->deleteChunksByNoteId(mdFilePath)) {
      g_vectorDb->rollback();
      goto dbEnd;
    }

    for (int i = 0; i < chunks.size(); ++i) {
      // vectors[i] 现在是 QList<float>，匹配insertChunk参数
      if (vectors[i].size() != embDim ||
          !g_vectorDb->insertChunk(mdFilePath, chunks[i].chunkIndex,
                                   chunks[i].content, vectors[i])) {
        insertAllOk = false;
        break;
      }
    }

    if (!insertAllOk) {
      g_vectorDb->rollback();
      goto dbEnd;
    }

    if (g_vectorDb->commit()) {
      dbSuccess = true;
    } else {
      g_vectorDb->rollback();
    }
  dbEnd:;  // 空语句占位，语法规范
  }

  qDebug() << "[BATCH] DB写入耗时:" << timer.elapsed() << "ms";
  return dbSuccess;
}

bool Notes::removeNoteVector(const QString& noteId) {
  if (!isLocalAIModel) return false;

  if (!g_vectorDb) {
    qWarning() << "删除向量：向量数据库打开失败 ";
    return false;
  }

  // ✅ 事务包裹双表删除，保证原子性
  bool ok = g_vectorDb->beginTransaction();
  if (!ok) {
    qWarning() << "❌ 删除向量开启事务失败 noteId:" << noteId;
    return false;
  }

  ok = g_vectorDb->deleteChunksByNoteId(noteId);
  if (ok) {
    g_vectorDb->commit();
    qDebug() << "✅ 删除笔记向量成功 noteId:" << noteId;
  } else {
    g_vectorDb->rollback();
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