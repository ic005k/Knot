#include "Reader.h"

#include <QKeyEvent>

#include "src/MainWindow.h"
#include "ui_MainWindow.h"

extern MainWindow *mw_one;
extern Ui::MainWindow *mui;
extern Method *m_Method;
extern ReaderSet *m_ReaderSet;

extern QString iniFile, iniDir, privateDir;
extern bool isZH_CN, isAndroid, isIOS, isEBook, isReport;
extern int fontSize;

extern int deleteDirfile(QString dirName);
extern QString loadText(QString textFile);
extern QString getTextEditLineText(QTextEdit *txtEdit, int i);
extern void TextEditToFile(QTextEdit *txtEdit, QString fileName);
extern bool StringToFile(QString buffers, QString fileName);
extern bool unzipToDir(const QString &zipPath, const QString &destDir);

bool isOpen = false;
bool isEpub, isText, isPDF, isEpubError;

QStringList readTextList, htmlFiles, tempHtmlList, ncxList;
QString strOpfPath, strOpfFile, oldOpfPath, fileName, ebookFile, strTitle,
    catalogueFile, strShowMsg, strEpubTitle, strPercent;

int sPos, totallines, totalPages, currentPage;
int baseLines = 50;
int htmlIndex = 0;
int minBytes = 200000;
int maxBytes = 400000;

int zlibMethod = 1;
int readerFontSize = 18;
int epubFileMethod = 2;

QByteArray bookFileData;

static int press_x;
static int press_y;
static int relea_x;
static int relea_y;

EpubReader *reader = nullptr;

Reader::Reader(QWidget *parent) : QDialog(parent) {
  qmlRegisterType<TextChunkModel>("EBook.Models", 1, 0, "TextChunkModel");

  this->installEventFilter(this);

  if (!isAndroid) mui->btnShareBook->hide();

  mui->btnAutoStop->hide();

  mui->btnBackDir->hide();
  mui->lblTitle->hide();
  mui->f_ReaderFun2->hide();
  mui->btnBackward->hide();
  mui->btnForward->hide();
  mui->textBrowser->hide();
  mui->lblCataInfo->hide();
  mui->lblCataInfo->adjustSize();
  mui->lblCataInfo->setWordWrap(true);
  mui->lblBookName->adjustSize();
  mui->lblBookName->setWordWrap(true);

  mui->textBrowser->horizontalScrollBar()->hide();
  mui->textBrowser->verticalScrollBar()->hide();

  QPalette pt = palette();
  pt.setBrush(QPalette::Text, Qt::black);
  pt.setBrush(QPalette::Base, QColor(235, 235, 235));
  pt.setBrush(QPalette::Highlight, Qt::red);
  pt.setBrush(QPalette::HighlightedText, Qt::white);
  mui->textBrowser->setPalette(pt);

  mui->btnPageNext->setStyleSheet("border:none");
  mui->btnPageUp->setStyleSheet("border:none");
  mui->btnSelText->setStyleSheet("border:none");

  mui->btnPageNext->hide();
  mui->btnPageUp->hide();
  mui->btnSelText->hide();
  mui->btnPages->setStyleSheet(
      "color: rgb(0, 0, 0);background-color: rgb(254, 234, 112);border: "
      "0px solid "
      "rgb(255,0,0);border-radius: 4px;"
      "font-weight: bold;");

  QFont f = this->font();
  f.setPointSize(10);
  f.setBold(true);
  mui->btnPages->setFont(f);

  tmeShowEpubMsg = new QTimer(mw_one);
  connect(tmeShowEpubMsg, SIGNAL(timeout()), this, SLOT(showEpubMsg()));

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

bool Reader::eventFilter(QObject *obj, QEvent *evn) {
  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
    }
  }

  return QWidget::eventFilter(obj, evn);
}

void Reader::keyReleaseEvent(QKeyEvent *event) { Q_UNUSED(event); }

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

  QPalette pt = palette();
  pt.setBrush(QPalette::Text, textColor);
  pt.setBrush(QPalette::Base, baseColor);
  pt.setBrush(QPalette::Highlight, Qt::red);
  pt.setBrush(QPalette::HighlightedText, Qt::white);
  mui->textBrowser->setPalette(pt);
}

