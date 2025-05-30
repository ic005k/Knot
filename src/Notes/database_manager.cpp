#include "database_manager.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>

extern std::unique_ptr<cppjieba::Jieba> jieba;

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent),
      m_defaultDir(
          QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)) {
}

bool DatabaseManager::initDatabase(const QString &path) {
  m_db = QSqlDatabase::addDatabase("QSQLITE");
  m_db.setDatabaseName(path);

  if (!m_db.open()) return false;
  setupDatabaseSchema();
  return true;
}

void DatabaseManager::setupDatabaseSchema() {
  m_db.exec(
      "CREATE TABLE IF NOT EXISTS documents ("
      "id INTEGER PRIMARY KEY,"
      "path TEXT UNIQUE,"
      "modified INTEGER,"
      "content TEXT,"
      "terms TEXT)");

  m_db.exec(
      "CREATE VIRTUAL TABLE IF NOT EXISTS fts_documents USING fts5(path, "
      "content, terms)");
}

void DatabaseManager::updateFilesIndex(const QString &directory) {
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
  m_db.exec("DELETE FROM documents");
  m_db.exec("DELETE FROM fts_documents");

  foreach (const QString &file, files) {
    processFile(dir.filePath(file));
  }

  // 提交事务
  if (!m_db.commit()) {
    qDebug() << "Commit error:" << m_db.lastError().text();
    m_db.rollback();
  }

  qInfo() << "重建索引耗时:" << timer.elapsed() << "ms";
}

void DatabaseManager::processFile(const QString &filePath) {
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

QStringList DatabaseManager::tokenize(const QString &text) {
  std::vector<std::string> words;
  QStringList result;

  // 分离中英文处理
  QString chinesePart;
  QString westernPart;

  for (const QChar &c : text) {
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
  QRegularExpression re("[^\\w]");
  QStringList westernWords = westernPart.split(re, Qt::SkipEmptyParts);

  // 合并结果
  for (const auto &w : words) {
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
    const QString &query, NoteIndexManager *indexManager, int limit) {
  QStringList keywords = tokenize(query);

  QStringList conditions;
  for (const QString &kw : keywords) {
    QString escaped = kw;
    escaped.replace("'", "''");
    conditions << QString("(content MATCH '\"%1\"' OR terms MATCH '\"%1\"')")
                      .arg(escaped);
  }

  QString sql =
      QString("SELECT path, content FROM fts_documents WHERE %1 LIMIT %2")
          .arg(conditions.join(" AND "))
          .arg(limit);

  QVector<SearchResult> results;
  QSqlQuery q(m_db);
  if (q.exec(sql)) {
    while (q.next()) {
      QString path = q.value(0).toString();
      QString content = q.value(1).toString();

      // results.append({path, extractPreview(content, keywords)});

      SearchResult item;
      item.filePath = path;
      item.preview = extractPreview(content, keywords);

      // 获取标题逻辑
      QString title = indexManager->getNoteTitle(path);
      item.title = !title.isEmpty() ? title : QFileInfo(path).baseName();

      results.append(item);
    }
  }
  return results;
}

QString DatabaseManager::extractPreview(const QString &content,
                                        const QStringList &keywords) {
  const int CONTEXT_LEN = 80;
  QString simplified = content.simplified();

  int firstPos = -1;
  for (const QString &kw : keywords) {
    int pos = simplified.indexOf(kw, 0, Qt::CaseInsensitive);
    if (pos != -1 && (firstPos == -1 || pos < firstPos)) {
      firstPos = pos;
    }
  }

  if (firstPos == -1) return simplified.left(CONTEXT_LEN) + "...";

  int start = qMax(0, firstPos - CONTEXT_LEN / 2);
  int end = qMin(simplified.length(), start + CONTEXT_LEN);
  QString preview = simplified.mid(start, end - start);

  for (const QString &kw : keywords) {
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

void DatabaseManager::updateFileIndex(const QString &filePath) {
  deleteFileIndex(filePath);

  executeTransactionWithRetry(
      [this, filePath]() -> bool {
        processFile(filePath);
        return true;
      },
      1);
}

void DatabaseManager::deleteFileIndex(const QString &filePath) {
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
      3);  // 重试3次

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
      } catch (const std::exception &e) {
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

QStringList DatabaseManager::scanMarkdownFiles(const QString &directory) const {
  QDir dir(directory);
  return dir.entryList({"*.md"}, QDir::Files | QDir::NoDotAndDotDot);
}
