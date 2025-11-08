#include "Reader.h"

#include <QKeyEvent>

#include "src/MainWindow.h"
#include "src/defines.h"
#include "ui_MainWindow.h"

QByteArray bookFileData;

static int press_x;
static int press_y;
static int relea_x;
static int relea_y;

EpubReader* reader = nullptr;

Reader::Reader(QWidget* parent) : QDialog(parent) {
  qmlRegisterType<TextChunkModel>("EBook.Models", 1, 0, "TextChunkModel");

  this->installEventFilter(this);
  notesModel = new QStandardItemModel(this);
  // 定义角色名（QML 里用的名字）
  notesModel->setItemRoleNames({{Qt::UserRole + 1, "quote"},
                                {Qt::UserRole + 2, "page"},
                                {Qt::UserRole + 3, "content"},
                                {Qt::UserRole + 4, "pageIndex"}});

  if (!isAndroid) mui->btnShareBook->hide();

  mui->btnAutoStop->hide();

  mui->lblTitle->hide();
  mui->f_ReaderNote->hide();
  mui->progPage->hide();

  mui->lblCataInfo->hide();
  mui->btnBackReader->hide();
  mui->lblCataInfo->adjustSize();
  mui->lblCataInfo->setWordWrap(true);
  mui->lblBookName->adjustSize();
  mui->lblBookName->setWordWrap(true);

  mui->qwViewBookNote->hide();

  QFont f = this->font();
  f.setPointSize(10);
  f.setBold(true);
  mui->btnPages->setFont(f);

  tmeAutoRun = new QTimer(mw_one);
  connect(tmeAutoRun, SIGNAL(timeout()), this, SLOT(autoRun()));

  strEndFlag = "<p align=center>-----" + tr("bottom") + "-----</p>";
  customCss = loadText(":/res/reader/main.css");
}

Reader::~Reader() {
  if (reader != nullptr) {
    delete reader;
    reader = nullptr;
  }
}

bool Reader::eventFilter(QObject* obj, QEvent* evn) {
  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
    }
  }

  return QWidget::eventFilter(obj, evn);
}

void Reader::keyReleaseEvent(QKeyEvent* event) { Q_UNUSED(event); }

void Reader::on_btnOpen_clicked() {
  if (isAndroid) {
    m_Method->openFilePicker();
    return;
  }

  openfile =
      QFileDialog::getOpenFileName(this, tr("Knot"), "", tr("Txt Files (*.*)"));

  QFileInfo fi(openfile);
  if (!fi.exists()) return;

#ifdef Q_OS_ANDROID
  openfile = m_Method->getRealPathFile(openfile);
#endif

  startOpenFile(openfile);
}

void Reader::setReaderStyle() {
  QColor textColor, baseColor;

  if (readerStyle == "1") {
    mui->qwReader->rootContext()->setContextProperty("backImgFile",
                                                     "/res/b.png");
    mui->qwReader->rootContext()->setContextProperty("myTextColor", "#664E30");

    mui->btnStyle3->setStyleSheet(
        "color: #00C78C;background-color: rgb(0, 0, 0);border: 2px solid "
        "rgb(0,0,255);border-radius: 4px;");

    mui->btnStyle1->setStyleSheet(
        "color: rgb(102, 78, 48);background-color: rgb(240, 222, 198);border: "
        "2px solid "
        "rgb(255,0,0);border-radius: 4px;");
    mui->btnStyle2->setStyleSheet(strStyle2_0);

    textColor = QColor(102, 78, 48);
    baseColor = QColor(240, 222, 198);
  }

  if (readerStyle == "2") {
    mui->qwReader->rootContext()->setContextProperty("backImgFile", "");
    mui->qwReader->rootContext()->setContextProperty(
        "myBackgroundColor", mui->editBackgroundColor->text());
    mui->qwReader->rootContext()->setContextProperty(
        "myTextColor", mui->editForegroundColor->text());

    mui->btnStyle3->setStyleSheet(
        "color: #00C78C;background-color: rgb(0, 0, 0);border: 2px solid "
        "rgb(0,0,255);border-radius: 4px;");

    mui->btnStyle1->setStyleSheet(
        "color: rgb(102, 78, 48);background-color: rgb(240, 222, 198);border: "
        "2px solid "
        "rgb(0,0,255);border-radius: 4px;");
    mui->btnStyle2->setStyleSheet(strStyle2_1);

    textColor = QColor(0, 0, 0);
    baseColor = QColor(255, 255, 255);
    mui->f_CustomColor->show();
  }

  if (readerStyle == "3") {
    mui->qwReader->rootContext()->setContextProperty("backImgFile",
                                                     "/res/b3.png");
    mui->qwReader->rootContext()->setContextProperty("myTextColor", "#2E8B57");

    mui->btnStyle3->setStyleSheet(
        "color: #00C78C;background-color: rgb(0, 0, 0);border: 2px solid "
        "rgb(255,0,0);border-radius: 4px;");

    mui->btnStyle1->setStyleSheet(
        "color: rgb(102, 78, 48);background-color: rgb(240, 222, 198);border: "
        "2px solid "
        "rgb(0,0,255);border-radius: 4px;");
    mui->btnStyle2->setStyleSheet(strStyle2_0);

    textColor = QColor(46, 139, 87);
    baseColor = QColor(0, 0, 0);
  }
}

void Reader::startOpenFile(QString openfile) {
  if (isReport) return;

  if (isAndroid) {
    closeMyPDF();
  }

  isEpubError = false;
  strShowMsg = "";
  strPercent = "";

  setReaderStyle();

  if (QFile(openfile).exists()) {
    isEBook = true;
    strTitle = "";
    catalogueFile = "";

    mui->btnReader->setEnabled(false);
    mui->f_ReaderFun->setEnabled(false);
    mui->lblTitle->hide();
    mui->qwCata->hide();
    mui->lblCataInfo->hide();

    QString bookName;

    QFileInfo fi(openfile);
    bookName = fi.fileName();

    ebookFile = openfile;
    strTitle =
        bookName + "    " + m_Method->getFileSize(QFile(ebookFile).size(), 2);

    mw_one->m_ReadTWThread->quit();
    mw_one->m_ReadTWThread->wait();

    if (!mw_one->initMain) {
      if (!fi.suffix().contains("pdf")) {
        QTimer::singleShot(100, this, []() { mw_one->showProgress(); });
      }
    }

    mw_one->myReadEBookThread->start();

  } else
    return;
}

void Reader::initInfoShowFont() {
  QFont font = this->font();
  if (!isAndroid) {
    font.setPointSize(7);
  } else {
    font.setPointSize(13);
  }

  mui->lblInfo->setFont(font);
}

void Reader::openFile(QString openfile) {
  qDebug() << "Starting to open files...";

  isOpen = false;
  oldOpfPath = strOpfPath;

  if (QFile(openfile).exists()) {
    QFileInfo fi(openfile);
    QString strSuffix0 = fi.suffix();
    QString strSuffix = strSuffix0.toLower();

    if (strSuffix != "epub" && strSuffix != "pdf" && strSuffix != "txt" &&
        strSuffix != "css" && strSuffix != "ini" && strSuffix != "md" &&
        strSuffix != "log" && strSuffix != "csv" && strSuffix != "tsv" &&
        strSuffix != "html" && strSuffix != "htm" && strSuffix != "xml" &&
        strSuffix != "json" && strSuffix != "yaml" && strSuffix != "yml" &&
        strSuffix != "rtf" && strSuffix != "c" && strSuffix != "cpp" &&
        strSuffix != "java" && strSuffix != "py" && strSuffix != "js" &&
        strSuffix != "php" && strSuffix != "go" && strSuffix != "rb" &&
        strSuffix != "hpp")
      return;

    if (strSuffix == "epub") {
      if (reader != nullptr) {
        delete reader;
        reader = nullptr;
      }
      reader = new EpubReader();
      if (!reader->open(openfile)) {
        return;
      }

      QString containerFile = "META-INF/container.xml";
      if (!reader->fileExists(containerFile)) {
        isEpub = false;
        isEpubError = true;
        qDebug() << "====== isEpub == false ======";
        return;
      }

      QString strFullPath;
      QByteArray containerXml = reader->readFile(containerFile);
      QStringList conList = readText(containerXml);
      for (int i = 0; i < conList.count(); i++) {
        QString str = conList.at(i);
        if (str.contains("full-path")) {
          QStringList list1 = str.split(" ");
          if (list1.count() > 0) {
            for (int j = 0; j < list1.count(); j++) {
              QString str1 = list1.at(j);
              if (str1.contains("full-path")) {
                QStringList list2 = str1.split("\"");
                if (list2.count() > 0) {
                  strFullPath = list2.at(1);
                }
              }
            }
          }
        }
      }

      qDebug() << "strFullPath=" << strFullPath;
      // "OEBPS/content.opf"
      QByteArray opfContent = reader->readFile(strFullPath);

      strOpfFile = strFullPath;
      QFileInfo fi(strOpfFile);
      strOpfPath = fi.path() + "/";
      if (strOpfPath == "./") strOpfPath = "";

      qDebug() << "containerXml=" << containerXml
               << "strOpfPath=" << strOpfPath;

      QStringList opfList = readText(opfContent);

      htmlFiles.clear();

      int opfCount = opfList.count();
      if (opfCount > 1) {
        for (int i = 0; i < opfCount; i++) {
          QString str0 = opfList.at(i);
          str0 = str0.trimmed();

          if (str0.contains("idref=") && str0.mid(0, 8) == "<itemref") {
            QString idref = get_idref(str0);

            QString qfile;
            qfile = strOpfPath + get_href(idref, opfList);
            htmlFiles.append(qfile);
          }
        }
      }

      if (htmlFiles.count() == 0) {
        isEpub = false;
        isEpubError = true;
        strOpfPath = oldOpfPath;
        qDebug() << "====== htmlFiles Count== 0 ======";
        return;
      } else {
        isEpub = true;
        isText = false;
        isPDF = false;

        ncx2html();

        QString str_cate = loadText(catalogueFile);
        StringToFile(str_cate, catalogueFile);
      }

    } else if (strSuffix == "pdf") {
      bookFileData = bookFileData.toBase64();
      isPDF = true;
      isText = false;
      isEpub = false;

    } else {  // txt file
      readTextList.clear();
      readTextList = readText(openfile);
      isText = true;
      isEpub = false;
      isPDF = false;
      sPos = 0;

      totallines = readTextList.count();
      totalPages = (totallines + baseLines - 1) / baseLines;
      currentPage = 0;
    }

    fileName = openfile;

    isOpen = true;

  }  // end file exists
}

QString Reader::get_idref(QString str0) {
  QString idref;
  str0 = str0.trimmed();
  if (str0.contains("idref=") && str0.mid(0, 8) == "<itemref") {
    QString str1 = str0;

    str1 = str1.replace("<", "");
    str1 = str1.replace("/>", "");
    QStringList list = str1.split(" ");
    for (int i = 0; i < list.count(); i++) {
      QString str2 = list.at(i);
      if (str2.contains("idref=")) {
        str2 = str2.replace("idref=", "");
        str2 = str2.replace("\"", "");
        str2 = str2.trimmed();
        idref = str2;
        break;
      }
    }
  }

  return idref;
}

QString Reader::get_href(QString idref, QStringList opfList) {
  for (int i = 0; i < opfList.count(); i++) {
    QString str0 = opfList.at(i);
    str0 = str0.trimmed();
    if (str0.contains("href") && str0.contains(idref) &&
        str0.mid(0, 5) == "<item" && str0.contains("htm")) {
      QString str1;

      /* Method 1 */
      for (int j = 0; j < str0.length(); j++) {
        if (str0.mid(j, 6) == "href=\"") {
          for (int m = j + 6; m < str0.length(); m++) {
            if (str0.mid(m, 1) == "\"") {
              str1 = str0.mid(j + 6, m - j - 6);
              // qDebug() <<"id="<<idref<< "href=" << str1;
              return str1;
              break;
            }
          }
        }
      }

      break;
    }
  }

  return "";
}

void Reader::saveReader(QString BookmarkText, bool isSetBookmark) {
  m_ReaderSet->saveScrollValue();

  QString endFile = iniDir + "bookini/" + currentBookName + ".ini";
  QSettings Reg(endFile, QSettings::IniFormat);

  QString bookmarkSn;
  if (isSetBookmark) {
    int countBookmark = Reg.value("/Bookmark/count", 0).toInt();
    countBookmark = countBookmark + 1;
    Reg.setValue("/Bookmark/count", countBookmark);
    bookmarkSn = QString::number(countBookmark - 1);
  }

  if (isText) {
    if (isSetBookmark) {
      Reg.setValue("/Bookmark/currentPage" + bookmarkSn, currentPage);
      Reg.setValue("/Bookmark/Name" + bookmarkSn, BookmarkText);
      Reg.setValue("/Bookmark/VPos" + bookmarkSn, getVPos());
      Reg.setValue("/Bookmark/isLandscape" + bookmarkSn, isLandscape);

    } else {
      Reg.setValue("/Reader/currentPage", currentPage);
    }
  }

  if (isEpub) {
    if (isSetBookmark) {
      Reg.setValue("/Bookmark/htmlIndex" + bookmarkSn, htmlIndex);
      Reg.setValue("/Bookmark/Name" + bookmarkSn, BookmarkText);
      Reg.setValue("/Bookmark/VPos" + bookmarkSn, getVPos());
      Reg.setValue("/Bookmark/isLandscape" + bookmarkSn, isLandscape);
    } else {
      Reg.setValue("/Reader/htmlIndex", htmlIndex);
      // dir
      Reg.setValue("/Reader/MainDirIndex", mainDirIndex);
    }
  }
  Reg.sync();

  if (!isSetBookmark) {
    // book list

    QString endFile = privateDir + "reader.ini";
    QSettings Reg1(endFile, QSettings::IniFormat);
    Reg1.setValue("/Reader/FileName", fileName);
    Reg1.setValue("/Reader/FontSize", readerFontSize);
    Reg1.setValue("/Reader/BookCount", bookList.count());
    for (int i = 0; i < bookList.count(); i++) {
      Reg1.setValue("/Reader/BookSn" + QString::number(i), bookList.at(i));
    }

    Reg1.sync();
  }
}

void Reader::initReader() {
  QSettings Reg(privateDir + "reader.ini", QSettings::IniFormat);

  readerStyle = Reg.value("/Reader/Style", "1").toString();
  scrollValue = Reg.value("/Reader/ScrollValue", "1").toReal();
  QString value = QString::number(scrollValue, 'f', 2);
  mui->lblSpeed->setText(tr("Scroll Speed") + " : " + value);
  m_ReaderSet->setScrollValue();

  QFont font;
  int fsize = Reg.value("/Reader/FontSize", 18).toInt();
  readerFontSize = fsize;
  mui->qwReader->rootContext()->setContextProperty("FontSize", fsize);
  mui->qwReader->rootContext()->setContextProperty("uiFontSize", fontSize);
  font.setPointSize(fsize);
  font.setLetterSpacing(QFont::AbsoluteSpacing, 2);  // 字间距

  fileName = Reg.value("/Reader/FileName").toString();
  if (!QFile(fileName).exists() && isZH_CN) fileName = ":/res/reader/test.txt";

  isInitReader = true;

  if (isAndroid) {
    if (m_Method->getKeyType() == "defaultopen" &&
        m_Method->getExecDone() == "false")
      isInitReader = false;

    QFileInfo fi(fileName);
    if (fi.suffix().toLower() == "pdf") {
      isPDF = true;
    }
  }

  getBookList();
}