void Reader::startOpenFile(QString openfile) {
  if (isReport) return;

  if (isAndroid) {
    closeMyPDF();
  }

  isEpubError = false;
  strShowMsg = "";
  strPercent = "";
  mui->lblEpubInfo->setFixedWidth(36);
  mui->pEpubProg->setMaximum(100);

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

    tmeShowEpubMsg->start(100);

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
  mui->lblEpubInfo->setFont(font);
  mui->pEpubProg->setFont(font);
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

      /* Method 2 */
      /*QStringList l0 = str0.split(" ");
      for (int j = 0; j < l0.count(); j++) {
        QString s0 = l0.at(j);
        if (s0.contains("href") && s0.contains("=")) {
          QString s1 = s0.split("=").at(1);
          s1 = s1.trimmed();
          s1.replace("\"", "");
          str1 = s1.trimmed();
          // qDebug() << "id=" << idref << "href=" << str1;
          return str1;
          break;
        }
      }*/

      break;
    }
  }

  return "";
}

void Reader::saveReader(QString BookmarkText, bool isSetBookmark) {
  m_ReaderSet->saveScrollValue();

  QSettings Reg(privateDir + "bookini/" + currentBookName + ".ini",
                QSettings::IniFormat);

  QString bookmarkSn;
  if (isSetBookmark) {
    int countBookmark = Reg.value("/Bookmark/count", 0).toInt();
    countBookmark = countBookmark + 1;
    Reg.setValue("/Bookmark/count", countBookmark);
    bookmarkSn = QString::number(countBookmark - 1);
  }

  if (isText) {
    if (isSetBookmark) {
      Reg.setValue("/Bookmark/currentPage" + bookmarkSn, currentPage - 1);
      Reg.setValue("/Bookmark/Name" + bookmarkSn, BookmarkText);
      Reg.setValue("/Bookmark/VPos" + bookmarkSn, getVPos());

    } else {
      Reg.setValue("/Reader/currentPage", currentPage - 1);
    }
  }

  if (isEpub) {
    if (isSetBookmark) {
      Reg.setValue("/Bookmark/htmlIndex" + bookmarkSn, htmlIndex);
      Reg.setValue("/Bookmark/Name" + bookmarkSn, BookmarkText);
      Reg.setValue("/Bookmark/VPos" + bookmarkSn, getVPos());
    } else {
      Reg.setValue("/Reader/htmlIndex", htmlIndex);
      // dir
      Reg.setValue("/Reader/MainDirIndex", mainDirIndex);
    }
  }

  if (isSetBookmark) {
  } else {
    // book list
    QSettings Reg1(privateDir + "reader.ini", QSettings::IniFormat);

    Reg1.setValue("/Reader/FileName", fileName);
    Reg1.setValue("/Reader/FontSize", readerFontSize);
    Reg1.setValue("/Reader/BookCount", bookList.count());
    for (int i = 0; i < bookList.count(); i++) {
      Reg1.setValue("/Reader/BookSn" + QString::number(i), bookList.at(i));
    }
  }
}

void Reader::initReader() {
  QSettings Reg(privateDir + "reader.ini", QSettings::IniFormat);

  readerStyle = Reg.value("/Reader/Style", "1").toString();
  scrollValue = Reg.value("/Reader/ScrollValue", "1").toReal();
  QString value = QString::number(scrollValue, 'f', 1);
  mui->lblSpeed->setText(tr("Scroll Speed") + " : " + value);

  QFont font;
  int fsize = Reg.value("/Reader/FontSize", 18).toInt();
  readerFontSize = fsize;
  mui->qwReader->rootContext()->setContextProperty("FontSize", fsize);
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
    if (fi.suffix().toLower() != "pdf") {
      if (m_Method->getExecDone() == "true") {
        startOpenFile(fileName);
      } else {
        if (m_Method->getKeyType() != "defaultopen") startOpenFile(fileName);
      }

    } else
      isPDF = true;
  } else {
    startOpenFile(fileName);
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

  // loadQMLText(currentTxt);

  QQuickItem *root = mui->qwReader->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "loadHtmlBuffer",
                            Q_ARG(QVariant, currentTxt));

  setAni();
}

void Reader::loadQMLText(QString str) {
  if (isText || isEpub) {
    QQuickItem *root = mui->qwReader->rootObject();
    QMetaObject::invokeMethod((QObject *)root, "loadText",
                              Q_ARG(QVariant, str));
  }
}

