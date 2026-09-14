#include <QBuffer>
#include <QPrinter>
#include <QTemporaryFile>

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

QByteArray Reader::txtToPdf(const QString& filePath) {
  QFile file(filePath);
  if (!file.open(
          QIODevice::ReadOnly)) {  // ⚠️ 去掉 Text 标志，以二进制读取原始字节
    qWarning() << "Failed to open txt file:" << filePath << file.errorString();
    return {};
  }
  QByteArray raw = file.readAll();
  file.close();

  // --- 编码处理：优先 UTF-8，回退 GBK ---
  QString txtContent;

  // 1. 先尝试 UTF-8（严格模式）
  auto utf8Decoder = QStringDecoder(
      QStringDecoder::Utf8, QStringConverter::Flag::ConvertInvalidToNull);
  if (utf8Decoder.isValid()) {
    txtContent = utf8Decoder(raw);
    // 如果解码结果包含 null 字符，说明不是合法 UTF-8
    if (!txtContent.contains('\0')) {
      goto encoding_done;
    }
  }

  // 2. UTF-8 失败，尝试 GBK
  {
    auto gbkDecoder = QStringDecoder("GBK");
    if (gbkDecoder.isValid()) {
      txtContent = gbkDecoder(raw);
      qInfo() << "TXT fallback to GBK:" << filePath;
    } else {
      // 3. GBK 也不可用，使用系统默认（最后兜底）
      txtContent = QString::fromLocal8Bit(raw);
      qWarning() << "GBK decoder unavailable, using local8Bit for:" << filePath;
    }
  }

encoding_done:
  // --- 文档构建 ---
  auto* doc = new QTextDocument();
  doc->setPlainText(txtContent);

  // ✅ 字号改为 15
  QFont font("Noto Sans CJK SC", 15);
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