void Reader::getBookList() {
  QSettings Reg(privateDir + "reader.ini", QSettings::IniFormat);

  // book list
  int count = Reg.value("/Reader/BookCount", 0).toInt();
  bookList.clear();
  for (int i = 0; i < count; i++) {
    QString str = Reg.value("/Reader/BookSn" + QString::number(i)).toString();
    QStringList list = str.split("|");
    if (QFile(list.at(1)).exists()) bookList.append(str);
  }
}

void Reader::setQMLText(QString txt1) {
  mui->qwReader->rootContext()->setContextProperty("isAni", QVariant(false));

  QStringList list = txt1.split("\n");
  QString str1 = "<html>\n<body>\n";
  QString str2 = "</body>\n</html>";
  QString qsShow;

  for (int i = 0; i < list.count(); i++) {
    qsShow = qsShow +
             "<p style='line-height:35px; width:100% ; text-indent:40px; '>" +
             list.at(i) + "</p>";
  }

  qsShow = str1 + qsShow + strEndFlag + str2;
  currentTxt = qsShow;

  QQuickItem* root = mui->qwReader->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "loadHtmlBuffer",
                            Q_ARG(QVariant, currentTxt));

  setAni();
}

void Reader::loadQMLText(QString str) {
  if (isText || isEpub) {
    QQuickItem* root = mui->qwReader->rootObject();
    QMetaObject::invokeMethod((QObject*)root, "loadText", Q_ARG(QVariant, str));
  }
}

QString Reader::getQMLText() {
  QVariant str;
  QQuickItem* root = mui->qwReader->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "getText",
                            Q_RETURN_ARG(QVariant, str));

  return str.toString();
}

void Reader::goUpPage() {
  if (isSelText) return;
  mui->lblTitle->hide();

  isPageNext = false;

  savePageVPos();

  if (isText) {
    QString txt1;
    if (currentPage > 0) {
      currentPage--;
      txt1 = updateContent();
    }

    setQMLText(txt1);
  }

  if (isEpub) {
    htmlIndex--;
    if (htmlIndex < 0) htmlIndex = 0;

    currentHtmlFile = htmlFiles.at(htmlIndex);
    setQMLHtml(currentHtmlFile, "", "");
  }

  setPageVPos();
  showInfo();
}

void Reader::goNextPage() {
  if (isSelText) return;
  mui->lblTitle->hide();

  isPageNext = true;

  if (currentPage > 0 || htmlFiles.count() > 1) savePageVPos();

  if (isText) {
    QString txt1;
    if (currentPage < totalPages - 1) {
      currentPage++;
      txt1 = updateContent();
    }

    setQMLText(txt1);
  }

  if (isEpub) {
    htmlIndex++;
    if (htmlIndex == htmlFiles.count()) htmlIndex = htmlFiles.count() - 1;

    currentHtmlFile = htmlFiles.at(htmlIndex);
    setQMLHtml(currentHtmlFile, "", "");
  }
  setPageVPos();
  showInfo();
}

QString Reader::updateContent() {
  int start = currentPage * baseLines;
  int end = qMin(start + baseLines, totallines);
  QString content;
  for (int i = start; i < end; ++i) {
    content += readTextList.at(i) + "\n";
  }
  return content;
}

void Reader::gotoCataList(QString htmlFile) {
  for (int i = 0; i < ncxList.count(); i++) {
    QString item = ncxList.at(i);
    QString str1 = item.split("===").at(1);
    // qDebug() << "gotoCataList:" << str1 << htmlFile ;
    if (str1.contains(htmlFile)) {
      currentCataIndex = i;
      break;
    }
  }
}

void Reader::openCataList(QString htmlFile) {
  savePageVPos();
  mui->lblCataInfo->hide();
  mui->qwCata->hide();
  mui->qwReader->show();
  mui->btnShowBookmark->setEnabled(true);

  initLink(htmlFile);
  m_Method->clearAllBakList(mui->qwCata);
}

void Reader::initLink(QString htmlFile) {
  for (int i = 0; i < htmlFiles.count(); i++) {
    QString str = htmlFiles.at(i);
    QString str1 = htmlFile;
    QStringList list = htmlFile.split("#");
    if (list.count() == 2) {
      str1 = list.at(0);
      QStringList list2 = str1.split("/");
      if (list2.count() > 0) {
        str1 = list2.at(list2.count() - 1);
      }
    }

    if (str.contains(str1)) {
      setEpubPagePosition(i, str);
      break;
    }
  }
}

void Reader::setEpubPagePosition(int index, QString htmlFile) {
  savePageVPos();
  htmlIndex = index;
  currentHtmlFile = htmlFiles.at(index);

  if (htmlFile.contains("#")) {
    QStringList list = htmlFile.split("#");
    QString html = list.at(0);
    QString skipid = list.at(1);
    QString strfile;
    int count0 = 0;
    for (int i = 0; i < htmlFiles.count(); i++) {
      if (index + i < htmlFiles.count()) {
        strfile = htmlFiles.at(index + i);
        QString buffers = loadText(strfile);
        if (buffers.contains(skipid)) {
          html = strfile;
          count0 = i;
          qDebug() << "setEpubPagePosition: html=" << html
                   << "count0=" << count0;
          break;
        }
      }
    }

    htmlIndex = index + count0;

    for (int i = 0; i < ncxList.count(); i++) {
      QString str = ncxList.at(i);
      QStringList l0 = str.split("===");
      QString strfile = l0.at(1);
      if (strfile.trimmed() == htmlFile) {
        strFind = l0.at(0);
        strFind = strFind.trimmed();
        break;
      }
    }

    setQMLHtml(html, "", skipid);

  } else {
    setQMLHtml(currentHtmlFile, "", "");
    setPageVPos();
  }

  showInfo();
}

QString Reader::processHtml(QString htmlFile, bool isWriteFile) {
  if (!isEpub || !reader->isOpen()) return "";

  // 获取屏幕宽度（用于判断是否需要缩放）
  int screenWidth = mw_one->width() - 60;
  qDebug() << "屏幕宽度:" << screenWidth;

  // 1. 读取原始HTML
  QByteArray ba = reader->readFile(htmlFile);
  QString strHtml = QString::fromUtf8(ba);

  // 根目录处理
  QString rootPath = strOpfPath;

  // 2. 清理无效空格
  strHtml.replace("　", " ");

  // 3. 引入自定义CSS（仅保留必要样式，图片缩放已在内存中完成）
  QString customCss = loadText(":/res/reader/main.css");
  // 移除原有图片样式（已无需依赖）

  static const QRegularExpression imgCssReg("img\\s*\\{[^}]*\\}");
  customCss = customCss.replace(imgCssReg, "");

  // 添加图片样式：确保缩放后图片左缩进为0
  customCss +=
      "img { margin-left: 0 !important; max-width: 100%; height: auto; }";
  // 仅保留其他样式（段落、标题等）
  strHtml.replace("</head>",
                  QString("<style>%1</style></head>").arg(customCss));

  // 4. 处理SVG中的image标签，替换为标准img标签
  static const QRegularExpression svgImageReg(
      "<image[^>]+>", QRegularExpression::DotMatchesEverythingOption);
  int svgPos = 0;
  QRegularExpressionMatch svgMatch;

  // 先处理所有SVG中的image标签，替换为标准img标签
  while ((svgPos = strHtml.indexOf(svgImageReg, svgPos, &svgMatch)) != -1) {
    QString imageTag = svgMatch.captured(0);
    QString imgTag = imageTag;

    // 将SVG的image标签替换为标准img标签
    imgTag.replace("<image", "<img");
    // 处理xlink:href属性为src
    imgTag.replace("xlink:href=", "src=");
    // 移除SVG特定属性
    static const QRegularExpression widthRegex("width=\"[^\"]+\"");
    static const QRegularExpression heightRegex("height=\"[^\"]+\"");
    static const QRegularExpression xRegex("x=\"[^\"]+\"");
    static const QRegularExpression yRegex("y=\"[^\"]+\"");
    imgTag.remove(widthRegex);
    imgTag.remove(heightRegex);
    imgTag.remove(xRegex);
    imgTag.remove(yRegex);

    // 替换原标签
    strHtml.replace(svgPos, svgMatch.capturedLength(), imgTag);
    svgPos += imgTag.length();
  }

  // 5. 处理所有img标签（包括刚从SVG转换过来的）
  static const QRegularExpression imgReg(
      "<img[^>]+>", QRegularExpression::DotMatchesEverythingOption);
  int pos = 0;
  QRegularExpressionMatch match;

  while ((pos = strHtml.indexOf(imgReg, pos, &match)) != -1) {
    QString imgTag = match.captured(0);
    QString modifiedImg = imgTag;

    // 提取图片路径
    static const QRegularExpression srcReg("src=[\"']([^\"']+)[\"']");
    QRegularExpressionMatch srcMatch = srcReg.match(modifiedImg);
    if (srcMatch.hasMatch()) {
      QString imgPathRel = srcMatch.captured(1);
      QString cleanedPath = imgPathRel.replace("../", "");
      QString imgPathAbs = rootPath + cleanedPath;
      imgPathAbs = QDir::cleanPath(imgPathAbs);

      // 读取图片原始数据
      QByteArray imgData = reader->readFile(imgPathAbs);
      if (imgData.isEmpty()) {
        qWarning() << "图片数据为空:" << imgPathAbs;
        pos += match.capturedLength();
        continue;
      }

      // 6. 关键：内存中加载并缩放图片
      QImage img;
      bool isImgValid = img.loadFromData(imgData);
      if (!isImgValid) {
        qWarning() << "图片格式无效:" << imgPathAbs;
        pos += match.capturedLength();
        continue;
      }

      // 原始尺寸
      int originalWidth = img.width();
      int originalHeight = img.height();
      qDebug() << "原始尺寸:" << originalWidth << "x" << originalHeight;

      // 缩放逻辑：仅当图片宽 > 屏幕宽时缩放
      QImage scaledImg = img;
      bool isScaled = false;
      if (originalWidth > screenWidth) {
        // 按比例缩放到屏幕宽度
        int scaledHeight = (originalHeight * screenWidth) / originalWidth;
        scaledImg = img.scaled(screenWidth, scaledHeight, Qt::KeepAspectRatio,
                               Qt::SmoothTransformation);
        qDebug() << "缩放后尺寸:" << screenWidth << "x" << scaledHeight;
        isScaled = true;
      }

      // 7. 缩放后的图片转换为字节数据（保持原格式）
      QByteArray scaledData;
      QBuffer buffer(&scaledData);
      buffer.open(QIODevice::WriteOnly);
      // 根据原图格式保存（JPEG/PNG等）
      QString format = "PNG";  // 默认用PNG
      if (imgPathAbs.endsWith(".jpg", Qt::CaseInsensitive) ||
          imgPathAbs.endsWith(".jpeg", Qt::CaseInsensitive)) {
        format = "JPEG";
        // JPEG可设置质量（0-100）
        scaledImg.save(&buffer, format.toUtf8().data(), 90);  // 高质量
      } else {
        scaledImg.save(&buffer, format.toUtf8().data());
      }
      buffer.close();

      // 8. 转换为Data URI
      QString mimeType = format == "JPEG" ? "image/jpeg" : "image/png";
      QString dataUri = QString("data:%1;base64,%2")
                            .arg(mimeType, QString(scaledData.toBase64()));

      // 9. 替换图片路径为缩放后的Data URI
      modifiedImg.replace(srcReg, QString("src=\"%1\"").arg(dataUri));

      // 确保图片没有左缩进，添加样式
      if (isScaled) {
        // 如果有style属性则添加，没有则创建
        if (modifiedImg.contains("style=")) {
          modifiedImg.replace("style=\"",
                              "style=\"margin-left: 0 !important; ");
        } else {
          modifiedImg.insert(4, " style=\"margin-left: 0 !important;\"");
        }
      }

      // 添加点击链接
      modifiedImg = QString("<a href=\"%1\" class=\"custom-img-link\">%2</a>")
                        .arg(dataUri, modifiedImg);
    }

    // 替换原标签
    strHtml.replace(pos, match.capturedLength(), modifiedImg);
    pos += modifiedImg.length();
  }

  // 10. 写入文件（如果需要）

  if (isWriteFile) {
    QFile file(privateDir + "ebook.html");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      file.write(strHtml.toUtf8());
      file.close();
    }
  }

  return strHtml;
}

//////////////////////////////////////////////////////////////////////////////////////////
void Reader::setQMLHtml(QString htmlFile, QString htmlBuffer, QString skipID) {
  if (reader->fileExists(htmlFile)) {
    htmlBuffer = processHtml(htmlFile, false);
  }
  htmlBuffer.append(strEndFlag);
  currentTxt = htmlBuffer;

  mui->qwReader->rootContext()->setContextProperty("isAni", QVariant(false));
  QQuickItem* root = mui->qwReader->rootObject();

  QMetaObject::invokeMethod((QObject*)root, "loadHtmlBuffer",
                            Q_ARG(QVariant, htmlBuffer));

  QFileInfo fi(htmlFile);
  mui->lblInfo->setText(
      tr("Info") + " : " + fi.baseName() + "  " +
      m_Method->getFileSize(reader->getFileSize(htmlFile), 2));

  qDebug() << "setQMLHtml:Html File=" << htmlFile;

  if (skipID != "") {
    setHtmlSkip(htmlFile, skipID);
  }

  gotoCataList(htmlFile);

  setAni();
}

void Reader::setAni() {
  if (isPageNext)
    mui->qwReader->rootContext()->setContextProperty("aniW", mw_one->width());
  else
    mui->qwReader->rootContext()->setContextProperty("aniW", -mw_one->width());
  mui->qwReader->rootContext()->setContextProperty("toW", 0);
  mui->qwReader->rootContext()->setContextProperty("isAni", true);
}

QStringList Reader::readText(QString textFile) {
  QStringList list, list1;

  if (QFile(textFile).exists()) {
    QFile file(textFile);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
      qDebug() << tr("Cannot read file %1:\n%2.")
                      .arg(QDir::toNativeSeparators(textFile),
                           file.errorString());
      return list1;
    }

    QByteArray data = file.readAll();
    file.close();

    QString text;

    text = m_Method->convertDataToUnicode(data);

    text.replace(">", ">\n");
    text.replace("<", "\n<");
    list = text.split("\n");

    // 使用索引遍历避免detach
    for (int i = 0; i < list.size(); ++i) {
      QString trimmed = list.at(i).trimmed();
      if (!trimmed.isEmpty()) {
        list1.append(trimmed);
      }
    }
  }

  return list1;
}

QStringList Reader::readText(QByteArray data) {
  QStringList list, list1;

  QString text;
  text = m_Method->convertDataToUnicode(data);

  text.replace(">", ">\n");
  text.replace("<", "\n<");
  list = text.split("\n");

  // 使用索引遍历避免detach
  for (int i = 0; i < list.size(); ++i) {
    QString trimmed = list.at(i).trimmed();
    if (!trimmed.isEmpty()) {
      list1.append(trimmed);
    }
  }

  return list1;
}

void Reader::closeEvent(QCloseEvent* event) { Q_UNUSED(event); }

void Reader::paintEvent(QPaintEvent* event) { Q_UNUSED(event); }

void Reader::goBookReadPosition() {
  if (isOpen) {
    QString file = iniDir + "bookini/" + currentBookName + ".ini";
    if (!QFile::exists(file))
      file = privateDir + "bookini/" + currentBookName + ".ini";

    QSettings Reg(file, QSettings::IniFormat);

    if (isText) {
      currentPage = Reg.value("/Reader/currentPage", 0).toULongLong();
      QString txt1 = updateContent();
      setQMLText(txt1);
    }

    if (isEpub) {
      htmlIndex = Reg.value("/Reader/htmlIndex", 0).toInt();

      if (htmlIndex >= htmlFiles.count()) {
        htmlIndex = 0;
      }

      currentHtmlFile = htmlFiles.at(htmlIndex);
      setQMLHtml(currentHtmlFile, "", "");
    }

    setPageVPos();
    showInfo();
  }
}

