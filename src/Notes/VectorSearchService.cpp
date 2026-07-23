#include "VectorSearchService.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QFileInfo>

#include "src/AI/EmbeddingEngine.h"
#include "src/AI/GlobalAI.h"
#include "src/AI/VectorDb.h"

VectorSearchService::VectorSearchService(EmbeddingEngine* engine,
                                         QObject* parent)
    : QObject(parent), m_engine(engine) {}

VectorSearchService::~VectorSearchService() = default;

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
  QStringList keywords =
      query.split(QRegularExpression("[\\s\\p{P}]+"), Qt::SkipEmptyParts);

  for (const auto& kw : keywords) {
    if (kw.length() < 2) continue;
    result.replace(
        QRegularExpression(QRegularExpression::escape(kw),
                           QRegularExpression::CaseInsensitiveOption),
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

  if (!m_engine || !g_vectorDb || !g_vectorDb->isOpen()) {
    qWarning() << "[VectorSearch] 引擎或向量库未就绪";
    emit searchFinished(0);
    return results;
  }

  // 1. Query 向量化
  QVector<float> queryVec = m_engine->encode(query);
  if (queryVec.isEmpty()) {
    emit searchFinished(0);
    return results;
  }

  // 2. 向量检索（返回带完整回源字段的 VectorHit）
  auto hits = g_vectorDb->searchWithContent(queryVec, topK, threshold);

  // 3. 回填元数据 + 映射回源字段
  {
    QReadLocker locker(&m_metaLock);
    results.reserve(hits.size());

    for (const auto& hit : hits) {
      SearchResultItem item;
      item.isVectorResult = true;
      item.score = hit.score;
      item.chunkIndex = hit.chunkIndex;
      item.lineNumber = -1;

      // ✅ 核心：完整映射回源定位字段
      item.charStart = hit.charStart;
      item.charEnd = hit.charEnd;
      item.sectionPath = hit.sectionPath;
      item.contentHash = hit.contentHash;

      // snippet 仅作为 Fallback 保留，高亮处理不影响原始坐标
      item.snippet = highlightKeywords(hit.content, query);

      // 回填笔记元数据
      auto it = m_noteMetaMap.find(hit.noteId);
      if (it != m_noteMetaMap.end()) {
        item.noteName = it->noteName;
        item.filePath = it->filePath;
      } else {
        // Fallback：noteId 本身即为文件路径
        item.noteName = QFileInfo(hit.noteId).baseName();
        item.filePath = hit.noteId;
      }

      results.append(item);
    }
  }

  qDebug() << "[VectorSearch] 完成, 结果数:" << results.size()
           << ", 耗时:" << timer.elapsed() << "ms";
  emit searchFinished(results.size());
  return results;
}