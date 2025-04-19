#pragma once
#include <QDebug>  // 确保包含头文件
#include <QFutureWatcher>
#include <QHash>
#include <QMetaType>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QPair>
#include <QSet>
#include <QtConcurrent>

// 定义位置信息结构体
struct KeywordPosition {
  int paragraphIndex;
  int charStart;
  int charEnd;
  QString context;

  // 声明序列化操作符为友元函数
  friend QDataStream &operator<<(QDataStream &out, const KeywordPosition &pos);
  friend QDataStream &operator>>(QDataStream &in, KeywordPosition &pos);
};

Q_DECLARE_METATYPE(KeywordPosition)
Q_DECLARE_METATYPE(QList<KeywordPosition>)

// 声明全局操作符重载（必须位于结构体定义之后）
QDataStream &operator<<(QDataStream &out, const KeywordPosition &pos);
QDataStream &operator>>(QDataStream &in, KeywordPosition &pos);

struct SearchResult {
  QString filePath;                     // 文件路径
  QString previewText;                  // 预览文本（带高亮标记）
  QList<KeywordPosition> highlightPos;  // 高亮位置偏移量
  // 原始搜索结果数据
  QList<KeywordPosition> rawPositions;

  int score = 0;

  QString fileTitle;
};

Q_DECLARE_METATYPE(SearchResult)

// KeywordPosition 的 QDebug 输出支持
inline QDebug operator<<(QDebug debug, const KeywordPosition &pos) {
  debug.nospace() << "{ Paragraph: " << pos.paragraphIndex
                  << ", Start: " << pos.charStart << ", End: " << pos.charEnd
                  << ", Context: \"" << pos.context.left(20)
                  << "...\" }";  // 显示前20字符避免过长
  return debug;
}

// SearchResult 的 QDebug 输出支持
inline QDebug operator<<(QDebug debug, const SearchResult &result) {
  debug.nospace() << "File: " << result.filePath << "\n"
                  << "Matches (" << result.highlightPos.size() << "):\n";
  for (const auto &pos : result.highlightPos) {
    debug << "  " << pos << "\n";
  }
  return debug;
}

class NotesSearchEngine : public QObject {
  Q_OBJECT

 public:
  explicit NotesSearchEngine(QObject *parent = nullptr);

  // 添加/更新文档到索引
  void indexDocument(const QString &path, const QString &content);

  // 异步批量构建索引
  // fullRebuild 参数（默认 false 表示增量索引）
  void buildIndexAsync(const QList<QString> &notePaths,
                       bool fullRebuild = false);

  // 执行搜索
  QList<SearchResult> search(const QString &query);

  // 添加文档数量查询方法
  int documentCount() const;

  void saveIndex(const QString &path);
  void loadIndex(const QString &path);

  bool hasDocument(const QString &path) const;

  // 获取文档原始内容
  QString getDocumentContent(const QString &path) const {
    return m_documents.value(path);
  }

 signals:
  void indexBuildProgress(int current, int total);
  void indexBuildFinished();

 private:
  // 预处理 Markdown
  QString preprocessMarkdown(const QString &content) const;

  // 中文分词
  QStringList tokenize(const QString &text) const;

  // 倒排索引结构：关键词 -> (文档路径 -> 位置列表)
  QHash<QString, QHash<QString, QList<KeywordPosition>>> m_invertedIndex;
  QHash<QString, QString> m_documents;

  // 文档内容按段落存储（路径 -> 段落列表）
  QHash<QString, QStringList> m_documentParagraphs;

  // 异步索引构建
  QFutureWatcher<void> m_indexWatcher;

  QMutex m_mutex;

  QPair<QString, QList<KeywordPosition>> generateHighlightPreview(
      const QString &content, const QList<KeywordPosition> &positions) const;
};