void Reader::setFontSize(int fontSize) {
  qreal pos1 = getVPos();
  qreal h1 = getVHeight();

  mui->qwReader->rootContext()->setContextProperty("FontSize", fontSize);

  m_Method->Sleep(100);
  qreal h2 = getVHeight();
  qreal pos2 = pos1 * h2 / h1;
  setVPos(pos2);
  textPos = pos2;

  readReadNote(cPage);
}

void Reader::PlainTextEditToFile(QPlainTextEdit* txtEdit, QString fileName) {
  QFile* file;
  file = new QFile;
  file->setFileName(fileName);
  bool ok = file->open(QIODevice::WriteOnly | QIODevice::Text);
  if (ok) {
    QTextStream out(file);
    out << txtEdit->toPlainText();
    file->close();
    delete file;
  } else
    qDebug() << "Write failure!" << fileName;
}

bool Reader::getQmlReadyEnd() {
  if (!mui->qwReader || !mui->qwReader->rootObject()) {
    return false;
  }

  QObject* rootObject = mui->qwReader->rootObject();
  QVariant resultVar;  // 先用QVariant接收（适配QML的类型传递）

  // 调用QML函数，用QVariant接收返回值
  QMetaObject::invokeMethod(
      rootObject, "getReadyEnd", Qt::DirectConnection,
      Q_RETURN_ARG(QVariant, resultVar)  // 关键：用QVariant接收
  );

  return resultVar.toBool();
}

void Reader::setQmlLandscape(bool isValue) {
  if (!mui->qwReader || !mui->qwReader->rootObject()) {
    return;
  }

  QObject* rootObject = mui->qwReader->rootObject();
  bool result = QMetaObject::invokeMethod(
      rootObject, "setLandscape",
      Qt::QueuedConnection,  // 推荐使用队列连接避免线程问题
      Q_ARG(QVariant, isValue));

  if (!result) {
  }
}

bool Reader::getLandscape() {
  QString file = iniDir + "bookini/" + currentBookName + ".ini";
  if (!QFile::exists(file))
    file = privateDir + "bookini/" + currentBookName + ".ini";

  QSettings Reg(file, QSettings::IniFormat);

  bool oldLandscape = isLandscape;
  bool newLandscape = false;

  QFileInfo fiHtml(currentHtmlFile);
  if (isEpub) {
    if (mui->qwCata->isVisible()) {
      textPos = Reg.value("/Reader/vpos  CataVPos", 0).toReal();
      int index = Reg.value("/Reader/vpos  CataIndex", 0).toReal();
      if (currentCataIndex > 0) index = currentCataIndex;
      m_Method->setCurrentIndexFromQW(mui->qwCata, index);
    } else {
      if (htmlIndex >= 0)
        textPos = Reg.value("/Reader/vpos" + fiHtml.baseName(), 0).toReal();

      QString key = "/Reader/vpos_" + fiHtml.baseName() + "_isLandscape";
      bool exists = Reg.contains(key);
      if (exists) {
        newLandscape = Reg.value(key, false).toBool();
      } else {
        newLandscape = isLandscape;
      }
    }
  }

  if (isText) {
    textPos = Reg.value("/Reader/vpos" + QString::number(currentPage), false)
                  .toReal();

    QString key =
        "/Reader/vpos_" + QString::number(currentPage) + "_isLandscape";
    bool exists = Reg.contains(key);
    if (exists) {
      newLandscape = Reg.value(key, false).toBool();
    } else {
      newLandscape = isLandscape;
    }
  }

  if (oldLandscape != newLandscape) {
    isLandscape = newLandscape;
  }

  return isLandscape;
}

void Reader::savePageVPos() {
  QString endFile = iniDir + "bookini/" + currentBookName + ".ini";
  QSettings Reg(endFile, QSettings::IniFormat);

  QFileInfo fiHtml(currentHtmlFile);
  textPos = getVPos();
  if (isEpub) {
    if (mui->qwCata->isVisible()) {
      Reg.setValue("/Reader/vpos  CataVPos", textPos);
      int index = m_Method->getCurrentIndexFromQW(mui->qwCata);
      Reg.setValue("/Reader/vpos  CataIndex", index);
    } else {
      if (htmlIndex >= 0) {
        Reg.setValue("/Reader/vpos" + fiHtml.baseName(), textPos);
        Reg.setValue("/Reader/vpos_" + fiHtml.baseName() + "_isLandscape",
                     isLandscape);

        qreal h = getVHeight();
        qreal ratio = textPos / h;
        Reg.setValue("/Reader/ratio" + fiHtml.baseName(), ratio);
      }
    }
  }

  if (isText) {
    Reg.setValue("/Reader/vpos" + QString::number(currentPage), textPos);
    Reg.setValue(
        "/Reader/vpos_" + QString::number(currentPage) + "_isLandscape",
        isLandscape);

    qreal h = getVHeight();
    qreal ratio = textPos / h;
    Reg.setValue("/Reader/ratio" + QString::number(currentPage), ratio);
  }

  Reg.sync();

  qDebug() << "saveVPos=" << textPos;
}

void Reader::setPageVPos() {
  QString file = iniDir + "bookini/" + currentBookName + ".ini";
  if (!QFile::exists(file))
    file = privateDir + "bookini/" + currentBookName + ".ini";

  QSettings Reg(file, QSettings::IniFormat);

  bool oldLandscape = isLandscape;
  bool newLandscape = false;
  qreal ratio = -1;

  QFileInfo fiHtml(currentHtmlFile);
  if (isEpub) {
    if (mui->qwCata->isVisible()) {
      textPos = Reg.value("/Reader/vpos  CataVPos", 0).toReal();
      int index = Reg.value("/Reader/vpos  CataIndex", 0).toReal();
      if (currentCataIndex > 0) index = currentCataIndex;
      m_Method->setCurrentIndexFromQW(mui->qwCata, index);
    } else {
      if (htmlIndex >= 0) {
        textPos = Reg.value("/Reader/vpos" + fiHtml.baseName(), 0).toReal();

        QString key = "/Reader/vpos_" + fiHtml.baseName() + "_isLandscape";
        bool exists = Reg.contains(key);
        if (exists) {
          newLandscape = Reg.value(key, false).toBool();
        } else {
          newLandscape = isLandscape;
        }

        ratio = Reg.value("/Reader/ratio" + fiHtml.baseName(), 0).toReal();
      }
    }
  }

  if (isText) {
    textPos =
        Reg.value("/Reader/vpos" + QString::number(currentPage), 0).toReal();

    QString key =
        "/Reader/vpos_" + QString::number(currentPage) + "_isLandscape";
    bool exists = Reg.contains(key);
    if (exists) {
      newLandscape = Reg.value(key, false).toBool();
    } else {
      newLandscape = isLandscape;
    }

    ratio =
        Reg.value("/Reader/ratio" + QString::number(currentPage), 0).toReal();
  }

  if (oldLandscape != newLandscape) {
    isLandscape = newLandscape;
    setQmlLandscape(isLandscape);
    while (!getQmlReadyEnd())
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
  }

  m_Method->Sleep(200);

  if (ratio > 0) {
    qreal h = getVHeight();
    textPos = h * ratio;
  }

  setVPos(textPos);

  qDebug() << "setVPos=" << textPos << "ratio=" << ratio;
}

void Reader::setVPos(qreal pos) {
  QQuickItem* root;
  if (mui->qwCata->isVisible())
    root = mui->qwCata->rootObject();
  else
    root = mui->qwReader->rootObject();

  QMetaObject::invokeMethod((QObject*)root, "setVPos", Q_ARG(QVariant, pos));
}

qreal Reader::getVPos() {
  QVariant itemCount;

  QQuickItem* root;
  if (mui->qwCata->isVisible())
    root = mui->qwCata->rootObject();
  else
    root = mui->qwReader->rootObject();

  QMetaObject::invokeMethod((QObject*)root, "getVPos",
                            Q_RETURN_ARG(QVariant, itemCount));
  textPos = itemCount.toDouble();
  return textPos;
}

QString Reader::getBookmarkTextFromQML() {
  QVariant item;
  QQuickItem* root = mui->qwReader->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "getBookmarkText",
                            Q_RETURN_ARG(QVariant, item));
  QString txt = item.toString();
  if (isZH_CN) {
    txt = txt.left(50);
  }
  return txt + "...";
}

qreal Reader::getVHeight() {
  QVariant itemCount;
  QQuickItem* root = mui->qwReader->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "getVHeight",
                            Q_RETURN_ARG(QVariant, itemCount));
  textHeight = itemCount.toDouble();
  return textHeight;
}

void Reader::showInfo() {
  cPage = 0;
  int tPage = 0;
  if (isText) {
    cPage = currentPage + 1;
    tPage = totalPages;
  }

  if (isEpub) {
    cPage = htmlIndex + 1;
    tPage = htmlFiles.count();
  }

  mui->btnPages->setText(QString::number(cPage) + "\n" +
                         QString::number(tPage));
  mui->progReader->setMaximum(tPage);
  mui->progReader->setValue(cPage);

  m_ReaderSet->updateProgress();

  updateReaderProperty(cPage, tPage);
  readReadNote(cPage);
}

void Reader::updateReaderProperty(int currentPage, int totalPages) {
  mui->qwReader->rootContext()->setContextProperty("currentPage", currentPage);
  mui->qwReader->rootContext()->setContextProperty("totalPages", totalPages);
}

void Reader::SplitFile(QString qfile) {
  QTextEdit* text_edit = new QTextEdit;
  QPlainTextEdit* plain_edit = new QPlainTextEdit;
  QPlainTextEdit* plain_editHead = new QPlainTextEdit;

  QFileInfo fi(qfile);

  QString text = loadText(qfile);
  text.replace("<", "\n<");
  text.replace(">", ">\n");
  text = text.trimmed();
  text_edit->setPlainText(text);
  int count = text_edit->document()->lineCount();
  for (int i = 0; i < count; i++) {
    QString str = getTextEditLineText(text_edit, i);
    str = str.trimmed();
    plain_editHead->appendPlainText(str);
    if (str == "</head>") break;
  }

  int countHead = plain_editHead->document()->lineCount();
  int countBody = count - countHead;
  int n;
  qint64 bb = fi.size();
  if (bb > minBytes && bb < maxBytes)
    n = 2;
  else
    n = bb / minBytes;

  int split = countBody / n;
  int breakLine = 0;
  for (int x = 1; x < n + 1; x++) {
    if (x == 1) {
      // 1
      for (int i = 0; i < count; i++) {
        QString str = getTextEditLineText(text_edit, i);
        plain_edit->appendPlainText(str);
        if (i == countHead + split) {
          plain_edit->appendPlainText("</body>");
          plain_edit->appendPlainText("</html>");
          breakLine = i;
          break;
        }
      }

      QString file1 = qfile;

      PlainTextEditToFile(plain_edit, file1);
      tempHtmlList.append(file1);
    }

    // 2...n-1
    if (x > 1 && x < n) {
      plain_edit->clear();
      plain_edit->setPlainText(plain_editHead->toPlainText());
      plain_edit->appendPlainText("<body>");
      for (int i = breakLine + 1; i < count; i++) {
        QString str = getTextEditLineText(text_edit, i);
        plain_edit->appendPlainText(str);
        if (i == countHead + split * x) {
          plain_edit->appendPlainText("</body>");
          plain_edit->appendPlainText("</html>");
          breakLine = i;
          break;
        }
      }

      QString file2 = fi.path() + "/" + fi.baseName() + "_" +
                      QString::number(x - 1) + "." + fi.suffix();

      PlainTextEditToFile(plain_edit, file2);
      tempHtmlList.append(file2);
    }

    if (x == n) {
      // n
      plain_edit->clear();
      plain_edit->setPlainText(plain_editHead->toPlainText());
      plain_edit->appendPlainText("<body>");
      for (int i = breakLine + 1; i < count; i++) {
        QString str = getTextEditLineText(text_edit, i);
        plain_edit->appendPlainText(str);
      }

      QString filen = fi.path() + "/" + fi.baseName() + "_" +
                      QString::number(x - 1) + "." + fi.suffix();

      PlainTextEditToFile(plain_edit, filen);
      tempHtmlList.append(filen);
    }

    strShowMsg = "SplitFile: " + m_Method->getFileSize(bb, 2) + "  " +
                 QString::number(x) + "->" + QString::number(n) + "  " +
                 fi.baseName();
  }
}

QString Reader::getNCX_File(QString path) {
  QDir* dir = new QDir(path);
  QStringList filter;
  filter << "*.ncx";
  dir->setNameFilters(filter);
  QList<QFileInfo>* fileInfo = new QList<QFileInfo>(dir->entryInfoList(filter));
  for (int i = 0; i < fileInfo->size(); i++) {
    if (fileInfo->at(i).exists()) {
      QString file = fileInfo->at(i).filePath();
      return file;
    }
  }
  return "";
}

