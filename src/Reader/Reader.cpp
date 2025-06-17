#include "Reader.h"

#include <QKeyEvent>

#include "src/MainWindow.h"
#include "ui_MainWindow.h"

extern MainWindow *mw_one;
extern Method *m_Method;
extern ReaderSet *m_ReaderSet;

extern QString iniFile, iniDir, privateDir;
extern bool zh_cn, isAndroid, isIOS, isEBook, isReport;
extern int fontSize;

extern int deleteDirfile(QString dirName);
extern QString loadText(QString textFile);
extern QString getTextEditLineText(QTextEdit *txtEdit, int i);
extern void TextEditToFile(QTextEdit *txtEdit, QString fileName);
extern void StringToFile(QString buffers, QString fileName);
extern bool unzipToDir(const QString &zipPath, const QString &destDir);

bool isOpen = false;
bool isEpub, isText, isPDF, isEpubError;

QStringList readTextList, htmlFiles, tempHtmlList, ncxList;
QString strOpfPath, oldOpfPath, fileName, ebookFile, strTitle, catalogueFile,
    strShowMsg, strEpubTitle, strPercent;

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

Reader::Reader(QWidget *parent) : QDialog(parent) {
  qmlRegisterType<TextChunkModel>("EBook.Models", 1, 0, "TextChunkModel");

  this->installEventFilter(this);

  if (!isAndroid) mw_one->ui->btnShareBook->hide();

  mw_one->ui->btnAutoStop->hide();

  mw_one->ui->btnBackDir->hide();
  mw_one->ui->lblTitle->hide();
  mw_one->ui->f_ReaderFun2->hide();
  mw_one->ui->btnBackward->hide();
  mw_one->ui->btnForward->hide();
  mw_one->ui->textBrowser->hide();
  mw_one->ui->lblCataInfo->hide();
  mw_one->ui->lblCataInfo->adjustSize();
  mw_one->ui->lblCataInfo->setWordWrap(true);
  mw_one->ui->lblBookName->adjustSize();
  mw_one->ui->lblBookName->setWordWrap(true);

  mw_one->ui->textBrowser->horizontalScrollBar()->hide();
  mw_one->ui->textBrowser->verticalScrollBar()->hide();

  QPalette pt = palette();
  pt.setBrush(QPalette::Text, Qt::black);
  pt.setBrush(QPalette::Base, QColor(235, 235, 235));
  pt.setBrush(QPalette::Highlight, Qt::red);
  pt.setBrush(QPalette::HighlightedText, Qt::white);
  mw_one->ui->textBrowser->setPalette(pt);

  mw_one->ui->btnPageNext->setStyleSheet("border:none");
  mw_one->ui->btnPageUp->setStyleSheet("border:none");
  mw_one->ui->btnSelText->setStyleSheet("border:none");

  mw_one->ui->btnPageNext->hide();
  mw_one->ui->btnPageUp->hide();
  mw_one->ui->btnSelText->hide();
  mw_one->ui->btnPages->setStyleSheet(
      "color: rgb(0, 0, 0);background-color: rgb(254, 234, 112);border: "
      "0px solid "
      "rgb(255,0,0);border-radius: 4px;"
      "font-weight: bold;");

  QFont f = this->font();
  f.setPointSize(10);
  f.setBold(true);
  mw_one->ui->btnPages->setFont(f);

  tmeShowEpubMsg = new QTimer(mw_one);
  connect(tmeShowEpubMsg, SIGNAL(timeout()), this, SLOT(showEpubMsg()));

  tmeAutoRun = new QTimer(mw_one);
  connect(tmeAutoRun, SIGNAL(timeout()), this, SLOT(autoRun()));

  f.setPointSize(10);
  mw_one->ui->lblEpubInfo->setFont(f);
  mw_one->ui->pEpubProg->setFont(f);

  strEndFlag = "<p align=center>-----" + tr("bottom") + "-----</p>";
}

