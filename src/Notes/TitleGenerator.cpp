#include "TitleGenerator.h"

#include <QCoreApplication>
#include <QRegularExpression>

namespace {

// 首行至少要有这么长才视为"主题行"；太短（如"记录""TODO"）就降级到首句。
constexpr int kMinFirstLineChars = 4;

QString defaultTitle() {
  // 每次调用时翻译，避免构造时 QTranslator 尚未安装导致拿到英文原文
  return QCoreApplication::translate("TitleGenerator", "Untitled Note");
}

}  // namespace

QString TitleGenerator::genNewTitle(const QString& text, int maxLength) {
  const QString t = text.trimmed();
  if (t.isEmpty()) return defaultTitle();

  // 1) 第一个非空行（语言无关；笔记首行天然是主题）
  const QString firstLine = firstNonEmptyLine(t);
  if (firstLine.size() >= kMinFirstLineChars)
    return elideTitle(firstLine, maxLength);

  // 2) 第一个句子（多语言句末标点）
  const QString s = firstSentence(t);
  if (!s.isEmpty()) return elideTitle(s, maxLength);

  // 3) 兜底：整段软截断
  return elideTitle(t, maxLength);
}

QString TitleGenerator::firstNonEmptyLine(const QString& text) {
  int pos = 0;
  while (pos < text.size()) {
    const int nl = text.indexOf(QLatin1Char('\n'), pos);
    const QString line =
        (nl < 0 ? text.mid(pos) : text.mid(pos, nl - pos)).trimmed();
    if (!line.isEmpty()) return line;
    if (nl < 0) break;
    pos = nl + 1;
  }
  return {};
}

QString TitleGenerator::firstSentence(const QString& text) {
  // 覆盖：中日文、英文、阿拉伯、天城文、缅甸、藏文、亚美尼亚、埃塞俄比亚
  // "." 后接空白或结尾才切，避免切坏 3.14 / v2.0
  static const QRegularExpression re(QStringLiteral(
      "[。！？!?…\\u06D4\\u0964\\u0965\\u104B\\u0F0D\\u0589\\u1362]"
      "|\\.(?=\\s|$)"));

  auto it = re.globalMatch(text);
  while (it.hasNext()) {
    const auto m = it.next();
    const QString s = text.left(m.capturedStart()).trimmed();
    if (!s.isEmpty()) return s;  // 跳过前置的空白段
  }
  return text.trimmed();
}

QString TitleGenerator::elideTitle(const QString& s, int maxLength) {
  if (maxLength <= 0 || s.size() <= maxLength) return s;
  int cut = maxLength;
  // 注意：必须用成员版 isHighSurrogate()，不能传 QChar 给静态版
  if (s.at(cut - 1).isHighSurrogate()) --cut;
  return s.left(cut) + QChar(0x2026);  // "…"
}