QString Reader::getQMLText() {
  QVariant str;
  QQuickItem *root = mui->qwReader->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "getText",
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
    if (list.count() == 2) str1 = list.at(0);
    QStringList list2 = str1.split("/");
    if (list2.count() > 0) {
      str1 = list2.at(list2.count() - 1);
    }

    if (str.contains(str1)) {
      setEpubPagePosition(i, htmlFile);
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
  QQuickItem *root = mui->qwReader->rootObject();

  QMetaObject::invokeMethod((QObject *)root, "loadHtmlBuffer",
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

void Reader::closeEvent(QCloseEvent *event) {
  Q_UNUSED(event);
  saveReader("", false);
  savePageVPos();
}

void Reader::paintEvent(QPaintEvent *event) { Q_UNUSED(event); }

void Reader::goBookReadPosition() {
  if (isOpen) {
    QSettings Reg(privateDir + "bookini/" + currentBookName + ".ini",
                  QSettings::IniFormat);

    if (isText) {
      currentPage = Reg.value("/Reader/currentPage", -1).toULongLong();
      goNextPage();
      showInfo();
    }

    if (isEpub) {
      htmlIndex = Reg.value("/Reader/htmlIndex", 0).toInt();

      if (htmlIndex >= htmlFiles.count()) {
        htmlIndex = 0;
      }

      currentHtmlFile = htmlFiles.at(htmlIndex);
      setQMLHtml(currentHtmlFile, "", "");

      setPageVPos();
      showInfo();

      qreal pos = getVPos();
      setVPos(pos + 0.01);
    }
  }
}

void Reader::setFontSize(int fontSize) {
  qreal pos1 = getVPos();
  qreal h1 = getVHeight();

  mui->qwReader->rootContext()->setContextProperty("FontSize", fontSize);

  qreal h2 = getVHeight();
  qreal pos2 = getNewVPos(pos1, h1, h2);
  setVPos(pos2);
  textPos = pos2;
}

void Reader::PlainTextEditToFile(QPlainTextEdit *txtEdit, QString fileName) {
  QFile *file;
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

void Reader::savePageVPos() {
  QSettings Reg(privateDir + "bookini/" + currentBookName + ".ini",
                QSettings::IniFormat);

  QFileInfo fiHtml(currentHtmlFile);
  textPos = getVPos();
  if (isEpub) {
    if (mui->qwCata->isVisible()) {
      Reg.setValue("/Reader/vpos  CataVPos", textPos);
      int index = m_Method->getCurrentIndexFromQW(mui->qwCata);
      Reg.setValue("/Reader/vpos  CataIndex", index);
    } else {
      if (htmlIndex >= 0)
        Reg.setValue("/Reader/vpos" + fiHtml.baseName(), textPos);
    }
  }

  if (isText) {
    Reg.setValue("/Reader/vpos" + QString::number(currentPage), textPos);
  }
}

void Reader::setPageVPos() {
  QSettings Reg(privateDir + "bookini/" + currentBookName + ".ini",
                QSettings::IniFormat);

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
    }
  }

  if (isText) {
    textPos =
        Reg.value("/Reader/vpos" + QString::number(currentPage), 0).toReal();
  }

  setVPos(textPos);
}

void Reader::setVPos(qreal pos) {
  QQuickItem *root;
  if (mui->qwCata->isVisible())
    root = mui->qwCata->rootObject();
  else
    root = mui->qwReader->rootObject();

  QMetaObject::invokeMethod((QObject *)root, "setVPos", Q_ARG(QVariant, pos));
}

qreal Reader::getVPos() {
  QVariant itemCount;

  QQuickItem *root;
  if (mui->qwCata->isVisible())
    root = mui->qwCata->rootObject();
  else
    root = mui->qwReader->rootObject();

  QMetaObject::invokeMethod((QObject *)root, "getVPos",
                            Q_RETURN_ARG(QVariant, itemCount));
  textPos = itemCount.toDouble();
  return textPos;
}

QString Reader::getBookmarkText() {
  QVariant item;
  QQuickItem *root = mui->qwReader->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "getBookmarkText",
                            Q_RETURN_ARG(QVariant, item));
  return item.toString();
}

qreal Reader::getVHeight() {
  QVariant itemCount;
  QQuickItem *root = mui->qwReader->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "getVHeight",
                            Q_RETURN_ARG(QVariant, itemCount));
  textHeight = itemCount.toDouble();
  return textHeight;
}

