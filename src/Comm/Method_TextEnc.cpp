#include "Method.h"

#include <QKeyEvent>
#include <QStringConverter>
#include <QStringDecoder>

QString Method::convertDataToUnicode(QByteArray data) {

  qInfo() << "Raw file bytes (first 40):" << data.left(40).toHex();
  qInfo() << "isUtf8 result:" << isUtf8(data);

  QString text = "";

  // 首先检查BOM
  if (data.startsWith("\xEF\xBB\xBF")) {
    text = QString::fromUtf8(data.mid(3)); // 跳过BOM
  } else if (data.startsWith("\xFF\xFE") || data.startsWith("\xFE\xFF")) {
    // UTF-16 BOM：使用 QStringDecoder 自动处理大小端和 BOM
    QStringDecoder decoder(QStringConverter::Utf16);
    text = decoder.decode(data);
  } else {
    // 使用更健壮的编码检测
    if (isUtf8(data)) {
      text = QString::fromUtf8(data);
    } else {
      // 尝试常见编码

      // 尝试GBK/GB2312 (中文)
      // Qt6 原生不再提供 codecForName("GBK")。
      // 在中文 Windows 下，fromLocal8Bit 等价于 GBK 解码。
      QString gbkText = QString::fromLocal8Bit(data);
      if (isValidText(gbkText)) {
        text = gbkText;
      } else {
        // 尝试 ISO 8859-1 (Latin-1)，Qt6 原生自带 fromLatin1
        text = QString::fromLatin1(data);
      }
    }
  }
  return text;
}

// 改进的UTF-8检测函数
bool Method::isUtf8(const QByteArray &data) {
  int i = 0;
  int length = data.length();
  int utf8Chars = 0;
  int invalidBytes = 0;

  while (i < length) {
    unsigned char c = static_cast<unsigned char>(data[i]);
    int bytes;

    // 判断UTF-8字符的字节数
    if ((c & 0x80) == 0) {
      bytes = 1; // 0xxxxxxx
    } else if ((c & 0xE0) == 0xC0) {
      bytes = 2; // 110xxxxx
    } else if ((c & 0xF0) == 0xE0) {
      bytes = 3; // 1110xxxx
    } else if ((c & 0xF8) == 0xF0) {
      bytes = 4; // 11110xxx
    } else {
      invalidBytes++;
      bytes = 1;
    }

    // 检查后续字节是否符合UTF-8格式
    if (i + bytes > length) {
      invalidBytes++;
      break;
    }

    for (int j = 1; j < bytes; j++) {
      unsigned char follow = static_cast<unsigned char>(data[i + j]);
      if ((follow & 0xC0) != 0x80) {
        invalidBytes++;
        break;
      }
    }

    if (bytes > 1)
      utf8Chars++;
    i += bytes;
  }

  // 如果没有发现UTF-8多字节字符，或者无效字节太多，认为不是UTF-8
  if (utf8Chars == 0)
    return false;
  double validRatio = 1.0 - (double)invalidBytes / length;
  return validRatio > 0.9; // 至少90%的字节有效
}

// 辅助函数：检查文本是否包含足够的有效字符
bool Method::isValidText(const QString &text) {
  int validChars = 0;
  int totalChars = text.length();

  if (totalChars == 0)
    return false;

  for (QChar c : text) {
    if (c.isPrint() || c.isSpace()) {
      validChars++;
    }
  }

  // 要求至少70%的字符是可打印的或空格
  return (double)validChars / totalChars > 0.7;
}
