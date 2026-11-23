#include "Method.h"

#include <QKeyEvent>
#include <QStringConverter>
#include <QStringDecoder>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#if defined(Q_OS_MACOS) || (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID))
#include <cerrno>
#include <iconv.h>
#endif

// ============================================================
// 平台无关的"字符集解码"封装
// ============================================================
namespace {

#ifdef Q_OS_WIN
// Windows：用 Win32 API 显式按代码页解码，不受系统区域设置影响
QString decodeWin32(const QByteArray &data, UINT codepage) {
  if (data.isEmpty())
    return {};
  const int wlen =
      MultiByteToWideChar(codepage, MB_ERR_INVALID_CHARS, data.constData(),
                          data.size(), nullptr, 0);
  if (wlen <= 0) // 非法字节序列
    return {};
  QString result;
  result.resize(wlen);
  MultiByteToWideChar(codepage, MB_ERR_INVALID_CHARS, data.constData(),
                      data.size(), reinterpret_cast<wchar_t *>(result.data()),
                      wlen);
  if (result.contains(QChar::ReplacementCharacter))
    return {};
  return result;
}
#endif // Q_OS_WIN

#if defined(Q_OS_MACOS) || (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID))
// Linux / macOS：用 iconv 显式按字符集名解码
QString decodeWithIconv(const QByteArray &data, const char *fromCodec) {
  iconv_t cd = iconv_open("UTF-8", fromCodec);
  if (cd == (iconv_t)(-1))
    return {};

  QByteArray input = data;
  char *inBuf = input.data();
  size_t inBytesLeft = static_cast<size_t>(input.size());

  QByteArray output;
  output.resize(static_cast<int>(inBytesLeft * 3 + 16));
  char *outBuf = output.data();
  size_t outBytesLeft = static_cast<size_t>(output.size());

  while (inBytesLeft > 0) {
    size_t ret = iconv(cd, &inBuf, &inBytesLeft, &outBuf, &outBytesLeft);
    if (ret == (size_t)(-1)) {
      if (errno == E2BIG) {
        int used = static_cast<int>(output.size() - outBytesLeft);
        output.resize(output.size() * 2);
        outBuf = output.data() + used;
        outBytesLeft = static_cast<size_t>(output.size() - used);
        continue;
      }
      iconv_close(cd);
      return {};
    }
  }

  int used = static_cast<int>(output.size() - outBytesLeft);
  iconv_close(cd);

  QString result = QString::fromUtf8(output.constData(), used);
  if (result.contains(QChar::ReplacementCharacter))
    return {};
  return result;
}
#endif // Linux / macOS

// 统一入口：GBK / GB18030
QString decodeGb18030(const QByteArray &data) {
#if defined(Q_OS_WIN)
  return decodeWin32(data, 936); // CP_GBK
#elif defined(Q_OS_MACOS) || (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID))
  return decodeWithIconv(data, "GB18030");
#else
  return QString::fromLocal8Bit(data); // Android / iOS / 其它
#endif
}

// 统一入口：繁体 Big5
QString decodeBig5(const QByteArray &data) {
#if defined(Q_OS_WIN)
  return decodeWin32(data, 950); // CP_BIG5
#elif defined(Q_OS_MACOS) || (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID))
  return decodeWithIconv(data, "BIG5");
#else
  return {};
#endif
}

} // namespace
// ============================================================

QString Method::convertDataToUnicode(QByteArray data) {
  qInfo() << "Raw file bytes (first 40):" << data.left(40).toHex();
  qInfo() << "isUtf8 result:" << isUtf8(data);

  if (data.isEmpty())
    return {};

  // ---- BOM ----
  if (data.startsWith(QByteArrayLiteral("\xEF\xBB\xBF")))
    return QString::fromUtf8(data.mid(3));

  if (data.startsWith(QByteArrayLiteral("\xFF\xFE")) ||
      data.startsWith(QByteArrayLiteral("\xFE\xFF"))) {
    QStringDecoder decoder(QStringConverter::Utf16);
    return decoder.decode(data);
  }

  // ---- 合法 UTF-8 ----
  if (isUtf8(data)) {
    QStringDecoder dec(QStringConverter::Utf8);
    return dec.decode(data);
  }

  // ---- 非 UTF-8：GBK/GB18030 → Big5 ----
  if (QString gbk = decodeGb18030(data); !gbk.isEmpty() && isValidText(gbk)) {
    return gbk;
  }
  if (QString big5 = decodeBig5(data); !big5.isEmpty() && isValidText(big5)) {
    return big5;
  }

  // 兜底
  return QString::fromLatin1(data);
}

bool Method::isUtf8(const QByteArray &data) {
  int i = 0;
  int length = data.length();
  int utf8Chars = 0;
  int invalidBytes = 0;

  while (i < length) {
    unsigned char c = static_cast<unsigned char>(data[i]);
    int bytes;

    if ((c & 0x80) == 0) {
      bytes = 1;
    } else if ((c & 0xE0) == 0xC0) {
      bytes = 2;
    } else if ((c & 0xF0) == 0xE0) {
      bytes = 3;
    } else if ((c & 0xF8) == 0xF0) {
      bytes = 4;
    } else {
      invalidBytes++;
      bytes = 1;
    }

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

  // 纯 ASCII 也是合法 UTF-8（语义修正，不影响结果）
  if (utf8Chars == 0 && invalidBytes == 0)
    return true;
  if (utf8Chars == 0)
    return false;

  double validRatio = 1.0 - (double)invalidBytes / length;
  return validRatio > 0.9;
}

bool Method::isValidText(const QString &text) {
  if (text.isEmpty())
    return false;

  int validChars = 0;
  for (QChar c : text) {
    if (c == QChar::ReplacementCharacter)
      return false;
    if (c.isPrint() || c.isSpace())
      ++validChars;
  }
  return static_cast<double>(validChars) / text.length() > 0.7;
}