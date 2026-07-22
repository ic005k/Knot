#include "src/AI/EmbeddingEngine.h"
#include "src/AI/GlobalAI.h"
#include "src/AI/VectorDb.h"
#include "src/Notes/MarkdownChunker.h"
#include "src/Notes/Notes.h"

#ifdef VECTOR_SEARCH

bool Notes::syncNoteVectorsBatchToDb(const QString& mdFilePath) {
  QString mdContent = loadNoteFullText(mdFilePath);

  // ===== 编码诊断日志 =====
  if (mdContent.contains(QChar::ReplacementCharacter)) {
    qWarning() << "[ENCODING_WARN] File may not be valid UTF-8, "
               << "content truncated or corrupted:" << mdFilePath
               << "| decoded_chars:" << mdContent.size();
  }

  if (mdContent.trimmed().isEmpty()) return true;

  if (!isLocalAIModel || !g_embEngine || !g_embEngine->isValid()) return false;
  auto* embEngine = dynamic_cast<EmbeddingEngine*>(g_embEngine.get());
  if (!embEngine || !g_vectorDb) return false;

  // ✅ 在锁外计算当前内容的 Hash，避免持锁做 CPU 密集操作
  QByteArray currentHashBytes =
      QCryptographicHash::hash(mdContent.toUtf8(), QCryptographicHash::Sha256)
          .toHex();
  QString currentHash = QString::fromLatin1(currentHashBytes);

  // ✅ 检查内容是否变更
  {
    QMutexLocker dbLock(&s_vecDbMutex);
    // 如果返回 false，说明 DB 中已有相同 hash 的记录，无需更新
    if (!g_vectorDb->isNoteContentChanged(mdFilePath, currentHash)) {
      int cnt = m_skipCount.fetchAndAddRelaxed(1);
      if (cnt == 0 || cnt % 100 == 99) {
        qDebug() << "[BATCH] 内容未变更, 累计跳过:" << (cnt + 1) << "条";
      }
      return true;
    }
  }

  ChunkConfig config;
  MarkdownChunker chunker(*embEngine, config);
  auto chunks = chunker.splitForBatch(mdFilePath, mdContent);
  if (chunks.isEmpty()) return true;

  QStringList texts;
  texts.reserve(chunks.size());
  for (const auto& c : chunks) texts.append(c.content);

  QElapsedTimer timer;

  // ====== Embedding 推理（独立计时） ======
  timer.start();

  std::vector<QList<float>> vectors;
  int embDim = 0;

  {
    QMutexLocker embLock(&s_embMutex);
    vectors = embEngine->encodeBatch(texts, 64);
    embDim = embEngine->embeddingDimension();
  }

  qDebug() << "[BATCH] encode耗时:" << timer.elapsed()
           << "ms, chunks:" << chunks.size();

  if ((int)vectors.size() != chunks.size()) {
    qWarning() << "[BATCH] encode数量不匹配: expected" << chunks.size() << "got"
               << vectors.size();
    return false;
  }

  // ====== DB 批量写入（独立计时） ======
  timer.restart();
  bool dbSuccess = false;
  {
    QMutexLocker dbLock(&s_vecDbMutex);
    bool insertAllOk = true;

    if (!g_vectorDb->beginTransaction()) goto dbEnd;

    // ✅ 批量写入前准备语句（零开销重入）
    if (!g_vectorDb->prepareInsertStmts()) {
      g_vectorDb->rollback();
      goto dbEnd;
    }

    if (!g_vectorDb->deleteChunksByNoteId(mdFilePath)) {
      g_vectorDb->rollback();
      goto dbEnd;
    }

    for (int i = 0; i < chunks.size(); ++i) {
      if (vectors[i].size() != embDim ||
          !g_vectorDb->executeInsert(mdFilePath, chunks[i].chunkIndex,
                                     chunks[i].content, currentHash,
                                     vectors[i])) {
        insertAllOk = false;
        break;
      }
    }

    // ✅ 批量写入后释放语句
    g_vectorDb->finalizeInsertStmts();

    if (!insertAllOk) {
      g_vectorDb->rollback();
      goto dbEnd;
    }

    if (g_vectorDb->commit()) {
      dbSuccess = true;
    } else {
      g_vectorDb->rollback();
    }
  dbEnd:;
  }

  qDebug() << "[BATCH] DB写入耗时:" << timer.elapsed() << "ms";
  return dbSuccess;
}

bool Notes::removeNoteVector(const QString& mdFilePath) {
  if (!isLocalAIModel) return false;
  if (!g_vectorDb) {
    qWarning() << "删除向量：向量数据库未初始化";
    return false;
  }

  // ✅ 关键：整个事务流程必须在 s_vecDbMutex 保护下执行
  // 共用 s_vecDbMutex 是保证数据一致性的必要条件
  QMutexLocker dbLock(&s_vecDbMutex);

  if (!g_vectorDb->beginTransaction()) {
    qWarning() << "❌ 删除向量开启事务失败 noteId:" << mdFilePath;
    return false;
  }

  if (g_vectorDb->deleteChunksByNoteId(mdFilePath)) {
    g_vectorDb->commit();
    qDebug() << "✅ 删除笔记向量成功 noteId:" << mdFilePath;
    return true;
  } else {
    g_vectorDb->rollback();
    qWarning() << "❌ 删除笔记向量失败 noteId:" << mdFilePath;
    return false;
  }
}

QString Notes::loadNoteFullText(const QString& mdPath) {
  QFile f(mdPath);
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return "";
  QString text = f.readAll();
  f.close();
  return text;
}
#endif