void Reader::proceImg() {
  QString imgdir = strOpfPath + "Images";
  QDir dir0(imgdir);
  if (!dir0.exists()) imgdir = strOpfPath + "images";
  QDir dir2(imgdir);
  if (!dir2.exists()) imgdir = strOpfPath + "graphics";
  QDir dir1(imgdir);
  if (!dir1.exists()) imgdir = strOpfPath;
  qDebug() << "Image Dir : " << imgdir;

  QDir* dir = new QDir(imgdir);
  QStringList filter;
  filter << "*.png"
         << "*.jpg"
         << "*.jpeg"
         << "*.bmp"
         << "*.svg";
  dir->setNameFilters(filter);
  QList<QFileInfo>* fileInfo = new QList<QFileInfo>(dir->entryInfoList(filter));
  for (int i = 0; i < fileInfo->size(); i++) {
    if (fileInfo->at(i).exists()) {
      QString file = fileInfo->at(i).filePath();
      QFileInfo fi(file);
      QString picFile = fi.path() + "/org-" + fi.fileName();
      QFile::copy(file, picFile);

      QImage img(file);
      double w, h;
      int new_w, new_h;
      w = img.width();
      h = img.height();
      double r = (double)w / h;
      if (w > mw_one->width() - 104 || file.contains("cover")) {
        new_w = mw_one->width() - 104;
        if (file.contains("cover")) new_w = mw_one->width() - 25;
        new_h = new_w / r;
        QPixmap pix;
        // pix = QPixmap::fromImage(img.scaled(new_w, new_h));
        pix = QPixmap::fromImage(img);
        pix = pix.scaled(new_w, new_h, Qt::KeepAspectRatio,
                         Qt::SmoothTransformation);
        pix.save(file);
      }
    }
  }

  QString strCover = getCoverPicFile(htmlFiles.at(0));
  qDebug() << "strCover=" << strCover << htmlFiles.at(0);
  if (QFile(strCover).exists()) {
    QImage img(strCover);
    double w, h;
    int new_w, new_h;
    w = img.width();
    h = img.height();
    double r = (double)w / h;
    new_w = mw_one->width() - 25;
    new_h = new_w / r;
    QPixmap pix;
    // pix = QPixmap::fromImage(img.scaled(new_w, new_h));
    pix = QPixmap::fromImage(img);
    pix =
        pix.scaled(new_w, new_h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pix.save(strCover);
  }
}

QString Reader::getUriRealPath(QString uripath) {
#ifdef Q_OS_ANDROID
  // 获取当前Activity实例
  QJniObject activity =
      QJniObject(QNativeInterface::QAndroidApplication::context());

  if (activity.isValid()) {
    // 将QString转换为jstring
    QJniObject javaUriPath = QJniObject::fromString(uripath);

    // 调用Java方法
    QJniObject result = activity.callMethod<jstring>(
        "getUriPath",                              // 方法名
        "(Ljava/lang/String;)Ljava/lang/String;",  // 方法签名
        javaUriPath.object<jstring>()              // 参数
    );

    if (result.isValid()) {
      QString realPath = result.toString();
      qDebug() << "RealPath:" << realPath;
      return realPath;
    } else {
      qWarning() << "JNI call returned invalid object";
    }
  } else {
    qWarning() << "Failed to get Android activity";
  }
#endif

  return uripath;
}

void Reader::getReadList() {
  for (int i = 0; i < bookList.count(); i++) {
    QString str = bookList.at(i);
    QStringList list = str.split("|");
    if (!QFile(list.at(1)).exists()) {
      bookList.removeAt(i);
      i--;
    }
  }

  if (bookList.count() == 0) return;

  m_Method->clearAllBakList(mui->qwBookList);
  for (int i = 0; i < bookList.count(); i++) {
    QString str = bookList.at(i);
    QStringList listBooks = str.split("|");
    QString bookName = listBooks.at(0);
    QString bookPath = listBooks.at(1);
    QString suffix;
    if (bookName.contains(".txt", Qt::CaseInsensitive)) {
      suffix = "txt";
    } else if (bookName.contains(".epub", Qt::CaseInsensitive)) {
      suffix = "epub";
    } else if (bookName.contains(".pdf", Qt::CaseInsensitive)) {
      suffix = "pdf";
    } else
      suffix = "none";

    m_Method->addItemToQW(mui->qwBookList, bookName, bookPath, "", suffix, 50);
  }

  for (int i = 0; i < bookList.count(); i++) {
    QString str = bookList.at(i);
    QStringList listBooks = str.split("|");
    if (listBooks.at(1) == fileName) {
      m_Method->setCurrentIndexFromQW(mui->qwBookList, i);
      break;
    }
  }
}

void Reader::clearAllReaderRecords() {
  int count = m_Method->getCountFromQW(mui->qwBookList);
  if (count == 0) return;

  m_Method->m_widget = new QWidget(mw_one);
  ShowMessage* m_ShowMsg = new ShowMessage(this);
  if (!m_ShowMsg->showMsg("Knot", tr("Clear all reading history") + " ? ", 2))
    return;

  m_Method->clearAllBakList(mui->qwBookList);
  bookList.clear();
  QFile file(privateDir + "reader.ini");
  if (file.exists()) file.remove();
}

void Reader::openBookListItem() {
  int index = m_Method->getCurrentIndexFromQW(mui->qwBookList);

  if (index < 0) return;

  QString str = bookList.at(index);
  QStringList listBooks = str.split("|");
  QString bookfile = listBooks.at(1);
  QFileInfo fi(bookfile);
  if (bookfile != fileName) {
    startOpenFile(bookfile);
  } else {
    if (fi.suffix().contains("pdf")) {
      startOpenFile(bookfile);
    }
  }

  isOpenBookListClick = true;
}

void Reader::backDir() {
  if (!QFile(fileName).exists()) return;

  setEpubPagePosition(mainDirIndex, "");
  qDebug() << "mainDirIndex: " << mainDirIndex;
  if (mainDirIndex == 0) {
    goNextPage();
    goUpPage();

  } else {
    goUpPage();
    goNextPage();
  }
}

QString Reader::getCoverPicFile(QString htmlFile) {
  QStringList list = readText(htmlFile);
  QString str0, str1;
  for (int i = 0; i < list.count(); i++) {
    str0 = list.at(i);
    str0 = str0.trimmed();
    // qDebug() << "str0=" << str0;
    str0 = str0.replace("<image", "<img");
    str0 = str0.replace("xlink:href=", "src=");
    if (str0.contains("<img") && str0.contains("src=")) {
      for (int j = 0; j < str0.length(); j++) {
        if (str0.mid(j, 5) == "src=\"") {
          for (int m = j + 5; m < str0.length(); m++) {
            if (str0.mid(m, 1) == "\"") {
              str1 = str0.mid(j + 5, m - j - 5);
              qDebug() << "img src=" << strOpfPath + str1;
              str1 = str1.replace("../", "");
              return strOpfPath + str1;
              break;
            }
          }
          break;
        }
      }
      break;
    }
  }
  return "";
}

// 拷贝文件夹：
bool Reader::copyDirectoryFiles(const QString& fromDir, const QString& toDir,
                                bool coverFileIfExist) {
  QDir sourceDir(fromDir);
  QDir targetDir(toDir);
  if (!targetDir.exists()) { /**< 如果目标目录不存在，则进行创建 */
    if (!targetDir.mkdir(targetDir.absolutePath())) return false;
  }

  QFileInfoList fileInfoList = sourceDir.entryInfoList();
  foreach (QFileInfo fileInfo, fileInfoList) {
    if (fileInfo.fileName() == "." || fileInfo.fileName() == "..") continue;

    if (fileInfo.isDir()) { /**< 当为目录时，递归的进行copy */
      if (!copyDirectoryFiles(fileInfo.filePath(),
                              targetDir.filePath(fileInfo.fileName()),
                              coverFileIfExist))
        return false;
    } else { /**< 当允许覆盖操作时，将旧文件进行删除操作 */
      if (coverFileIfExist && targetDir.exists(fileInfo.fileName())) {
        targetDir.remove(fileInfo.fileName());
      }

      /// 进行文件copy
      if (!QFile::copy(fileInfo.filePath(),
                       targetDir.filePath(fileInfo.fileName()))) {
        return false;
      }
    }
  }
  return true;
}

void Reader::on_hSlider_sliderReleased(int position) {
  mui->lblTitle->hide();

  int max = 0;
  if (isText) max = totalPages;
  if (isEpub) max = htmlFiles.count();
  if (position >= max) position = max;
  sPos = position;
  getLines();
  setPageVPos();
}

void Reader::getLines() {
  QString qsShow;

  mui->hSlider->setMinimum(1);

  if (isText) {
    mui->hSlider->setMaximum(totalPages);
    currentPage = mui->hSlider->value() - 1;
    mui->btnPages->setText(QString::number(currentPage + 1) + "\n" +
                           QString::number(totalPages));

    QString txt1;
    txt1 = updateContent();

    qsShow =
        "<p style='line-height:32px; width:100% ; white-space: pre-wrap; "
        "'>" +
        txt1 + "</p>";
    setQMLText(qsShow);
  }

  if (isEpub) {
    mui->hSlider->setMaximum(htmlFiles.count());
    htmlIndex = sPos - 1;
    if (htmlIndex < 0) htmlIndex = 0;

    currentHtmlFile = htmlFiles.at(htmlIndex);
    setQMLHtml(currentHtmlFile, "", "");
  }
}

void Reader::showCatalogue() {
  closeSelText();
  savePageVPos();

  if (mui->qwCata->isVisible()) {
    mui->lblCataInfo->hide();
    mui->qwCata->hide();
    mui->qwReader->show();
    mui->btnShowBookmark->setEnabled(true);

  } else {
    mui->qwReader->hide();
    mui->lblCataInfo->show();
    mui->qwCata->show();
    mui->btnShowBookmark->setEnabled(false);

    m_Method->clearAllBakList(mui->qwCata);
    for (int i = 0; i < ncxList.count(); i++) {
      QString item = ncxList.at(i);
      QString str0, str1;
      str0 = item.split("===").at(0);
      str1 = item.split("===").at(1);
      m_Method->addItemToQW(mui->qwCata, str0, str1, "", "", 0);
    }
  }

  setPageVPos();
  showInfo();
}

QString Reader::getNavFileInternalPath(const QByteArray& opfContent) {
  // 先将QByteArray转为QString（按UTF-8编码，EPUB标准编码）
  QString opfStr = QString::fromUtf8(opfContent);

  // 简单处理：直接查找包含 'id="nav"' 的item标签
  int navItemStart = opfStr.indexOf("=\"nav\"");
  if (navItemStart == -1) {
    qWarning() << "未找到id=\"nav\"的导航文件";
    return QString();
  }

  // 从item标签开始向前找href属性（简单字符串匹配）
  int itemStart = opfStr.lastIndexOf("<item", navItemStart);
  if (itemStart == -1) {
    qWarning() << "未找到包含id=\"nav\"的item标签";
    return QString();
  }

  // 从item标签内提取href的值
  int hrefStart = opfStr.indexOf("href=\"", itemStart);
  if (hrefStart == -1) {
    qWarning() << "id=\"nav\"的item标签中未找到href属性";
    return QString();
  }
  hrefStart += 6;  // 跳过 "href=\""

  int hrefEnd = opfStr.indexOf("\"", hrefStart);
  if (hrefEnd == -1) {
    qWarning() << "href属性值格式错误";
    return QString();
  }

  // 提取href的值（导航文件相对路径）
  return opfStr.mid(hrefStart, hrefEnd - hrefStart);
}

QStringList Reader::ncx2html() {
  catalogueFile = privateDir + "catalogue.html";
  QStringList htmlList;
  ncxList.clear();
  bool isNCX = false;
  QString ncxFile = strOpfPath + "toc.ncx";
  QStringList epubFiles = reader->getAllFilePaths();
  for (int i = 0; i < epubFiles.count(); i++) {
    QString file = epubFiles.at(i);
    QString l_file = file.toLower();
    if (l_file.contains(".ncx")) {
      ncxFile = file;
      isNCX = true;
      qDebug() << "ncxFile=" << ncxFile;
      break;
    }
  }

  if (!isNCX) {
    QByteArray ba = reader->readFile(strOpfFile);
    QString nav = strOpfPath + getNavFileInternalPath(ba);
    qDebug() << "nav=" << nav;

    QByteArray nav_data = reader->readFile(nav);
    QList<TocItem> tocItems = parseTocFromNavFile(nav_data);

    for (int i = 0; i < tocItems.count(); i++) {
      qDebug() << tocItems.at(i).title << "====>>" << tocItems.at(i).href;
      ncxList.append(tocItems.at(i).title + "===" + strOpfPath +
                     tocItems.at(i).href);
    }

    strEpubTitle = getEpub3Title(strOpfFile);
    mui->lblCataInfo->setText(strEpubTitle);
    // debugPrintTocItems(tocItems, 0);

    return htmlList;
  }

  if (!reader->fileExists(ncxFile)) {
    return htmlList;
  }

  QPlainTextEdit* plain_edit = new QPlainTextEdit;
  plain_edit->appendPlainText("<html>");
  plain_edit->appendPlainText("<body>");
  plain_edit->appendPlainText("<style>.my-link {color: #336699;} </style>");

  QTextEdit* text_edit = new QTextEdit;
  QTextEdit* text_edit0 = new QTextEdit;

  QByteArray data = reader->readFile(ncxFile);
  QString strHtml0 = m_Method->convertDataToUnicode(data);

  strHtml0 = strHtml0.replace("　", " ");
  strHtml0 = strHtml0.replace("<", "\n<");
  strHtml0 = strHtml0.replace(">", ">\n");

  text_edit0->setPlainText(strHtml0);
  for (int i = 0; i < text_edit0->document()->lineCount(); i++) {
    QString str = getTextEditLineText(text_edit0, i);
    str = str.trimmed();
    if (str != "") {
      text_edit->append(str);
    }
  }

  QString strAuthor;
  for (int i = 0; i < text_edit->document()->lineCount(); i++) {
    QString str = getTextEditLineText(text_edit, i);
    str = str.trimmed();
    QString str0, str1, str2;
    bool isAdd = false;
    bool isAddTitle = false;

    if (str == "<docTitle>") {
      str0 = getTextEditLineText(text_edit, i + 2);
      str0 = str0.trimmed();
      strEpubTitle = str0;
      isAddTitle = true;
    }

    if (str == "<docAuthor>") {
      strAuthor = getTextEditLineText(text_edit, i + 2);
      strAuthor = strAuthor.trimmed();
    }

    if (isAddTitle) {
      plain_edit->appendPlainText("<div>");
      plain_edit->appendPlainText("<h3>" + str0 + "</h3>");
      plain_edit->appendPlainText("</div>");

      plain_edit->appendPlainText("<ul>");
    }

    if (str == "<navLabel>") {
      str1 = getTextEditLineText(text_edit, i + 2);
      str1 = str1.trimmed();

      str2 = getTextEditLineText(text_edit, i + 5);
      str2.replace("<content src=", "");
      str2.replace("/>", "");
      str2.replace("\"", "");
      str2 = str2.trimmed();

      if (!str1.contains("</")) {
        isAdd = true;
      }
    }

    if (isAdd) {
      // qDebug() << "ncx2html:" << str1 << str2;

      plain_edit->appendPlainText("<div>");
      plain_edit->appendPlainText("<li><a href=" + strOpfPath + str2 +
                                  " class=my-link >" + str1 + "</a></li>");
      plain_edit->appendPlainText("</div>");

      ncxList.append(str1 + "===" + strOpfPath + str2);

      QString str3 = str2;
      QStringList list3 = str3.split("#");
      if (list3.count() == 2) {
        str3 = list3.at(0);
      }
      if (!htmlList.contains(strOpfPath + str3)) {
        htmlList.append(strOpfPath + str3);
      }
    }
  }

  plain_edit->appendPlainText("</ul>");
  plain_edit->appendPlainText("</body>");
  plain_edit->appendPlainText("</html>");

  PlainTextEditToFile(plain_edit, catalogueFile);

  if (strEpubTitle != "") {
    if (strAuthor != "")
      strEpubTitle = strEpubTitle + " ( " + strAuthor + " ) ";
    mui->lblCataInfo->setText(strEpubTitle);
    qDebug() << "ncx title=" << strEpubTitle << "author=" << strAuthor;
  }

  return htmlList;
}

void Reader::setHtmlSkip(QString htmlFile, QString skipID) {
  QTextBrowser* textBrowser = new QTextBrowser();
  textBrowser->setFixedHeight(mui->qwReader->height());
  textBrowser->setFixedWidth(mui->qwReader->width());
  QFont font = mui->qwReader->font();
  font.setPixelSize(readerFontSize);
  font.setFamily(mui->btnFont->font().family());
  font.setLetterSpacing(QFont::AbsoluteSpacing, 2);
  textBrowser->setFont(font);

  if (isEpub) {
    QString str = loadText(htmlFile);
    str.replace("..", strOpfPath);
    QDir dir;
    dir.setCurrent(strOpfPath);
    textBrowser->setHtml(str);
  }

  qDebug() << "strFind=" << strFind << "skipID=" << skipID;

  for (int i = 0; i < 100; i++) {
    if (textBrowser->find(strFind)) {
      int curpos = textBrowser->textCursor().position();

      QTextCursor cursor;
      cursor = textBrowser->textCursor();
      cursor.setPosition(curpos);
      cursor.setPosition(curpos + 2, QTextCursor::KeepAnchor);
      textBrowser->setTextCursor(cursor);
      QString s0 = cursor.selectedText();

      qDebug() << "cursor pos=" << curpos << "s0=" << s0 << s0.trimmed();

      if (s0.trimmed().length() < 2) {
        setTextAreaCursorPos(curpos);

        if (curpos > 50) {
          qreal scrollPos = getVPos();
          scrollPos = scrollPos + textBrowser->height() / 2;
          setVPos(scrollPos);
        }
        break;
      }
    }
  }
}

QString Reader::getSkipText(QString htmlFile, QString skipID) {
  QStringList list = readText(htmlFile);
  QString hxHeight = "24";
  for (int i = 0; i < list.count(); i++) {
    QString item = list.at(i);
    if (item.contains(skipID)) {
      if (item.contains("h1")) hxHeight = "32";
      if (item.contains("h2")) hxHeight = "24";
      if (item.contains("h3")) hxHeight = "18";
      if (item.contains("h4")) hxHeight = "16";
      if (item.contains("h5")) hxHeight = "13";
      if (item.contains("h6")) hxHeight = "12";

      QStringList l0 = item.split(">");
      QString txt = l0.at(1);
      txt = txt.split(">").at(0);
      txt = txt.trimmed();

      if (txt == "") {
        item = list.at(i + 1);
        l0 = item.split(">");
        txt = l0.at(1);
        txt = txt.split("<").at(0);
        txt = txt.trimmed();
      }

      qDebug() << "skipID=" << skipID << "txt=" << txt << hxHeight;
      return txt + "===" + hxHeight;
      break;
    }
  }

  return "";
}

void Reader::removeBookList() {
  int index = m_Method->getCurrentIndexFromQW(mui->qwBookList);
  if (index <= 0) return;

  ShowMessage* msg = new ShowMessage(mw_one);
  if (!msg->showMsg("Knot", tr("Remove from list?"), 2)) return;

  bookList.removeAt(index);
  m_Method->delItemFromQW(mui->qwBookList, index);
  saveReader("", false);
}

void Reader::readBookDone() {
  if (isOpenBookListClick) {
    mw_one->on_btnBackBookList_clicked();
    isOpenBookListClick = false;
  }

  if (isEpubError) {
    mui->btnReader->setEnabled(true);
    mui->f_ReaderFun->setEnabled(true);
    mw_one->closeProgress();

    ShowMessage* msg = new ShowMessage(mw_one);
    msg->showMsg("Knot", tr("The EPUB file was opened with an error."), 1);

    if (!isText) {
      if (htmlFiles.count() > 0) isEpub = true;
    }
    return;
  }

  QFileInfo fi(fileName);
  QString epubName = strEpubTitle;
  QString name_l = epubName.toLower();
  if (epubName == "" || name_l.contains("unknown")) epubName = fi.baseName();
  if (isEpub)
    currentBookName = epubName;
  else
    currentBookName = fi.baseName();

  const int MAX_BASIC_FILENAME_LENGTH = 250;
  if (currentBookName.length() > MAX_BASIC_FILENAME_LENGTH) {
    currentBookName = currentBookName.left(MAX_BASIC_FILENAME_LENGTH);
  }
  QString extName = fi.suffix();
  currentBookName = currentBookName + "_" + extName;

  // 依次替换所有禁止的特殊字符为空字符串
  currentBookName.replace("\\", "");
  currentBookName.replace("/", "");
  currentBookName.replace(":", "");
  currentBookName.replace("*", "");
  currentBookName.replace("?", "");
  currentBookName.replace("\"", "");
  currentBookName.replace("<", "");
  currentBookName.replace(">", "");
  currentBookName.replace("|", "");

  if (isText || isEpub) {
    strShowMsg = "Read  EBook End...";

    mui->qwReader->show();
    mui->f_ReaderFun->show();
    mui->progReader->show();
    mui->btnPages->show();
    mui->btnShowBookmark->show();
    mui->btnAutoRun->show();

    if (mui->frameBookList->isVisible()) {
      mui->frameBookList->hide();
      mui->frameReader->show();
    }

    mui->qwReader->rootContext()->setContextProperty("strText", "");
    mui->qwReader->rootContext()->setContextProperty("isSelText", isSelText);
    mui->qwReader->rootContext()->setContextProperty("isAni", true);
    mui->qwReader->rootContext()->setContextProperty("aniW", mw_one->width());
    mui->qwReader->rootContext()->setContextProperty("toW", 0);

    if (mui->qwReader->source().isEmpty()) {
      mui->qwReader->setSource(
          QUrl(QStringLiteral("qrc:/src/qmlsrc/reader.qml")));
    }

    if (isEpub) {
      mui->lblInfo->show();
      if (QFile(catalogueFile).exists()) {
        mui->btnCatalogue->show();
      } else
        mui->btnCatalogue->hide();
    }

    if (isText) {
      mui->btnCatalogue->hide();
      mui->lblInfo->hide();
    }
  }

  mui->lblBookName->setText(strTitle);

  for (int i = 0; i < bookList.count(); i++) {
    QString str = bookList.at(i);
    if (str.contains(fileName)) {
      bookList.removeAt(i);
      break;
    }
  }
  bookList.insert(0, strTitle + "|" + fileName);

  if (isPDF) {
    qDebug() << "===Read Pdf... ..." << fileName;

    mui->btnAutoRun->hide();
    mui->btnShowBookmark->hide();
    mui->progReader->hide();
    mui->qwReader->hide();
    mui->f_ReaderFun->show();
    mui->btnPages->hide();
    mui->btnCatalogue->hide();

#ifdef Q_OS_ANDROID

    if (!mw_one->initMain) {
      if (mui->frameMain->isVisible()) {
        mui->frameMain->hide();
        mui->frameBookList->show();
      }
      if (mui->frameReader->isVisible()) {
        mui->frameReader->hide();
        mui->frameBookList->show();
      }
      getReadList();
      openMyPDF(fileName);
    }
#else

#endif
  }

  goBookReadPosition();

  saveReader("", false);

  mw_one->on_DelayCloseProgressBar();

  if (!isInitReader) {
    if (!isPDF) {
      while (!mui->btnReader->isEnabled())
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
      mui->btnReader->click();
      setDefaultOpen("none");
    }
  } else
    isInitReader = false;

  qDebug() << "read book done...";
}

void Reader::setStatusBarHide() {
#ifdef Q_OS_ANDROID

  QJniObject m_activity = QNativeInterface::QAndroidApplication::context();
  m_activity.callMethod<void>("setStatusBarHide", "()V");

#endif
  isStatusBarShow = false;
}

void Reader::setStatusBarShow() {
#ifdef Q_OS_ANDROID

  QJniObject m_activity = QNativeInterface::QAndroidApplication::context();
  m_activity.callMethod<void>("setStatusBarShow", "()V");

#endif
  isStatusBarShow = true;
}

void Reader::selectText() {
  m_ReaderSet->close();

  if (!isSelText) {
    isSelText = true;

    mui->f_ReaderFun->hide();
    mui->f_ReaderNote->show();

    if (isGetBookmarkText) {
      closeSelText();
      isGetBookmarkText = false;
    }

  } else {
    closeSelText();
  }
}

void Reader::closeSelText() {
  if (isSelText) {
    isSelText = false;

    mui->f_ReaderNote->hide();

    mui->qwReader->show();
    mui->f_ReaderFun->show();
  }
}

void Reader::setPageScroll0() {
  qreal cpos = getVPos();
  qreal th = getVHeight();
  int readerHeight = mui->qwReader->height();
  if (th < readerHeight) return;
  int fontHeight = m_Method->getFontHeight();
  qreal newpos = cpos - readerHeight + fontHeight;

  if (newpos < readerHeight + fontHeight) newpos = -fontHeight;

  setVPos(newpos);
}

void Reader::setPageScroll1() {
  qreal cpos = getVPos();
  qreal th = getVHeight();
  int readerHeight = mui->qwReader->height();
  if (th < readerHeight) return;
  int fontHeight = m_Method->getFontHeight();
  qreal newpos = cpos + readerHeight - fontHeight;
  if (newpos + readerHeight - fontHeight > th) {
    newpos = th - readerHeight + fontHeight;
  }
  setVPos(newpos);
}

QStringList Reader::getCurrentBookmarkList() {
  QStringList list;

  QString file = iniDir + "bookini/" + currentBookName + ".ini";
  if (!QFile::exists(file))
    file = privateDir + "bookini/" + currentBookName + ".ini";

  QSettings Reg(file, QSettings::IniFormat);

  int count = Reg.value("/Bookmark/count", 0).toInt();
  for (int i = 0; i < count; i++) {
    QString txt = Reg.value("/Bookmark/Name" + QString::number(i)).toString();
    list.insert(0, txt);
  }
  return list;
}

void Reader::clickBookmarkList(int i) {
  int count = m_Method->getCountFromQW(mui->qwBookmark);
  int index = count - 1 - i;

  QString file = iniDir + "bookini/" + currentBookName + ".ini";
  if (!QFile::exists(file))
    file = privateDir + "bookini/" + currentBookName + ".ini";

  QSettings Reg(file, QSettings::IniFormat);

  if (isText) {
    currentPage =
        Reg.value("/Bookmark/currentPage" + QString::number(index)).toInt();
    textPos = Reg.value("/Bookmark/VPos" + QString::number(index)).toReal();
    isLandscape =
        Reg.value("/Bookmark/isLandscape" + QString::number(index)).toBool();

    if (currentPage <= totalPages) {
      QString txt1 = updateContent();
      setQMLText(txt1);
    }
  }

  if (isEpub) {
    htmlIndex =
        Reg.value("/Bookmark/htmlIndex" + QString::number(index)).toInt();
    textPos = Reg.value("/Bookmark/VPos" + QString::number(index)).toReal();
    isLandscape =
        Reg.value("/Bookmark/isLandscape" + QString::number(index)).toBool();
    if (htmlIndex >= htmlFiles.count()) {
      htmlIndex = 0;
    }

    currentHtmlFile = htmlFiles.at(htmlIndex);
    setQMLHtml(currentHtmlFile, "", "");
  }

  showInfo();

  setQmlLandscape(isLandscape);
  setVPos(textPos);

  mui->qwBookmark->hide();
  mui->qwReader->show();
  mui->btnCatalogue->setEnabled(true);
}

void Reader::showBookmarkList() {
  QStringList list = getCurrentBookmarkList();
  m_Method->clearAllBakList(mui->qwBookmark);
  for (int i = 0; i < list.count(); i++) {
    m_Method->addItemToQW(mui->qwBookmark, list.at(i), "", "", "", 0);
  }
}

void Reader::ContinueReading() {
  while (!mui->btnReader->isEnabled())
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

  mui->btnReader->click();
}

void Reader::openMyPDF(QString uri) {
  Q_UNUSED(uri);
#ifdef Q_OS_ANDROID

  QJniObject jPath = QJniObject::fromString(uri);
  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  activity.callMethod<void>("openMyPDF", "(Ljava/lang/String;)V",
                            jPath.object<jstring>());

#endif
}

void Reader::closeMyPDF() {
#ifdef Q_OS_ANDROID

  // QJniObject activity = QNativeInterface::QAndroidApplication::context();
  QJniObject::callStaticMethod<void>("com.xhh.pdfui/PDFActivity", "closeMyPDF",
                                     "()V");

#endif
}

void Reader::shareBook() {
  int index = m_Method->getCurrentIndexFromQW(mui->qwBookList);

  if (index < 0) return;

  QString str = bookList.at(index);
  QStringList listBooks = str.split("|");
  QString bookfile = listBooks.at(1);
  if (QFile::exists(bookfile)) {
    mw_one->m_ReceiveShare->shareImage(tr("Share to"), bookfile, "*/*");
  }
}

bool Reader::eventFilterReader(QObject* watch, QEvent* evn) {
  if (isShowNote) return true;
  if (mui->f_ReaderNote->isVisible()) return true;
  if (dlgEditBookNote != nullptr) {
    if (dlgEditBookNote->isVisible()) return true;
  }

  // 统一处理鼠标和触摸事件的坐标变量
  static QPointF pressPos;    // 按下/触摸开始位置
  static QPointF currentPos;  // 当前移动位置

  // 1. 处理触摸事件（安卓移动端）
  if (evn->type() == QEvent::TouchBegin || evn->type() == QEvent::TouchUpdate ||
      evn->type() == QEvent::TouchEnd) {
    QTouchEvent* touchEvent = static_cast<QTouchEvent*>(evn);
    const QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->points();
    if (touchPoints.isEmpty()) return false;

    // 只处理单指触摸（翻页通常不需要多指）
    const QTouchEvent::TouchPoint& touchPoint = touchPoints.first();
    currentPos = touchPoint.position();  // 触摸点当前位置（相对控件）

    if (evn->type() == QEvent::TouchBegin) {
      // 触摸开始：对应鼠标按下
      pressPos = touchPoint.position();
      mw_one->isMousePress = true;
      mw_one->isMouseMove = false;

      // mw_one->timerMousePress->start(1300);
      if (isAutoRun)
        mui->qwReader->rootContext()->setContextProperty("isAutoRun",
                                                         QVariant(false));

      // 映射到原有鼠标事件的坐标变量
      press_x = pressPos.x();
      press_y = pressPos.y();
      x = 0;
      y = 0;
      w = mui->qwReader->width();
      h = mui->qwReader->height();
      touchEvent->accept();
      return handleTouchPress(pressPos);
      return true;
    } else if (evn->type() == QEvent::TouchUpdate) {
      // 触摸移动：对应鼠标移动
      mw_one->isMouseMove = true;
      relea_x = currentPos.x();
      relea_y = currentPos.y();

      if (isAutoRun)
        mui->qwReader->rootContext()->setContextProperty("isAutoRun",
                                                         QVariant(false));

      if (mw_one->isMousePress) {
        if (!isLandscape) {
          if ((relea_x - press_x) > 75 && qAbs(relea_y - press_y) < 35) {
            int cn = mui->btnPages->text().split("\n").at(0).toInt();
            if (cn != 1) showBookPageUp();
          } else if ((press_x - relea_x) > 75 && qAbs(relea_y - press_y) < 35) {
            int cn = mui->btnPages->text().split("\n").at(0).toInt();
            int tn = mui->btnPages->text().split("\n").at(1).toInt();
            if (cn != tn) showBookPageNext();
          } else
            closeBookPage();
        } else {
          if ((press_y - relea_y) > 75 && qAbs(relea_x - press_x) < 35) {
            int cn = mui->btnPages->text().split("\n").at(0).toInt();
            int tn = mui->btnPages->text().split("\n").at(1).toInt();
            if (cn != tn) showBookPageNext();
          } else if ((relea_y - press_y) > 75 && qAbs(relea_x - press_x) < 35) {
            int cn = mui->btnPages->text().split("\n").at(0).toInt();
            if (cn != 1) showBookPageUp();
          } else
            closeBookPage();
        }
      }
      touchEvent->accept();
      return true;
    } else if (evn->type() == QEvent::TouchEnd) {
      // 触摸结束：对应鼠标释放

      mw_one->isMousePress = false;
      relea_x = currentPos.x();
      relea_y = currentPos.y();
      mui->lblTitle->hide();
      QQuickItem* root = mui->qwReader->rootObject();

      // 鼠标释放时的翻页逻辑

      if (!isLandscape) {
        if ((relea_x - press_x) > 75 && qAbs(relea_y - press_y) < 35) {
          if (isText && currentPage == 0) {
            QMetaObject::invokeMethod(root, "setX", Q_ARG(QVariant, 0));
            return true;
          }
          if (isEpub && htmlIndex <= 0) {
            QMetaObject::invokeMethod(root, "setX", Q_ARG(QVariant, 0));
            return true;
          }

          mw_one->on_btnPageUp_clicked();
          closeBookPage();
        }
        if ((press_x - relea_x) > 75 && qAbs(relea_y - press_y) < 35) {
          if (isText && currentPage == totalPages - 1) {
            QMetaObject::invokeMethod(root, "setX", Q_ARG(QVariant, 0));
            return true;
          }
          if (isEpub && htmlIndex + 1 >= htmlFiles.count()) {
            QMetaObject::invokeMethod(root, "setX", Q_ARG(QVariant, 0));
            return true;
          }

          mw_one->on_btnPageNext_clicked();
          closeBookPage();
        }
      } else {
        if ((press_y - relea_y) > 75 && qAbs(relea_x - press_x) < 35) {
          if (isText && currentPage == totalPages - 1) {
            QMetaObject::invokeMethod(root, "setX", Q_ARG(QVariant, 0));
            return true;
          }
          if (isEpub && htmlIndex + 1 >= htmlFiles.count()) {
            QMetaObject::invokeMethod(root, "setX", Q_ARG(QVariant, 0));
            return true;
          }

          mw_one->on_btnPageNext_clicked();
          closeBookPage();
        }
        if ((relea_y - press_y) > 75 && qAbs(relea_x - press_x) < 35) {
          if (isText && currentPage == 0) {
            QMetaObject::invokeMethod(root, "setX", Q_ARG(QVariant, 0));
            return true;
          }
          if (isEpub && htmlIndex <= 0) {
            QMetaObject::invokeMethod(root, "setX", Q_ARG(QVariant, 0));
            return true;
          }

          mw_one->on_btnPageUp_clicked();
          closeBookPage();
        }
      }

      if (isText || isEpub) {
        QMetaObject::invokeMethod(root, "setX", Q_ARG(QVariant, 0));
      }
      mw_one->curx = 0;

      if (isAutoRun)
        mui->qwReader->rootContext()->setContextProperty("isAutoRun", true);

      touchEvent->accept();
      return true;
    }
  }

  // 2. 处理鼠标事件（PC端）
  QMouseEvent* event = static_cast<QMouseEvent*>(evn);
  if (watch == mui->qwReader) {
    int length = 75;

    if (mui->qwReader->isVisible()) {
      if (event->type() == QEvent::MouseButtonPress) {
        mw_one->isMousePress = true;
        mw_one->isMouseMove = false;

        press_x =
            event->position()
                .x();  // 注意：Qt6中用position()获取相对位置，globalPosition()是全局位置
        press_y = event->position().y();
        x = 0;
        y = 0;
        w = mui->qwReader->width();
        h = mui->qwReader->height();

        // if (!mw_one->isMouseMove) mw_one->timerMousePress->start(1300);
        if (isAutoRun)
          mui->qwReader->rootContext()->setContextProperty("isAutoRun",
                                                           QVariant(false));
      }

      if (event->type() == QEvent::MouseMove) {
        QPointF pos = event->position();
        relea_x = pos.x();
        relea_y = pos.y();
        if (mw_one->isMousePress && qAbs(relea_x - press_x) > 20 &&
            qAbs(relea_y - press_y) < 20) {
          mw_one->isMouseMove = true;
        }

        if (mw_one->isMousePress) {
          if (!isLandscape) {
            if ((relea_x - press_x) > length && qAbs(relea_y - press_y) < 35) {
              int cn = mui->btnPages->text().split("\n").at(0).toInt();
              if (cn != 1) {
                showBookPageUp();
              }
            } else if ((press_x - relea_x) > length &&
                       qAbs(relea_y - press_y) < 35) {
              int cn = mui->btnPages->text().split("\n").at(0).toInt();
              int tn = mui->btnPages->text().split("\n").at(1).toInt();
              if (cn != tn) {
                showBookPageNext();
              }
            } else
              closeBookPage();
          } else {
            if ((press_y - relea_y) > length && qAbs(relea_x - press_x) < 35) {
              int cn = mui->btnPages->text().split("\n").at(0).toInt();
              int tn = mui->btnPages->text().split("\n").at(1).toInt();
              if (cn != tn) {
                showBookPageNext();
              }
            } else if ((relea_y - press_y) > length &&
                       qAbs(relea_x - press_x) < 35) {
              int cn = mui->btnPages->text().split("\n").at(0).toInt();
              if (cn != 1) {
                showBookPageUp();
              }
            } else {
              closeBookPage();
            }
          }
        }
      }

      if (event->type() == QEvent::MouseButtonDblClick) {
        // 双击逻辑

        int h3 = mui->qwReader->height() / 3;
        int mY = event->position().y();
        int qwY = mui->qwReader->y();

        if ((mY > qwY + h3) && (mY < qwY + h3 * 2)) {
          on_SetReaderFunVisible();
        }

        if ((mY > qwY) && (mY < qwY + h3)) {
          // mw_one->m_Reader->setPageScroll0();
        }

        if (mY > qwY + h3 * 2) {
          // mw_one->m_Reader->setPageScroll1();
          if (!isAutoRun)
            mui->btnAutoRun->click();
          else if (isAutoRun)
            mui->btnAutoStop->click();
        }
      }

      if (event->type() == QEvent::MouseButtonRelease) {
        QPointF pos = event->position();
        relea_x = pos.x();
        relea_y = pos.y();
        mui->lblTitle->hide();
        QQuickItem* root = mui->qwReader->rootObject();

        mw_one->isMousePress = false;

        // 竖屏和横屏翻页逻辑
        if (!isLandscape) {
          if ((relea_x - press_x) > length && qAbs(relea_y - press_y) < 35) {
            if (isText) {
              if (currentPage == 0) {
                QMetaObject::invokeMethod(root, "setX", Q_ARG(QVariant, 0));
                return QWidget::eventFilter(watch, evn);
              }
            } else if (isEpub) {
              if (htmlIndex <= 0) {
                QMetaObject::invokeMethod(root, "setX", Q_ARG(QVariant, 0));
                return QWidget::eventFilter(watch, evn);
              }
            }

            mw_one->on_btnPageUp_clicked();
            closeBookPage();
          }

          if ((press_x - relea_x) > length && qAbs(relea_y - press_y) < 35) {
            if (isText) {
              if (currentPage == totalPages - 1) {
                QMetaObject::invokeMethod(root, "setX", Q_ARG(QVariant, 0));
                return QWidget::eventFilter(watch, evn);
              }
            } else if (isEpub) {
              if (htmlIndex + 1 >= htmlFiles.count()) {
                QMetaObject::invokeMethod(root, "setX", Q_ARG(QVariant, 0));
                return QWidget::eventFilter(watch, evn);
              }
            }

            mw_one->on_btnPageNext_clicked();
            closeBookPage();
          }
        } else {
          if ((press_y - relea_y) > length && qAbs(relea_x - press_x) < 35) {
            if (isText) {
              if (currentPage == totalPages - 1) {
                QMetaObject::invokeMethod(root, "setX", Q_ARG(QVariant, 0));
                return QWidget::eventFilter(watch, evn);
              }
            } else if (isEpub) {
              if (htmlIndex + 1 >= htmlFiles.count()) {
                QMetaObject::invokeMethod(root, "setX", Q_ARG(QVariant, 0));
                return QWidget::eventFilter(watch, evn);
              }
            }

            mw_one->on_btnPageNext_clicked();
            closeBookPage();
          }

          if ((relea_y - press_y) > length && qAbs(relea_x - press_x) < 35) {
            if (isText) {
              if (currentPage == 0) {
                QMetaObject::invokeMethod(root, "setX", Q_ARG(QVariant, 0));
                return QWidget::eventFilter(watch, evn);
              }
            } else if (isEpub) {
              if (htmlIndex <= 0) {
                QMetaObject::invokeMethod(root, "setX", Q_ARG(QVariant, 0));
                return QWidget::eventFilter(watch, evn);
              }
            }

            mw_one->on_btnPageUp_clicked();
            closeBookPage();
          }
        }

        if (isText || isEpub)
          QMetaObject::invokeMethod(root, "setX", Q_ARG(QVariant, 0));

        mw_one->curx = 0;

        if (isAutoRun)
          mui->qwReader->rootContext()->setContextProperty("isAutoRun", true);
      }
    }
  }

  return QWidget::eventFilter(watch, evn);
}

bool Reader::handleTouchPress(const QPointF& globalPos) {
  // 记录按下位置和时间
  static qint64 lastPressTime = 0;
  qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

  // 双击检测（300ms内两次按下）
  if (currentTime - lastPressTime < 350) {
    handleDoubleClick(globalPos);
    lastPressTime = 0;
    return true;
  }
  lastPressTime = currentTime;

  // 更新状态
  mw_one->isMousePress = true;
  mw_one->isMouseMove = false;

  // 启动长按计时器
  if (!mw_one->isMouseMove) {
    // mw_one->timerMousePress->start(1300);
  }

  // 记录按下位置
  press_x = globalPos.x();
  press_y = globalPos.y();

  // 初始化区域参数
  x = 0;
  y = 0;
  w = mui->qwReader->width();
  h = mui->qwReader->height();

  return true;
}

void Reader::handleDoubleClick(const QPointF& globalPos) {
  // 计算区域划分
  int h3 = mui->qwReader->height() / 3;
  int qwY = mui->qwReader->y();
  int mY = globalPos.y();

  // 中间区域：显示/隐藏功能
  if ((mY > qwY + h3) && (mY < qwY + h3 * 2)) {
    on_SetReaderFunVisible();
  }
  // 上部分区域
  else if ((mY > qwY) && (mY < qwY + h3)) {
    // setPageScroll0();
  }
  // 下部分区域
  else if (mY > qwY + h3 * 2) {
    if (!isAutoRun)
      mui->btnAutoRun->click();
    else if (isAutoRun)
      mui->btnAutoStop->click();
  }
}

bool Reader::getDefaultOpen() {
  QSettings Reg(privateDir + "choice_book.ini", QSettings::IniFormat);

  QString type = Reg.value("book/type", "filepicker").toString();
  if (type == "defaultopen")
    return true;
  else
    return false;
}

void Reader::setDefaultOpen(QString value) {
  QSettings Reg(privateDir + "choice_book.ini", QSettings::IniFormat);

  Reg.setValue("book/type", value);
}

void Reader::autoRun() {
  qreal a = getVPos();
  qreal h = getVHeight();

  if (!isLandscape) {
    if (a + mui->qwReader->height() >= h) mui->btnAutoStop->click();
  } else {
    if (a + mui->qwReader->width() >= h) mui->btnAutoStop->click();
  }

  a = a + scrollValue;
  setVPos(a);
}

void Reader::setTextAreaCursorPos(int nCursorPos) {
  QQuickItem* root;
  root = mui->qwReader->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "setTextAreaCursorPos",
                            Q_ARG(QVariant, nCursorPos));
}