qreal Reader::getNewVPos(qreal pos1, qreal h1, qreal h2) {
  qreal pos2;
  pos2 = pos1 * h2 / h1;
  return pos2;
}

void Reader::showInfo() {
  if (isText) {
    mui->btnPages->setText(QString::number(currentPage + 1) + "\n" +
                           QString::number(totalPages));
    mui->progReader->setMaximum(totalPages);
    mui->progReader->setValue(currentPage + 1);
  }

  if (isEpub) {
    mui->btnPages->setText(QString::number(htmlIndex + 1) + "\n" +
                           QString::number(htmlFiles.count()));
    mui->progReader->setMaximum(htmlFiles.count());
    mui->progReader->setValue(htmlIndex + 1);
  }

  m_ReaderSet->updateProgress();
}

void Reader::SplitFile(QString qfile) {
  QTextEdit *text_edit = new QTextEdit;
  QPlainTextEdit *plain_edit = new QPlainTextEdit;
  QPlainTextEdit *plain_editHead = new QPlainTextEdit;

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
  QDir *dir = new QDir(path);
  QStringList filter;
  filter << "*.ncx";
  dir->setNameFilters(filter);
  QList<QFileInfo> *fileInfo = new QList<QFileInfo>(dir->entryInfoList(filter));
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

  QDir *dir = new QDir(imgdir);
  QStringList filter;
  filter << "*.png"
         << "*.jpg"
         << "*.jpeg"
         << "*.bmp"
         << "*.svg";
  dir->setNameFilters(filter);
  QList<QFileInfo> *fileInfo = new QList<QFileInfo>(dir->entryInfoList(filter));
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
  ShowMessage *m_ShowMsg = new ShowMessage(this);
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
bool Reader::copyDirectoryFiles(const QString &fromDir, const QString &toDir,
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

QString Reader::getNavFileInternalPath(const QByteArray &opfContent) {
  // 先将QByteArray转为QString（按UTF-8编码，EPUB标准编码）
  QString opfStr = QString::fromUtf8(opfContent);

  // 简单处理：直接查找包含 'id="nav"' 的item标签
  int navItemStart = opfStr.indexOf("id=\"nav\"");
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

    // debugPrintTocItems(tocItems, 0);

    return htmlList;
  }

  if (!reader->fileExists(ncxFile)) {
    return htmlList;
  }

  QPlainTextEdit *plain_edit = new QPlainTextEdit;
  plain_edit->appendPlainText("<html>");
  plain_edit->appendPlainText("<body>");
  plain_edit->appendPlainText("<style>.my-link {color: #336699;} </style>");

  QTextEdit *text_edit = new QTextEdit;
  QTextEdit *text_edit0 = new QTextEdit;

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

  // TextEditToFile(text_edit, privateDir + "ncx_test.txt");

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
  QTextBrowser *textBrowser = new QTextBrowser();
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

void Reader::showEpubMsg() {
  if (strShowMsg != "") {
    mui->lblEpubInfo->show();
    mui->pEpubProg->show();
    if (strPercent != "") {
      mui->lblEpubInfo->setText(strPercent + "% ");
    }
    mui->pEpubProg->setValue(strPercent.toInt());
    mui->pEpubProg->setFormat(strShowMsg);
  }
}

void Reader::removeBookList() {
  int index = m_Method->getCurrentIndexFromQW(mui->qwBookList);
  if (index <= 0) return;

  ShowMessage *msg = new ShowMessage(mw_one);
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
    tmeShowEpubMsg->stop();

    mui->lblEpubInfo->hide();
    mui->pEpubProg->hide();
    mui->btnReader->setEnabled(true);
    mui->f_ReaderFun->setEnabled(true);
    mw_one->closeProgress();

    ShowMessage *msg = new ShowMessage(mw_one);
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
  QString extName = fi.suffix();
  currentBookName = currentBookName + "_" + extName;

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
    mui->qwReader->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/reader.qml")));

    if (isEpub) {
      mui->lblInfo->show();
      mui->qwReader->rootContext()->setContextProperty("htmlPath", strOpfPath);
      if (QFile(catalogueFile).exists()) {
        mui->btnCatalogue->show();
      } else
        mui->btnCatalogue->hide();
    }

    if (isText) {
      mui->btnBackDir->hide();
      mui->btnCatalogue->hide();
      mui->lblInfo->hide();
    }
  }

  mui->lblBookName->setText(strTitle);
  mui->btnBackDir->hide();
  tmeShowEpubMsg->stop();
  mui->lblEpubInfo->hide();
  mui->pEpubProg->hide();

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
    mui->btnSelText->setIcon(QIcon(":/res/choice1.png"));
    isSelText = true;

    mui->textBrowser->setReadOnly(true);
    QFont font;
    font.setPixelSize(readerFontSize);
    font.setFamily(mui->btnFont->font().family());
    font.setLetterSpacing(QFont::AbsoluteSpacing, 2);
    mui->textBrowser->setFont(font);

    mui->textBrowser->setHtml(currentTxt);

    mui->qwReader->hide();
    mui->textBrowser->show();

    qreal h0 = getVHeight();
    qreal h1 = mui->textBrowser->document()->size().height();
    qreal s0 = getVPos();
    qreal s1 = (s0 * h1) / h0;
    mui->textBrowser->verticalScrollBar()->setSliderPosition(s1);
    qDebug() << "s0=" << s0 << "h0=" << h0 << "s1=" << s1 << "h1=" << h1;

    mw_one->mydlgSetText->setFixedWidth(mw_one->width() - 4);
    mw_one->mydlgSetText->init(
        mw_one->geometry().x() +
            (mw_one->width() - mw_one->mydlgSetText->width()) / 2,
        mw_one->geometry().y(), mw_one->mydlgSetText->width(),
        mw_one->mydlgSetText->height());

  } else {
    closeSelText();
  }
}

