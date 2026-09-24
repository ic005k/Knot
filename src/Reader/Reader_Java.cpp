
#include <QBuffer>
#include <QByteArray>
#include <QDate>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QIODevice>
#include <QPageLayout>
#include <QPageSize>
#include <QPdfWriter>
#include <QPrinter>
#include <QString>
#include <QStringDecoder>
#include <QTemporaryFile>
#include <QTextDocument>
#include <QTextStream>
#include <QUuid>
#include <QXmlStreamWriter>
#include <cerrno>
#include <cstring>

#include "Comm/Method.h"
#include "Reader.h"

#ifdef Q_OS_ANDROID
#include <QJniEnvironment>
#endif

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
  if (pdfData.isEmpty()) return;

  QJniObject instance = QJniObject::getStaticObjectField(
      "com/x/artifex/mupdf/mini/DocumentActivity", "mPdfActivity",
      "Lcom/x/artifex/mupdf/mini/DocumentActivity;");

  if (instance.isValid()) {
    QJniEnvironment env;

    // 传递 PDF 数据
    jbyteArray jPdfArray = env->NewByteArray(pdfData.size());
    env->SetByteArrayRegion(
        jPdfArray, 0, pdfData.size(),
        reinterpret_cast<const jbyte*>(pdfData.constData()));
    instance.callMethod<void>("setConvertedPdfBuffer", "([B)V", jPdfArray);
  }
#endif
}

QByteArray Reader::txtToPdf(const QString& filePath) {
  // ================= 1. 读取与解码文本 =================
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly)) {
    qWarning() << "Failed to open txt file:" << filePath << file.errorString();
    return {};
  }
  QByteArray raw = file.readAll();
  file.close();

  QString txtContent;

  // 检查 UTF-8 BOM
  if (raw.startsWith("\xEF\xBB\xBF")) {
    txtContent = QString::fromUtf8(raw.mid(3));
  } else {
    // 尝试严格 UTF-8 解码
    auto utf8Dec = QStringDecoder(QStringDecoder::Utf8,
                                  QStringDecoder::Flag::ConvertInvalidToNull);
    QString test = utf8Dec(raw);
    bool isValidUtf8 =
        utf8Dec.isValid() && !test.contains('\0') && !utf8Dec.hasError();

    if (isValidUtf8) {
      txtContent = test;
    } else {
      // 非 UTF-8 → 使用 JNI 解码 GBK
      txtContent = decodeGbkViaJni(raw);
      qInfo() << "TXT decoded as GBK via JNI:" << filePath;
    }
  }

  // ================= 2. 构建文档 =================
  auto* doc = new QTextDocument();
  doc->setPlainText(txtContent);

  QFont font("Noto Sans CJK SC", 16);
  font.setStyleStrategy(QFont::PreferAntialias);
  doc->setDefaultFont(font);

  // ================= 3. 内存 PDF 输出 =================
  QByteArray pdfData;
  QBuffer buffer(&pdfData);

  if (!buffer.open(QIODevice::WriteOnly)) {
    qWarning() << "Failed to open memory buffer for PDF generation";
    delete doc;
    return {};
  }

  // ✅ 使用 QPdfWriter 直接写入 QBuffer (内存)
  QPdfWriter writer(&buffer);
  writer.setPageSize(QPageSize(QPageSize::A4));
  writer.setPageOrientation(QPageLayout::Portrait);
  writer.setResolution(300);  // 对标原 QPrinter::HighResolution，保证清晰度
  writer.setTitle(QFileInfo(filePath).fileName());

  // 执行渲染（QTextDocument::print 接受 QPagedPaintDevice，QPdfWriter
  // 继承自它）
  doc->print(&writer);

  buffer.close();

  // ================= 4. 清理与返回 =================
  delete doc;

  if (pdfData.isEmpty()) {
    qWarning() << "Generated PDF data is empty for:" << filePath;
  }

  return pdfData;
}

void Reader::setFb2DataToJava(QString txtFile) {
#ifdef Q_OS_ANDROID
  QByteArray fb2Data = txtToFb2(txtFile);
  if (fb2Data.isEmpty()) return;

  QJniObject instance = QJniObject::getStaticObjectField(
      "com/x/artifex/mupdf/mini/DocumentActivity", "mPdfActivity",
      "Lcom/x/artifex/mupdf/mini/DocumentActivity;");

  if (instance.isValid()) {
    QJniEnvironment env;

    jbyteArray jArray = env->NewByteArray(fb2Data.size());
    env->SetByteArrayRegion(
        jArray, 0, fb2Data.size(),
        reinterpret_cast<const jbyte*>(fb2Data.constData()));

    instance.callMethod<void>("setConvertedFb2Buffer", "([B)V", jArray);
  }
#endif
}