void Reader::showOrHideBookmark() {
  mui->btnAutoStop->click();

  if (mui->f_ReaderSet->isVisible()) {
    mw_one->on_btnBackReaderSet_clicked();
  }
  if (mui->qwBookmark->isHidden()) {
    mui->qwReader->hide();
    mui->qwBookmark->show();
    showBookmarkList();
    mui->btnCatalogue->setEnabled(false);
  } else {
    mui->qwBookmark->hide();
    mui->qwReader->show();
    mui->btnCatalogue->setEnabled(true);
  }
}

void Reader::on_SetReaderFunVisible() {
  if (mui->f_ReaderFun->isHidden()) {
    mui->f_ReaderFun->show();
    mui->lblBookName->show();
  } else {
    mui->f_ReaderFun->hide();
    mui->lblBookName->hide();
    m_ReaderSet->hide();
  }

  qreal vpos = getVPos();
  qreal vh = getVHeight();
  qreal ra = vpos / vh;
  bool isTemp = isLandscape;
  mui->qwReader->setSource(QUrl(QStringLiteral("qrc:/src/qmlsrc/reader.qml")));

  setQmlLandscape(isTemp);

  if (!isLandscape) w = mui->qwReader->width();

  if (isEpub) setQMLHtml(currentHtmlFile, "", "");
  if (isText) {
    QString txt1 = updateContent();
    setQMLText(txt1);
  }

  vh = getVHeight();
  vpos = vh * ra;
  setVPos(vpos);
}

