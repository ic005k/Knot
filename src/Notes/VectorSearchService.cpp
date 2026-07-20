#include "VectorSearchService.h"

#include <QDebug>
#include <QElapsedTimer>
#include <algorithm>
#include <cmath>

VectorSearchService::VectorSearchService(EmbeddingEngine* engine,
                                         QObject* parent)
    : QObject(parent), m_engine(engine) {}

VectorSearchService::~VectorSearchService() {}

void VectorSearchService::registerNoteMeta(const QString& noteId,
                                           const QString& filePath,
                                           const QString& noteName) {
  QWriteLocker locker(&m_metaLock);
  m_noteMetaMap[noteId] = {filePath, noteName};
}

void VectorSearchService::unregisterNoteMeta(const QString& noteId) {
  QWriteLocker locker(&m_metaLock);
  m_noteMetaMap.remove(noteId);
}

QString VectorSearchService::highlightKeywords(const QString& text,
                                               const QString& query) const {
  if (text.isEmpty() || query.isEmpty()) return text;

  QString result = text;
  // 简单分词：按空格和标点拆分query
  QStringList keywords =
      query.split(QRegularExpression("[\\s\\p{P}]+"), Qt::SkipEmptyParts);

  for (const auto& kw : keywords) {
    if (kw.length() < 2) continue;  // 跳过单字符
    // 大小写不敏感替换，用 <mark> 包裹
    result.replace(
        QRegularExpression(kw, QRegularExpression::CaseInsensitiveOption),
        QString("<mark>%1</mark>").arg(kw));
  }
  return result;
}

QVector<SearchResultItem> VectorSearchService::search(const QString& query,
                                                      int topK,
                                                      float threshold) {
  emit searchStarted();
  QElapsedTimer timer;
  timer.start();
  QVector<SearchResultItem> results;

  if (!m_engine) {  // ✅ 用全局引擎检查有效性
    emit searchFinished(0);
    return results;
  }

  if (!g_vectorDb || !g_vectorDb->isOpen()) {
    qWarning() << "[VectorSearch] 全局向量库未就绪";
    emit searchFinished(0);
    return {};
  }

  // 1. Query 向量化
  QVector<float> queryVec = m_engine->encode(query);
  if (queryVec.isEmpty()) {
    emit searchFinished(0);
    return results;
  }

  // 2. ✅ 使用 . 而非 -> 调用数据库搜索
  auto hits = g_vectorDb->searchWithContent(queryVec, topK, threshold);

  // 3. 回填元数据 + 组装结果
  {
    QReadLocker locker(&m_metaLock);
    for (const auto& hit : hits) {
      SearchResultItem item;
      item.isVectorResult = true;
      item.score = hit.score;
      item.chunkIndex = hit.chunkIndex;
      item.lineNumber = -1;
      item.snippet =
          highlightKeywords(hit.content, query);  // ✅ 直接使用DB返回的原文

      auto it = m_noteMetaMap.find(hit.noteId);
      if (it != m_noteMetaMap.end()) {
        item.noteName = it->noteName;
        item.filePath = it->filePath;
      } else {
        item.noteName = "未知笔记";
        item.filePath = "";
      }
      results.append(item);
    }
  }

  qDebug() << "[VectorSearch] 完成, 结果数:" << results.size()
           << ", 耗时:" << timer.elapsed() << "ms";
  emit searchFinished(results.size());
  return results;
}

/////////////////////////////////////////////////////////////////////////

// 调试接口

int VectorSearchService::debugIndexSize() const {
  return (g_vectorDb && g_vectorDb->isOpen()) ? g_vectorDb->countChunks() : 0;
}

bool VectorSearchService::debugAddAndSearch(const QString& testId,
                                            const QVector<float>& vec,
                                            const QString& query) {
  Q_UNUSED(query);
  if (!g_vectorDb || !g_vectorDb->isOpen()) return false;

  bool inserted = g_vectorDb->insertChunk(testId, 0, "__DIAG__", vec);
  qDebug() << "[DIAG] insertChunk结果:" << inserted;
  if (!inserted) return false;

  qDebug() << "[DIAG] 写入后索引大小:" << debugIndexSize();

  auto hits = g_vectorDb->searchWithContent(vec, 5, 0.0f);
  qDebug() << "[DIAG] 自检索命中数:" << hits.size();

  bool foundSelf = false;
  for (const auto& h : hits) {
    qDebug() << "  -> noteId:" << h.noteId << "score:" << h.score
             << "content:" << h.content.left(30);
    if (h.noteId == testId && h.content == "__DIAG__") foundSelf = true;
  }
  return foundSelf;
}

void VectorSearchService::debugRemove(const QString& testId) {
  if (g_vectorDb && g_vectorDb->isOpen()) {
    g_vectorDb->deleteChunksByNoteId(testId);
    qDebug() << "[DIAG] 清理后索引大小:" << debugIndexSize();
  }
}