QByteArray Reader::txtToFb2(const QString& filePath) {
  //  1. 读取与解码文本（完全保持原样）
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly)) {
    qWarning() << "Failed to open txt file:" << filePath << file.errorString();
    return {};
  }
  QByteArray raw = file.readAll();
  file.close();

  QString txtContent;

  if (raw.startsWith("\xEF\xBB\xBF")) {
    txtContent = QString::fromUtf8(raw.mid(3));
  } else {
    auto utf8Dec = QStringDecoder(QStringDecoder::Utf8,
                                  QStringDecoder::Flag::ConvertInvalidToNull);
    QString test = utf8Dec(raw);
    bool isValidUtf8 =
        utf8Dec.isValid() && !test.contains('\0') && !utf8Dec.hasError();

    if (isValidUtf8) {
      txtContent = test;
    } else {
      txtContent = decodeGbkViaJni(raw);
      qInfo() << "TXT decoded as GBK via JNI:" << filePath;
    }
  }

  txtContent.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
  txtContent.replace(QChar('\r'), QChar('\n'));

  // 过滤 XML 1.0 非法控制字符
  for (int i = 0; i < txtContent.size(); ++i) {
    ushort code = txtContent[i].unicode();
    if (code < 0x20 && code != 0x09 && code != 0x0A && code != 0x0D) {
      txtContent[i] = QChar(0xFFFD);
    }
  }

  // ================= 2. 纯 QXmlStreamWriter 生成 FB2 =================
  QByteArray fb2Data;
  QXmlStreamWriter xml(&fb2Data);
  xml.setAutoFormatting(true);
  xml.setAutoFormattingIndent(2);

  // ✅ 使用 writeStartDocument 统一处理 XML 声明 + 编码
  // 这会正确写入 <?xml version="1.0" encoding="UTF-8"?> 且不带多余 BOM
  xml.writeStartDocument(QStringLiteral("1.0"), true);

  // ✅ 手动写入 DOCTYPE（QXmlStreamWriter 不支持，但必须放在根元素之前）
  // 注意：writeStartDocument 之后、writeStartElement 之前插入原始文本
  // QXmlStreamWriter 允许在两个 write 调用之间直接操作底层设备
  // 但更安全的方式是把 DOCTYPE 作为 processing instruction 或直接拼接

  // 由于 QXmlStreamWriter 无法写 DOCTYPE，我们采用分段策略：
  // 先让 writer 写完所有内容到临时 buffer，再手动拼接 DOCTYPE
  QByteArray bodyData;
  QXmlStreamWriter bodyXml(&bodyData);
  bodyXml.setAutoFormatting(true);
  bodyXml.setAutoFormattingIndent(2);
  // ⚠️ 不再调用 writeStartDocument，避免重复 XML 声明

  const QString bookTitle = QFileInfo(filePath).completeBaseName();

  bodyXml.writeStartElement(QStringLiteral("FictionBook"));
  bodyXml.writeDefaultNamespace(
      QStringLiteral("http://www.gribuser.ru/xml/fictionbook/2.0"));
  bodyXml.writeNamespace(QStringLiteral("http://www.w3.org/1999/xlink"),
                         QStringLiteral("l"));

  // --- description ---
  bodyXml.writeStartElement(QStringLiteral("description"));

  bodyXml.writeStartElement(QStringLiteral("title-info"));
  bodyXml.writeTextElement(QStringLiteral("genre"), QStringLiteral("prose"));
  bodyXml.writeStartElement(QStringLiteral("author"));
  bodyXml.writeTextElement(QStringLiteral("first-name"),
                           QStringLiteral("Unknown"));
  bodyXml.writeTextElement(QStringLiteral("last-name"),
                           QStringLiteral("Author"));
  bodyXml.writeEndElement();
  bodyXml.writeTextElement(QStringLiteral("book-title"), bookTitle);
  bodyXml.writeTextElement(QStringLiteral("lang"), QStringLiteral("zh"));
  bodyXml.writeTextElement(QStringLiteral("src-lang"), QStringLiteral("zh"));
  bodyXml.writeStartElement(QStringLiteral("annotation"));
  bodyXml.writeTextElement(QStringLiteral("p"),
                           QStringLiteral("Converted from TXT by MuPDFMini"));
  bodyXml.writeEndElement();
  bodyXml.writeEndElement();  // title-info

  bodyXml.writeStartElement(QStringLiteral("document-info"));
  bodyXml.writeStartElement(QStringLiteral("author"));
  bodyXml.writeTextElement(QStringLiteral("nickname"),
                           QStringLiteral("MuPDFMini"));
  bodyXml.writeEndElement();
  bodyXml.writeTextElement(QStringLiteral("program-used"),
                           QStringLiteral("MuPDFMini"));
  bodyXml.writeTextElement(QStringLiteral("id"),
                           QUuid::createUuid().toString(QUuid::WithoutBraces));
  bodyXml.writeTextElement(QStringLiteral("version"), QStringLiteral("1.0"));
  bodyXml.writeTextElement(QStringLiteral("date"),
                           QDate::currentDate().toString(Qt::ISODate));
  bodyXml.writeEndElement();  // document-info

  bodyXml.writeStartElement(QStringLiteral("publish-info"));
  bodyXml.writeTextElement(QStringLiteral("book-name"), bookTitle);
  bodyXml.writeTextElement(QStringLiteral("publisher"),
                           QStringLiteral("Unknown"));
  bodyXml.writeTextElement(QStringLiteral("city"), QStringLiteral("Unknown"));
  bodyXml.writeTextElement(QStringLiteral("year"),
                           QString::number(QDate::currentDate().year()));
  bodyXml.writeEndElement();  // publish-info

  bodyXml.writeEndElement();  // description

  // --- body ---
  bodyXml.writeStartElement(QStringLiteral("body"));
  bodyXml.writeStartElement(QStringLiteral("section"));

  // ✅ section 必须有 title
  bodyXml.writeStartElement(QStringLiteral("title"));
  bodyXml.writeTextElement(QStringLiteral("p"), bookTitle);
  bodyXml.writeEndElement();  // title

  const QStringList lines = txtContent.split(QChar('\n'));
  for (const QString& line : lines) {
    const QString trimmed = line.trimmed();
    if (trimmed.isEmpty()) continue;
    bodyXml.writeTextElement(QStringLiteral("p"), trimmed);
  }

  bodyXml.writeEndElement();  // section
  bodyXml.writeEndElement();  // body
  bodyXml.writeEndElement();  // FictionBook
  bodyXml.writeEndDocument();

  // ✅ 最终拼接：XML声明 + DOCTYPE + body，确保字节流干净无双重BOM
  fb2Data =
      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      "<!DOCTYPE FictionBook SYSTEM \"FictionBook2.dtd\">\n" +
      bodyData;

  if (fb2Data.isEmpty()) {
    qWarning() << "Generated FB2 data is empty for:" << filePath;
  }

  return fb2Data;
}