///////////////////////////////////////////////////////////////////////////////////
// TextChunkModel
///////////////////////////////////////////////////////////////////////////////////

// 1：正确初始化
TextChunkModel::TextChunkModel(QObject* parent) : QAbstractListModel(parent) {
  // 初始化角色名
  m_roleNames[TextRole] = "text";
}

void TextChunkModel::splitContent(const QString& fullText) {
  beginResetModel();
  m_chunks.clear();
  m_chunks.append(fullText);
  endResetModel();

  return;

  // 扩展正则表达式匹配范围
  static QRegularExpression regex(
      R"((</?([a-zA-Z]+)[^>]*>)(.*?)(?=</?\2[^>]*>|$))",  // 匹配完整标签结构
      QRegularExpression::CaseInsensitiveOption |
          QRegularExpression::DotMatchesEverythingOption |
          QRegularExpression::UseUnicodePropertiesOption);

  // 分阶段处理策略
  QString remainingText = fullText;
  int lastPos = 0;

  // 第一阶段：匹配完整标签块
  QRegularExpressionMatchIterator it = regex.globalMatch(remainingText);
  while (it.hasNext()) {
    QRegularExpressionMatch match = it.next();
    QString fullTagBlock = match.captured(0);

    // 验证嵌套层级
    if (isValidNesting(fullTagBlock)) {
      m_chunks.append(fullTagBlock);
      lastPos = match.capturedEnd();
    } else {
      // 处理异常情况
      handleComplexStructure(remainingText, lastPos);
    }
  }

  // 第二阶段：处理剩余文本
  if (lastPos < remainingText.length()) {
    QString remaining = remainingText.mid(lastPos);
    if (!remaining.trimmed().isEmpty()) {
      m_chunks.append(remaining);
    }
  }

  endResetModel();
}

// 辅助方法：验证标签嵌套有效性
bool TextChunkModel::isValidNesting(const QString& htmlBlock) {
  QStack<QString> tagStack;
  QRegularExpression tagRegex(R"(<(/?)([a-zA-Z]+)[^>]*>)");

  QRegularExpressionMatchIterator it = tagRegex.globalMatch(htmlBlock);
  while (it.hasNext()) {
    QRegularExpressionMatch match = it.next();
    QString tagName = match.captured(2).toLower();
    if (match.captured(1).isEmpty()) {  // 开始标签
      tagStack.push(tagName);
    } else {  // 结束标签
      if (tagStack.isEmpty() || tagStack.pop() != tagName) {
        return false;
      }
    }
  }
  return tagStack.isEmpty();
}

// 处理复杂结构（递归实现）
void TextChunkModel::handleComplexStructure(QString& text, int& currentPos) {
  QRegularExpression deepRegex(R"(<(div|section|article)\b[^>]*>)",
                               QRegularExpression::CaseInsensitiveOption);
  QRegularExpressionMatch match = deepRegex.match(text, currentPos);

  if (match.hasMatch()) {
    QString containerTag = match.captured(1);
    QString endTag = QString("</%1>").arg(containerTag);

    int start = match.capturedStart();
    int end = text.indexOf(endTag, match.capturedEnd());

    if (end != -1) {
      m_chunks.append(text.mid(start, end - start + endTag.length()));
      currentPos = end + endTag.length();
    }
  }
}

// 3：完善角色定义
QHash<int, QByteArray> TextChunkModel::roleNames() const {
  return {
      {TextRole, "text"},           // 自定义角色
      {Qt::DisplayRole, "display"}  // 保留默认角色
  };
}

// 4：正确实现数据访问
QVariant TextChunkModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.row() >= m_chunks.size()) return QVariant();

  if (role == TextRole || role == Qt::DisplayRole)
    return m_chunks.at(index.row());

  return QVariant();
}

// 5：正确清空数据
void TextChunkModel::clear() {
  beginResetModel();
  m_chunks.clear();
  endResetModel();
}

// 6：正确实现追加方法
void TextChunkModel::appendChunks(const QStringList& chunks) {
  if (chunks.isEmpty()) return;

  beginInsertRows(QModelIndex(), m_chunks.size(),
                  m_chunks.size() + chunks.size() - 1);
  m_chunks.append(chunks);
  endInsertRows();
}

int TextChunkModel::rowCount(const QModelIndex& parent) const {
  return parent.isValid() ? 0 : m_chunks.size();
}