Reader::~Reader() {}

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
    mw_one->ui->qwReader->rootContext()->setContextProperty("backImgFile",
                                                            "/res/b.png");
    mw_one->ui->qwReader->rootContext()->setContextProperty("myTextColor",
                                                            "#664E30");

    mw_one->ui->btnStyle3->setStyleSheet(
        "color: #00C78C;background-color: rgb(0, 0, 0);border: 2px solid "
        "rgb(0,0,255);border-radius: 4px;");

    mw_one->ui->btnStyle1->setStyleSheet(
        "color: rgb(102, 78, 48);background-color: rgb(240, 222, 198);border: "
        "2px solid "
        "rgb(255,0,0);border-radius: 4px;");
    mw_one->ui->btnStyle2->setStyleSheet(strStyle2_0);

    textColor = QColor(102, 78, 48);
    baseColor = QColor(240, 222, 198);
  }

  if (readerStyle == "2") {
    mw_one->ui->qwReader->rootContext()->setContextProperty("backImgFile", "");
    mw_one->ui->qwReader->rootContext()->setContextProperty(
        "myBackgroundColor", mw_one->ui->editBackgroundColor->text());
    mw_one->ui->qwReader->rootContext()->setContextProperty(
        "myTextColor", mw_one->ui->editForegroundColor->text());

    mw_one->ui->btnStyle3->setStyleSheet(
        "color: #00C78C;background-color: rgb(0, 0, 0);border: 2px solid "
        "rgb(0,0,255);border-radius: 4px;");

    mw_one->ui->btnStyle1->setStyleSheet(
        "color: rgb(102, 78, 48);background-color: rgb(240, 222, 198);border: "
        "2px solid "
        "rgb(0,0,255);border-radius: 4px;");
    mw_one->ui->btnStyle2->setStyleSheet(strStyle2_1);

    textColor = QColor(0, 0, 0);
    baseColor = QColor(255, 255, 255);
    mw_one->ui->f_CustomColor->show();
  }

  if (readerStyle == "3") {
    mw_one->ui->qwReader->rootContext()->setContextProperty("backImgFile",
                                                            "/res/b3.png");
    mw_one->ui->qwReader->rootContext()->setContextProperty("myTextColor",
                                                            "#2E8B57");

    mw_one->ui->btnStyle3->setStyleSheet(
        "color: #00C78C;background-color: rgb(0, 0, 0);border: 2px solid "
        "rgb(255,0,0);border-radius: 4px;");

    mw_one->ui->btnStyle1->setStyleSheet(
        "color: rgb(102, 78, 48);background-color: rgb(240, 222, 198);border: "
        "2px solid "
        "rgb(0,0,255);border-radius: 4px;");
    mw_one->ui->btnStyle2->setStyleSheet(strStyle2_0);

    textColor = QColor(46, 139, 87);
    baseColor = QColor(0, 0, 0);
  }

  QPalette pt = palette();
  pt.setBrush(QPalette::Text, textColor);
  pt.setBrush(QPalette::Base, baseColor);
  pt.setBrush(QPalette::Highlight, Qt::red);
  pt.setBrush(QPalette::HighlightedText, Qt::white);
  mw_one->ui->textBrowser->setPalette(pt);
}

void Reader::startOpenFile(QString openfile) {
  if (isReport) return;

  if (isAndroid) closeMyPDF();

  isEpubError = false;
  strShowMsg = "";
  strPercent = "";
  mw_one->ui->lblEpubInfo->setFixedWidth(36);
  mw_one->ui->pEpubProg->setMaximum(100);

  setReaderStyle();

  if (QFile(openfile).exists()) {
    isEBook = true;
    strTitle = "";
    catalogueFile = "";

    mw_one->ui->btnReader->setEnabled(false);
    mw_one->ui->f_ReaderFun->setEnabled(false);
    mw_one->ui->lblTitle->hide();
    mw_one->ui->qwCata->hide();
    mw_one->ui->lblCataInfo->hide();

    QString bookName;

    /*#ifdef Q_OS_ANDROID
        QString name;
        name = getUriRealPath(openfile);
        QStringList lista = name.split("/");
        bookName = lista.at(lista.count() - 1);
    #else
        QFileInfo fi(openfile);
        bookName = fi.fileName();
    #endif*/

    QFileInfo fi(openfile);
    bookName = fi.fileName();

    ebookFile = openfile;
    strTitle =
        bookName + "    " + m_Method->getFileSize(QFile(ebookFile).size(), 2);

    mw_one->m_ReadTWThread->quit();
    mw_one->m_ReadTWThread->wait();

    // if (isAndroid)
    //   m_Method->showAndroidProgressBar();
    // else
    if (!mw_one->initMain) mw_one->showProgress();

    tmeShowEpubMsg->start(100);

    mw_one->myReadEBookThread->start();

  } else
    return;
}

