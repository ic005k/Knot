#include "NotesSearchEngine.h"

#include <QDebug>
#include <QFile>
#include <QRegularExpression>
#include <memory>

#include "SearchResultModel.h"
#include "lib/cppjieba/Jieba.hpp"

extern std::unique_ptr<cppjieba::Jieba> jieba;

NotesSearchEngine::NotesSearchEngine(QObject *parent) : QObject(parent) {
  // 注册 KeywordPosition 以便与 Qt 信号槽兼容
  qRegisterMetaType<KeywordPosition>("KeywordPosition");

  // qRegisterMetaTypeStreamOperators<KeywordPosition>("KeywordPosition");
  qRegisterMetaType<KeywordPosition>();  // 关键注册

  connect(&m_indexWatcher, &QFutureWatcher<void>::progressValueChanged, this,
          [this](int progress) {
            // 计算当前进度和总任务数
            int total = m_indexWatcher.progressMaximum();
            emit indexBuildProgress(progress, total);
          });

  connect(&m_indexWatcher, &QFutureWatcher<void>::finished, this,
          &NotesSearchEngine::indexBuildFinished);
}

QString NotesSearchEngine::preprocessMarkdown(const QString &content) const {
  // 1. 移除代码块（支持嵌套和跨行）
  static QRegularExpression codeBlockRegex(R"((```[\s\S]*?```)|(`[^`]*?`))");

  // 2. 移除链接和图片
  static QRegularExpression linkRegex(R"(\[([^\]]+)\]\([^\)]+\))");
  static QRegularExpression imageRegex(R"(!\[.*?\]\([^\)]+\))");

  // 3. 移除格式符号（**、*、__、_）
  static QRegularExpression boldRegex(R"(\*\*(.*?)\*\*|__(.*?)__)");
  static QRegularExpression italicRegex(R"(\*(.*?)\*|_(.*?)_)");

  QString cleaned = content;
  cleaned
      .replace(codeBlockRegex, "")                    // 移除代码块和内联代码
      .replace(linkRegex, "\\1")                      // 保留链接文本
      .replace(imageRegex, "")                        // 完全移除图片
      .replace(boldRegex, "\\1\\2")                   // 保留加粗内容
      .replace(italicRegex, "\\1\\2")                 // 保留斜体内容
      .replace(QRegularExpression("#{1,6}\\s*"), "")  // 移除标题标记
      .simplified();

  return cleaned;
}

QStringList NotesSearchEngine::tokenize(const QString &text) const {
  QStringList tokens;
  std::string input = text.toStdString();
  std::vector<std::string> words;

  // 判断是否为中文（超过50%字符为CJK）
  int cjkCount = 0;
  for (QChar ch : text) {
    if (ch.script() == QChar::Script_Han ||
        ch.script() == QChar::Script_Katakana ||
        ch.script() == QChar::Script_Hiragana) {
      cjkCount++;
    }
  }
  bool isChinese = (cjkCount * 2 > text.length());

  if (isChinese) {
    // 中文分词
    jieba->Cut(input, words);
    for (const auto &word : words) {
      if (!word.empty()) {
        tokens.append(QString::fromStdString(word));
      }
    }
  } else {
    // 非中文：按单词切分
    tokens = text.toLower().split(QRegularExpression("[^\\p{L}0-9_]+"),
                                  Qt::SkipEmptyParts);
  }

  return tokens;
}

