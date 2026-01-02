#include "database_manager.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>
#include <QTextStream>

extern std::unique_ptr<cppjieba::Jieba> jieba;

DatabaseManager::DatabaseManager(QObject* parent)
    : QObject(parent),
      m_defaultDir(
          QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)) {
}

bool DatabaseManager::initDatabase(const QString& path) {
  m_db = QSqlDatabase::addDatabase("QSQLITE");
  m_db.setDatabaseName(path);

  if (!m_db.open()) return false;
  setupDatabaseSchema();
  return true;
}

void DatabaseManager::setupDatabaseSchema() {
  QSqlQuery query(m_db);
  bool success1 = query.exec(
      "CREATE TABLE IF NOT EXISTS documents ("
      "id INTEGER PRIMARY KEY,"
      "path TEXT UNIQUE,"
      "modified INTEGER,"
      "content TEXT,"
      "terms TEXT)");

  // 错误处理
  if (!success1) {
    qWarning() << "Failed to create table:" << query.lastError().text();
  }

  bool success2 = query.exec(
      "CREATE VIRTUAL TABLE IF NOT EXISTS fts_documents USING fts5("
      "path, content, terms)");

  // 错误处理
  if (!success2) {
    qWarning() << "Failed to create virtual table fts_documents:"
               << query.lastError().text();
  }
}

void DatabaseManager::updateFilesIndex(const QString& directory) {
  if (directory.isEmpty()) {
    emit errorOccurred("Invalid directory path");
    return;
  }

  QElapsedTimer timer;
  timer.start();

  QDir dir(directory);
  auto files = dir.entryList({"*.md"}, QDir::Files | QDir::NoDotAndDotDot);

  // 开始事务
  m_db.transaction();

  // 清空旧数据
  QSqlQuery query(m_db);
  query.exec("DELETE FROM documents");
  query.exec("DELETE FROM fts_documents");

  foreach (const QString& file, files) {
    processFile(dir.filePath(file));
  }

  // 提交事务
  if (!m_db.commit()) {
    qDebug() << "Commit error:" << m_db.lastError().text();
    m_db.rollback();
  }

  qInfo() << "重建索引耗时:" << timer.elapsed() << "ms";
}