QVariantMap TextChunkModel::get(int index) const {
  QVariantMap result;

  // 检查索引是否有效
  if (index < 0 || index >= m_chunks.size()) {
    qWarning() << "Invalid index:" << index;
    return result;  // 返回空对象
  }

  // 通过角色名填充数据（需与 roleNames() 中的定义一致）
  result["text"] = m_chunks.at(index);  // 文本数据存储在 m_chunks

  return result;
}

//////////////////////////////////////////////////////////////////////////////////////

// 解析nav文件内容，生成目录结构
QList<TocItem> Reader::parseTocFromNavFile(const QByteArray& navContent) {
  QList<TocItem> tocItems;  // 最终的目录列表
  QXmlStreamReader reader(navContent);

  // 解析XML，查找目录容器
  while (!reader.atEnd() && !reader.hasError()) {
    QXmlStreamReader::TokenType token = reader.readNext();

    // 1. 定位到目录根容器 <nav epub:type="toc">
    if (token == QXmlStreamReader::StartElement) {
      // 修正：QStringView -> QString 比较
      if (reader.name().toString() == "nav") {  // 此处添加.toString()
        // 检查是否为目录容器（epub:type="toc"）
        QXmlStreamAttributes attrs = reader.attributes();
        QString epubType = attrs.value("epub:type").toString();
        if (epubType == "toc") {
          //  进入容器后，解析内部的列表结构
          tocItems = parseOlElement(reader);
          break;  // 解析完目录后退出
        }
      }
    }
  }

  // 处理解析错误
  if (reader.hasError()) {
    qWarning() << "解析目录失败：" << reader.errorString();
  } else if (tocItems.isEmpty()) {
    qWarning() << "未在nav文件中找到有效的目录结构（<nav epub:type=\"toc\">）";
  }

  return tocItems;
}

// 递归解析 <ol> 元素（提取章节列表）
QList<TocItem> Reader::parseOlElement(QXmlStreamReader& reader) {
  QList<TocItem> items;

  while (!reader.atEnd() && !reader.hasError()) {
    QXmlStreamReader::TokenType token = reader.readNext();

    // 遇到 </ol> 则结束当前层级解析（修正比较）
    if (token == QXmlStreamReader::EndElement &&
        reader.name().toString() == "ol") {  // 此处添加.toString()
      break;
    }

    // 解析每个列表项 <li>（修正比较）
    if (token == QXmlStreamReader::StartElement &&
        reader.name().toString() == "li") {   // 此处添加.toString()
      TocItem item = parseLiElement(reader);  // 解析单个章节
      if (!item.title.isEmpty()) {
        items.append(item);
      }
    }
  }

  return items;
}

// 解析 <li> 元素（提取单个章节的标题、链接和子章节）
TocItem Reader::parseLiElement(QXmlStreamReader& reader) {
  TocItem item;

  while (!reader.atEnd() && !reader.hasError()) {
    QXmlStreamReader::TokenType token = reader.readNext();

    // 遇到 </li> 则结束当前章节解析（修正比较）
    if (token == QXmlStreamReader::EndElement &&
        reader.name().toString() == "li") {  // 此处添加.toString()
      break;
    }

    // 解析章节链接 <a> 标签（标题和href）（修正比较）
    if (token == QXmlStreamReader::StartElement &&
        reader.name().toString() == "a") {  // 此处添加.toString()
      // 提取链接（href属性）
      QXmlStreamAttributes attrs = reader.attributes();
      item.href = attrs.value("href").toString();

      // 提取标题（<a>标签内的文本）
      if (reader.readNext() == QXmlStreamReader::Characters) {
        item.title = reader.text().toString().trimmed();  // 去除首尾空格
      }
    }

    // 解析子章节（嵌套的 <ol> 元素）（修正比较）
    if (token == QXmlStreamReader::StartElement &&
        reader.name().toString() == "ol") {      // 此处添加.toString()
      item.childItems = parseOlElement(reader);  // 递归解析子章节
    }
  }

  return item;
}

// 递归打印目录项（辅助函数）
void Reader::debugPrintTocItems(const QList<TocItem>& tocItems, int level) {
  // 层级缩进（每级缩进4个空格，方便区分层级）
  QString indent(level * 4, ' ');

  for (const TocItem& item : tocItems) {
    // 打印当前目录项的标题和链接
    qDebug() << indent << "标题：" << item.title << "，链接：" << item.href;

    // 递归打印子目录（层级+1）
    if (!item.childItems.isEmpty()) {
      debugPrintTocItems(item.childItems, level + 1);
    }
  }
}

QString Reader::getEpub3Title(const QString& opfFile) {
  // 读取OPF文件内容
  QByteArray opfContent = reader->readFile(opfFile);

  // 检查文件内容是否读取成功
  if (opfContent.isEmpty()) {
    qWarning() << "无法读取OPF文件内容:" << opfFile;
    return QString();
  }

  // 创建XML解析器（注意：直接使用QByteArray作为参数，无需取地址）
  QXmlStreamReader xml(opfContent);

  // 解析XML内容
  while (!xml.atEnd() && !xml.hasError()) {
    // 读取下一个元素
    QXmlStreamReader::TokenType token = xml.readNext();

    // 只处理开始元素
    if (token == QXmlStreamReader::StartElement) {
      // 检查是否是dc:title元素
      if (isDcTitleElement(xml)) {
        // 读取元素文本内容（包含所有子节点文本）
        QString title =
            xml.readElementText(QXmlStreamReader::IncludeChildElements);
        return title;
      }
    }
  }

  // 检查是否有解析错误
  if (xml.hasError()) {
    qWarning() << "XML解析错误:" << xml.errorString()
               << "位置:" << xml.lineNumber() << "行," << xml.columnNumber()
               << "列";
  } else {
    qWarning() << "在OPF文件中未找到dc:title元素";
  }

  return QString();
}

bool Reader::isDcTitleElement(const QXmlStreamReader& xml) {
  // Dublin Core命名空间URI
  const QStringView dcNamespace = u"http://purl.org/dc/elements/1.1/"_qs;

  // 检查元素本地名称是否为"title"且命名空间是否为dc
  // 使用QStringView字面量避免类型不匹配
  return xml.name() == u"title"_qs && xml.namespaceUri() == dcNamespace;
}

QString Reader::getReadTotalTime() {
  endDateTime = QDateTime::currentDateTime();
  qint64 seconds = startDateTime.secsTo(endDateTime);
  double hours = qMax(0.0, seconds / 3600.0);
  writeTotalHours(totalHours + hours);
  QString display = QString::number(totalHours + hours, 'f', 1);
  return display;
}

void Reader::closeReader() {
  QString time = getReadTotalTime();
  qDebug() << time;

  mui->btnAutoStop->click();
  m_ReaderSet->close();
  if (mui->f_ReaderNote->isVisible()) {
    mw_one->on_btnCancelSel_clicked();
  }
  if (mui->f_ReaderSet->isVisible()) {
    mw_one->on_btnBackReaderSet_clicked();
  }

  saveReader("", false);
  savePageVPos();

  closeViewBookNote();

  cancelKeepScreenOn();

  mui->frameMain->show();
  mui->frameReader->hide();
}

void Reader::openReader() {
  if (!isOne) {
    initReader();
  }

  if (isPDF) {
    if (isAndroid) {
      mui->frameMain->hide();
      mui->frameBookList->show();

      getReadList();

      openMyPDF(fileName);
      return;
    }
  }

  mui->frameMain->hide();
  mui->frameReader->show();
  mui->f_ReaderFun->show();

  mw_one->isReaderVisible = true;
  mw_one->isMemoVisible = false;

  if (!isOne) {
    startOpenFile(fileName);
    isOne = true;
  }

  startDateTime = QDateTime::currentDateTime();
  totalHours = readTotalHours();
  keepScreenOn();
}

double Reader::readTotalHours() {
  QString path = iniDir + "reader.json";
  QFile file(path);
  if (!file.exists()) return 0.0;

  if (!file.open(QIODevice::ReadOnly)) return 0.0;

  QByteArray data = file.readAll();
  file.close();

  QJsonDocument doc = QJsonDocument::fromJson(data);
  if (doc.isNull()) return 0.0;

  QJsonObject obj = doc.object();
  return obj.value("totalHours").toDouble(0.0);
}

bool Reader::writeTotalHours(double value) {
  QString path = iniDir + "reader.json";
  QJsonObject obj;
  obj.insert("totalHours", value);

  QJsonDocument doc(obj);

  QFile file(path);
  if (!file.open(QIODevice::WriteOnly)) return false;

  file.write(doc.toJson(QJsonDocument::Indented));
  file.close();
  return true;
}

void Reader::showTextFun() {
  if (mui->f_ReaderNote->isHidden()) {
    mui->f_ReaderFun->hide();
    mui->f_ReaderNote->show();
  }
}

void Reader::showBookPageNext() {
  QQuickItem* root = mui->qwReader->rootObject();
  if (!root) {
    qWarning() << "[Reader] showBookPageNext: QML root object is null (QML not "
                  "loaded?)";
    return;
  }
  bool invokeOk = QMetaObject::invokeMethod((QObject*)root, "showBookPageNext");
  if (!invokeOk) {
    qWarning() << "[Reader] showBookPageNext: invoke QML function failed "
                  "(function not found?)";
  }
}

void Reader::showBookPageUp() {
  QQuickItem* root = mui->qwReader->rootObject();
  if (!root) {
    qWarning()
        << "[Reader] showBookPageUp: QML root object is null (QML not loaded?)";
    return;
  }
  bool invokeOk = QMetaObject::invokeMethod((QObject*)root, "showBookPageUp");
  if (!invokeOk) {
    qWarning() << "[Reader] showBookPageUp: invoke QML function failed "
                  "(function not found?)";
  }
}

void Reader::closeBookPage() {
  QQuickItem* root = mui->qwReader->rootObject();
  if (!root) {
    qWarning()
        << "[Reader] closeBookPage: QML root object is null (QML not loaded?)";
    return;
  }
  bool invokeOk = QMetaObject::invokeMethod((QObject*)root, "closeBookPage");
  if (!invokeOk) {
    qWarning() << "[Reader] closeBookPage: invoke QML function failed "
                  "(function not found?)";
  }
}

void Reader::addBookNote() {
  if (mui->editSetText->text().trimmed() == "") return;

  dlgAddBookNote = new QDialog(mw_one);
  dlgAddBookNote->setFixedSize(mw_one->geometry().width() - 2, 350);
  dlgAddBookNote->setWindowTitle(tr("Note"));

  QTextEdit* textEdit = new QTextEdit(dlgAddBookNote);
  textEdit->verticalScrollBar()->setStyleSheet(m_Method->vsbarStyleBig);
  textEdit->setAcceptRichText(false);

  initTextToolbarDynamic(dlgAddBookNote);
  EditEventFilter* editFilter =
      new EditEventFilter(textToolbarDynamic, dlgAddBookNote);
  textEdit->installEventFilter(editFilter);
  textEdit->viewport()->installEventFilter(editFilter);

  QDialogButtonBox* buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dlgAddBookNote);
  buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Ok"));
  buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

  QObject::connect(buttonBox, &QDialogButtonBox::accepted, dlgAddBookNote,
                   &QDialog::accept);
  QObject::connect(buttonBox, &QDialogButtonBox::rejected, dlgAddBookNote,
                   &QDialog::reject);

  connect(dlgAddBookNote, &QDialog::finished, this, [this](int result) {
    Q_UNUSED(result);
    closeTextToolBar();
  });

  QVBoxLayout* vlayout = new QVBoxLayout(dlgAddBookNote);
  QHBoxLayout* layout = new QHBoxLayout(dlgAddBookNote);
  vlayout->addWidget(textEdit);
  vlayout->addLayout(layout);
  vlayout->addWidget(buttonBox);

  m_Method->set_ToolButtonStyle(dlgAddBookNote);

  //----------------------------------------------

  layout->setSpacing(10);

  // 创建按钮组，设置互斥
  QButtonGroup* btnGroup = new QButtonGroup(this);
  btnGroup->setExclusive(true);

  QList<QPushButton*> colorButtons;

  QStringList colorList = {
      "#8500FF00",  // 保留：半透明纯绿（基准色，辅助标记）
      "#85FF0000",  // 半透明纯红（重点提醒/错误标记，醒目不刺眼）
      "#850000FF",  // 半透明纯蓝（关键信息/重要补充，与红色对比强烈）
      "#85FFFF00",  // 半透明纯黄（核心高亮/荧光笔平替，视觉焦点）
      "#8500FFFF",  // 半透明纯青（特殊注释/技术细节，独特不相近）
      "#85FF00FF"   // 半透明纯洋红（个性化标记/主观标注，区分度拉满）
  };

  for (const QString& colorStr : colorList) {
    QPushButton* btn = new QPushButton(this);
    btn->setFixedSize(50, 50);
    btn->setCheckable(true);

    // 设置样式
    QString style = QString(R"(
    QPushButton {
        border: 2px outset #666;
        background-color: %1;
    }
    QPushButton:checked {
        border: 2px solid red;         /* 选中时用红色边框 */
        background-color: %1;
    }
    )")
                        .arg(colorStr);

    btn->setStyleSheet(style);

    // 把原始颜色字符串存到按钮属性里
    btn->setProperty("colorCode", colorStr);

    layout->addWidget(btn);
    btnGroup->addButton(btn);

    colorButtons.append(btn);
  }

  // 默认选中第一个按钮
  if (!colorButtons.isEmpty()) {
    colorButtons.first()->setChecked(true);
  }

  // 连接信号槽（获取选中的颜色）
  strColor = "#8500FF00";  //(默认)
  connect(btnGroup,
          QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this,
          [=](QAbstractButton* btn) {
            strColor = btn->property("colorCode").toString();
          });

  //-------------------------------------------------------------------

  QRect parentRect = mw_one->geometry();
  int x = parentRect.x() + (parentRect.width() - dlgAddBookNote->width()) / 2;
  int y = parentRect.y() + 1;
  dlgAddBookNote->move(x, y);

  if (dlgAddBookNote->exec() == QDialog::Accepted) {
    QString noteText = textEdit->toPlainText();
    // 在这里处理用户输入的笔记内容

    qDebug() << strColor;
    saveReadNote(cPage, startNote, endNote, strColor, noteText,
                 mui->editSetText->text().trimmed());
    readReadNote(cPage);

    qDebug() << "Note added:" << noteText;
  } else {
    qDebug() << "Note canceled.";
  }
}