void Reader::setEpubDataToJava(const QString& txtFile) {
#ifdef Q_OS_ANDROID
  QByteArray epubData = txtToEpub(txtFile);
  if (epubData.isEmpty()) return;

  QJniObject instance = QJniObject::getStaticObjectField(
      "com/x/artifex/mupdf/mini/DocumentActivity", "mPdfActivity",
      "Lcom/x/artifex/mupdf/mini/DocumentActivity;");

  if (instance.isValid()) {
    QJniEnvironment env;
    jbyteArray jArray = env->NewByteArray(epubData.size());
    env->SetByteArrayRegion(
        jArray, 0, epubData.size(),
        reinterpret_cast<const jbyte*>(epubData.constData()));
    instance.callMethod<void>("setConvertedEpubBuffer", "([B)V", jArray);
  }
#endif
}

QByteArray Reader::txtToEpub(const QString& filePath) {
  // ==================== 1. 读取与解码文本 ====================
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly)) {
    qWarning() << "Failed to open txt file:" << filePath << file.errorString();
    return {};
  }
  QByteArray raw = file.readAll();
  file.close();

  QString txtContent;
  if (raw.startsWith("\xEF\xBB\xBF")) {
    txtContent = QString::fromUtf8(raw.mid(3));
  } else {
    auto utf8Dec = QStringDecoder(QStringDecoder::Utf8,
                                  QStringDecoder::Flag::ConvertInvalidToNull);
    QString test = utf8Dec(raw);
    bool isValidUtf8 =
        utf8Dec.isValid() && !test.contains('\0') && !utf8Dec.hasError();
    if (isValidUtf8) {
      txtContent = test;
    } else {
      txtContent = decodeGbkViaJni(raw);
      qInfo() << "TXT decoded as GBK via JNI:" << filePath;
    }
  }

  // 标准化换行
  txtContent.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
  txtContent.replace(QChar('\r'), QChar('\n'));

  const QString bookTitle = QFileInfo(filePath).completeBaseName();
  const QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);

  // ==================== 2. 构建 EPUB 各组件 ====================

  QByteArray mimetype("application/epub+zip");

  QByteArray containerXml =
      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      "<container version=\"1.0\" "
      "xmlns=\"urn:oasis:names:tc:opendocument:xmlns:container\">\n"
      "  <rootfiles>\n"
      "    <rootfile full-path=\"OEBPS/content.opf\" "
      "media-type=\"application/oebps-package+xml\"/>\n"
      "  </rootfiles>\n"
      "</container>";

  QString contentOpf =
      QStringLiteral(
          "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
          "<package xmlns=\"http://www.idpf.org/2007/opf\" version=\"2.0\" "
          "unique-identifier=\"bookid\">\n"
          "  <metadata xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n"
          "    <dc:title>%1</dc:title>\n"
          "    <dc:creator>Unknown Author</dc:creator>\n"
          "    <dc:identifier id=\"bookid\">urn:uuid:%2</dc:identifier>\n"
          "    <dc:language>zh</dc:language>\n"
          "  </metadata>\n"
          "  <manifest>\n"
          "    <item id=\"chapter1\" href=\"chapter1.xhtml\" "
          "media-type=\"application/xhtml+xml\"/>\n"
          "  </manifest>\n"
          "  <spine>\n"
          "    <itemref idref=\"chapter1\"/>\n"
          "  </spine>\n"
          "</package>")
          .arg(bookTitle.toHtmlEscaped(), uuid);

  QStringList lines = txtContent.split(QChar('\n'));
  QString bodyContent;
  bodyContent.reserve(txtContent.size() * 2);
  for (const QString& line : lines) {
    QString trimmed = line.trimmed();
    if (trimmed.isEmpty()) continue;
    trimmed.replace(QLatin1Char('&'), QStringLiteral("&amp;"));
    trimmed.replace(QLatin1Char('<'), QStringLiteral("&lt;"));
    trimmed.replace(QLatin1Char('>'), QStringLiteral("&gt;"));
    bodyContent += QStringLiteral("<p>") + trimmed + QStringLiteral("</p>\n");
  }

  QString chapterXhtml =
      QStringLiteral(
          "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
          "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" "
          "\"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n"
          "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
          "<head><title>%1</title></head>\n"
          "<body>\n%2</body>\n"
          "</html>")
          .arg(bookTitle.toHtmlEscaped(), bodyContent);

  // ==================== 3. 使用 QuaZip 打包到内存 ====================
  QByteArray epubData;
  QBuffer buffer(&epubData);
  if (!buffer.open(QIODevice::WriteOnly)) {
    qWarning() << "Failed to open EPUB memory buffer";
    return {};
  }

  QuaZip zip(&buffer);
  if (!zip.open(QuaZip::mdCreate)) {
    qWarning() << "QuaZip create failed:" << zip.getZipError();
    buffer.close();
    return {};
  }
  zip.setFileNameCodec("UTF-8");

  // Helper lambda: 添加 STORED 条目（不压缩）
  // QuaZipFile::open 参数顺序:
  //   mode, newInfo, password, crc, method, level, raw, windowBits, memLevel,
  //   strategy
  // method=0 即 STORED
  auto addStoredEntry = [&](const QString& name,
                            const QByteArray& data) -> bool {
    QuaZipFile zf(&zip);
    QuaZipNewInfo ni(name);
    if (!zf.open(QIODevice::WriteOnly, ni, nullptr, 0, 0, 0, false, 15, 9,
                 Z_DEFAULT_STRATEGY)) {
      qWarning() << "Failed to add STORED entry:" << name
                 << "error:" << zf.getZipError();
      return false;
    }
    zf.write(data);
    zf.close();
    return true;
  };

  // Helper lambda: 添加 DEFLATED 条目（压缩）
  auto addDeflatedEntry = [&](const QString& name,
                              const QByteArray& data) -> bool {
    QuaZipFile zf(&zip);
    QuaZipNewInfo ni(name);
    if (!zf.open(QIODevice::WriteOnly, ni, nullptr, 0, Z_DEFLATED,
                 Z_DEFAULT_COMPRESSION, false, 15, 9, Z_DEFAULT_STRATEGY)) {
      qWarning() << "Failed to add DEFLATED entry:" << name
                 << "error:" << zf.getZipError();
      return false;
    }
    zf.write(data);
    zf.close();
    return true;
  };

  // ⚠️ mimetype 必须第一个写入，且必须 STORED
  if (!addStoredEntry("mimetype", mimetype)) {
    zip.close();
    buffer.close();
    return {};
  }

  addStoredEntry("META-INF/container.xml", containerXml);
  addDeflatedEntry("OEBPS/content.opf", contentOpf.toUtf8());
  addDeflatedEntry("OEBPS/chapter1.xhtml", chapterXhtml.toUtf8());

  zip.close();
  buffer.close();

  if (epubData.isEmpty()) {
    qWarning() << "Generated EPUB data is empty for:" << filePath;
  } else {
    qInfo() << "EPUB generated successfully, size=" << epubData.size();
  }

  return epubData;
}