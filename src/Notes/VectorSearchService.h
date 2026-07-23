#ifndef VECTORSEARCHSERVICE_H
#define VECTORSEARCHSERVICE_H

#include <QHash>
#include <QObject>
#include <QReadWriteLock>
#include <QRegularExpression>
#include <QString>
#include <QVector>

class EmbeddingEngine;
class VectorDb;

/// 统一搜索结果项（兼容分词与向量两种模式）
struct SearchResultItem {
  QString noteName;     // 笔记标题（用于UI列表显示）
  QString filePath;     // MD文件完整路径（用于点击跳转）
  QString snippet;      // Fallback 摘要（仅在回源失败时使用）
  float score = 0.0f;   // 相关度分数
  int chunkIndex = -1;  // 分块序号
  int lineNumber = -1;  // 行号（分词模式有效，向量模式为-1）
  bool isVectorResult = false;

  // ✅ 新增：向量搜索回源定位字段（与 VectorHit 对齐）
  qint64 charStart = -1;
  qint64 charEnd = -1;
  QString sectionPath;
  QByteArray contentHash;
};

class VectorSearchService : public QObject {
  Q_OBJECT
 public:
  explicit VectorSearchService(EmbeddingEngine* engine,
                               QObject* parent = nullptr);
  ~VectorSearchService() override;

  QVector<SearchResultItem> search(const QString& query, int topK = 20,
                                   float threshold = 0.3f);

  void registerNoteMeta(const QString& noteId, const QString& filePath,
                        const QString& noteName);
  void unregisterNoteMeta(const QString& noteId);

 signals:
  void searchStarted();
  void searchFinished(int resultCount);

 private:
  QString highlightKeywords(const QString& text, const QString& query) const;

  EmbeddingEngine* m_engine;

  struct NoteMeta {
    QString filePath;
    QString noteName;
  };
  QHash<QString, NoteMeta> m_noteMetaMap;
  QReadWriteLock m_metaLock;
};

#endif  // VECTORSEARCHSERVICE_H