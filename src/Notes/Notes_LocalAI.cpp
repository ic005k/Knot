#include <QCryptographicHash>
#include <QElapsedTimer>
#include <QFile>
#include <QMutexLocker>
#include <QTextCodec>

#include "src/AI/EmbeddingEngine.h"
#include "src/AI/GlobalAI.h"
#include "src/AI/VectorDb.h"
#include "src/Notes/MarkdownChunker.h"
#include "src/Notes/Notes.h"

#ifdef VECTOR_SEARCH

bool Notes::syncNoteVectorsBatchToDb(const QString& mdFilePath) {
  // ✅ 一次性读取原始字节 + 计算稳定指纹 + 安全解码
  QFile f(mdFilePath);
  if (!f.open(QIODevice::ReadOnly)) {
    qWarning() << "[BATCH] Failed to open note file, sync aborted:"
               << mdFilePath << "| error:" << f.errorString();
    return false;
  }
  QByteArray rawBytes = f.readAll();
  f.close();

  // 指纹基于磁盘原始字节，跨平台、跨版本永远稳定
  QString currentHash =
      QCryptographicHash::hash(rawBytes, QCryptographicHash::Sha256).toHex();

  // 从原始字节安全解码
  QString mdContent = readNoteFileSafeFromRaw(rawBytes);

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

  // ✅ 检查内容是否变更
  {
    QMutexLocker dbLock(&s_vecDbMutex);
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

  std::vector<QList<float>> rawVectors;
  int embDim = 0;

  {
    QMutexLocker embLock(&s_embMutex);
    rawVectors = embEngine->encodeBatch(texts, 64);
    embDim = embEngine->embeddingDimension();
  }

  qDebug() << "[BATCH] encode耗时:" << timer.elapsed()
           << "ms, chunks:" << chunks.size();

  if (static_cast<int>(rawVectors.size()) != chunks.size()) {
    qWarning() << "[BATCH] encode数量不匹配: expected" << chunks.size() << "got"
               << rawVectors.size();
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
      // ✅ 类型转换：QList<float> → QVector<float> 以匹配 VectorDb 接口
      const QVector<float> vec(rawVectors[i].begin(), rawVectors[i].end());

      if (vec.size() != embDim) {
        insertAllOk = false;
        break;
      }

      // ✅ 严格对齐 VectorDb.h 的 9 参数签名及顺序
      if (!g_vectorDb->executeInsert(
              mdFilePath,             // noteId
              chunks[i].chunkIndex,   // chunkIndex
              chunks[i].charStart,    // charStart (qint64)
              chunks[i].charEnd,      // charEnd (qint64)
              chunks[i].sectionPath,  // sectionPath
              chunks[i].content,      // content (fallback)
              chunks[i].contentHash,  // contentHash (原始32字节SHA-256)
              currentHash,            // noteHash
              vec)) {                 // vec (QVector<float>)
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

QString Notes::readNoteFileSafeFromRaw(const QByteArray& raw) {
  if (raw.isEmpty()) return {};

  if (raw.startsWith("\xEF\xBB\xBF")) {
    QString text = QString::fromUtf8(raw.mid(3));
    text.replace("\r\n", "\n").replace('\r', '\n');
    return text;
  }

  QTextCodec::ConverterState state;
  QTextCodec* utf8Codec = QTextCodec::codecForName("UTF-8");
  QString text = utf8Codec->toUnicode(raw.constData(), raw.size(), &state);

  bool hasGbkPattern = false;
  if (state.invalidChars > 0) {
    for (qsizetype i = 0; i < raw.size() - 1 && !hasGbkPattern; ++i) {
      quint8 hi = static_cast<quint8>(raw[i]);
      quint8 lo = static_cast<quint8>(raw[i + 1]);
      if (hi >= 0x81 && hi <= 0xFE && lo >= 0x40 && lo <= 0xFE)
        hasGbkPattern = true;
    }
  }

  if (state.invalidChars > 0 && hasGbkPattern) {
    qDebug() << "[ENCODING] Fallback to GBK, invalidChars:"
             << state.invalidChars;
    text = QTextCodec::codecForName("GBK")->toUnicode(raw);
  }

  text.replace("\r\n", "\n").replace('\r', '\n');
  return text;
}