void Reader::editBookNote(int index, int page, const QString& content) {
  dlgEditBookNote = new QDialog(mw_one);
  dlgEditBookNote->setFixedSize(mw_one->geometry().width() - 2, 350);
  dlgEditBookNote->setWindowTitle(tr("Note"));

  QTextEdit* textEdit = new QTextEdit(dlgEditBookNote);
  textEdit->verticalScrollBar()->setStyleSheet(m_Method->vsbarStyleBig);

  initTextToolbarDynamic(dlgEditBookNote);
  EditEventFilter* editFilter =
      new EditEventFilter(textToolbarDynamic, dlgEditBookNote);
  textEdit->installEventFilter(editFilter);
  textEdit->viewport()->installEventFilter(editFilter);

  QDialogButtonBox* buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dlgEditBookNote);
  buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Ok"));
  buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

  QObject::connect(buttonBox, &QDialogButtonBox::accepted, dlgEditBookNote,
                   &QDialog::accept);
  QObject::connect(buttonBox, &QDialogButtonBox::rejected, dlgEditBookNote,
                   &QDialog::reject);

  connect(dlgEditBookNote, &QDialog::finished, this, [this](int result) {
    Q_UNUSED(result);
    closeTextToolBar();
  });

  QVBoxLayout* vlayout = new QVBoxLayout(dlgEditBookNote);
  QHBoxLayout* layout = new QHBoxLayout(dlgEditBookNote);
  vlayout->addWidget(textEdit);
  vlayout->addLayout(layout);
  vlayout->addWidget(buttonBox);

  m_Method->set_ToolButtonStyle(dlgEditBookNote);

  //----------------------------------------------

  layout->setSpacing(10);

  // 创建按钮组，设置互斥
  QButtonGroup* btnGroup = new QButtonGroup(this);
  btnGroup->setExclusive(true);

  QList<QPushButton*> colorButtons;

  QStringList colorList = {
      "#8500FF00",  // 保留：半透明纯绿（基准色，辅助标记）
      "#85FF0000",  // 半透明纯红（重点提醒/错误标记，醒目不刺眼）
      "#850000FF",  // 半透明纯蓝（关键信息/重要补充，与红色对比强烈）
      "#85FFFF00",  // 半透明纯黄（核心高亮/荧光笔平替，视觉焦点）
      "#8500FFFF",  // 半透明纯青（特殊注释/技术细节，独特不相近）
      "#85FF00FF"   // 半透明纯洋红（个性化标记/主观标注，区分度拉满）
  };

  for (const QString& colorStr : colorList) {
    QPushButton* btn = new QPushButton(this);
    btn->setFixedSize(50, 50);
    btn->setCheckable(true);

    // 设置样式
    QString style = QString(R"(
    QPushButton {
        border: 2px outset #666;
        background-color: %1;
    }
    QPushButton:checked {
        border: 2px solid red;         /* 选中时用红色边框 */
        background-color: %1;
    }
    )")
                        .arg(colorStr);

    btn->setStyleSheet(style);

    // 把原始颜色字符串存到按钮属性里
    btn->setProperty("colorCode", colorStr);

    layout->addWidget(btn);
    btnGroup->addButton(btn);

    colorButtons.append(btn);
  }

  // 默认选中第一个按钮
  if (!colorButtons.isEmpty()) {
    colorButtons.first()->setChecked(true);
  }

  // 连接信号槽（获取选中的颜色）
  strColor = "#8500FF00";  //(默认)
  connect(btnGroup,
          QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this,
          [=](QAbstractButton* btn) {
            strColor = btn->property("colorCode").toString();
          });

  //-------------------------------------------------------------------

  QRect parentRect = mw_one->geometry();
  int x = parentRect.x() + (parentRect.width() - dlgEditBookNote->width()) / 2;
  int y = parentRect.y() + 1;
  dlgEditBookNote->move(x, y);

  textEdit->setPlainText(content);

  if (dlgEditBookNote->exec() == QDialog::Accepted) {
    QString noteText = textEdit->toPlainText();

    int mpage = 0;
    if (page == -1)
      mpage = cPage;
    else
      mpage = page;

    updateReadNote(mpage, index, noteText, strColor);

    if (mui->qwViewBookNote->isVisible()) {
      modifyText2(currentNoteListIndex, noteText);
    }

    qDebug() << "Note added:" << noteText;
  } else {
    qDebug() << "Note canceled.";
  }
}

void Reader::viewBookNote() {
  if (mui->qwViewBookNote->source().isEmpty()) {
    mui->qwViewBookNote->rootContext()->setContextProperty("m_Reader",
                                                           mw_one->m_Reader);
    mui->qwViewBookNote->rootContext()->setContextProperty("fontSize",
                                                           fontSize);
    mui->qwViewBookNote->rootContext()->setContextProperty("isDark", isDark);
    mui->qwViewBookNote->rootContext()->setContextProperty("notesModel",
                                                           notesModel);

    mui->qwViewBookNote->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/viewbooknote.qml")));
  }

  initBookNoteValue(-1, -1);

  mui->qwReader->hide();
  mui->qwViewCate->hide();
  mui->qwBookmark->hide();
  mui->qwViewBookNote->show();
  appendNoteDataToQmlList();

  mui->btnBackReaderSet->click();
}

void Reader::closeViewBookNote() {
  if (mui->qwViewBookNote->isHidden()) return;
  mui->qwViewBookNote->hide();
  mui->qwReader->show();
}

void Reader::appendNoteDataToQmlList() {
  // 清空旧数据
  notesModel->clear();

  QString file = iniDir + "memo/readnote/" + currentBookName + ".json";

  QFile jsonFile(file);
  if (!jsonFile.open(QIODevice::ReadOnly)) {
    qWarning() << "无法打开文件:" << file;
    return;
  }

  QByteArray data = jsonFile.readAll();
  jsonFile.close();

  QJsonDocument doc = QJsonDocument::fromJson(data);
  if (doc.isNull()) {
    qWarning() << "JSON 解析失败";
    return;
  }

  QJsonObject rootObj = doc.object();

  // 遍历所有页码
  for (auto it = rootObj.begin(); it != rootObj.end(); ++it) {
    int page = it.key().toInt();  // 页码
    QJsonArray notesArray = it.value().toArray();

    // 遍历该页的所有笔记
    for (int i = 0; i < notesArray.size(); ++i) {
      QJsonObject noteObj = notesArray[i].toObject();
      QString content = noteObj["content"].toString();
      QString quote = noteObj["quote"].toString();

      // 创建 item
      QStandardItem* item = new QStandardItem();

      // 存三个字段到不同角色
      item->setData(quote, Qt::UserRole + 1);    // quote
      item->setData(page, Qt::UserRole + 2);     // page
      item->setData(content, Qt::UserRole + 3);  // content
      item->setData(i, Qt::UserRole + 4);        // 保存 page 内索引

      notesModel->appendRow(item);
    }
  }
}

void Reader::saveReadNote(int page, int start, int end, const QString& color,
                          const QString& content, const QString& quote) {
  QString file = iniDir + "memo/readnote/" + currentBookName + ".json";

  // 确保目录存在
  QDir().mkpath(QFileInfo(file).path());

  // 读取已有 JSON 数据
  QJsonDocument doc;
  if (QFile::exists(file)) {
    QFile f(file);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QByteArray data = f.readAll();
      doc = QJsonDocument::fromJson(data);
      f.close();
    }
  }

  // 如果文件为空，创建一个空对象
  if (doc.isNull()) {
    doc.setObject(QJsonObject());
  }

  QJsonObject root = doc.object();

  // 获取 page 对应的数组（如果不存在则新建）
  QJsonArray pageArray;
  if (root.contains(QString::number(page))) {
    pageArray = root[QString::number(page)].toArray();
  }

  // 创建新的笔记对象
  QJsonObject noteObj;
  noteObj["start"] = start;
  noteObj["end"] = end;
  noteObj["color"] = color;
  noteObj["content"] = content;
  noteObj["quote"] = quote;
  noteObj["timestamp"] = QDateTime::currentMSecsSinceEpoch();  // 增加时间戳

  // 追加到数组
  pageArray.append(noteObj);

  // 更新 root 对象
  root[QString::number(page)] = pageArray;
  doc.setObject(root);

  // 写回文件
  QFile f(file);
  if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
    f.write(doc.toJson(QJsonDocument::Indented));
    f.close();
  }
}

void Reader::readReadNote(int page) {
  QString file = iniDir + "memo/readnote/" + currentBookName + ".json";

  QFile f(file);
  if (!f.exists()) {
    qDebug() << "Note file not exists:" << file;
    emit notesLoaded(QVariantList());  // 发送空列表
    return;
  }

  if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "Failed to open note file:" << f.errorString();
    return;
  }

  QByteArray data = f.readAll();
  f.close();

  QJsonDocument doc = QJsonDocument::fromJson(data);
  if (doc.isNull()) {
    qDebug() << "Failed to parse JSON";
    return;
  }

  QJsonObject root = doc.object();

  // 获取 page 对应的笔记数组
  if (root.contains(QString::number(page))) {
    QJsonArray pageArray = root[QString::number(page)].toArray();

    // 转换为 QVariantList，方便 QML 接收
    QVariantList notesList;
    for (int i = 0; i < pageArray.size(); ++i) {
      QJsonObject noteObj = pageArray[i].toObject();
      QVariantMap noteMap;
      noteMap["start"] = noteObj["start"].toInt();
      noteMap["end"] = noteObj["end"].toInt();
      noteMap["color"] = noteObj["color"].toString();
      noteMap["content"] = noteObj["content"].toString();
      notesList.append(noteMap);
    }

    // TODO: 将 notesList 传递给 QML 的 notesModel
    emit notesLoaded(notesList);
  } else {
    qDebug() << "No notes for page:" << page;
    emit notesLoaded(QVariantList());  // 发送空列表
  }
}

void Reader::delReadNote(int index) {
  int page = cPage;
  QString file = iniDir + "memo/readnote/" + currentBookName + ".json";

  // 如果文件不存在，直接返回
  if (!QFile::exists(file)) {
    qDebug() << "Note file not exists:" << file;
    return;
  }

  // 打开并读取 JSON
  QFile f(file);
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "Failed to open note file for reading:" << f.errorString();
    return;
  }

  QByteArray data = f.readAll();
  f.close();

  QJsonDocument doc = QJsonDocument::fromJson(data);
  if (doc.isNull()) {
    qDebug() << "Failed to parse JSON";
    return;
  }

  QJsonObject root = doc.object();

  // 检查 page 是否存在
  QString pageKey = QString::number(page);
  if (!root.contains(pageKey)) {
    qDebug() << "No notes for page:" << page;
    return;
  }

  QJsonArray notesArray = root[pageKey].toArray();

  // 检查 index 是否有效
  if (index < 0 || index >= notesArray.size()) {
    qDebug() << "Invalid note index:" << index;
    return;
  }

  // 删除指定索引的笔记
  notesArray.removeAt(index);

  // 如果删除后该 page 没有笔记，可将其从 JSON 中移除（可选）
  if (notesArray.isEmpty()) {
    root.remove(pageKey);
  } else {
    root[pageKey] = notesArray;
  }

  // 写回文件
  if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qDebug() << "Failed to open note file for writing:" << f.errorString();
    return;
  }

  doc.setObject(root);
  f.write(doc.toJson(QJsonDocument::Indented));
  f.close();

  qDebug() << "Note at index" << index << "on page" << page
           << "has been deleted.";

  // 刷新 QML 中的笔记模型(备选)
  // readReadNote(page);
}

void Reader::updateReadNote(int page, int index, const QString& content,
                            const QString& color) {
  QString file = iniDir + "memo/readnote/" + currentBookName + ".json";

  // 文件不存在则无法更新
  if (!QFile::exists(file)) {
    qDebug() << "Note file not exists:" << file;
    return;
  }

  // 读取 JSON 文件
  QFile f(file);
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "Failed to open note file for reading:" << f.errorString();
    return;
  }

  QByteArray data = f.readAll();
  f.close();

  QJsonDocument doc = QJsonDocument::fromJson(data);
  if (doc.isNull()) {
    qDebug() << "Failed to parse JSON";
    return;
  }

  QJsonObject root = doc.object();
  QString pageKey = QString::number(page);

  // 如果 page 不存在，直接返回
  if (!root.contains(pageKey)) {
    qDebug() << "No notes for page:" << page;
    return;
  }

  QJsonArray notesArray = root[pageKey].toArray();

  // 检查 index 是否有效
  if (index < 0 || index >= notesArray.size()) {
    qDebug() << "Invalid note index:" << index;
    return;
  }

  // 更新
  QJsonObject noteObj = notesArray[index].toObject();
  noteObj["content"] = content;
  noteObj["color"] = color;
  notesArray.replace(index, noteObj);

  // 写回 JSON
  root[pageKey] = notesArray;
  doc.setObject(root);

  if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qDebug() << "Failed to open note file for writing:" << f.errorString();
    return;
  }

  f.write(doc.toJson(QJsonDocument::Indented));
  f.close();

  qDebug() << "Note at index" << index << "on page" << page
           << "has been updated.";

  // 刷新 QML 模型
  if (page == cPage) {
    readReadNote(page);
  }
}

void Reader::initBookNoteValue(int cindex, int cpage) {
  QQuickItem* root = mui->qwViewBookNote->rootObject();
  if (!root) {
    qWarning("Error: QML root object not found!");
    return;
  }

  // 调用 QML 函数 initValue(cindex, cpage)
  QMetaObject::invokeMethod(root, "initValue",
                            Qt::DirectConnection,  // 连接方式：同步调用
                            Q_ARG(QVariant, cindex), Q_ARG(QVariant, cpage));
}

void Reader::setShowNoteValue(bool value) { isShowNote = value; }

void Reader::setNoteListCurrentIndexValue(int value) {
  currentNoteListIndex = value;
}

void Reader::modifyText2(int currentIndex, const QString& text) {
  QQuickItem* root = mui->qwViewBookNote->rootObject();
  if (!root) {
    qWarning("Error: QML root object not found!");
    return;
  }

  // 调用 QML 函数 modifyText2(currentIndex, text)
  QVariant returnValue;
  QMetaObject::invokeMethod(
      root, "modifyText2", Q_RETURN_ARG(QVariant, returnValue),
      Q_ARG(QVariant, currentIndex), Q_ARG(QVariant, text));

  qDebug() << "QML modifyText2 called, return value:" << returnValue;
}

void Reader::keepScreenOn() {
#ifdef Q_OS_ANDROID
  // 获取当前 Android Activity
  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  if (activity.isValid()) {
    QJniObject::callStaticMethod<void>(
        "com/x/MyActivity",           // Java 类路径（注意用 / 代替 .）
        "keepScreenOn",               // 方法名
        "(Landroid/app/Activity;)V",  // 方法签名
        activity.object<jobject>());
  } else {
    qWarning() << "keepScreenOn: activity is invalid";
  }
#endif
}

void Reader::cancelKeepScreenOn() {
#ifdef Q_OS_ANDROID

  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  if (activity.isValid()) {
    QJniObject::callStaticMethod<void>("com/x/MyActivity", "cancelKeepScreenOn",
                                       "(Landroid/app/Activity;)V",
                                       activity.object<jobject>());
  } else {
    qWarning() << "cancelKeepScreenOn: activity is invalid";
  }

#endif
}

void Reader::resetTextSelection() {
  QQuickItem* root = mui->qwReader->rootObject();
  if (!root) {
    qWarning() << "QML rootObject is null, cannot reset selection.";
    return;
  }

  QVariant result;
  bool success = QMetaObject::invokeMethod(root, "resetTextSelection",
                                           Q_RETURN_ARG(QVariant, result));

  if (success) {
    qDebug() << "调用 QML 重置选择函数成功";
  } else {
    qWarning() << "调用 QML 重置选择函数失败，可能函数名错误或对象不存在";
  }

  setBookPagePressHold(false);
}

void Reader::setBookPagePressHold(bool value) {
  QQuickItem* root = mui->qwReader->rootObject();
  if (!root) {
    qWarning() << "QML rootObject is null, cannot set book page press hold.";
    return;
  }

  QVariant result;
  bool success = QMetaObject::invokeMethod(root, "setBookPagePressHold",
                                           Q_RETURN_ARG(QVariant, result),
                                           Q_ARG(QVariant, value));

  if (success) {
    qDebug() << "调用 QML setBookPagePressHold 成功，值已设为" << value;
  } else {
    qWarning()
        << "调用 QML setBookPagePressHold 失败，可能函数名错误或对象不存在";
  }
}

void Reader::setEditText(const QString& txt, const QString& direction) {
  mui->editSetText->setText(txt);
  if (direction == "left")
    mui->editSetText->setCursorPosition(0);
  else
    mui->editSetText->setCursorPosition(mui->editSetText->text().length());
}

void Reader::setStartEnd(int start, int end) {
  startNote = start;
  endNote = end;
}