// 在索引文档时，将内容分割为段落，并记录每个关键词的位置
void NotesSearchEngine::indexDocument(const QString &path,
                                      const QString &content) {
  QMutexLocker locker(&m_mutex);
  QString cleaned = preprocessMarkdown(content);

  // 1. 彻底清理旧索引
  if (m_documents.contains(path)) {
    // 获取旧内容并分词
    QString oldContent = m_documents[path];
    QStringList oldKeywords = tokenize(oldContent);

    // 遍历所有旧关键词，完全移除路径关联
    for (const QString &keyword : oldKeywords) {
      if (m_invertedIndex.contains(keyword)) {
        QHash<QString, QList<KeywordPosition>> &docMap =
            m_invertedIndex[keyword];
        docMap.remove(path);  // 直接删除路径条目
        if (docMap.isEmpty()) {
          m_invertedIndex.remove(keyword);  // 清理空关键词
        }
      }
    }
    m_documents.remove(path);
    m_documentParagraphs.remove(path);
  }

  // 2. 预处理并插入新内容
  m_documents[path] = cleaned;
  QStringList paragraphs = cleaned.split("\n", Qt::SkipEmptyParts);
  m_documentParagraphs[path] = paragraphs;

  // 3. 构建新索引
  for (int paraIndex = 0; paraIndex < paragraphs.size(); ++paraIndex) {
    const QString paragraph = paragraphs[paraIndex];
    QStringList keywords = tokenize(paragraph);
    for (const QString &keyword : keywords) {
      // 查找关键词在段落中的所有位置
      int pos = 0;
      while ((pos = paragraph.indexOf(keyword, pos, Qt::CaseInsensitive)) !=
             -1) {
        KeywordPosition position;
        position.paragraphIndex = paraIndex;
        position.charStart = pos;
        position.charEnd = pos + keyword.length();
        position.context = paragraph.mid(qMax(0, pos - 20), 40);  // 上下文截取
        m_invertedIndex[keyword][path].append(position);
        pos += keyword.length();
      }
    }
  }
}

void NotesSearchEngine::buildIndexAsync(const QList<QString> &notePaths,
                                        bool fullRebuild) {
  // 仅在 fullRebuild 为 true 时清空索引
  if (fullRebuild) {
    QMutexLocker locker(&m_mutex);
    m_invertedIndex.clear();
    m_documents.clear();
    m_documentParagraphs.clear();
    qDebug() << "已清空旧索引（全量构建）";
  }

  QFuture<void> future = QtConcurrent::run([this, notePaths]() {
    for (const QString &path : notePaths) {
      QFile file(path);
      if (file.open(QIODevice::ReadOnly)) {
        QString content = QString::fromUtf8(file.readAll());
        indexDocument(path, content);  // 新增或更新文档
      }
    }
  });
  m_indexWatcher.setFuture(future);
}

QList<SearchResult> NotesSearchEngine::search(const QString &query) {
  QString cleanedQuery = preprocessMarkdown(query);
  QStringList queryKeywords = tokenize(cleanedQuery);

  QHash<QString, QList<KeywordPosition>> combinedResults;
  QHash<QString, int> docScores;

  // 收集匹配结果
  for (const QString &keyword : queryKeywords) {
    auto it = m_invertedIndex.find(keyword);
    if (it != m_invertedIndex.end()) {
      for (auto docIt = it->begin(); docIt != it->end(); ++docIt) {
        combinedResults[docIt.key()].append(docIt.value());
        docScores[docIt.key()] += docIt.value().size();
      }
    }
  }

  // 处理每个文档
  QList<SearchResult> results;
  for (auto it = combinedResults.begin(); it != combinedResults.end(); ++it) {
    SearchResult result;
    result.filePath = it.key();
    result.score = docScores[it.key()];

    // 位置处理流程
    QList<KeywordPosition> rawPositions = it.value();

    // 去重
    QSet<QPair<int, int>> uniquePositions;
    QList<KeywordPosition> filtered;
    for (const auto &pos : rawPositions) {
      QPair<int, int> key(pos.paragraphIndex, pos.charStart);
      if (!uniquePositions.contains(key)) {
        uniquePositions.insert(key);
        filtered.append(pos);
      }
    }

    // 合并邻近位置
    std::sort(filtered.begin(), filtered.end(),
              [](const KeywordPosition &a, const KeywordPosition &b) {
                return (a.paragraphIndex < b.paragraphIndex) ||
                       (a.paragraphIndex == b.paragraphIndex &&
                        a.charStart < b.charStart);
              });

    QList<KeywordPosition> merged;
    for (const auto &pos : filtered) {
      if (!merged.isEmpty() &&
          pos.paragraphIndex == merged.last().paragraphIndex &&
          pos.charStart <= merged.last().charEnd + 3) {
        merged.last().charEnd = qMax(merged.last().charEnd, pos.charEnd);
      } else {
        merged.append(pos);
      }
    }

    // 生成带调整位置的预览
    QPair<QString, QList<KeywordPosition>> previewData =
        generateHighlightPreview(m_documents[it.key()], merged);

    // 填充结果
    result.previewText = previewData.first;
    result.highlightPos = previewData.second;
    result.rawPositions = merged;
    results.append(result);
  }

  // 排序结果
  std::sort(results.begin(), results.end(),
            [](const SearchResult &a, const SearchResult &b) {
              return a.score > b.score;
            });

  return results;
}