void DatabaseManager::processFile(const QString& filePath) {
  QString m_filePath = QFileInfo(filePath).canonicalFilePath();

  QFile file(m_filePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

  QTextStream in(&file);
  QString content = in.readAll();
  file.close();

  QStringList terms = tokenize(content);
  qint64 modified = QFileInfo(file).lastModified().toSecsSinceEpoch();

  QSqlQuery q(m_db);
  q.prepare(
      "INSERT INTO documents (path, modified, content, terms) "
      "VALUES (?, ?, ?, ?)");
  q.addBindValue(m_filePath);
  q.addBindValue(modified);
  q.addBindValue(content);
  q.addBindValue(terms.join(' '));
  q.exec();

  q.prepare(
      "INSERT INTO fts_documents (path, content, terms) VALUES (?, ?, ?)");
  q.addBindValue(m_filePath);
  q.addBindValue(content);
  q.addBindValue(terms.join(' '));
  q.exec();
}

QStringList DatabaseManager::tokenize(const QString& text) {
  std::vector<std::string> words;
  QStringList result;

  // 分离中英文处理
  QString chinesePart;
  QString westernPart;

  for (const QChar& c : text) {
    if (c.unicode() >= 0x4E00 && c.unicode() <= 0x9FA5) {
      chinesePart.append(c);
    } else {
      westernPart.append(c);
    }
  }

  // 中文分词
  if (!chinesePart.isEmpty()) {
    std::string s = chinesePart.toStdString();
    jieba->CutForSearch(s, words);
  }

  // 西文分词
  static QRegularExpression re("[^\\w]");
  QStringList westernWords = westernPart.split(re, Qt::SkipEmptyParts);

  // 合并结果
  for (const auto& w : words) {
    result << QString::fromStdString(w);
  }
  result.append(westernWords);

  // Qt5
  // return result.toSet().toList();  // 去重

  // Qt6 去重新方案（保留原始顺序）
  QVector<QString> vec = result.toVector();
  std::sort(vec.begin(), vec.end());
  auto last = std::unique(vec.begin(), vec.end());
  vec.erase(last, vec.end());
  return vec.toList();
}

QVector<DatabaseManager::SearchResult> DatabaseManager::searchDocuments(
    const QString& query, NoteIndexManager* indexManager, int limit) {
  Q_UNUSED(limit);
  QStringList keywords = tokenize(query);

  QStringList conditions;
  for (const QString& kw : std::as_const(keywords)) {
    QString escaped = kw;
    escaped.replace("'", "''");
    conditions << QString("(content MATCH '\"%1\"' OR terms MATCH '\"%1\"')")
                      .arg(escaped);
  }

  // 函数内部判断：如果 limit 为 -1 则不添加 LIMIT 子句
  limit = -1;
  QString sql;
  if (limit <= 0) {
    sql = QString("SELECT path, content FROM fts_documents WHERE %1")
              .arg(conditions.join(" AND "));
  } else {
    sql = QString("SELECT path, content FROM fts_documents WHERE %1 LIMIT %2")
              .arg(conditions.join(" AND "))
              .arg(limit);
  }

  QVector<SearchResult> results;
  QSqlQuery q(m_db);
  if (q.exec(sql)) {
    while (q.next()) {
      QString path = q.value(0).toString();
      QString content = q.value(1).toString();

      SearchResult item;
      item.filePath = path;
      item.preview = extractPreview(content, keywords);

      // 获取标题逻辑
      QString title = indexManager->getNoteTitle(path);
      item.title = !title.isEmpty() ? title : QFileInfo(path).baseName();

      results.append(item);
    }
  }

  qDebug() << "resultsCount=" << results.count();
  return results;
}

QString DatabaseManager::extractPreview(const QString& content,
                                        const QStringList& keywords) {
  const int CONTEXT_LEN = 80;
  QString simplified = content.simplified();

  int firstPos = -1;
  for (const QString& kw : keywords) {
    int pos = simplified.indexOf(kw, 0, Qt::CaseInsensitive);
    if (pos != -1 && (firstPos == -1 || pos < firstPos)) {
      firstPos = pos;
    }
  }

  if (firstPos == -1) return simplified.left(CONTEXT_LEN) + "...";

  int start = qMax(0, firstPos - CONTEXT_LEN / 2);
  int end = qMin(simplified.length(), start + CONTEXT_LEN);
  QString preview = simplified.mid(start, end - start);

  for (const QString& kw : keywords) {
    QRegularExpression re(QString("(%1)").arg(QRegularExpression::escape(kw)),
                          QRegularExpression::CaseInsensitiveOption);

    // 以粗体显示
    // preview.replace(re, "<b>\\1</b>");

    preview.replace(re,
                    "<span style='background-color:#fff9c4; color:#c62828; "
                    "padding:2px; border-radius:2px;'>\\1</span>");
  }

  return (start > 0 ? "..." : "") + preview +
         (end < simplified.length() ? "..." : "");
}

void DatabaseManager::updateFileIndex(const QString& filePath) {
  QFileInfo fi(filePath);
  if (!fi.exists()) return;

  deleteFileIndex(filePath);

  executeTransactionWithRetry(
      [this, filePath]() -> bool {
        processFile(filePath);
        return true;
      },
      1);
}

// 添加批量处理方法，一次性处理多个文件
void DatabaseManager::updateFileIndexes(const QStringList& filePaths) {
  if (filePaths.isEmpty()) return;

  // 先收集所有存在的文件路径（过滤无效文件）
  QStringList validFilePaths;
  for (const QString& filePath : filePaths) {
    if (QFile::exists(filePath)) {
      validFilePaths << filePath;
    }
  }

  if (validFilePaths.isEmpty()) return;

  // 批量执行事务，处理所有有效文件
  executeTransactionWithRetry(
      [this, validFilePaths]() -> bool {
        // 在一个事务中处理所有文件，减少数据库操作开销
        for (const QString& filePath : validFilePaths) {
          // 先删除旧索引
          deleteFileIndex(filePath);
          // 再处理新索引
          processFile(filePath);
        }
        return true;
      },
      1);
}

void DatabaseManager::deleteFileIndex(const QString& filePath) {
  QFileInfo fi(filePath);
  if (!fi.exists()) return;

  QElapsedTimer timer;
  timer.start();

  // 获取规范路径
  const QString canonicalPath = QFileInfo(filePath).canonicalFilePath();
  if (canonicalPath.isEmpty()) {
    qWarning() << "无效路径:" << filePath;
    return;
  }

  // 使用新的事务模板
  bool success = executeTransactionWithRetry(
      [&]() {
        // 删除主表
        QSqlQuery delMain(m_db);
        delMain.prepare("DELETE FROM documents WHERE path = ?");
        delMain.addBindValue(canonicalPath);
        if (!delMain.exec()) {
          throw std::runtime_error("主表删除失败: " +
                                   delMain.lastError().text().toStdString());
        }

        // 删除FTS表（如果使用全文索引）
        QSqlQuery delFTS(m_db);
        delFTS.prepare("DELETE FROM fts_documents WHERE path = ?");
        delFTS.addBindValue(canonicalPath);
        if (!delFTS.exec()) {
          throw std::runtime_error("FTS表删除失败: " +
                                   delFTS.lastError().text().toStdString());
        }

        return true;  // 返回成功标志
      },
      1);  // 重试次数（之前是3），目前重试1次

  qInfo() << "删除操作耗时:" << timer.elapsed() << "ms"
          << "结果:" << (success ? "成功" : "失败");
}

// 增强版事务模板
bool DatabaseManager::executeTransactionWithRetry(std::function<bool()> ops,
                                                  int retries) {
  for (int i = 0; i < retries; ++i) {
    if (m_db.transaction()) {
      try {
        bool result = ops();
        if (result && m_db.commit()) {
          return true;
        }
        m_db.rollback();
      } catch (const std::exception& e) {
        m_db.rollback();
        qCritical() << "事务异常:" << e.what();
      }
    }
    QThread::msleep(50 * (i + 1));  // 指数退避重试
  }
  return false;
}

void DatabaseManager::validateIndex() {
  QSqlQuery q("SELECT COUNT(*) FROM documents");
  if (q.exec() && q.next()) {
    int dbCount = q.value(0).toInt();
    int actualCount = scanMarkdownFiles(m_defaultDir).size();
    if (dbCount != actualCount) {
      updateFilesIndex(m_defaultDir);
    }
  }
}

QStringList DatabaseManager::scanMarkdownFiles(const QString& directory) const {
  QDir dir(directory);
  return dir.entryList({"*.md"}, QDir::Files | QDir::NoDotAndDotDot);
}

// 核心功能：清理指定目录下，本地已删除但数据库中仍存在的MD文件记录
void DatabaseManager::cleanMissingFileRecords(const QString& directory) {
  if (directory.isEmpty()) {
    emit errorOccurred("Invalid directory path (cleanMissingFileRecords)");
    return;
  }

  QElapsedTimer timer;
  timer.start();
  QDir targetDir(directory);

  // ✅ 原有防御1：目录不存在直接返回
  if (!targetDir.exists()) {
    qInfo() << "Directory not exist, no clean needed (directory:" << directory
            << ")";
    return;
  }

  // 步骤1：获取本地实际存在的MD文件
  QSet<QString> localExistedFiles;
  const QStringList localMdFiles =
      targetDir.entryList({"*.md"}, QDir::Files | QDir::NoDotAndDotDot);
  for (const QString& fileName : localMdFiles) {
    const QString canonicalPath =
        QFileInfo(targetDir.filePath(fileName)).canonicalFilePath();
    if (!canonicalPath.isEmpty()) {
      localExistedFiles.insert(canonicalPath);
    }
  }

  // ✅ 新增【本次边缘场景专属防御】：本地文件全部被删除，日志标注，提前预警
  bool isLocalFilesAllDeleted = localExistedFiles.isEmpty();
  if (isLocalFilesAllDeleted) {
    qWarning() << "=== Edge Case: All local MD files are deleted! Will clean "
                  "all db records ===";
  }

  // 步骤2：获取数据库中所有存储的MD文件路径
  QStringList dbStoredFiles;
  QSqlQuery query("SELECT path FROM documents", m_db);
  if (!query.exec()) {
    qWarning() << "Failed to get db files (cleanMissingFileRecords):"
               << query.lastError().text();
    return;
  }
  while (query.next()) {
    dbStoredFiles.append(query.value(0).toString());
  }

  // ✅ 原有防御2：数据库为空直接返回
  if (dbStoredFiles.isEmpty()) {
    qInfo() << "Database is empty, no clean needed (directory:" << directory
            << ")";
    return;
  }

  // 步骤3：筛选出“数据库有但本地无”的无效路径
  QList<QString> invalidDbPaths;
  for (const QString& dbPath : dbStoredFiles) {
    if (!dbPath.isEmpty() && !localExistedFiles.contains(dbPath)) {
      invalidDbPaths.append(dbPath);
    }
  }

  // ✅ 原有防御3：无无效记录直接返回
  if (invalidDbPaths.isEmpty()) {
    qInfo() << "No missing file records to clean (directory:" << directory
            << ")";
    return;
  }

  // ===================== ✅ 核心修改：批量删除 替代 循环逐条删除
  // =====================
  // 不管是「少量无效记录」还是「全量无效记录（本地全删）」，全部批量删除
  const bool cleanSuccess = executeTransactionWithRetry(
      [&]() -> bool {
        // 拼接批量删除的IN条件：?占位符，防止SQL注入，安全高效
        QStringList placeholders;
        for (int i = 0; i < invalidDbPaths.size(); ++i) {
          placeholders.append("?");
        }
        QString inCondition = placeholders.join(",");

        // 1. 批量删除主表 documents 所有无效记录 【1次IO 替代 N次IO】
        QSqlQuery delMain(m_db);
        delMain.prepare(QString("DELETE FROM documents WHERE path IN (%1)")
                            .arg(inCondition));
        for (const QString& path : invalidDbPaths) {
          delMain.addBindValue(path);
        }
        if (!delMain.exec()) {
          throw std::runtime_error(QString("Batch delete documents failed: %1")
                                       .arg(delMain.lastError().text())
                                       .toStdString());
        }

        // 2. 批量删除FTS索引表 fts_documents 所有无效记录 【1次IO 替代 N次IO】
        QSqlQuery delFts(m_db);
        delFts.prepare(QString("DELETE FROM fts_documents WHERE path IN (%1)")
                           .arg(inCondition));
        for (const QString& path : invalidDbPaths) {
          delFts.addBindValue(path);
        }
        if (!delFts.exec()) {
          throw std::runtime_error(
              QString("Batch delete fts_documents failed: %1")
                  .arg(delFts.lastError().text())
                  .toStdString());
        }

        return true;
      },
      3  // 保留3次重试，足够应对临时锁问题
  );

  // 输出结果日志（保留原有日志）
  if (cleanSuccess) {
    qInfo() << "Clean missing file records success! Count:"
            << invalidDbPaths.size()
            << "LocalAllDeleted:" << isLocalFilesAllDeleted
            << "Time:" << timer.elapsed() << "ms (directory:" << directory
            << ")";
  } else {
    qWarning() << "Clean missing file records failed (directory:" << directory
               << ")";
  }
}

// 实现关闭数据库连接的函数
void DatabaseManager::closeDatabase() {
  // 检查数据库是否处于打开状态
  if (m_db.isOpen()) {
    // 关闭数据库连接
    m_db.close();
  }

  // 移除默认连接（关键步骤，确保释放资源）
  if (QSqlDatabase::contains(QSqlDatabase::defaultConnection)) {
    QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
  }
}

// 实现删除数据库文件的函数
bool DatabaseManager::deleteDatabaseFile(const QString& dbPath) {
  // 步骤1：先关闭所有数据库连接
  closeDatabase();

  // 步骤2：检查是否还有未关闭的连接（防御性检查）
  if (QSqlDatabase::contains(QSqlDatabase::defaultConnection)) {
    qWarning() << "数据库连接仍未释放，无法删除文件";
    return false;
  }

  // 步骤3：删除主数据库文件
  QFile dbFile(dbPath);
  if (!dbFile.exists()) {
    qWarning() << "数据库文件不存在:" << dbPath;
    return false;
  }

  bool mainFileDeleted = dbFile.remove();
  if (!mainFileDeleted) {
    qWarning() << "删除主数据库文件失败:" << dbFile.errorString();
    return false;
  }

  // 步骤4：删除SQLite可能生成的临时文件
  QString shmFile = dbPath + "-shm";
  QString walFile = dbPath + "-wal";

  if (QFile::exists(shmFile) && !QFile::remove(shmFile)) {
    qWarning() << "删除SHM文件失败:" << shmFile;
  }

  if (QFile::exists(walFile) && !QFile::remove(walFile)) {
    qWarning() << "删除WAL文件失败:" << walFile;
  }

  // 最终验证
  if (!QFile::exists(dbPath)) {
    qInfo() << "数据库文件删除成功:" << dbPath;
    return true;
  } else {
    qWarning() << "数据库文件删除后仍存在，可能被其他进程占用";
    return false;
  }
}
