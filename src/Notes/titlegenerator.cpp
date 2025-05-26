
#include "titlegenerator.h"

const QRegularExpression TitleGenerator::sentenceEnd("[。！？!?\n]");

static QString getDefaultTitle();

TitleGenerator::TitleGenerator() {
  // 改为使用QStringList的构造函数初始化
  m_stopWords = QStringList()
                // 中文停用词
                << "的" << "了" << "在" << "是" << "我" << "有" << "和"
                << "就" << "不" << "人" << "都" << "一" << "一个" << "上"
                << "也" << "很" << "到" << "说" << "要" << "去" << "你"
                << "会" << "着" << "没有" << "看" << "好" << "自己"
                << "这"
                // 英文停用词
                << "the" << "of" << "in" << "to" << "a" << "and" << "that"
                << "it" << "with" << "as" << "for" << "on" << "at" << "by"
                << "from" << "up" << "about" << "into" << "over" << "after"
                << "under" << "before" << "between" << "but" << "he" << "she"
                << "or" << "an" << "if" << "his" << "her" << "its" << "their";

  defaultTitle = getDefaultTitle();
}

QString TitleGenerator::genNewTitle(const QString& text, int maxLength) {
  if (text.isEmpty()) return defaultTitle;

  // 无论什么语言统一返回首句
  return generateByFirstSentence(text, maxLength);

  // 判断是否为中文文本（至少包含3个中文字符）
  bool isChineseText = false;
  int chineseCharCount = 0;
  for (QChar c : text) {
    if (isChineseCharacter(c)) {
      chineseCharCount++;
      if (chineseCharCount >= 3) {
        isChineseText = true;
        break;
      }
    }
  }

  // 中文文本使用关键词提取
  if (isChineseText) {
    QStringList words = tokenize(text);
    QString keywordTitle = generateByKeywords(words, maxLength);
    if (!keywordTitle.isEmpty() && keywordTitle != defaultTitle)
      return keywordTitle;
  }

  // 非中文文本或关键词提取失败时，直接返回第一句
  return generateByFirstSentence(text, maxLength);
}

QStringList TitleGenerator::tokenize(const QString& text) {
  QStringList tokens;
  QString currentWord;
  for (QChar c : text) {
    // 中文字符单独成词
    if (isChineseCharacter(c)) {
      if (!currentWord.isEmpty()) {
        tokens.append(currentWord);
        currentWord.clear();
      }
      tokens.append(c);
    }
    // 英文字符和数字按连续序列分词
    else if (c.isLetterOrNumber()) {
      currentWord.append(c);
    }
    // 其他字符（标点等）作为分词边界
    else {
      if (!currentWord.isEmpty()) {
        tokens.append(currentWord);
        currentWord.clear();
      }
    }
  }
  // 添加最后一个词
  if (!currentWord.isEmpty()) tokens.append(currentWord);
  return tokens;
}

QStringList TitleGenerator::filterStopWords(const QStringList& words) {
  QStringList filteredWords;
  for (const QString& word : words) {
    if (!m_stopWords.contains(word) && word.length() > 1)
      filteredWords.append(word);
  }
  return filteredWords;
}

QString TitleGenerator::generateByKeywords(const QStringList& words,
                                           int maxLength) {
  if (words.isEmpty()) return defaultTitle;
  // 统计词频
  QMap<QString, int> wordCounts;
  for (const QString& word : words) {
    wordCounts[word] = wordCounts.value(word, 0) + 1;
  }
  // 过滤停用词
  QStringList filteredWords = filterStopWords(words);
  if (filteredWords.isEmpty()) return defaultTitle;
  // 按词频排序
  QList<QPair<QString, int>> sortedWords;
  sortedWords.reserve(filteredWords.size());  // 避免多次内存重新分配
  for (auto it = filteredWords.begin(); it != filteredWords.end(); ++it) {
    sortedWords.append(qMakePair(*it, wordCounts[*it]));
  }
  std::sort(sortedWords.begin(), sortedWords.end(),
            [](const QPair<QString, int>& a, const QPair<QString, int>& b) {
              return a.second > b.second;
            });
  // 提取前 3 个关键词
  QStringList keywords;
  for (int i = 0; i < qMin(3, sortedWords.size()); ++i) {
    keywords.append(sortedWords[i].first);
  }
  // 组合关键词为标题
  QString title = keywords.join(" ");
  // 截断过长标题
  if (title.length() > maxLength) {
    title = title.left(maxLength) + "...";
  }
  return title.isEmpty() ? defaultTitle : title;
}

QString TitleGenerator::generateByFirstSentence(const QString& text,
                                                int maxLength) {
  // 直接使用静态正则表达式对象
  QStringList sentences = text.split(sentenceEnd, Qt::SkipEmptyParts);

  if (sentences.isEmpty()) return defaultTitle;

  // 获取第一个句子并去除前后空格
  QString firstSentence = sentences.first().trimmed();

  // 截断过长标题
  if (firstSentence.length() > maxLength) {
    firstSentence = firstSentence.left(maxLength) + "...";
  }

  return firstSentence.isEmpty() ? defaultTitle : firstSentence;
}

bool TitleGenerator::isChineseCharacter(QChar c) {
  // 基本汉字 Unicode 范围
  ushort code = c.unicode();
  return (code >= 0x4E00 && code <= 0x9FFF);
}

static QString getDefaultTitle() {
  return QCoreApplication::translate("TitleGenerator", "Untitled Note");
}