void Reader::openFile(QString openfile) {
  qDebug() << "Starting to open files...";

  isOpen = false;
  oldOpfPath = strOpfPath;

  if (QFile(openfile).exists()) {
    QFileInfo fi(openfile);
    QString strSuffix = fi.suffix();

    if (strSuffix == "epub") {
      QString dirpath, dirpath1;
      dirpath = privateDir + "temp0/";
      dirpath1 = privateDir + "temp/";

      QString temp = privateDir + "temp.zip";
      QFile::remove(temp);
      if (!QFile::copy(openfile, temp)) {
        QMessageBox box;
        box.setText(openfile + "\n!=\n" + temp);
        box.exec();
      }

      deleteDirfile(dirpath);
      QDir dir;
      dir.mkdir(dirpath);

      m_Method->decompressWithPassword(temp, dirpath, "");

      qDebug() << "openFile:" << openfile << "dirpath=" << dirpath;

      QString strFullPath;
      QString strContainer = dirpath + "META-INF/container.xml";
      if (!QFile(strContainer).exists()) {
        isEpub = false;
        isEpubError = true;
        qDebug() << "====== isEpub == false ======";
        return;
      }

      QStringList conList = readText(strContainer);
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

      QString strOpfFile = dirpath + strFullPath;
      QFileInfo fi(strOpfFile);
      strOpfPath = fi.path() + "/";
      QStringList opfList = readText(strOpfFile);
      tempHtmlList.clear();
      QStringList tempList;

      qDebug() << "strOpfFile=" << strOpfFile;

      int opfCount = opfList.count();
      if (opfCount > 1) {
        for (int i = 0; i < opfCount; i++) {
          QString str0 = opfList.at(i);
          str0 = str0.trimmed();

          if (str0.contains("idref=") && str0.mid(0, 8) == "<itemref") {
            QString idref = get_idref(str0);

            QString qfile;
            qfile = strOpfPath + get_href(idref, opfList);
            tempList.append(qfile);

            strShowMsg = QFileInfo(qfile).baseName();
          }

          if (opfCount > 0) {
            double percent = (double)i / (double)opfCount;
            strPercent = QString::number(percent * 100, 'f', 0);
          }
        }
      }

      QStringList htmlList = ncx2html();

      if (tempList.count() == 0) {
        tempList = htmlList;
      }

      int count_1 = tempList.count();
      for (int i = 0; i < count_1; i++) {
        QString qfile = tempList.at(i);
        QFileInfo fi(qfile);
        if (fi.exists() && !tempHtmlList.contains(qfile)) {
          if (epubFileMethod == 1) {
            if (fi.size() <= minBytes) {
              tempHtmlList.append(qfile);

            } else {
              SplitFile(qfile);
            }
          }
          if (epubFileMethod == 2) {
            tempHtmlList.append(qfile);
          }
        }

        if (count_1 > 0) {
          double percent = (double)i / (double)count_1;
          strPercent = QString::number(percent * 100, 'f', 0);
        }
      }

      if (tempHtmlList.count() == 0) {
        isEpub = false;
        isEpubError = true;
        strOpfPath = oldOpfPath;
        qDebug() << "====== tempHtmlList Count== 0 ======";
        return;
      } else {
        isEpub = true;
        isText = false;
        isPDF = false;

        strShowMsg = "Del temp ...";
        deleteDirfile(dirpath1);
        // strShowMsg = "Copy temp0 to temp ...";
        // copyDirectoryFiles(dirpath, dirpath1, true);
        // strShowMsg = "Del temp0 ...";
        // deleteDirfile(dirpath);
        strShowMsg = "Rename temp0 to temp...";
        QDir dir;
        dir.rename(dirpath, dirpath1);

        htmlFiles.clear();

        strOpfPath.replace(dirpath, dirpath1);
        for (int i = 0; i < tempHtmlList.count(); i++) {
          QString str = tempHtmlList.at(i);
          str.replace(dirpath, dirpath1);
          htmlFiles.append(str);
        }

        QFile(strOpfPath + "main.css").remove();
        QFile::copy(":/res/main.css", strOpfPath + "main.css");

        QStringList temp_l0 = ncxList;
        ncxList.clear();
        for (int i = 0; i < temp_l0.count(); i++) {
          QString item = temp_l0.at(i);
          item.replace(dirpath, dirpath1);
          ncxList.append(item);
        }

        catalogueFile.replace(dirpath, dirpath1);
        QString str_cate = loadText(catalogueFile);
        str_cate.replace(dirpath, dirpath1);
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
  mw_one->ui->lblSpeed->setText(tr("Scroll Speed") + " : " + value);

  QFont font;
  int fsize = Reg.value("/Reader/FontSize", 18).toInt();
  readerFontSize = fsize;
  mw_one->ui->qwReader->rootContext()->setContextProperty("FontSize", fsize);
  font.setPointSize(fsize);
  font.setLetterSpacing(QFont::AbsoluteSpacing, 2);  // 字间距

  fileName = Reg.value("/Reader/FileName").toString();
  // if (!QFile(fileName).exists() && zh_cn) fileName = ":/res/test.txt";

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
  mw_one->ui->qwReader->rootContext()->setContextProperty("isAni",
                                                          QVariant(false));

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

  QQuickItem *root = mw_one->ui->qwReader->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "loadHtmlBuffer",
                            Q_ARG(QVariant, currentTxt));

  setAni();
}

void Reader::loadQMLText(QString str) {
  if (isText || isEpub) {
    QQuickItem *root = mw_one->ui->qwReader->rootObject();
    QMetaObject::invokeMethod((QObject *)root, "loadText",
                              Q_ARG(QVariant, str));
  }
}

QString Reader::getQMLText() {
  QVariant str;
  QQuickItem *root = mw_one->ui->qwReader->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "getText",
                            Q_RETURN_ARG(QVariant, str));

  return str.toString();
}

void Reader::goUpPage() {
  if (isSelText) return;
  mw_one->ui->lblTitle->hide();

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
  mw_one->ui->lblTitle->hide();

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
  mw_one->ui->lblCataInfo->hide();
  mw_one->ui->qwCata->hide();
  mw_one->ui->qwReader->show();
  mw_one->ui->btnShowBookmark->setEnabled(true);

  initLink(htmlFile);
  m_Method->clearAllBakList(mw_one->ui->qwCata);
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
  if (!isEpub) return "";

  QPlainTextEdit *plain_edit = new QPlainTextEdit;
  QTextEdit *text_edit = new QTextEdit;
  QString strHtml = loadText(htmlFile);
  strHtml.replace("　", " ");
  strHtml.replace("<", "\n<");
  strHtml.replace(">", ">\n");

  strHtml.replace("file:///" + strOpfPath, "");

  strHtml.replace(".css", "");
  strHtml.replace("font-family:", "font0-family:");
  strHtml.replace("font-size:", "font0-size:");
  strHtml.replace("font color", "font color0");
  strHtml = strHtml.trimmed();

  text_edit->setPlainText(strHtml);

  for (int i = 0; i < text_edit->document()->lineCount(); i++) {
    QString str = getTextEditLineText(text_edit, i);
    str = str.trimmed();

    if (str.contains("</head>")) {
      QString css =
          "<link href=\"../main.css\" rel=\"stylesheet\" type=\"text/css\" "
          "/>";
      css.replace("../", "file:///" + strOpfPath);
      plain_edit->appendPlainText(css);
      plain_edit->appendPlainText("</head>");
    } else {
      if (str.trimmed() != "") {
        if (str.contains("<image") && str.contains("xlink:href=")) {
          str.replace("xlink:href=", "src=");
          str.replace("<image", "<img");
          str.replace("height", "height1");
          str.replace("width", "width1");
        }

        if (str.mid(0, 4) == "<img") {
          QString str1 = str;
          QStringList list = str1.split(" ");
          QString strSrc;
          for (int k = 0; k < list.count(); k++) {
            QString s1 = list.at(k);
            if (s1.contains("src=")) {
              strSrc = s1;
              break;
            }
          }
          strSrc = strSrc.replace("src=", "");
          strSrc = strSrc.replace("/>", "");

          QString strimg = strSrc;
          strimg = strimg.replace("\"", "");
          QString imgFile = strOpfPath + strimg;
          imgFile = imgFile.replace("../", "");

          // qDebug() << "imgFile=" << imgFile;

          strSrc = "file:///" + imgFile;
          str.replace(strimg, strSrc);
          strSrc = "\"" + strSrc + "\"";

          bool isCover = false;
          QString imgFile_l = imgFile.toLower();
          if (imgFile_l.contains("cover")) {
            isCover = true;
          }
          int nw = 0;
          if (isCover) {
            nw = mw_one->ui->qwReader->width() - 25;
          } else {
            nw = mw_one->ui->qwReader->width() - 104;
          }
          QString strw = " width = " + QString::number(nw);
          QImage img(imgFile);

          if (!isCover) {
            if (img.width() >= nw) {
              str = str.replace(" width = ", " width1 = ");
              str = str.replace("/>", strw + " />");
              str = "<a href=" + strSrc + ">" + str + " </a>";
            } else {
              str = "<a href=" + strSrc + ">" + str + "</a>";
            }
          }

          if (isCover) {
            str = str.replace(" width = ", " width1 = ");
            str = str.replace("/>", strw + " />");
            str = str + " </a>";
          }

          // qDebug() << "strSrc=" << strSrc << str;

          str = str.replace("width=", "width1=");
          str = str.replace("height=", "height1=");
        }

        plain_edit->appendPlainText(str);
      }
    }
  }

  if (isWriteFile) PlainTextEditToFile(plain_edit, htmlFile);

  return plain_edit->toPlainText();
}

void Reader::setQMLHtml(QString htmlFile, QString htmlBuffer, QString skipID) {
  if (QFile::exists(htmlFile)) {
    htmlBuffer = processHtml(htmlFile, false);
  }
  htmlBuffer.append(strEndFlag);
  currentTxt = htmlBuffer;

  mw_one->ui->qwReader->rootContext()->setContextProperty("isAni", false);
  QQuickItem *root = mw_one->ui->qwReader->rootObject();

  QMetaObject::invokeMethod((QObject *)root, "loadHtmlBuffer",
                            Q_ARG(QVariant, htmlBuffer));

  //  QMetaObject::invokeMethod((QObject*)root, "loadHtml",
  //                          Q_ARG(QVariant, htmlFile), Q_ARG(QVariant,
  //                          skipID));

  QFileInfo fi(htmlFile);
  mw_one->ui->lblInfo->setText(
      tr("Info") + " : " + fi.baseName() + "  " +
      m_Method->getFileSize(QFile(htmlFile).size(), 2));

  qDebug() << "setQMLHtml:Html File=" << htmlFile;

  if (skipID != "") {
    setHtmlSkip(htmlFile, skipID);
  }

  gotoCataList(htmlFile);

  setAni();
}

void Reader::setAni() {
  if (isPageNext)
    mw_one->ui->qwReader->rootContext()->setContextProperty("aniW",
                                                            mw_one->width());
  else
    mw_one->ui->qwReader->rootContext()->setContextProperty("aniW",
                                                            -mw_one->width());
  mw_one->ui->qwReader->rootContext()->setContextProperty("toW", 0);
  mw_one->ui->qwReader->rootContext()->setContextProperty("isAni", true);
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

    // 首先检查BOM
    if (data.startsWith("\xEF\xBB\xBF")) {
      text = QString::fromUtf8(data.mid(3));  // 跳过BOM
    } else if (data.startsWith("\xFF\xFE") || data.startsWith("\xFE\xFF")) {
      // UTF-16 BOM
      QTextCodec *codec = QTextCodec::codecForName("UTF-16");
      text = codec->toUnicode(data);
    } else {
      // 使用更健壮的编码检测
      if (isUtf8(data)) {
        text = QString::fromUtf8(data);
      } else {
        // 尝试常见编码
        QTextCodec *codec = nullptr;

        // 尝试GBK/GB2312 (中文)
        codec = QTextCodec::codecForName("GBK");
        QString gbkText = codec->toUnicode(data);
        if (isValidText(gbkText)) {
          text = gbkText;
        } else {
          // 尝试ISO 8859-1 (Latin-1)
          codec = QTextCodec::codecForName("ISO 8859-1");
          text = codec->toUnicode(data);
        }
      }
    }

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

// 改进的UTF-8检测函数
bool Reader::isUtf8(const QByteArray &data) {
  int i = 0;
  int length = data.length();
  int utf8Chars = 0;
  int invalidBytes = 0;

  while (i < length) {
    unsigned char c = static_cast<unsigned char>(data[i]);
    int bytes;

    // 判断UTF-8字符的字节数
    if ((c & 0x80) == 0) {
      bytes = 1;  // 0xxxxxxx
    } else if ((c & 0xE0) == 0xC0) {
      bytes = 2;  // 110xxxxx
    } else if ((c & 0xF0) == 0xE0) {
      bytes = 3;  // 1110xxxx
    } else if ((c & 0xF8) == 0xF0) {
      bytes = 4;  // 11110xxx
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

    if (bytes > 1) utf8Chars++;
    i += bytes;
  }

  // 如果没有发现UTF-8多字节字符，或者无效字节太多，认为不是UTF-8
  if (utf8Chars == 0) return false;
  double validRatio = 1.0 - (double)invalidBytes / length;
  return validRatio > 0.9;  // 至少90%的字节有效
}

// 辅助函数：检查文本是否包含足够的有效字符
bool Reader::isValidText(const QString &text) {
  int validChars = 0;
  int totalChars = text.length();

  if (totalChars == 0) return false;

  for (QChar c : text) {
    if (c.isPrint() || c.isSpace()) {
      validChars++;
    }
  }

  // 要求至少70%的字符是可打印的或空格
  return (double)validChars / totalChars > 0.7;
}

QString Reader::GetCorrectUnicode(const QByteArray &text) {
  // 尝试 UTF-8 解码
  QStringDecoder utf8Decoder(QStringDecoder::Utf8);
  QString strtext = utf8Decoder.decode(text);

  // 如果 UTF-8 解码失败，尝试系统本地编码（如 GBK）
  if (utf8Decoder.hasError()) {
    QStringDecoder localDecoder(QStringConverter::System);
    strtext = localDecoder.decode(text);

    // 如果本地编码也失败，则直接按原始数据构造 QString（可能乱码）
    if (localDecoder.hasError()) {
      strtext = QString::fromLatin1(text);
    }
  }

  return strtext;
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

  mw_one->ui->qwReader->rootContext()->setContextProperty("FontSize", fontSize);

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
    if (mw_one->ui->qwCata->isVisible()) {
      Reg.setValue("/Reader/vpos  CataVPos", textPos);
      int index = m_Method->getCurrentIndexFromQW(mw_one->ui->qwCata);
      Reg.setValue("/Reader/vpos  CataIndex", index);
    } else {
      if (htmlIndex >= 0)
        Reg.setValue("/Reader/vpos" + fiHtml.baseName(), textPos);
    }
  }

  if (isText) {
    Reg.setValue("/Reader/vpos" + QString::number(currentPage), textPos);
  }

  qDebug() << "savePageVPos:" << textPos;
}

void Reader::setPageVPos() {
  QSettings Reg(privateDir + "bookini/" + currentBookName + ".ini",
                QSettings::IniFormat);

  QFileInfo fiHtml(currentHtmlFile);
  if (isEpub) {
    if (mw_one->ui->qwCata->isVisible()) {
      textPos = Reg.value("/Reader/vpos  CataVPos", 0).toReal();
      int index = Reg.value("/Reader/vpos  CataIndex", 0).toReal();
      if (currentCataIndex > 0) index = currentCataIndex;
      m_Method->setCurrentIndexFromQW(mw_one->ui->qwCata, index);
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

  qDebug() << "setPageVPos:" << textPos;
}

void Reader::setVPos(qreal pos) {
  QQuickItem *root;
  if (mw_one->ui->qwCata->isVisible())
    root = mw_one->ui->qwCata->rootObject();
  else
    root = mw_one->ui->qwReader->rootObject();

  QMetaObject::invokeMethod((QObject *)root, "setVPos", Q_ARG(QVariant, pos));
}

qreal Reader::getVPos() {
  QVariant itemCount;

  QQuickItem *root;
  if (mw_one->ui->qwCata->isVisible())
    root = mw_one->ui->qwCata->rootObject();
  else
    root = mw_one->ui->qwReader->rootObject();

  QMetaObject::invokeMethod((QObject *)root, "getVPos",
                            Q_RETURN_ARG(QVariant, itemCount));
  textPos = itemCount.toDouble();
  return textPos;
}

QString Reader::getBookmarkText() {
  QVariant item;
  QQuickItem *root = mw_one->ui->qwReader->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "getBookmarkText",
                            Q_RETURN_ARG(QVariant, item));
  return item.toString();
}

qreal Reader::getVHeight() {
  QVariant itemCount;
  QQuickItem *root = mw_one->ui->qwReader->rootObject();
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
    mw_one->ui->btnPages->setText(QString::number(currentPage + 1) + "\n" +
                                  QString::number(totalPages));
    mw_one->ui->progReader->setMaximum(totalPages);
    mw_one->ui->progReader->setValue(currentPage + 1);
  }

  if (isEpub) {
    mw_one->ui->btnPages->setText(QString::number(htmlIndex + 1) + "\n" +
                                  QString::number(htmlFiles.count()));
    mw_one->ui->progReader->setMaximum(htmlFiles.count());
    mw_one->ui->progReader->setValue(htmlIndex + 1);
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

  m_Method->clearAllBakList(mw_one->ui->qwBookList);
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

    m_Method->addItemToQW(mw_one->ui->qwBookList, bookName, bookPath, "",
                          suffix, 50);
  }

  for (int i = 0; i < bookList.count(); i++) {
    QString str = bookList.at(i);
    QStringList listBooks = str.split("|");
    if (listBooks.at(1) == fileName) {
      m_Method->setCurrentIndexFromQW(mw_one->ui->qwBookList, i);
      break;
    }
  }
}

void Reader::clearAllReaderRecords() {
  int count = m_Method->getCountFromQW(mw_one->ui->qwBookList);
  if (count == 0) return;

  m_Method->m_widget = new QWidget(mw_one);
  ShowMessage *m_ShowMsg = new ShowMessage(this);
  if (!m_ShowMsg->showMsg("Knot", tr("Clear all reading history") + " ? ", 2))
    return;

  m_Method->clearAllBakList(mw_one->ui->qwBookList);
  bookList.clear();
  QFile file(privateDir + "reader.ini");
  if (file.exists()) file.remove();
}

void Reader::openBookListItem() {
  int index = m_Method->getCurrentIndexFromQW(mw_one->ui->qwBookList);

  if (index < 0) return;

  QString str = bookList.at(index);
  QStringList listBooks = str.split("|");
  QString bookfile = listBooks.at(1);
  if (bookfile != fileName) {
    startOpenFile(bookfile);
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
  mw_one->ui->lblTitle->hide();

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

  mw_one->ui->hSlider->setMinimum(1);

  if (isText) {
    mw_one->ui->hSlider->setMaximum(totalPages);
    currentPage = mw_one->ui->hSlider->value() - 1;
    mw_one->ui->btnPages->setText(QString::number(currentPage + 1) + "\n" +
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
    mw_one->ui->hSlider->setMaximum(htmlFiles.count());
    htmlIndex = sPos - 1;
    if (htmlIndex < 0) htmlIndex = 0;

    currentHtmlFile = htmlFiles.at(htmlIndex);
    setQMLHtml(currentHtmlFile, "", "");
  }
}

void Reader::showCatalogue() {
  closeSelText();
  savePageVPos();

  if (mw_one->ui->qwCata->isVisible()) {
    mw_one->ui->lblCataInfo->hide();
    mw_one->ui->qwCata->hide();
    mw_one->ui->qwReader->show();
    mw_one->ui->btnShowBookmark->setEnabled(true);

  } else {
    mw_one->ui->qwReader->hide();
    mw_one->ui->lblCataInfo->show();
    mw_one->ui->qwCata->show();
    mw_one->ui->btnShowBookmark->setEnabled(false);

    m_Method->clearAllBakList(mw_one->ui->qwCata);
    for (int i = 0; i < ncxList.count(); i++) {
      QString item = ncxList.at(i);
      QString str0, str1;
      str0 = item.split("===").at(0);
      str1 = item.split("===").at(1);
      m_Method->addItemToQW(mw_one->ui->qwCata, str0, str1, "", "", 0);
    }
  }

  setPageVPos();
  showInfo();
}

QStringList Reader::ncx2html() {
  QStringList htmlList;
  ncxList.clear();
  QString ncxFile;
  QStringList fileList;
  QStringList fmt;
  fmt.append("ncx");
  mw_one->m_NotesList->getAllFiles(strOpfPath, fileList, fmt);
  if (fileList.count() >= 1) {
    ncxFile = fileList.at(0);
  }

  if (!QFile(ncxFile).exists()) {
    return htmlList;
  }

  QPlainTextEdit *plain_edit = new QPlainTextEdit;
  plain_edit->appendPlainText("<html>");
  plain_edit->appendPlainText("<body>");
  plain_edit->appendPlainText("<style>.my-link {color: #336699;} </style>");

  QTextEdit *text_edit = new QTextEdit;
  QTextEdit *text_edit0 = new QTextEdit;
  QString strHtml0 = loadText(ncxFile);
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
  catalogueFile = strOpfPath + "catalogue.html";
  PlainTextEditToFile(plain_edit, catalogueFile);

  if (strEpubTitle != "") {
    if (strAuthor != "")
      strEpubTitle = strEpubTitle + " ( " + strAuthor + " ) ";
    mw_one->ui->lblCataInfo->setText(strEpubTitle);
    qDebug() << "ncx title=" << strEpubTitle << "author=" << strAuthor;
  }

  return htmlList;
}

void Reader::setHtmlSkip(QString htmlFile, QString skipID) {
  QTextBrowser *textBrowser = new QTextBrowser();
  textBrowser->setFixedHeight(mw_one->ui->qwReader->height());
  textBrowser->setFixedWidth(mw_one->ui->qwReader->width());
  QFont font = mw_one->ui->qwReader->font();
  font.setPixelSize(readerFontSize);
  font.setFamily(mw_one->ui->btnFont->font().family());
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
    mw_one->ui->lblEpubInfo->show();
    mw_one->ui->pEpubProg->show();
    if (strPercent != "") {
      mw_one->ui->lblEpubInfo->setText(strPercent + "% ");
    }
    mw_one->ui->pEpubProg->setValue(strPercent.toInt());
    mw_one->ui->pEpubProg->setFormat(strShowMsg);
  }
}

void Reader::removeBookList() {
  int index = m_Method->getCurrentIndexFromQW(mw_one->ui->qwBookList);
  if (index <= 0) return;

  ShowMessage *msg = new ShowMessage(mw_one);
  if (!msg->showMsg("Knot", tr("Remove from list?"), 2)) return;

  bookList.removeAt(index);
  m_Method->delItemFromQW(mw_one->ui->qwBookList, index);
  saveReader("", false);
}

void Reader::readBookDone() {
  if (isOpenBookListClick) {
    mw_one->on_btnBackBookList_clicked();
    isOpenBookListClick = false;
  }

  if (isEpubError) {
    tmeShowEpubMsg->stop();

    mw_one->ui->lblEpubInfo->hide();
    mw_one->ui->pEpubProg->hide();
    mw_one->ui->btnReader->setEnabled(true);
    mw_one->ui->f_ReaderFun->setEnabled(true);
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

    mw_one->ui->qwReader->show();
    mw_one->ui->f_ReaderFun->show();
    mw_one->ui->progReader->show();
    mw_one->ui->btnPages->show();
    mw_one->ui->btnShowBookmark->show();
    mw_one->ui->btnAutoRun->show();

    if (mw_one->ui->frameBookList->isVisible()) {
      mw_one->ui->frameBookList->hide();
      mw_one->ui->frameReader->show();
    }

    mw_one->ui->qwReader->rootContext()->setContextProperty("strText", "");
    mw_one->ui->qwReader->rootContext()->setContextProperty("isSelText",
                                                            isSelText);
    mw_one->ui->qwReader->rootContext()->setContextProperty("isAni", true);
    mw_one->ui->qwReader->rootContext()->setContextProperty("aniW",
                                                            mw_one->width());
    mw_one->ui->qwReader->rootContext()->setContextProperty("toW", 0);
    mw_one->ui->qwReader->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/reader.qml")));

    if (isEpub) {
      mw_one->ui->lblInfo->show();
      mw_one->ui->qwReader->rootContext()->setContextProperty("htmlPath",
                                                              strOpfPath);
      if (QFile(catalogueFile).exists()) {
        mw_one->ui->btnCatalogue->show();
      } else
        mw_one->ui->btnCatalogue->hide();
    }

    if (isText) {
      mw_one->ui->btnBackDir->hide();
      mw_one->ui->btnCatalogue->hide();
      mw_one->ui->lblInfo->hide();
    }
  }

  mw_one->ui->lblBookName->setText(strTitle);
  mw_one->ui->btnBackDir->hide();
  tmeShowEpubMsg->stop();
  mw_one->ui->lblEpubInfo->hide();
  mw_one->ui->pEpubProg->hide();

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

    mw_one->ui->btnAutoRun->hide();
    mw_one->ui->btnShowBookmark->hide();
    mw_one->ui->progReader->hide();
    mw_one->ui->qwReader->hide();
    mw_one->ui->f_ReaderFun->show();
    mw_one->ui->btnPages->hide();
    mw_one->ui->btnCatalogue->hide();

#ifdef Q_OS_ANDROID

    if (!mw_one->initMain) {
      if (mw_one->ui->frameMain->isVisible()) {
        mw_one->ui->frameMain->hide();
        mw_one->ui->frameBookList->show();
      }
      if (mw_one->ui->frameReader->isVisible()) {
        mw_one->ui->frameReader->hide();
        mw_one->ui->frameBookList->show();
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
      while (!mw_one->ui->btnReader->isEnabled())
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
      mw_one->ui->btnReader->click();
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
    mw_one->ui->btnSelText->setIcon(QIcon(":/res/choice1.png"));
    isSelText = true;

    mw_one->ui->textBrowser->setReadOnly(true);
    QFont font;
    font.setPixelSize(readerFontSize);
    font.setFamily(mw_one->ui->btnFont->font().family());
    font.setLetterSpacing(QFont::AbsoluteSpacing, 2);
    mw_one->ui->textBrowser->setFont(font);

    mw_one->ui->textBrowser->setHtml(currentTxt);

    mw_one->ui->qwReader->hide();
    mw_one->ui->textBrowser->show();

    qreal h0 = getVHeight();
    qreal h1 = mw_one->ui->textBrowser->document()->size().height();
    qreal s0 = getVPos();
    qreal s1 = (s0 * h1) / h0;
    mw_one->ui->textBrowser->verticalScrollBar()->setSliderPosition(s1);
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
    mw_one->ui->btnSelText->setIcon(QIcon(":/res/choice0.png"));
    mw_one->ui->textBrowser->hide();
    mw_one->ui->qwReader->show();
    mw_one->mydlgSetText->close();
  }
}

void Reader::setPageScroll0() {
  qreal cpos = getVPos();
  qreal th = getVHeight();
  int readerHeight = mw_one->ui->qwReader->height();
  if (th < readerHeight) return;
  int fontHeight = m_Method->getFontHeight();
  qreal newpos = cpos - readerHeight + fontHeight;

  if (newpos < readerHeight + fontHeight) newpos = -fontHeight;

  setVPos(newpos);
}

void Reader::setPageScroll1() {
  qreal cpos = getVPos();
  qreal th = getVHeight();
  int readerHeight = mw_one->ui->qwReader->height();
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
  int count = m_Method->getCountFromQW(mw_one->ui->qwBookmark);
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

  mw_one->ui->qwBookmark->hide();
  mw_one->ui->qwReader->show();
  mw_one->ui->btnCatalogue->setEnabled(true);
}

void Reader::showBookmarkList() {
  QStringList list = getCurrentBookmarkList();
  m_Method->clearAllBakList(mw_one->ui->qwBookmark);
  for (int i = 0; i < list.count(); i++) {
    m_Method->addItemToQW(mw_one->ui->qwBookmark, list.at(i), "", "", "", 0);
  }
}

void Reader::setPanelVisible() { mw_one->on_SetReaderFunVisible(); }

void Reader::ContinueReading() {
  while (!mw_one->ui->btnReader->isEnabled())
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

  mw_one->ui->btnReader->click();
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
  int index = m_Method->getCurrentIndexFromQW(mw_one->ui->qwBookList);

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
  if (watch == mw_one->ui->qwReader) {
    int length = 75;

    if (mw_one->ui->textBrowser->isHidden()) {
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
            int cn = mw_one->ui->btnPages->text().split("\n").at(0).toInt();
            if (cn != 1) {
              mw_one->m_PageIndicator->setPicRight();
            }
          } else if ((press_x - relea_x) > length &&
                     qAbs(relea_y - press_y) < 35) {
            // qDebug() << "book left...";
            int cn = mw_one->ui->btnPages->text().split("\n").at(0).toInt();
            int tn = mw_one->ui->btnPages->text().split("\n").at(1).toInt();
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
      w = mw_one->ui->qwReader->width();
      h = mw_one->ui->qwReader->height();
    }

    if (event->type() == QEvent::MouseButtonDblClick) {
      if (m_Method->isClickLink) {
        m_Method->isClickLink = false;
      }

      int h3 = mw_one->ui->qwReader->height() / 3;
      int mY = event->globalPosition().y();
      int qwY = mw_one->ui->qwReader->y();

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
      mw_one->ui->lblTitle->hide();
      QQuickItem *root = mw_one->ui->qwReader->rootObject();

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
    if (touchPoints.count() == 1 && watch == mw_one->ui->qwReader) {
      const QTouchEvent::TouchPoint &touchPoint = touchPoints.first();

      // 将触摸点位置转换为全局坐标
      QPointF globalPos =
          mw_one->ui->qwReader->mapToGlobal(touchPoint.position().toPoint());

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
    if (watch == mw_one->ui->qwReader) {
      // ... 保留原有的桌面端鼠标事件处理代码 ...
      // 这里放置您原有的鼠标事件处理逻辑
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
  w = mw_one->ui->qwReader->width();
  h = mw_one->ui->qwReader->height();

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
      int cn = mw_one->ui->btnPages->text().split("\n").at(0).toInt();
      if (cn != 1) {
        mw_one->m_PageIndicator->setPicRight();
      }
    }
    // 向左滑动
    else if ((press_x - relea_x) > length && qAbs(relea_y - press_y) < 35) {
      int cn = mw_one->ui->btnPages->text().split("\n").at(0).toInt();
      int tn = mw_one->ui->btnPages->text().split("\n").at(1).toInt();
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

  mw_one->ui->lblTitle->hide();
  mw_one->isMousePress = false;
  mw_one->isTurnThePage = false;

  QQuickItem *root = mw_one->ui->qwReader->rootObject();
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
  int h3 = mw_one->ui->qwReader->height() / 3;
  int qwY = mw_one->ui->qwReader->y();
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

  if (a + mw_one->ui->qwReader->height() >= h) mw_one->ui->btnAutoStop->click();

  a = a + scrollValue;
  setVPos(a);
}

void Reader::setTextAreaCursorPos(int nCursorPos) {
  QQuickItem *root;
  root = mw_one->ui->qwReader->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "setTextAreaCursorPos",
                            Q_ARG(QVariant, nCursorPos));
}

//===============================================================================================
// TextChunkModel
//===============================================================================================

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
