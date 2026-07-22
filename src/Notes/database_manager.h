#pragma once
#include <QDebug>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QMutexLocker>
#include <QObject>
#include <QPair>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QThread>
#include <QVector>
#include <lib/cppjieba/Jieba.hpp>

// #include "Notes.h"

class NoteIndexManager;

struct SearchResult {
  QString filePath;
  QString title;
  QString preview;
};

struct ExactMatchResult {
  QString filePath;
  QString title;
  QString preview;            // 已清洗、截断的上下文摘要
  int lineNumber;             // 首次匹配行号（用于跳转）
  int matchCount;             // 匹配总数
  QList<int> allLineNumbers;  // 所有匹配行号（用于文件内上下条跳转）
};

class DatabaseManager : public QObject {
  Q_OBJECT
 public:
  explicit DatabaseManager(QObject* parent = nullptr);
  bool initDatabase(const QString& path = "search.db");
  void updateFilesIndex(const QString& directory);
  QVector<SearchResult> searchDocuments(const QString& query,
                                        NoteIndexManager* indexManager,
                                        int limit = 20);

  void updateFileIndex(const QString& filePath);  // 强制更新单个文件
  void deleteFileIndex(const QString& filePath);  // 强制移除索引
  void validateIndex();                           // 索引完整性检查

  void cleanMissingFileRecords(const QString& directory);
  void updateFileIndexes(const QStringList& filePaths);
  bool deleteDatabaseFile(const QString& dbPath);
  void closeDatabase();
  void batchDeleteFileIndexes(const QStringList& filePaths);
 signals:
  void errorOccurred(const QString& message);
  void indexingProgress(int processed, int total);

 private:
  QSqlDatabase m_db;
  // cppjieba::Jieba m_jieba;

  QStringList tokenize(const QString& text);
  void processFile(const QString& filePath);
  QString extractPreview(const QString& content, const QStringList& keywords);
  void setupDatabaseSchema();

  QString m_defaultDir;
  QStringList scanMarkdownFiles(const QString& directory) const;

  bool executeTransactionWithRetry(std::function<bool()> ops, int retries);
};