void Reader::closeSelText() {
  if (isSelText) {
    isSelText = false;
    mui->btnSelText->setIcon(QIcon(":/res/choice0.png"));
    mui->textBrowser->hide();
    mui->qwReader->show();
    mw_one->mydlgSetText->close();
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
  QSettings Reg(privateDir + "bookini/" + currentBookName + ".ini",
                QSettings::IniFormat);

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
  QSettings Reg(privateDir + "bookini/" + currentBookName + ".ini",
                QSettings::IniFormat);

  if (isText) {
    currentPage =
        Reg.value("/Bookmark/currentPage" + QString::number(index)).toInt();
    goNextPage();
    textPos = Reg.value("/Bookmark/VPos" + QString::number(index)).toReal();
  }

  if (isEpub) {
    htmlIndex =
        Reg.value("/Bookmark/htmlIndex" + QString::number(index)).toInt();
    textPos = Reg.value("/Bookmark/VPos" + QString::number(index)).toReal();
    if (htmlIndex >= htmlFiles.count()) {
      htmlIndex = 0;
    }

    currentHtmlFile = htmlFiles.at(htmlIndex);
    setQMLHtml(currentHtmlFile, "", "");

    showInfo();
  }
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

void Reader::setPanelVisible() { mw_one->on_SetReaderFunVisible(); }

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

bool Reader::eventFilterReader(QObject *watch, QEvent *evn) {
  QMouseEvent *event = static_cast<QMouseEvent *>(evn);
  if (watch == mui->qwReader) {
    int length = 75;

    if (mui->textBrowser->isHidden()) {
      if (event->type() == QEvent::MouseButtonPress) {
        mw_one->isMousePress = true;
        mw_one->isMouseMove = false;
        if (!mw_one->isMouseMove) mw_one->timerMousePress->start(1300);
      }

      if (event->type() == QEvent::MouseButtonRelease) {
        mw_one->isMousePress = false;
      }

      if (event->type() == QEvent::MouseMove) {
        mw_one->isMouseMove = true;

        if (mw_one->isMousePress) {
          if ((relea_x - press_x) > length && qAbs(relea_y - press_y) < 35) {
            // qDebug() << "book right...";
            int cn = mui->btnPages->text().split("\n").at(0).toInt();
            if (cn != 1) {
              mw_one->m_PageIndicator->setPicRight();
            }
          } else if ((press_x - relea_x) > length &&
                     qAbs(relea_y - press_y) < 35) {
            // qDebug() << "book left...";
            int cn = mui->btnPages->text().split("\n").at(0).toInt();
            int tn = mui->btnPages->text().split("\n").at(1).toInt();
            if (cn != tn) {
              mw_one->m_PageIndicator->setPicLeft();
            }
          } else
            mw_one->m_PageIndicator->close();
        }
      }
    }

    if (event->type() == QEvent::MouseButtonPress) {
      mw_one->isMousePress = true;

      press_x = event->globalPosition().x();
      press_y = event->globalPosition().y();
      x = 0;
      y = 0;
      w = mui->qwReader->width();
      h = mui->qwReader->height();
    }

    if (event->type() == QEvent::MouseButtonDblClick) {
      if (m_Method->isClickLink) {
        m_Method->isClickLink = false;
      }

      int h3 = mui->qwReader->height() / 3;
      int mY = event->globalPosition().y();
      int qwY = mui->qwReader->y();

      if ((mY > qwY + h3) && (mY < qwY + h3 * 2)) {
        mw_one->on_SetReaderFunVisible();
      }

      if ((mY > qwY) && (mY < qwY + h3)) {
        mw_one->m_Reader->setPageScroll0();
      }

      if (mY > qwY + h3 * 2) {
        mw_one->m_Reader->setPageScroll1();
      }
    }

    if (event->type() == QEvent::MouseButtonRelease) {
      QPointF globalPos = event->globalPosition();
      relea_x = globalPos.x();  // 提取 x 坐标（整数或浮点数，根据需求转换）
      relea_y = globalPos.y();  // 提取 y 坐标
      mui->lblTitle->hide();
      QQuickItem *root = mui->qwReader->rootObject();

      mw_one->isTurnThePage = false;
      mw_one->isMousePress = false;

      // Right Slide
      if ((relea_x - press_x) > length && qAbs(relea_y - press_y) < 35) {
        if (isText) {
          if (currentPage == 0) {
            if (isText || isEpub) {
              QMetaObject::invokeMethod((QObject *)root, "setX",
                                        Q_ARG(QVariant, 0));
              return QWidget::eventFilter(watch, evn);
            }
          }
        } else if (isEpub) {
          if (htmlIndex <= 0) {
            if (isText || isEpub) {
              QMetaObject::invokeMethod((QObject *)root, "setX",
                                        Q_ARG(QVariant, 0));
              return QWidget::eventFilter(watch, evn);
            }
          }
        }
        mw_one->isTurnThePage = true;

        mw_one->on_btnPageUp_clicked();
        mw_one->m_PageIndicator->close();
      }

      // Left Slide
      if ((press_x - relea_x) > length && qAbs(relea_y - press_y) < 35) {
        if (isText) {
          if (currentPage == totalPages - 1) {
            if (isText || isEpub) {
              QMetaObject::invokeMethod((QObject *)root, "setX",
                                        Q_ARG(QVariant, 0));
              return QWidget::eventFilter(watch, evn);
            }
          }
        } else if (isEpub) {
          if (htmlIndex + 1 >= htmlFiles.count()) {
            if (isText || isEpub) {
              QMetaObject::invokeMethod((QObject *)root, "setX",
                                        Q_ARG(QVariant, 0));
              return QWidget::eventFilter(watch, evn);
            }
          }
        }
        mw_one->isTurnThePage = true;

        mw_one->on_btnPageNext_clicked();
        mw_one->m_PageIndicator->close();
      }

      if (isText || isEpub)
        QMetaObject::invokeMethod((QObject *)root, "setX", Q_ARG(QVariant, 0));

      mw_one->curx = 0;
    }

    if (event->type() == QEvent::MouseMove) {
      QPointF globalPos = event->globalPosition();
      relea_x = globalPos.x();  // 提取 x 坐标（整数或浮点数，根据需求转换）
      relea_y = globalPos.y();  // 提取 y 坐标
      if (mw_one->isMousePress && qAbs(relea_x - press_x) > 20 &&
          qAbs(relea_y - press_y) < 20) {
        mw_one->isMouseMove = true;
      }

      mw_one->isMouseMove = true;
    }
  }

  return QWidget::eventFilter(watch, evn);
}

bool Reader::eventFilterReaderAndroid(QObject *watch, QEvent *evn) {
  // 处理触摸事件（Android）
  if (evn->type() == QEvent::TouchBegin || evn->type() == QEvent::TouchUpdate ||
      evn->type() == QEvent::TouchEnd || evn->type() == QEvent::TouchCancel) {
    QTouchEvent *touchEvent = static_cast<QTouchEvent *>(evn);
    const QList<QTouchEvent::TouchPoint> &touchPoints = touchEvent->points();

    // 只处理单指触摸
    if (touchPoints.count() == 1 && watch == mui->qwReader) {
      const QTouchEvent::TouchPoint &touchPoint = touchPoints.first();

      // 将触摸点位置转换为全局坐标
      QPointF globalPos =
          mui->qwReader->mapToGlobal(touchPoint.position().toPoint());

      // 使用正确的枚举类型和值
      auto state = touchPoint.state();
      if (state == QEventPoint::Pressed) {
        return handleTouchPress(globalPos);
      } else if (state == QEventPoint::Updated) {
        return handleTouchMove(globalPos);
      } else if (state == QEventPoint::Released) {
        return handleTouchRelease(globalPos);
      } else {
        mw_one->isMousePress = false;
        return true;
      }
    }
    return false;
  }

  // 处理桌面端的鼠标事件（保留原有逻辑）
  if (evn->type() == QEvent::MouseButtonPress ||
      evn->type() == QEvent::MouseMove ||
      evn->type() == QEvent::MouseButtonRelease ||
      evn->type() == QEvent::MouseButtonDblClick) {
    QMouseEvent *event = static_cast<QMouseEvent *>(evn);
    if (watch == mui->qwReader) {
      Q_UNUSED(event);
      // ... 保留原有的桌面端鼠标事件处理代码 ...
      // 这里放置原有的鼠标事件处理逻辑
      // 为了简洁，此处省略重复代码
    }
  }

  return QWidget::eventFilter(watch, evn);
}

bool Reader::handleTouchPress(const QPointF &globalPos) {
  // 记录按下位置和时间
  static qint64 lastPressTime = 0;
  qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

  // 双击检测（300ms内两次按下）
  if (currentTime - lastPressTime < 300) {
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
    mw_one->timerMousePress->start(1300);
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

bool Reader::handleTouchMove(const QPointF &globalPos) {
  // 更新当前位置
  relea_x = globalPos.x();
  relea_y = globalPos.y();

  // 设置移动状态
  mw_one->isMouseMove = true;

  // 检测滑动方向（在按下状态下）
  if (mw_one->isMousePress) {
    int length = 75;

    // 向右滑动
    if ((relea_x - press_x) > length && qAbs(relea_y - press_y) < 35) {
      int cn = mui->btnPages->text().split("\n").at(0).toInt();
      if (cn != 1) {
        mw_one->m_PageIndicator->setPicRight();
      }
    }
    // 向左滑动
    else if ((press_x - relea_x) > length && qAbs(relea_y - press_y) < 35) {
      int cn = mui->btnPages->text().split("\n").at(0).toInt();
      int tn = mui->btnPages->text().split("\n").at(1).toInt();
      if (cn != tn) {
        mw_one->m_PageIndicator->setPicLeft();
      }
    }
    // 其他情况
    else {
      mw_one->m_PageIndicator->close();
    }
  }

  // 检测是否有效移动
  if (mw_one->isMousePress && qAbs(relea_x - press_x) > 20 &&
      qAbs(relea_y - press_y) < 20) {
    mw_one->isMouseMove = true;
  }

  return true;
}

bool Reader::handleTouchRelease(const QPointF &globalPos) {
  // 更新位置和状态
  relea_x = globalPos.x();
  relea_y = globalPos.y();

  mui->lblTitle->hide();
  mw_one->isMousePress = false;
  mw_one->isTurnThePage = false;

  QQuickItem *root = mui->qwReader->rootObject();
  int length = 75;

  // 向右滑动结束
  if ((relea_x - press_x) > length && qAbs(relea_y - press_y) < 35) {
    if (isText) {
      if (currentPage == 0) {
        if (isText || isEpub) {
          QMetaObject::invokeMethod(root, "setX", Q_ARG(QVariant, 0));
          return true;
        }
      }
    } else if (isEpub) {
      if (htmlIndex <= 0) {
        if (isText || isEpub) {
          QMetaObject::invokeMethod(root, "setX", Q_ARG(QVariant, 0));
          return true;
        }
      }
    }

    mw_one->isTurnThePage = true;
    mw_one->on_btnPageUp_clicked();
    mw_one->m_PageIndicator->close();
  }
  // 向左滑动结束
  else if ((press_x - relea_x) > length && qAbs(relea_y - press_y) < 35) {
    if (isText) {
      if (currentPage == totalPages - 1) {
        if (isText || isEpub) {
          QMetaObject::invokeMethod(root, "setX", Q_ARG(QVariant, 0));
          return true;
        }
      }
    } else if (isEpub) {
      if (htmlIndex + 1 >= htmlFiles.count()) {
        if (isText || isEpub) {
          QMetaObject::invokeMethod(root, "setX", Q_ARG(QVariant, 0));
          return true;
        }
      }
    }

    mw_one->isTurnThePage = true;
    mw_one->on_btnPageNext_clicked();
    mw_one->m_PageIndicator->close();
  }

  // 重置位置（文本和EPUB格式）
  if (isText || isEpub) {
    QMetaObject::invokeMethod(root, "setX", Q_ARG(QVariant, 0));
  }

  mw_one->curx = 0;
  return true;
}

void Reader::handleDoubleClick(const QPointF &globalPos) {
  // 处理链接点击状态
  if (m_Method->isClickLink) {
    m_Method->isClickLink = false;
  }

  // 计算区域划分
  int h3 = mui->qwReader->height() / 3;
  int qwY = mui->qwReader->y();
  int mY = globalPos.y();

  // 中间区域：显示/隐藏功能
  if ((mY > qwY + h3) && (mY < qwY + h3 * 2)) {
    mw_one->on_SetReaderFunVisible();
  }
  // 上部分区域：滚动到顶部
  else if ((mY > qwY) && (mY < qwY + h3)) {
    mw_one->m_Reader->setPageScroll0();
  }
  // 下部分区域：滚动到底部
  else if (mY > qwY + h3 * 2) {
    mw_one->m_Reader->setPageScroll1();
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

  if (a + mui->qwReader->height() >= h) mui->btnAutoStop->click();

  a = a + scrollValue;
  setVPos(a);
}

void Reader::setTextAreaCursorPos(int nCursorPos) {
  QQuickItem *root;
  root = mui->qwReader->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "setTextAreaCursorPos",
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

///////////////////////////////////////////////////////////////////////////////////
// TextChunkModel
///////////////////////////////////////////////////////////////////////////////////

// 1：正确初始化
TextChunkModel::TextChunkModel(QObject *parent) : QAbstractListModel(parent) {
  // 初始化角色名
  m_roleNames[TextRole] = "text";
}

void TextChunkModel::splitContent(const QString &fullText) {
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
bool TextChunkModel::isValidNesting(const QString &htmlBlock) {
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
void TextChunkModel::handleComplexStructure(QString &text, int &currentPos) {
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
QVariant TextChunkModel::data(const QModelIndex &index, int role) const {
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
void TextChunkModel::appendChunks(const QStringList &chunks) {
  if (chunks.isEmpty()) return;

  beginInsertRows(QModelIndex(), m_chunks.size(),
                  m_chunks.size() + chunks.size() - 1);
  m_chunks.append(chunks);
  endInsertRows();
}

int TextChunkModel::rowCount(const QModelIndex &parent) const {
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
QList<TocItem> Reader::parseTocFromNavFile(const QByteArray &navContent) {
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
QList<TocItem> Reader::parseOlElement(QXmlStreamReader &reader) {
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
TocItem Reader::parseLiElement(QXmlStreamReader &reader) {
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
void Reader::debugPrintTocItems(const QList<TocItem> &tocItems, int level) {
  // 层级缩进（每级缩进4个空格，方便区分层级）
  QString indent(level * 4, ' ');

  for (const TocItem &item : tocItems) {
    // 打印当前目录项的标题和链接
    qDebug() << indent << "标题：" << item.title << "，链接：" << item.href;

    // 递归打印子目录（层级+1）
    if (!item.childItems.isEmpty()) {
      debugPrintTocItems(item.childItems, level + 1);
    }
  }
}
