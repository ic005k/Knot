#ifndef TITLEGENERATOR_H
#define TITLEGENERATOR_H

#include <QString>

class TitleGenerator {
 public:
  TitleGenerator() = default;
  // 根据笔记内容生成建议标题；maxLength <= 0 表示不截断
  QString genNewTitle(const QString& text, int maxLength = 30);

 private:
  QString firstNonEmptyLine(const QString& text);
  QString firstSentence(const QString& text);
  static QString elideTitle(const QString& s, int maxLength);
};

#endif  // TITLEGENERATOR_H