#ifndef VECTORSEARCHSERVICE_H
#define VECTORSEARCHSERVICE_H

#include <QHash>
#include <QMutex>
#include <QObject>
#include <QReadLocker>
#include <QReadWriteLock>
#include <QRegularExpression>
#include <QString>
#include <QVector>
#include <QWriteLocker>

#include "src/AI/EmbeddingEngine.h"
#include "src/AI/VectorDb.h"

class BaseEmbeddingEngine;

// 统一搜索结果项（兼容分词与向量两种模式）
struct SearchResultItem {
  QString noteName;  // 笔记标题（用于UI列表显示）
  QString filePath;  // MD文件完整路径（用于点击跳转）
  QString
      snippet;  // 内容摘要/匹配片段（向量模式为chunk_text，分词模式为匹配行）
  float score;  // 相关度分数（向量模式为余弦相似度，分词模式可设为1.0或匹配数）
  int chunkIndex;       // 分块序号（向量模式有效，分词模式为-1）
  int lineNumber;       // 行号（分词模式有效，向量模式若未记录则为-1）
  bool isVectorResult;  // 标识来源，便于UI差异化渲染（如显示分数徽章）
};

class VectorSearchService : public QObject {
  Q_OBJECT
 public:
  explicit VectorSearchService(BaseEmbeddingEngine* engine,
                               QObject* parent = nullptr);

  ~VectorSearchService();

  /**
   * @brief 执行语义搜索
   * @param query 用户搜索文本
   * @param topK 返回结果数量
   * @param threshold 最低相似度阈值 (0.0~1.0)，低于此值的结果将被过滤
   * @return 按相关度降序排列的搜索结果列表
   */
  QVector<SearchResultItem> search(const QString& query, int topK = 20,
                                   float threshold = 0.3f);

  /**
   * @brief 注册笔记元数据映射（在笔记保存/索引时调用）
   * @param noteId 笔记唯一ID
   * @param filePath MD文件完整路径
   * @param noteName 笔记显示名称
   */
  void registerNoteMeta(const QString& noteId, const QString& filePath,
                        const QString& noteName);

  /**
   * @brief 注销笔记元数据（在笔记删除时调用）
   */
  void unregisterNoteMeta(const QString& noteId);

 signals:
  void searchStarted();
  void searchFinished(int resultCount);

 private:
  VectorDb m_vectorDb;
  BaseEmbeddingEngine* m_engine;
  QMutex m_mutex;

  // noteId -> {filePath, noteName} 的内存映射
  // 避免每次搜索都去遍历文件系统或查询主数据库
  struct NoteMeta {
    QString filePath;
    QString noteName;
  };
  QHash<QString, NoteMeta> m_noteMetaMap;
  QReadWriteLock m_metaLock;

  // 内部辅助方法
  QString highlightKeywords(const QString& text, const QString& query) const;
};

#endif  // VECTORSEARCHSERVICE_H