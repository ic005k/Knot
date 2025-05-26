#ifndef TITLEGENERATOR_H
#define TITLEGENERATOR_H
#include <QCoreApplication>
#include <QMap>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QTranslator>

class TitleGenerator {
 public:
  TitleGenerator();
  // 主接口函数：根据笔记内容生成标题
  QString genNewTitle(const QString& text, int maxLength = 30);

 private:
  // 分词函数
  QStringList tokenize(const QString& text);
  // 过滤停用词
  QStringList filterStopWords(const QStringList& words);
  // 基于关键词提取的标题生成
  QString generateByKeywords(const QStringList& words, int maxLength);
  // 基于首句提取的标题生成
  QString generateByFirstSentence(const QString& text, int maxLength);
  // 中文字符判断
  bool isChineseCharacter(QChar c);
  // 停用词表
  QStringList m_stopWords;

  static const QRegularExpression sentenceEnd;

  QString defaultTitle;
};

#endif  // TITLEGENERATOR_H
