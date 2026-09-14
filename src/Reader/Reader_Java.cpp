
#include <QBuffer>
#include <QByteArray>
#include <QFile>
#include <QFont>
#include <QPageLayout>
#include <QPageSize>
#include <QPrinter>
#include <QString>
#include <QTemporaryFile>
#include <QTextDocument>
#include <QTextStream>
#include <cerrno>
#include <cstring>

#include "Reader.h"

#ifdef Q_OS_ANDROID
#include <QJniEnvironment>
#endif

void Reader::openReadListWindow(QStringList list) {
#ifdef Q_OS_ANDROID
  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  // 构造 Java ArrayList<String>
  QJniObject jArrayList("java/util/ArrayList", "()V");

  for (const QString& item : list) {
    QJniObject jItem = QJniObject::fromString(item);
    jArrayList.callMethod<bool>("add", "(Ljava/lang/Object;)Z", jItem.object());
  }

  // 调用Java方法 openReadListWindow(ArrayList<String>)
  activity.callMethod<void>("openReadListWindow", "(Ljava/util/ArrayList;)V",
                            jArrayList.object());
#endif
}

void Reader::setPdfDataToJava(QString txtFile) {
#ifdef Q_OS_ANDROID
  QByteArray pdfData = txtToPdf(txtFile);
  if (pdfData.isEmpty()) {
    return;
  }

  QJniObject instance = QJniObject::getStaticObjectField(
      "com/x/artifex/mupdf/mini/DocumentActivity", "mPdfActivity",
      "Lcom/x/artifex/mupdf/mini/DocumentActivity;");

  if (instance.isValid()) {
    QJniEnvironment env;
    // 创建byte数组
    jbyteArray jPdfArray = env->NewByteArray(pdfData.size());
    // 填充二进制数据
    env->SetByteArrayRegion(
        jPdfArray, 0, pdfData.size(),
        reinterpret_cast<const jbyte*>(pdfData.constData()));

    instance.callMethod<void>("setConvertedPdfBuffer", "([B)V", jPdfArray);
  }
#endif
}

static QString decodeGbkViaJni(const QByteArray& gbk) {
#ifdef Q_OS_ANDROID

  // 将 QByteArray 转为 Java byte[]
  QJniEnvironment env;
  jbyteArray jBytes = env->NewByteArray(gbk.size());
  env->SetByteArrayRegion(jBytes, 0, gbk.size(),
                          reinterpret_cast<const jbyte*>(gbk.constData()));

  QJniObject result = QJniObject::callStaticObjectMethod(
      "com/x/artifex/mupdf/mini/DocumentActivity", "decodeGbk",
      "([B)Ljava/lang/String;", jBytes);

  env->DeleteLocalRef(jBytes);

  if (result.isValid()) {
    return result.toString();
  }

  qWarning() << "JNI decodeGbk failed, fallback to Latin1";

#endif
  return QString::fromLatin1(gbk);
}

QByteArray Reader::txtToPdf(const QString& filePath) {
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly)) {
    qWarning() << "Failed to open txt file:" << filePath << file.errorString();
    return {};
  }
  QByteArray raw = file.readAll();
  file.close();

  QString txtContent;

  // 1. 检查 UTF-8 BOM
  if (raw.startsWith("\xEF\xBB\xBF")) {
    txtContent = QString::fromUtf8(raw.mid(3));
  } else {
    // 2. 尝试严格 UTF-8 解码
    auto utf8Dec = QStringDecoder(QStringDecoder::Utf8,
                                  QStringDecoder::Flag::ConvertInvalidToNull);
    QString test = utf8Dec(raw);
    bool isValidUtf8 =
        utf8Dec.isValid() && !test.contains('\0') && !utf8Dec.hasError();

    if (isValidUtf8) {
      txtContent = test;
    } else {
      // 3. ⭐ 非 UTF-8 → 使用 iconv 解码 GBK（Android NDK 原生支持）
      txtContent = decodeGbkViaJni(raw);
      qInfo() << "TXT decoded as GBK via iconv:" << filePath;
    }
  }

  // --- 文档构建 ---
  auto* doc = new QTextDocument();
  doc->setPlainText(txtContent);

  // ✅ 字号改为 15
  QFont font("Noto Sans CJK SC", 16);
  font.setStyleStrategy(QFont::PreferAntialias);
  doc->setDefaultFont(font);

  // --- PDF 输出 ---
  QTemporaryFile tmpPdf;
  tmpPdf.setAutoRemove(true);
  if (!tmpPdf.open()) {
    delete doc;
    return {};
  }
  QString tmpPath = tmpPdf.fileName();
  tmpPdf.close();

  auto* printer = new QPrinter(QPrinter::HighResolution);
  printer->setOutputFormat(QPrinter::PdfFormat);
  printer->setOutputFileName(tmpPath);
  printer->setPageSize(QPageSize(QPageSize::A4));
  printer->setPageOrientation(QPageLayout::Portrait);

  doc->print(printer);

  tmpPdf.open();
  QByteArray pdfData = tmpPdf.readAll();
  tmpPdf.close();

  delete printer;
  delete doc;
  return pdfData;
}