int NotesSearchEngine::documentCount() const { return m_documents.size(); }

void NotesSearchEngine::saveIndex(const QString &path) {
  QFile file(path);
  if (file.open(QIODevice::WriteOnly)) {
    QDataStream stream(&file);
    stream << m_invertedIndex << m_documentParagraphs;
  }
}

void NotesSearchEngine::loadIndex(const QString &path) {
  QFile file(path);
  if (file.open(QIODevice::ReadOnly)) {
    QDataStream stream(&file);
    stream >> m_invertedIndex >> m_documentParagraphs;
  }
}

bool NotesSearchEngine::hasDocument(const QString &path) const {
  return m_documents.contains(path);
}

QDataStream &operator<<(QDataStream &out, const KeywordPosition &pos) {
  out << pos.paragraphIndex << pos.charStart << pos.charEnd << pos.context;
  return out;
}

QDataStream &operator>>(QDataStream &in, KeywordPosition &pos) {
  in >> pos.paragraphIndex >> pos.charStart >> pos.charEnd >> pos.context;
  return in;
}

QPair<QString, QList<KeywordPosition>>
NotesSearchEngine::generateHighlightPreview(
    const QString &content, const QList<KeywordPosition> &positions) const {
  QStringList paragraphs = content.split('\n');
  QStringList previewParts;
  QList<KeywordPosition> adjustedPositions;

  int globalOffset = 0;
  QMap<int, QList<KeywordPosition>> paraPositions;

  // 按段落分组
  for (const auto &pos : positions) {
    paraPositions[pos.paragraphIndex].append(pos);
  }

  // 处理每个包含高亮的段落
  foreach (int paraIndex, paraPositions.keys()) {
    if (paraIndex >= paragraphs.size()) continue;

    const QString &para = paragraphs[paraIndex];
    QList<KeywordPosition> poses = paraPositions[paraIndex];

    // 计算上下文范围
    int minStart = INT_MAX, maxEnd = 0;
    for (const auto &pos : poses) {
      minStart = qMin(minStart, pos.charStart);
      maxEnd = qMax(maxEnd, pos.charEnd);
    }

    // --- 新增优化逻辑：动态扩展上下文至句子边界 ---
    int contextStart = qMax(0, minStart - 30);
    int contextEnd = qMin(para.length(), maxEnd + 30);

    // 向左扩展至最近的句号或限制50字符
    while (contextStart > 0 &&
           para[contextStart] != QChar(0x3002) &&  // 中文句号 Unicode: U+3002
           para[contextStart] != '.' && (minStart - contextStart) < 50) {
      contextStart--;
    }

    // 向右扩展至最近的句号或限制50字符
    while (contextEnd < para.length() &&
           para[contextEnd] != QChar(0x3002) &&  // 中文句号 Unicode: U+3002
           para[contextEnd] != '.' && (contextEnd - maxEnd) < 50) {
      contextEnd++;
    }

    // 确保扩展后的范围有效
    contextStart = qMax(0, contextStart);
    contextEnd = qMin(para.length(), contextEnd);
    QString context = para.mid(contextStart, contextEnd - contextStart);
    // ------------------------------------------

    // 构建段落前缀
    QString prefix = QString("[Paragraph %1] ").arg(paraIndex + 1);
    if (contextStart > 0) context.prepend("...");
    if (contextEnd < para.length()) context.append("...");

    // 调整位置信息
    for (auto &pos : poses) {
      KeywordPosition adjusted = pos;
      adjusted.charStart =
          prefix.length() + (pos.charStart - contextStart) + globalOffset;
      adjusted.charEnd =
          prefix.length() + (pos.charEnd - contextStart) + globalOffset;
      adjustedPositions.append(adjusted);
    }

    // 构建带标签的预览文本
    QString fragment = prefix + context;
    std::sort(poses.begin(), poses.end(),
              [](auto &a, auto &b) { return a.charStart > b.charStart; });

    // for (const auto &pos : poses) {
    //   int localStart = prefix.length() + (pos.charStart - contextStart);
    //   fragment.insert(localStart + 23, "</span>");
    //   fragment.insert(localStart, "<span style='color:#e74c3c;'>");
    // }

    previewParts.append(fragment);
    globalOffset += fragment.length() + 4;  // 4为<br>标签长度
  }

  return qMakePair(previewParts.join("<br>"), adjustedPositions);
}
