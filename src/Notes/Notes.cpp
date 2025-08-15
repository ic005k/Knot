#include "src/Notes/Notes.h"

#include "cmark-gfm-core-extensions.h"
#include "cmark_wrapper.h"
#include "src/MainWindow.h"
#include "subscript.h"
#include "superscript.h"
#include "ui_MainWindow.h"
#include "ui_Notes.h"

extern MainWindow *mw_one;
extern Ui::MainWindow *mui;
extern Method *m_Method;
extern QTreeWidget *twrb, *tw;

extern QString iniFile, iniDir, privateDir, currentMDFile, imgFileName, appName,
    encPassword, errorInfo;
extern bool isAndroid, isIOS, isDark, isPasswordError;
extern int fontSize;
extern QRegularExpression regxNumber;

extern int deleteDirfile(QString dirName);
extern QString loadText(QString textFile);
extern QString getTextEditLineText(QTextEdit *txtEdit, int i);
extern void TextEditToFile(QTextEdit *txtEdit, QString fileName);
extern void StringToFile(QString buffers, QString fileName);

extern WebDavHelper *listWebDavFiles(const QString &url,
                                     const QString &username,
                                     const QString &password);
extern CloudBackup *m_CloudBackup;

extern QString markdownToHtmlWithMath(const QString &md);

NoteIndexManager::NoteIndexManager(QObject *parent) : QObject{parent} {}

Notes::Notes(QWidget *parent) : QDialog(parent), ui(new Ui::Notes) {
  ui->setupUi(this);
  m_NoteIndexManager = new NoteIndexManager();

  initEditor();

  init_md();

  QString path = iniDir + "memo/";
  QDir dir(path);
  if (!dir.exists()) dir.mkdir(path);
  htmlFileName = iniDir + "memo.html";

  this->installEventFilter(this);
  this->setModal(true);
  this->layout()->setContentsMargins(5, 5, 5, 5);
  ui->frameEdit->layout()->setContentsMargins(0, 0, 0, 0);

  timerEditPanel = new QTimer(this);
  connect(timerEditPanel, SIGNAL(timeout()), this, SLOT(on_showEditPanel()));

  timerEditNote = new QTimer(this);
  connect(timerEditNote, SIGNAL(timeout()), this, SLOT(on_editNote()));

  bCursorVisible = true;
  timerCur = new QTimer(this);
  connect(this, SIGNAL(sendUpdate()), this, SLOT(update()));
  connect(timerCur, SIGNAL(timeout()), this, SLOT(timerSlot()));

  ui->btnColor->hide();
  ui->lblCount->hide();

  ui->btnFind->hide();
  ui->editFind->setMinimumWidth(65);

  mui->lblNoteGraphView->hide();

  m_Method->set_ToolButtonStyle(this);

  // 创建快捷键：绑定 Ctrl+F
  QShortcut *shortcut1 = new QShortcut(QKeySequence("Ctrl+F"), this);
  connect(shortcut1, &QShortcut::activated, this, [this]() {
    ui->editFind->setFocus();
    ui->editFind->selectAll();
  });
}

void Notes::initEditor() {
#ifndef Q_OS_ANDROID

  m_EditSource = new QsciScintilla(this);
  m_EditSource->setUtf8(true);
  m_EditSource->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_EditSource->installEventFilter(this);
  m_EditSource->viewport()->installEventFilter(this);
  m_EditSource->setContentsMargins(1, 1, 1, 1);
  m_EditSource->setStyleSheet("border:none");

  connect(m_EditSource->verticalScrollBar(), SIGNAL(valueChanged(int)), this,
          SLOT(editVSBarValueChanged()));
  connect(m_EditSource, &QsciScintilla::textChanged, this,
          &Notes::on_editSource_textChanged);
  ui->frameEdit->layout()->addWidget(m_EditSource);
  m_EditSource->setFocus();

#endif
}

void Notes::showEvent(QShowEvent *event) {
  QWidget::showEvent(event);
  if (!m_initialized) {
    int btn_h = ui->btnNext->height();
    ui->btnDone->setFixedHeight(btn_h);
    ui->btnDone->setFixedWidth(btn_h);
    int m_size = btn_h * 1.0;
    ui->btnDone->setIconSize(QSize(m_size - 9, m_size - 9));

    ui->btnView->setFixedHeight(btn_h);
    ui->btnView->setFixedWidth(btn_h);
    ui->btnView->setIconSize(QSize(m_size, m_size));

#ifndef Q_OS_ANDROID
    QFont font = mw_one->font();
    m_EditSource->setFont(font);
    markdownLexer->setFont(font);
#endif

    m_initialized = true;
  }
}

void Notes::init() {
  int w = this->width();
  if (mw_one->width() > w) w = mw_one->width();
  this->setGeometry(this->x(), mw_one->geometry().y(), w, mw_one->height());
}

void Notes::wheelEvent(QWheelEvent *e) { Q_UNUSED(e); }

Notes::~Notes() { delete ui; }

void Notes::keyReleaseEvent(QKeyEvent *event) { event->accept(); }

void Notes::editVSBarValueChanged() {}

void Notes::resizeEvent(QResizeEvent *event) { Q_UNUSED(event); }

void Notes::on_btnDone_clicked() { saveMainNotes(); }

void Notes::MD2Html(QString mdFile) {
  QString strmd = loadText(mdFile);

  strmd = strmd.replace("images/", "file://" + iniDir + "memo/images/");

  QString htmlString;
  htmlString = markdownToHtmlWithMath(strmd);
  addImagePathToHtml(htmlString);
}

QString Notes::imageToBase64(const QString &path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) return "";
  QByteArray data = file.readAll();
  return "data:image/png;base64," + data.toBase64();
}

void Notes::saveMainNotes() {
#ifndef Q_OS_ANDROID

  mw_one->strLatestModify = tr("Modi Notes");

  if (isTextChange) {
    QString text = m_EditSource->text();
    text = formatMDText(text);
    StringToFile(text, currentMDFile);

    qDebug() << "Save Note: " << currentMDFile;

    updateMDFileToSyncLists(currentMDFile);

    mw_one->m_NotesList->m_dbManager.updateFileIndex(currentMDFile);
  }

  isTextChange = false;

#endif
}

void Notes::updateMDFileToSyncLists(QString currentMDFile) {
  QString zipMD = privateDir + "KnotData/memo/" +
                  QFileInfo(currentMDFile).fileName() + ".zip";

  if (!m_Method->compressFileWithZlib(currentMDFile, zipMD,
                                      Z_DEFAULT_COMPRESSION)) {
    errorInfo = tr("An error occurred while compressing the file.");
    ShowMessage *msg = new ShowMessage(this);
    msg->showMsg("Knot", errorInfo, 1);
    return;
  }

  QString enc_file = m_Method->useEnc(zipMD);
  if (enc_file != "") zipMD = enc_file;

  appendToSyncList(zipMD);
}

bool Notes::eventFilter(QObject *obj, QEvent *evn) {
  QKeyEvent *keyEvent = static_cast<QKeyEvent *>(evn);
  if (evn->type() == QEvent::KeyRelease) {
    if (keyEvent->key() == Qt::Key_Back) {
    }

    if (evn->type() == QEvent::KeyPress) {
    }
  }

  if (evn->type() == QEvent::KeyPress) {
    if (keyEvent->key() == Qt::Key_Escape) {
      close();
      evn->accept();  // 表明这个事件已经被处理
    }
  }

#ifndef Q_OS_ANDROID
  if (obj == m_EditSource) {
    if (evn->type() == QEvent::KeyPress) {
      if (keyEvent->key() != Qt::Key_Back) {
      }
    }
  }

  if (obj == m_EditSource->viewport()) {
    if (evn->type() == QEvent::MouseButtonPress) {
    }
  }
#endif

  return QWidget::eventFilter(obj, evn);
}

void Notes::on_KVChanged() {
  if (!pAndroidKeyboard->isVisible()) {
    this->setGeometry(mw_one->geometry().x(), mw_one->geometry().y(),
                      mw_one->width(), mw_one->mainHeight);
  } else {
    if (newHeight > 0) {
      this->setGeometry(mw_one->geometry().x(), mw_one->geometry().y(),
                        mw_one->width(), newHeight);
    }
  }
}

QString Notes::Deciphering(const QString &fileName) {
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly)) {
    QMessageBox::warning(this, tr("Load Ds File"), file.errorString(),
                         QMessageBox::Yes);
  }

  return QByteArray::fromBase64(file.readAll());

  file.close();
}

QString Notes::getDateTimeStr() {
  int y, m, d, hh, mm, s;
  y = QDate::currentDate().year();
  m = QDate::currentDate().month();
  d = QDate::currentDate().day();
  hh = QTime::currentTime().hour();
  mm = QTime::currentTime().minute();
  s = QTime::currentTime().second();

  QString s_m, s_d, s_hh, s_mm, s_s;
  s_m = QString::number(m);
  if (s_m.length() == 1) s_m = "0" + s_m;

  s_d = QString::number(d);
  if (s_d.length() == 1) s_d = "0" + s_d;

  s_hh = QString::number(hh);
  if (s_hh.length() == 1) s_hh = "0" + s_hh;

  s_mm = QString::number(mm);
  if (s_mm.length() == 1) s_mm = "0" + s_mm;

  s_s = QString::number(s);
  if (s_s.length() == 1) s_s = "0" + s_s;

  QString newname = QString::number(y) + s_m + s_d + "_" + s_hh + s_mm + s_s;
  return newname;
}

void Notes::on_btnPic_clicked() {
  QString fileName;
  fileName = QFileDialog::getOpenFileName(NULL, tr("Knot"), "",
                                          tr("Picture Files (*.*)"));

  insertImage(fileName, false);
}

QString Notes::insertImage(QString fileName, bool isToAndroidView) {
  QFileInfo fi(fileName);
  QString strImage, tarImageFile;
  if (fi.exists()) {
    QDir dir;
    dir.mkpath(iniDir + "memo/images/");

    int rand = QRandomGenerator::global()->generate();
    if (rand < 0) rand = 0 - rand;
    QString strTar = iniDir + "memo/images/" + getDateTimeStr() + "_" +
                     QString::number(rand) + ".png";
    if (QFile(strTar).exists()) QFile(strTar).remove();

    int nLeftMargin = 9 + 9 + 6;

    QImage img(fileName);
    double w, h;
    int new_w, new_h;
    w = img.width();
    h = img.height();

    int w0 = 1024;
    double r = (double)w / h;
    if (w > w0 - nLeftMargin) {
      new_w = w0 - nLeftMargin;
      new_h = new_w / r;

    } else {
      new_w = w;
      new_h = h;
    }

    if (!isAndroid) {
      ShowMessage *msg = new ShowMessage();
      msg->ui->btnCancel->setText(tr("No"));
      msg->ui->btnOk->setText(tr("Yes"));
      bool isYes = msg->showMsg(
          "Knot", tr("Is the original size of the image used?"), 2);
      if (isYes) {
        new_w = w;
        new_h = h;
      }
    }

    QPixmap pix;
    pix = QPixmap::fromImage(img);
    pix =
        pix.scaled(new_w, new_h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pix.save(strTar);
    tarImageFile = strTar;

    strTar = strTar.replace(iniDir + "memo/", "");
    strImage = "\n\n![image](" + strTar + ")\n\n";

    if (!isAndroid) {
#ifndef Q_OS_ANDROID
      m_EditSource->insert(strImage);
#endif
    } else {
      if (isToAndroidView) insertNote(strImage);
    }

    qDebug() << "pic=" << strTar << nLeftMargin;
  }

  QString zipImg =
      privateDir + "KnotData/memo/images/" + QFileInfo(tarImageFile).fileName();
  QFile::copy(tarImageFile, zipImg);
  zipImg = m_Method->useEnc(zipImg);

  appendToSyncList(zipImg);

  return strImage;
}

QStringList Notes::getImgFileFromHtml(QString htmlfile) {
  QStringList list;
  QString strHtml = loadText(htmlfile);
  strHtml = strHtml.replace("><", ">\n<");
  QTextEdit *edit = new QTextEdit;
  edit->setPlainText(strHtml);
  for (int i = 0; i < edit->document()->lineCount(); i++) {
    QString str = getTextEditLineText(edit, i).trimmed();
    if (str.contains("<img src=")) {
      str = str.replace("<img src=", "");
      str = str.replace("/>", "");
      str = str.replace("\"", "");
      str = str.trimmed();
      qDebug() << str;
      list.append(str);
    }
  }
  return list;
}

void Notes::zipMemo() {
  QDir::setCurrent(iniDir);

#ifdef Q_OS_LINUX
  QProcess *pro = new QProcess;
  pro->execute("zip", QStringList() << "-r"
                                    << "memo.zip"
                                    << "memo");
  pro->waitForFinished();
#endif

#ifdef Q_OS_MACOS
  QProcess *pro = new QProcess;
  pro->execute("zip", QStringList() << "-r"
                                    << "memo.zip"
                                    << "memo");
  pro->waitForFinished();
#endif

#ifdef Q_OS_WIN

  QString strZip, strExec, strzip, tagDir;
  tagDir = "memo";
  strZip = iniDir + "memo.zip";
  QTextEdit *txtEdit = new QTextEdit();
  strzip = qApp->applicationDirPath() + "/zip.exe";
  strzip = "\"" + strzip + "\"";
  strZip = "\"" + strZip + "\"";
  strExec = iniDir;
  strExec = "\"" + strExec + "\"";
  QString strCommand1;
  QString strx = "\"" + tagDir + "\"";
  strCommand1 = strzip + " -r " + strZip + " " + strx;
  txtEdit->append(strCommand1);
  QString fileName = iniDir + "zip.bat";
  TextEditToFile(txtEdit, fileName);

  QString exefile = iniDir + "zip.bat";
  QProcess *pro = new QProcess;
  pro->execute("cmd.exe", QStringList() << "/c" << exefile);
  pro->waitForFinished();

#endif

#ifdef Q_OS_ANDROID

  QJniObject javaZipFile = QJniObject::fromString(iniDir + "memo.zip");
  QJniObject javaZipDir = QJniObject::fromString(iniDir + "memo");
  QJniObject m_activity = QNativeInterface::QAndroidApplication::context();
  m_activity.callStaticMethod<void>("com.x/MyActivity", "compressFileToZip",
                                    "(Ljava/lang/String;Ljava/lang/String;)V",
                                    javaZipDir.object<jstring>(),
                                    javaZipFile.object<jstring>());

#endif
}

void Notes::unzip(QString zipfile) {
  deleteDirfile(iniDir + "memo");
  QDir::setCurrent(iniDir);
#ifdef Q_OS_MACOS
  QProcess *pro = new QProcess;
  pro->execute("unzip", QStringList() << "-o" << zipfile << "-d" << iniDir);
  pro->waitForFinished();
#endif

#ifdef Q_OS_LINUX
  QProcess *pro = new QProcess;
  pro->execute("unzip", QStringList() << "-o" << zipfile << "-d" << iniDir);
  pro->waitForFinished();
#endif

#ifdef Q_OS_WIN
  QString strZip, strExec, strUnzip, tagDir;
  tagDir = iniDir;
  strZip = zipfile;
  QTextEdit *txtEdit = new QTextEdit();
  strUnzip = qApp->applicationDirPath() + "/7z.exe";
  qDebug() << qApp->applicationDirPath() << ".....";
  strUnzip = "\"" + strUnzip + "\"";
  strZip = "\"" + strZip + "\"";
  strExec = iniDir;
  strExec = "\"" + strExec + "\"";
  QString strCommand1;
  QString strx = "\"" + tagDir + "\"";
  strCommand1 = strUnzip + " x " + strZip + " -o" + strx + " -y";
  txtEdit->append(strCommand1);
  QString fileName = iniDir + "un.bat";
  TextEditToFile(txtEdit, fileName);

  QProcess::execute("cmd.exe", QStringList() << "/c" << fileName);

#endif

#ifdef Q_OS_ANDROID

  QJniObject javaZipFile = QJniObject::fromString(zipfile);
  QJniObject javaZipDir = QJniObject::fromString(iniDir);
  QJniObject m_activity = QNativeInterface::QAndroidApplication::context();
  m_activity.callStaticMethod<void>(
      "com.x/MyActivity", "Unzip", "(Ljava/lang/String;Ljava/lang/String;)V",
      javaZipFile.object<jstring>(), javaZipDir.object<jstring>());

#endif
}

QString Notes::addImagePathToHtml(QString strhtml) {
  QTextEdit *edit = new QTextEdit;
  QPlainTextEdit *edit1 = new QPlainTextEdit;
  strhtml = strhtml.replace("><", ">\n<");
  edit->setPlainText(strhtml);
  QString str, str_2, str_3;
  for (int i = 0; i < edit->document()->lineCount(); i++) {
    str = getTextEditLineText(edit, i);
    str = str.trimmed();
    if (str.mid(0, 4) == "<img" && str.contains("file://")) {
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

      QStringList list1 = strSrc.split("/memo/");
      if (list1.count() > 1)
        strSrc = "\"file://" + iniDir + "memo/" + list1.at(1) + " ";

      QStringList list2 = str1.split("/memo/");
      if (list2.count() > 1) str_2 = list2.at(1);
      str = "<img src=\"file:///" + iniDir + "memo/" + str_2;
      str = "<a href=" + strSrc + ">" + str + "</a>";

      QStringList list3 = str_2.split("\"");
      if (list3.count() > 0) str_3 = list3.at(0);

      QString imgFile = iniDir + "memo/" + str_3;
      QImage img(imgFile);
      if (img.width() >= mw_one->width() - 25) {
        QString strW = QString::number(mw_one->width() - 25);
        QString a1 = "width = ";
        str = str.replace("/>", a1 + "\"" + strW + "\"" + " />");
      }
    }

    edit1->appendPlainText(str);
  }

  QString strEnd = edit1->toPlainText();
  edit1->clear();
  edit1->setPlainText(strEnd);

  mw_one->m_Reader->PlainTextEditToFile(edit1, htmlFileName);

  return strEnd;
}

void Notes::refreshQMLVPos(qreal newPos) {
  QSettings Reg(privateDir + "notes.ini", QSettings::IniFormat);

  if (QFile(currentMDFile).exists()) {
    Reg.setValue("/MainNotes/SlidePos" + currentMDFile, newPos);
  }
}

qreal Notes::getVPos() { return sliderPos; }

qreal Notes::getVHeight() { return textHeight; }

void Notes::on_btnInsertTable_clicked() {
#ifndef Q_OS_ANDROID

  QString table1 = "|Title1|Title2|\n";
  QString table2 = "|------|------|\n";
  QString table = table1 + table2;
  m_EditSource->insert(table);

#endif
}

void Notes::on_btnS1_clicked() {
#ifndef Q_OS_ANDROID

  QString str = m_EditSource->selectedText();
  if (str == "") str = tr("Bold Italic");
  if (!m_EditSource->hasSelectedText())
    m_EditSource->insert("_**" + str + "**_");
  else
    m_EditSource->replaceSelectedText("_**" + str + "**_");

#endif
}

void Notes::on_btnS2_clicked() {
#ifndef Q_OS_ANDROID

  QString str = m_EditSource->selectedText();
  if (str == "") str = tr("Italic");
  if (!m_EditSource->hasSelectedText())
    m_EditSource->insert("_" + str + "_");
  else
    m_EditSource->replaceSelectedText("_" + str + "_");

#endif
}

void Notes::on_btnS3_clicked() {
#ifndef Q_OS_ANDROID

  QString str = m_EditSource->selectedText();
  if (str == "") str = tr("Underline");

  if (!m_EditSource->hasSelectedText())
    m_EditSource->insert("<u>" + str + "</u>");
  else
    m_EditSource->replaceSelectedText("<u>" + str + "</u>");

#endif
}

void Notes::on_btnS4_clicked() {
#ifndef Q_OS_ANDROID

  QString str = m_EditSource->selectedText();
  if (str == "") str = tr("Strickout");

  if (!m_EditSource->hasSelectedText())
    m_EditSource->insert("~~" + str + "~~");
  else
    m_EditSource->replaceSelectedText("~~" + str + "~~");

#endif
}

void Notes::on_btnColor_clicked() {
#ifndef Q_OS_ANDROID

  QString strColor = m_Method->getCustomColor();
  if (strColor.isEmpty()) return;

  QString str = m_EditSource->selectedText();
  if (str == "") str = tr("Color");
  m_EditSource->insert("<font color=" + strColor + ">" + str + "</font>");

#endif
}

QColor Notes::StringToColor(QString mRgbStr) {
  QColor color(mRgbStr.toUInt(NULL, 16));
  return color;
}

void Notes::on_btnS5_clicked() {
#ifndef Q_OS_ANDROID

  QString str = m_EditSource->selectedText();
  if (str == "") str = tr("Bold");
  if (!m_EditSource->hasSelectedText())
    m_EditSource->insert("**" + str + "**");
  else
    m_EditSource->replaceSelectedText("**" + str + "**");

#endif
}

void Notes::on_btnPaste_clicked() {
#ifndef Q_OS_ANDROID

  const QClipboard *clipboard = QApplication::clipboard();
  const QMimeData *mimeData = clipboard->mimeData();
  if (mimeData->hasImage()) {
    QImage img = qvariant_cast<QImage>(mimeData->imageData());
    if (!img.isNull()) {
      QPixmap pix;
      QString strTar = privateDir + "temppic.png";
      pix = QPixmap::fromImage(img);
      pix = pix.scaled(img.width(), img.height(), Qt::KeepAspectRatio,
                       Qt::SmoothTransformation);
      pix.save(strTar);
      insertImage(strTar, false);
    }
  } else
    m_EditSource->paste();

#endif
}

bool Notes::eventFilterQwNote(QObject *watch, QEvent *event) {
  return QWidget::eventFilter(watch, event);
}

void Notes::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);
  return;
}

void Notes::timerSlot() {
  if (bCursorVisible) {
    bCursorVisible = false;
  } else {
    bCursorVisible = true;
  }

  emit sendUpdate();
}

void Notes::closeEvent(QCloseEvent *event) {
  Q_UNUSED(event);
  saveEditorState(currentMDFile);

#ifndef Q_OS_ANDROID
  strNoteText = m_EditSource->text().trimmed();
#endif

  m_Method->Sleep(100);

  if (isTextChange) {
    ShowMessage *msg = new ShowMessage(this);
    msg->ui->btnOk->setText(tr("Yes") + " (Y)");
    msg->ui->btnCancel->setText(tr("No") + " (N)");
    if (msg->showMsg(tr("Notes"), tr("Do you want to save the notes?"), 2)) {
      saveMainNotes();
    }
  }

  if (isSetNewNoteTitle()) {
    TitleGenerator generator;
    new_title = generator.genNewTitle(strNoteText);
    mui->btnRename->click();
  }
}

void Notes::syncToWebDAV() {
  if (mui->chkAutoSync->isChecked() && mui->chkWebDAV->isChecked()) {
    if (notes_sync_files.count() > 0) {
      m_CloudBackup->uploadFilesToWebDAV(notes_sync_files);
    }
  }
}

bool Notes::isSetNewNoteTitle() {
  QString title = mw_one->m_NotesList->noteTitle;
  if (title.trimmed() == "无标题笔记" || title.trimmed() == "Untitled Note") {
    return true;
  }

  return false;
}

void Notes::on_editSource_textChanged() { isTextChange = true; }

void Notes::show_findText() {
#ifndef Q_OS_ANDROID

#endif
}

void Notes::show_findTextBack() {}

void Notes::findText() {}

void Notes::on_btnFind_clicked() {
  if (ui->editFind->text().trimmed() == "") return;
  show_findText();
}

void Notes::on_btnPrev_clicked() {
  ui->editFind->setFocus();
  searchPrevious();
}

void Notes::on_btnNext_clicked() {
  ui->editFind->setFocus();
  searchNext();
}

void Notes::on_editFind_returnPressed() { searchNext(); }

void Notes::on_editFind_textChanged(const QString &arg1) {
  searchText(arg1.trimmed(), true);
  // searchWithCount(arg1.trimmed());
  m_lastSearchText = arg1.trimmed();
}

bool Notes::selectPDFFormat(QPrinter *printer) {
  QSettings settings;

  // select the page size
  QStringList pageSizeStrings;
  pageSizeStrings << QStringLiteral("A0") << QStringLiteral("A1")
                  << QStringLiteral("A2") << QStringLiteral("A3")
                  << QStringLiteral("A4") << QStringLiteral("A5")
                  << QStringLiteral("A6") << QStringLiteral("A7")
                  << QStringLiteral("A8") << QStringLiteral("A9")
                  << tr("Letter");
  QList<QPageSize::PageSizeId> pageSizes;
  pageSizes << QPageSize::A0 << QPageSize::A1 << QPageSize::A2 << QPageSize::A3
            << QPageSize::A4 << QPageSize::A5 << QPageSize::A6 << QPageSize::A7
            << QPageSize::A8 << QPageSize::A9 << QPageSize::Letter;

  PrintPDF *idlg1 = new PrintPDF(this);
  QString pageSizeString =
      idlg1->getItem(tr("Page size"), tr("Page size"), pageSizeStrings, 4);

  /*QString pageSizeString = QInputDialog::getItem(
      this, tr("Page size"), tr("Page size:"), pageSizeStrings,
      settings.value(QStringLiteral("Printer/NotePDFExportPageSize"), 4)
          .toInt(),
      false, &ok);*/

  if (pageSizeString.isEmpty()) {
    return false;
  }

  int pageSizeIndex = pageSizeStrings.indexOf(pageSizeString);
  if (pageSizeIndex == -1) {
    return false;
  }

  QPageSize pageSize(pageSizes.at(pageSizeIndex));
  settings.setValue(QStringLiteral("Printer/NotePDFExportPageSize"),
                    pageSizeIndex);
  printer->setPageSize(pageSize);

  // select the orientation
  QStringList orientationStrings;
  orientationStrings << tr("Portrait") << tr("Landscape");
  QList<QPageLayout::Orientation> orientations;
  orientations << QPageLayout::Portrait << QPageLayout::Landscape;

  PrintPDF *idlg2 = new PrintPDF(this);
  QString orientationString = idlg2->getItem(
      tr("Orientation"), tr("Orientation"), orientationStrings, 0);

  /*QString orientationString = QInputDialog::getItem(
      this, tr("Orientation"), tr("Orientation:"), orientationStrings,
      settings.value(QStringLiteral("Printer/NotePDFExportOrientation"), 0)
          .toInt(),
      false, &ok);*/

  if (orientationString.isEmpty()) {
    return false;
  }

  int orientationIndex = orientationStrings.indexOf(orientationString);
  if (orientationIndex == -1) {
    return false;
  }

  printer->setPageOrientation(orientations.at(orientationIndex));
  settings.setValue(QStringLiteral("Printer/NotePDFExportOrientation"),
                    orientationIndex);

#ifdef Q_OS_ANDROID
  pdfFileName = "/storage/emulated/0/KnotBak/" +
                mw_one->m_NotesList->noteTitle + QStringLiteral(".pdf");
#else
  QFileDialog dialog(NULL, QStringLiteral("NotePDFExport"));
  dialog.setFileMode(QFileDialog::AnyFile);
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  dialog.setNameFilter(tr("PDF files") + QStringLiteral(" (*.pdf)"));
  dialog.setWindowTitle(tr("Export current note as PDF"));
  dialog.selectFile(mw_one->m_NotesList->noteTitle + QStringLiteral(".pdf"));
  int ret = dialog.exec();

  if (ret != QDialog::Accepted) {
    return false;
  }

  pdfFileName = dialog.selectedFiles().at(0);
#endif

  if (pdfFileName.isEmpty()) {
    return false;
  }

  if (QFileInfo(pdfFileName).suffix().isEmpty()) {
    fileName.append(QLatin1String(".pdf"));
  }

  printer->setOutputFormat(QPrinter::PdfFormat);
  printer->setOutputFileName(pdfFileName);

  return true;
}

void Notes::on_btnPDF_clicked() {
  MD2Html(currentMDFile);
  QString html = loadText(htmlFileName);
  html = html.replace("file://", "");
  auto doc = new QTextDocument(this);
  doc->setHtml(html);

  auto *printer = new QPrinter(QPrinter::HighResolution);

  if (selectPDFFormat(printer)) {
    doc->print(printer);

    if (isAndroid) {
      m_Method->m_widget = new QWidget(this);
      ShowMessage *msg1 = new ShowMessage(this);
      msg1->ui->btnCancel->setText(tr("No"));
      msg1->ui->btnOk->setText(tr("Yes"));
      if (msg1->showMsg("PDF",
                        tr("The PDF file is successfully exported.") + "\n\n" +
                            tr("Want to share this PDF file?") + "\n\n" +
                            pdfFileName,
                        2)) {
        if (QFile::exists(pdfFileName)) {
          mw_one->m_ReceiveShare->shareImage(tr("Share to"), pdfFileName,
                                             "*/*");
        }
      }
    }
  }

  delete printer;
}

void Notes::editNote() { openEditUI(); }

void Notes::showNoteList() { openNotesUI(); }

void Notes::on_editNote() {
  timerEditNote->stop();
  openEditUI();
}

void Notes::setOpenSearchResultForAndroid(bool isValue, QString strSearchText) {
  Q_UNUSED(isValue);
  Q_UNUSED(strSearchText);
#ifdef Q_OS_ANDROID

  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  if (activity.isValid()) {
    // 调用Java方法，注意方法签名(Z)V
    activity.callMethod<void>("setOpenSearchResult", "(Z)V", isValue);

    QJniObject jFile = QJniObject::fromString(strSearchText);
    activity.callMethod<void>("setSearchText", "(Ljava/lang/String;)V",
                              jFile.object<jstring>());
  }

#endif
}

void Notes::openAndroidNoteEditor() {
  setOpenSearchResultForAndroid(mw_one->isOpenSearchResult,
                                mw_one->mySearchText);

#ifdef Q_OS_ANDROID

  // Qt6 实现：通过 Native Interface 获取 Activity
  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  activity.callMethod<void>("openNoteEditor", "()V");

#endif

  mw_one->isOpenSearchResult = false;
}

void Notes::openMDWindow() {
#ifdef Q_OS_ANDROID

  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  activity.callStaticMethod<void>("com.x/MyActivity", "openMDWindow", "()V");

#endif
}

void Notes::appendNote(QString str) {
  Q_UNUSED(str);
#ifdef Q_OS_ANDROID

  QJniObject jTitle = QJniObject::fromString(str);
  QJniObject m_activity = QNativeInterface::QAndroidApplication::context();
  m_activity.callStaticMethod<void>("com.x/NoteEditor", "appendNote",
                                    "(Ljava/lang/String;)V",
                                    jTitle.object<jstring>());

#endif
}

void Notes::insertNote(QString str) {
  Q_UNUSED(str);
#ifdef Q_OS_ANDROID

  QJniObject jTitle = QJniObject::fromString(str);
  QJniObject m_activity = QNativeInterface::QAndroidApplication::context();
  m_activity.callStaticMethod<void>("com.x/NoteEditor", "insertNote",
                                    "(Ljava/lang/String;)V",
                                    jTitle.object<jstring>());

#endif
}

auto Notes::getAndroidNoteConfig(QString key) {
  QSettings Reg(privateDir + "note_text.ini", QSettings::IniFormat);

  auto value = Reg.value(key);
  return value;
}

void Notes::setAndroidNoteConfig(QString key, QString value) {
  QSettings Reg(privateDir + "note_text.ini", QSettings::IniFormat);

  QString mdFileName = QFileInfo(currentMDFile).baseName();
  if (Reg.value("/cpos/" + mdFileName).toString() == "")
    Reg.setValue("/cpos/" + mdFileName, "0");

  Reg.setValue(key, value);
}

void Notes::delImage() {
  ShowMessage *m_ShowMsg = new ShowMessage(this);
  if (!m_ShowMsg->showMsg("Knot", tr("Delete this image?"), 2)) return;

  QFileInfo fi(imgFileName);
  QString name = fi.fileName();
  QString buffers = loadText(currentMDFile);

  int startPos, endPos, length;
  int index = 0;
  QStringList imgTitleList;
  while (buffers.indexOf("![", index) != -1) {
    startPos = buffers.indexOf("![", index) + 2;
    if (startPos - 2 >= 0) {
      endPos = buffers.indexOf("]", startPos + 1);
      length = endPos - startPos;
      QString subStr = buffers.mid(startPos, length);
      imgTitleList.append(subStr);
      qDebug() << "delImage():" << startPos << length << subStr;
      index = endPos + 1;
    }
  }

  for (int i = 0; i < imgTitleList.count(); i++) {
    QString title = imgTitleList.at(i);
    QString strImg =
        "![" + title + "](file://" + imgDir + "memo/images/" + name + ")";
    buffers.replace(strImg, "");
  }

  qreal oldPos = getVPos();
  QImage img(imgFileName);
  int nImagHeight = img.height();
  qreal newPos = oldPos - nImagHeight;
  refreshQMLVPos(newPos);

  buffers = formatMDText(buffers);
  StringToFile(buffers, currentMDFile);

  mw_one->on_btnBackImg_clicked();
}

void Notes::delLink(QString link) {
  QString mdBuffers = loadText(currentMDFile);

  if (!mdBuffers.contains(link)) {
    link.replace("http://", "");
  }
  mdBuffers.replace(link, "");
  mdBuffers.replace("[]()", "");

  if (mdBuffers.contains("]()")) {
    int startPos, endPos, length;
    int index = 0;
    QStringList titleList;
    while (mdBuffers.indexOf("]()", index) != -1) {
      startPos = mdBuffers.indexOf("[", index) + 1;
      if (startPos - 2 >= 0) {
        endPos = mdBuffers.indexOf("]()", startPos + 1);
        length = endPos - startPos;
        QString subStr = mdBuffers.mid(startPos, length);
        titleList.append(subStr);
        qDebug() << "delLink():" << startPos << length << subStr;
        index = endPos + 1;
      }
    }

    for (int i = 0; i < titleList.count(); i++) {
      QString title = titleList.at(i);
      QString str = "[" + title + "]()";
      mdBuffers.replace(str, "");
    }
  }

  mdBuffers = formatMDText(mdBuffers);
  StringToFile(mdBuffers, currentMDFile);
}

void Notes::javaNoteToQMLNote() {
  if (isSetNewNoteTitle()) {
    TitleGenerator generator;
    if (isAndroid) {
      strNoteText = loadText(currentMDFile);
    }
    new_title = generator.genNewTitle(strNoteText);
    mui->btnRename->click();
  }

  QString zipMD = privateDir + "KnotData/memo/" +
                  QFileInfo(currentMDFile).fileName() + ".zip";

  if (!m_Method->compressFileWithZlib(currentMDFile, zipMD,
                                      Z_DEFAULT_COMPRESSION)) {
    errorInfo = tr("An error occurred while compressing the file.");
    ShowMessage *msg = new ShowMessage(this);
    msg->showMsg("Knot", errorInfo, 1);
    return;
  }

  QString enc_file = m_Method->useEnc(zipMD);
  if (enc_file != "") zipMD = enc_file;

  appendToSyncList(zipMD);

  mw_one->m_NotesList->m_dbManager.updateFileIndex(currentMDFile);
}

QString Notes::formatMDText(QString text) {
  for (int i = 0; i < 10; i++) text.replace("\n\n\n", "\n\n");

  return text;
}

void Notes::init_all_notes() {
  mw_one->m_NotesList->initNotesList();
  mw_one->m_NotesList->initRecycle();

  // load note
  currentMDFile = mw_one->m_NotesList->getCurrentMDFile();
  qDebug() << "currentMDFile=" << currentMDFile;
  if (QFile::exists(currentMDFile)) {
  } else {
    loadEmptyNote();
  }
}

void Notes::loadEmptyNote() {
  currentMDFile = "";
  MD2Html(currentMDFile);

  mui->lblNoteGraphView->setText("");
  mw_one->m_NotesList->noteTitle = "";
}

void Notes::openNotesUI() {
  init_all_notes();

  isSaveNotesConfig = false;

  mw_one->m_NotesList->needDelNotes();

  mw_one->isMemoVisible = true;
  mw_one->isReaderVisible = false;

  mui->frameMain->hide();
  mui->frameNoteList->show();

  mw_one->m_NotesList->set_memo_dir();

  if (tw->topLevelItemCount() == 0) {
    mui->lblNoteBook->setText(tr("Note Book"));
    mui->lblNoteList->setText(tr("Note List"));
    return;
  }

  mw_one->m_NotesList->loadAllNoteBook();
  mw_one->m_NotesList->localNotesItem();
  mw_one->m_NotesList->setNoteLabel();

  mw_one->closeProgress();

  if (isRequestOpenNoteEditor) {
    isRequestOpenNoteEditor = false;
    openEditUI();
  }
}

bool NoteIndexManager::loadIndex(const QString &indexPath) {
  QFile file(indexPath);
  if (!file.open(QIODevice::ReadOnly)) return false;

  QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
  QJsonObject root = doc.object();
  QJsonObject data = root["data"].toObject();

  m_index.clear();
  for (auto it = data.begin(); it != data.end(); ++it) {
    m_index.insert(it.key(), it.value().toString());
  }

  m_currentIndexPath = indexPath;
  return true;
}

bool NoteIndexManager::saveIndex(const QString &indexPath) {
  QJsonObject root;
  root["version"] = 1.0;

  QJsonObject data;
  for (auto it = m_index.constBegin(); it != m_index.constEnd(); ++it) {
    data.insert(it.key(), it.value());
  }
  root["data"] = data;

  QFile file(indexPath);
  if (!file.open(QIODevice::WriteOnly)) return false;

  file.write(QJsonDocument(root).toJson());
  return true;
}

QString NoteIndexManager::getNoteTitle(const QString &filePath) const {
  return m_index.value(QDir::cleanPath(filePath),
                       QFileInfo(filePath).baseName());
}

void NoteIndexManager::setNoteTitle(const QString &filePath,
                                    const QString &title) {
  QString cleanPath = QDir::cleanPath(filePath);
  if (title.isEmpty()) {
    m_index.remove(cleanPath);
  } else {
    m_index[cleanPath] = title;
  }
}

void Notes::openEditUI() {
  qDebug() << "currentMDFile=" << currentMDFile;
  if (!QFile::exists(currentMDFile)) {
    ShowMessage *msg = new ShowMessage(mw_one);
    msg->showMsg(appName,
                 tr("The current note does not exist. Please select another "
                    "note or create a new note."),
                 0);

    return;
  }

  mw_one->m_NotesList->refreshRecentOpen(mw_one->m_NotesList->noteTitle);
  mw_one->m_NotesList->saveRecentOpen();

  if (isAndroid) {
    m_Method->setMDFile(currentMDFile);
    setAndroidNoteConfig("/cpos/currentMDFile",
                         QFileInfo(currentMDFile).baseName());

    openAndroidNoteEditor();
    return;
  }

#ifndef Q_OS_ANDROID

  mw_one->mainHeight = mw_one->height();

  init();

  QString mdString = loadText(currentMDFile);
  m_EditSource->setWrapMode(QsciScintilla::WrapNone);
  m_EditSource->setText(mdString);
  m_EditSource->setWrapMode(QsciScintilla::WrapWord);  // 按单词换行

  show();

  m_Method->Sleep(100);

  restoreEditorState(currentMDFile);
  m_EditSource->setFocus();

  m_Method->Sleep(200);

  isTextChange = false;

  if (mw_one->isOpenSearchResult) {
    QString findText = mw_one->mySearchText;
    if (findText.length() > 0) {
      m_EditSource->SendScintilla(QsciScintilla::SCI_SETANCHOR, 0);
      m_EditSource->SendScintilla(QsciScintilla::SCI_SETCURRENTPOS, 0);

      ui->editFind->setText(findText);
      on_btnNext_clicked();
    }
  }

  mw_one->isOpenSearchResult = false;

#endif
}

void Notes::openNotes() {
  mw_one->m_NotesList->needDelWebDAVFiles.clear();
  isPasswordError = false;

  if (mui->chkAutoSync->isChecked() && mui->chkWebDAV->isChecked()) {
    mw_one->showProgress();

    m_CloudBackup->createRemoteWebDAVDir();

    orgRemoteDateTime.clear();
    orgRemoteFiles.clear();
    remoteFiles.clear();
    QString url = m_CloudBackup->getWebDAVArgument();

    // get md files
    m_CloudBackup->getRemoteFileList(url + "KnotData/memo/");
    while (!m_CloudBackup->isGetRemoteFileListEnd)
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    qDebug() << m_CloudBackup->webdavFileList
             << m_CloudBackup->webdavDateTimeList;
    for (int i = 0; i < m_CloudBackup->webdavFileList.count(); i++) {
      orgRemoteFiles.append(m_CloudBackup->webdavFileList.at(i));
      orgRemoteDateTime.append(m_CloudBackup->webdavDateTimeList.at(i));
    }

    // get md image files
    m_CloudBackup->getRemoteFileList(url + "KnotData/memo/images");
    while (!m_CloudBackup->isGetRemoteFileListEnd)
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    qDebug() << m_CloudBackup->webdavFileList
             << m_CloudBackup->webdavDateTimeList;
    for (int i = 0; i < m_CloudBackup->webdavFileList.count(); i++) {
      orgRemoteFiles.append(m_CloudBackup->webdavFileList.at(i));
      orgRemoteDateTime.append(m_CloudBackup->webdavDateTimeList.at(i));
    }

    WebDavHelper *helper =
        listWebDavFiles(url + "KnotData/", m_CloudBackup->USERNAME,
                        m_CloudBackup->APP_PASSWORD);
    helper->setParent(this);

    // 连接信号
    QObject::connect(
        helper, &WebDavHelper::listCompleted, this,
        [=](const QList<QPair<QString, QDateTime>> &files) {
          qDebug() << "获取到文件列表:";
          qDebug() << "共找到" << files.size() << "个文件:";

          if (files.size() == 0) {
            openNotesUI();
            return;
          }

          for (const auto &[path, mtime] : files) {
            qDebug() << "路径:" << path
                     << "修改时间:" << mtime.toString("yyyy-MM-dd hh:mm:ss");
            QString remote_f = path;
            remote_f = remote_f.replace("/dav/", "");  // 此处需注意

            orgRemoteFiles.append(remote_f);
            orgRemoteDateTime.append(mtime);

            if (path.contains("mainnotes.ini.zip")) break;
          }

          for (int j = 0; j < orgRemoteFiles.count(); j++) {
            QString or_file = orgRemoteFiles.at(j);
            QDateTime or_datetime = orgRemoteDateTime.at(j);

            QString local_file = privateDir + or_file;

            QDateTime local_datetime = QFileInfo(local_file).lastModified();

            // 先设置时区
            local_datetime.setTimeZone(QTimeZone::systemTimeZone());
            // 再统一时区
            local_datetime = local_datetime.toTimeZone(QTimeZone::utc());

            if (or_datetime > local_datetime || !QFile::exists(local_file)) {
              remoteFiles.append(or_file);
            }
          }

          if (remoteFiles.count() > 0) {
            // 初始化下载器
            WebDavDownloader *downloader = new WebDavDownloader(
                m_CloudBackup->USERNAME, m_CloudBackup->APP_PASSWORD, this);

            // 连接信号
            QObject::connect(downloader, &WebDavDownloader::progressChanged,
                             [](int current, int total, QString file) {
                               qDebug() << QString("进度: %1/%2  当前文件: %3")
                                               .arg(current)
                                               .arg(total)
                                               .arg(file);
                             });

            QObject::connect(
                downloader, &WebDavDownloader::downloadFinished, this,
                [=](bool success, QString error) {
                  qDebug() << (success ? "下载成功" : "下载失败: " + error);
                  if (success) {
                    startBackgroundProcessRemoteFiles();
                  } else {
                    qDebug() << "下载失败：" << error;
                    ShowMessage *msg = new ShowMessage(this);
                    msg->showMsg(
                        appName,
                        tr("Synchronization failed. Please try again later."),
                        1);
                    openNotesUI();
                  }
                });

            // 开始下载（1并发,根据文件的下载个数）
            QString lf = privateDir;
            qDebug() << "lf=" << lf;
            int maxConcurrentDownloads = 1;
            downloader->downloadFiles(remoteFiles, lf, maxConcurrentDownloads);
          }

          if (remoteFiles.count() == 0) openNotesUI();
        });

    QObject::connect(helper, &WebDavHelper::errorOccurred, this,
                     [=](const QString &error) {
                       qDebug() << "操作失败:" << error;
                       openNotesUI();
                     });
  } else

    openNotesUI();
}

void Notes::startBackgroundProcessRemoteFiles() {
  QFuture<void> future =
      QtConcurrent::run([=]() { processRemoteFiles(remoteFiles); });

  // 使用 QFutureWatcher 监控进度
  QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
  connect(watcher, &QFutureWatcher<void>::finished, [=]() {
    qDebug() << "Remote files process completed";
    openNotesUI();
    watcher->deleteLater();
  });
  watcher->setFuture(future);
}

void Notes::processRemoteFiles(QStringList remoteFiles) {
  for (int i = 0; i < remoteFiles.count(); i++) {
    QString file = remoteFiles.at(i);
    QString pDir, pFile, kFile, asFile, zFile;
    pFile = privateDir + file;
    zFile = pFile;
    asFile = file;

    if (file.contains("mainnotes.ini.zip")) {
      pDir = privateDir + "KnotData";
      pFile = pFile.replace(".zip", "");
      kFile = iniDir + asFile.replace("KnotData/", "");
      kFile = kFile.replace(".zip", "");
      qDebug() << "file=" << file;
      qDebug() << "pFile=" << pFile;
      qDebug() << "kFile" << kFile;

      QString dec_file = m_Method->useDec(zFile);
      if (dec_file != "") zFile = dec_file;

      if (!m_Method->decompressFileWithZlib(zFile, pFile)) {
        mw_one->closeProgress();
        errorInfo =
            tr("Decompression failed. Please check in "
               "Preferences that the passwords are "
               "consistent across all platforms.");

        ShowMessage *msg = new ShowMessage();
        msg->showMsg("Knot", errorInfo, 1);
        isPasswordError = true;
        QFile::remove(zFile);
        return;
      }

      if (isPasswordError == false) {
        QFileInfo pFileInfo(pFile);
        QFileInfo kFileInfo(kFile);
        if (pFileInfo.lastModified() > kFileInfo.lastModified()) {
          QFile::remove(kFile);
          if (QFile::copy(pFile, kFile)) {
            qDebug() << "kFile:" << kFile << " Update successfully. ";
          }
        }
      } else {
        QFile::remove(zFile);
      }
    }

    if (file.contains(".md.zip")) {
      pDir = privateDir + "KnotData/memo";
      pFile = pFile.replace(".zip", "");
      kFile = iniDir + asFile.replace("KnotData/", "");
      kFile = kFile.replace(".zip", "");
      qDebug() << "file=" << file;
      qDebug() << "pFile=" << pFile;
      qDebug() << "kFile=" << kFile;
      qDebug() << "zFile=" << zFile;

      QString dec_file = m_Method->useDec(zFile);
      if (dec_file != "") zFile = dec_file;

      if (QFile::exists(zFile)) {
        if (!m_Method->decompressFileWithZlib(zFile, pFile)) {
          mw_one->closeProgress();
          errorInfo =
              tr("Decompression failed. Please check in "
                 "Preferences that the passwords are "
                 "consistent across all platforms.");

          ShowMessage *msg = new ShowMessage();
          msg->showMsg("Knot", errorInfo, 1);
          isPasswordError = true;
          QFile::remove(zFile);
          QFile::remove(privateDir + "KnotData/mainnotes.ini.zip");
          return;
        }
      }

      if (isPasswordError == false) {
        QFileInfo pFileInfo(pFile);
        QFileInfo kFileInfo(kFile);
        if (pFileInfo.lastModified() > kFileInfo.lastModified()) {
          QFile::remove(kFile);
          QFile::copy(pFile, kFile);
          mw_one->m_NotesList->m_dbManager.updateFileIndex(kFile);
        }
      } else {
        QFile::remove(zFile);
      }
    }

    if (file.contains(".png")) {
      pFile = m_Method->useDec(pFile);
      kFile = iniDir + asFile.replace("KnotData/", "");

      qDebug() << "file=" << file;
      qDebug() << "pFile=" << pFile;
      qDebug() << "kFile" << kFile;

      QFileInfo pFileInfo(pFile);
      QFileInfo kFileInfo(kFile);
      if (pFileInfo.lastModified() > kFileInfo.lastModified()) {
        QFile::remove(kFile);
        QFile::copy(pFile, kFile);
      }
    }
  }
}

void Notes::updateMainnotesIniToSyncLists() {
  qDebug() << "isSaveNotesConfig=" << isSaveNotesConfig;

  if (isSaveNotesConfig) {
    QString zipMainnotes = privateDir + "KnotData/mainnotes.ini.zip";

    if (!m_Method->compressFileWithZlib(iniDir + "mainnotes.ini", zipMainnotes,
                                        Z_DEFAULT_COMPRESSION)) {
      errorInfo = tr("An error occurred while compressing the file.");
      ShowMessage *msg = new ShowMessage(this);
      msg->showMsg("Knot", errorInfo, 1);
      return;
    }

    QString enc_file = m_Method->useEnc(zipMainnotes);
    if (enc_file != "") zipMainnotes = enc_file;

    appendToSyncList(zipMainnotes);
  }
}

void Notes::initMarkdownLexer() {
#ifndef Q_OS_ANDROID

  // 创建 Lexer
  markdownLexer = new QsciLexerMarkdown(m_EditSource);
  m_EditSource->setLexer(markdownLexer);

  m_EditSource->SendScintilla(
      QsciScintilla::SCI_STYLERESETDEFAULT);  // Scintilla 重置
  m_EditSource->SendScintilla(QsciScintilla::SCI_STYLERESETDEFAULT);
  m_EditSource->setCaretForegroundColor(QColor(0, 0, 0));  // 光标颜色
  m_EditSource->recolor();

  // 验证 Lexer
  qDebug() << "Lexer 状态：" << (m_EditSource->lexer() ? "已设置" : "未设置");
  if (m_EditSource->lexer()) {
    qDebug() << "Lexer 语言：" << m_EditSource->lexer()->language();
  }

  // 调试输出当前字体
  qDebug() << "代码块字体："
           << markdownLexer->font(QsciLexerMarkdown::CodeBlock).family();

  // 打印所有样式详情
  for (int i = 0; i < QsciLexerMarkdown::Header1; ++i) {
    qDebug() << "Style" << i << "Desc:" << markdownLexer->description(i)
             << "Color:" << markdownLexer->color(i)
             << "BG:" << markdownLexer->paper(i);
  }

  qDebug() << "Header1 颜色："
           << markdownLexer->color(QsciLexerMarkdown::Header1);

#endif
}

void Notes::initMarkdownLexerDark() {
#ifndef Q_OS_ANDROID

  //  创建前确保清空原有 Lexer
  m_EditSource->setLexer(nullptr);

  //  创建深色模式基础配置
  markdownLexer = new QsciLexerMarkdown(m_EditSource);

  //  设置全局默认颜色（必须首先配置）
  markdownLexer->setDefaultPaper(QColor("#1E1E1E"));  // 全局背景色
  markdownLexer->setDefaultColor(QColor("#D4D4D4"));  // 全局默认文本颜色

  //  按样式类型逐个配置（覆盖所有 Markdown 元素）
  markdownLexer->setColor(QColor("#569CD6"),
                          QsciLexerMarkdown::Header1);  // H1 蓝色
  markdownLexer->setColor(QColor("#4EC9B0"),
                          QsciLexerMarkdown::Header2);  // H2 青蓝色
  markdownLexer->setColor(QColor("#C586C0"),
                          QsciLexerMarkdown::Header3);  // H3 粉紫色
  markdownLexer->setColor(QColor("#9CDCFE"),
                          QsciLexerMarkdown::Link);  // 链接 浅蓝
  markdownLexer->setColor(QColor("#CE9178"),
                          QsciLexerMarkdown::CodeBlock);  // 代码块文字
  markdownLexer->setPaper(QColor("#2D2D2D"),
                          QsciLexerMarkdown::CodeBlock);  // 代码块背景

  markdownLexer->setColor(QColor("#D7BA7D"),
                          QsciLexerMarkdown::BlockQuote);  // 引用块

  // 应用 Lexer
  m_EditSource->setLexer(markdownLexer);

  // 禁止自动恢复默认样式（关键！）
  // m_EditSource->SendScintilla(QsciScintilla::SCI_STYLERESETDEFAULT); //
  // 不要调用这个！

  // 附加视觉优化
  m_EditSource->setCaretLineBackgroundColor(QColor("#2D2D30"));  // 当前行高亮
  m_EditSource->setCaretForegroundColor(QColor("#FFFFFF"));      // 光标颜色
  m_EditSource->setMarginsBackgroundColor(QColor("#1E1E1E"));    // 行号栏背景
  m_EditSource->setMarginsForegroundColor(QColor("#858585"));    // 行号颜色
  m_EditSource->setWrapMode(QsciScintilla::WrapWord);            // 自动换行

  // 强制刷新颜色
  m_EditSource->recolor();

#endif
}

#ifndef Q_OS_ANDROID
void Notes::initMarkdownEditor(QsciScintilla *editor) {
  // 强制编码和默认样式
  // editor->setUtf8(true);

  // 设置字体（继承默认字体避免冲突）
  QFont defaultFont = QFont(this->font().family(), fontSize);
  markdownLexer->setFont(defaultFont, -1);  // -1 表示所有样式使用该字体
  markdownLexer->setFont(defaultFont, QsciLexerMarkdown::CodeBlock);

  editor->setFolding(QsciScintilla::BoxedTreeFoldStyle);

  // 自动缩进
  editor->setAutoIndent(true);

  // 括号匹配
  editor->setBraceMatching(QsciScintilla::SloppyBraceMatch);

  // 显示行号
  editor->setMarginLineNumbers(1, true);

  // 配置行号边距（Margin 0）
  editor->setMarginType(0, QsciScintilla::NumberMargin);  // 声明行号边距类型

  // 设置行号字体
  editor->SendScintilla(QsciScintilla::SCI_STYLESETFONT,
                        QsciScintilla::STYLE_LINENUMBER,
                        QFontInfo(QFont("Consolas", 10)).family().toUtf8());

  // 获取当前字体（需与行号字体一致）
  QFont font("Consolas", 10);  // 或使用编辑器当前字体：m_EditSource->font()

  // 创建字体度量对象
  QFontMetrics metrics(font);

  // 计算最大行号文本宽度（例如 " 9999 "）
  int maxLineNumber = 10000;  // 预估最大行号
  int textWidth = metrics.horizontalAdvance(QString(" %1 ").arg(maxLineNumber));

  // 设置行号边距宽度
  editor->setMarginWidth(0, textWidth + 4);  // +4 像素作为边距缓冲

  if (isDark) {
    // === 暗黑主题配色 ===
    editor->setMarginsBackgroundColor(
        QColor(0x1E, 0x1E, 0x1E));  // 边距背景色 #1E1E1E
    editor->setMarginsForegroundColor(
        QColor(0x7F, 0x7F, 0x7F));  // 行号文字色 #7F7F7F

  } else {
    // ===== 亮色模式配置 =====
    // 行号边距背景色（浅灰白，RGB(240,240,240)）
    editor->setMarginsBackgroundColor(QColor(240, 240, 240));

    // 行号文字颜色（中灰色，RGB(96,96,96)）
    editor->setMarginsForegroundColor(QColor(96, 96, 96));
  }

  // 配置符号边距（Margin 1）
  editor->setMarginType(1, QsciScintilla::SymbolMargin);
  editor->setMarginWidth(1, 5);

  // 边距2：断点符号边距
  editor->setMarginType(2, QsciScintilla::SymbolMargin);
  editor->setMarginWidth(2, 5);
  editor->setMarginBackgroundColor(2, QColor("#FFE4E1"));

  // editor->setMarginWidth(1, 0);  // Margin 1 不可见
  // editor->setMarginWidth(2, 0);  // Margin 2 不可见

  // 高亮当前行
  editor->setCaretWidth(2);  // 光标宽度
  editor->setCaretLineVisible(true);
  if (isDark) {
    editor->setCaretLineBackgroundColor(QColor(45, 45, 48, 60));  // 半透明
  } else
    editor->setCaretLineBackgroundColor(QColor(255, 248, 220, 255));

  // 显示代码折叠
  editor->SendScintilla(QsciScintilla::SCI_SETPROPERTY, "fold", "1");
  editor->SendScintilla(QsciScintilla::SCI_SETFOLDFLAGS,
                        16);  // 带边框的折叠图标

  // 自动换行配置
  editor->setWrapVisualFlags(QsciScintilla::WrapFlagByText);  // 视觉标记
  editor->setWrapIndentMode(QsciScintilla::WrapIndentSame);   // 缩进对齐

  // 滚动条控制
  editor->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  // 隐藏水平条 按需显示垂直条
  editor->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  // 边缘参考线
  // editor->setEdgeMode(QsciScintilla::EdgeLine);
  // editor->setEdgeColumn(100);
  // editor->setEdgeColor(QColor("#FFA07A"));

  // 配置搜索高亮指示器
  const int INDICATOR_SEARCH = 1;  // 使用一个未使用的指示器编号
  editor->SendScintilla(QsciScintilla::SCI_INDICSETSTYLE, INDICATOR_SEARCH,
                        QsciScintilla::INDIC_ROUNDBOX);
  editor->SendScintilla(QsciScintilla::SCI_INDICSETFORE, INDICATOR_SEARCH,
                        QColor(Qt::yellow).rgb());
  editor->SendScintilla(QsciScintilla::SCI_INDICSETALPHA, INDICATOR_SEARCH,
                        100);
  editor->SendScintilla(QsciScintilla::SCI_INDICSETUNDER, INDICATOR_SEARCH,
                        true);  // 在文字下方绘制
}

#endif

// 查找关键词
void Notes::searchText(const QString &text, bool forward) {
#ifndef Q_OS_ANDROID

  m_lastSearchText = text;

  // 调整光标起始位置，避免重复匹配
  int line, index;
  m_EditSource->getCursorPosition(&line, &index);
  if (!forward) {
    if (index > 0) {
      index--;  // 向后搜索时，光标左移一个字符
    } else if (line > 0) {
      line--;
      index = m_EditSource->lineLength(line);  // 跳转到上一行末尾
    }
    m_EditSource->setCursorPosition(line, index);
  }

  // 参数顺序：text, 正则, 区分大小写, 全词匹配, 循环搜索, 向前
  bool found =
      m_EditSource->findFirst(text, false, false, false, true, forward);

  if (!found) {
    // QMessageBox::information(this, "搜索",
    //                          forward ? "已到达文档末尾" : "已到达文档开头");
  }

#endif
}

// 查找下一个
void Notes::searchNext() {
  if (!m_lastSearchText.isEmpty()) {
    searchText(m_lastSearchText, true);  // 使用 true 表示向前搜索
  }
}

// 查找上一个
void Notes::searchPrevious() {
  if (!m_lastSearchText.isEmpty()) {
    searchText(m_lastSearchText, false);  // 使用 false 表示向后搜索
  }
}

void Notes::jumpToNextMatch() {
#ifndef Q_OS_ANDROID

  if (m_matchPositions.isEmpty()) return;

  m_currentMatchIndex = (m_currentMatchIndex + 1) % m_matchPositions.size();
  auto pos = m_matchPositions[m_currentMatchIndex];

  // 设置选择范围
  m_EditSource->SendScintilla(QsciScintilla::SCI_SETSELECTIONSTART, pos.first);
  m_EditSource->SendScintilla(QsciScintilla::SCI_SETSELECTIONEND, pos.second);

  // 滚动到可见区域
  int line = m_EditSource->SendScintilla(QsciScintilla::SCI_LINEFROMPOSITION,
                                         pos.first);
  m_EditSource->ensureLineVisible(line);

#endif
}

void Notes::jumpToPrevMatch() {
#ifndef Q_OS_ANDROID

  if (m_matchPositions.isEmpty()) return;

  m_currentMatchIndex = (m_currentMatchIndex - 1 + m_matchPositions.size()) %
                        m_matchPositions.size();
  auto pos = m_matchPositions[m_currentMatchIndex];

  // 设置选择范围
  m_EditSource->SendScintilla(QsciScintilla::SCI_SETSELECTIONSTART, pos.first);
  m_EditSource->SendScintilla(QsciScintilla::SCI_SETSELECTIONEND, pos.second);

  // 滚动到可见区域
  int line = m_EditSource->SendScintilla(QsciScintilla::SCI_LINEFROMPOSITION,
                                         pos.first);
  m_EditSource->ensureLineVisible(line);

#endif
}

// 获取搜索结果的匹配总数
int Notes::getSearchMatchCount(const QString &text) {
  if (text.isEmpty()) return 0;

#ifndef Q_OS_ANDROID

  // 保存当前编辑状态
  int originalPos =
      m_EditSource->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS);
  int originalAnchor =
      m_EditSource->SendScintilla(QsciScintilla::SCI_GETANCHOR);

  // 初始化搜索参数
  int count = 0;
  bool found = m_EditSource->findFirst(text,
                                       false,  // 不使用正则表达式
                                       false,  // 不区分大小写（根据需求调整）
                                       false,  // 不全词匹配（根据需求调整）
                                       false,  // 禁用循环查找
                                       true);  // 正向搜索

  // 遍历所有匹配项
  while (found) {
    count++;
    found = m_EditSource->findNext();
  }

  // 恢复原始状态
  m_EditSource->SendScintilla(QsciScintilla::SCI_SETCURRENTPOS, originalPos);
  m_EditSource->SendScintilla(QsciScintilla::SCI_SETANCHOR, originalAnchor);

  return count;

#endif
}

void Notes::openBrowserOnce(const QString &htmlPath) {
  QDesktopServices::openUrl(QUrl::fromLocalFile(htmlPath));
}

void Notes::on_btnView_clicked() {
  ui->btnDone->click();
  mui->btnOpenNote->click();
}

void Notes::init_md() {
#ifndef Q_OS_ANDROID
  if (isDark) {
    initMarkdownLexerDark();
    m_EditSource->verticalScrollBar()->setStyleSheet(
        mw_one->m_MainHelper->darkPCScrollbarStyle);
  } else {
    initMarkdownLexer();
    m_EditSource->verticalScrollBar()->setStyleSheet(
        mw_one->m_MainHelper->lightPCScrollbarStyle);
  }

  initMarkdownEditor(m_EditSource);
#endif
}

#include <QSettings>

void Notes::saveEditorState(const QString &filePath) {
#ifndef Q_OS_ANDROID

  // 指定 INI 格式和文件路径
  QSettings settings(privateDir + "editor_config.ini", QSettings::IniFormat);

  // 使用文件路径作为分组名（需处理特殊字符）
  QString groupName = "Documents/" + QFileInfo(filePath).canonicalFilePath();
  groupName.replace("/", "_");  // 避免 / 导致分组层级问题
  settings.beginGroup(groupName);

  // 保存光标位置
  int line, index;
  m_EditSource->getCursorPosition(&line, &index);
  settings.setValue("cursorLine", line);
  settings.setValue("cursorIndex", index);
  settings.setValue("vsbar",
                    m_EditSource->verticalScrollBar()->sliderPosition());

  settings.endGroup();

#endif
}

void Notes::restoreEditorState(const QString &filePath) {
#ifndef Q_OS_ANDROID

  QSettings settings(privateDir + "editor_config.ini", QSettings::IniFormat);

  QString groupName = "Documents/" + QFileInfo(filePath).canonicalFilePath();
  groupName.replace("/", "_");
  settings.beginGroup(groupName);

  // 恢复光标
  int line = settings.value("cursorLine", 0).toInt();
  int index = settings.value("cursorIndex", 0).toInt();
  int vsbar = settings.value("vsbar", 0).toInt();

  m_EditSource->verticalScrollBar()->setSliderPosition(vsbar);
  m_EditSource->setCursorPosition(line, index);

  settings.endGroup();

#endif
}

void Notes::previewNote() {
  if (!QFile::exists(currentMDFile)) return;

  mw_one->showProgress();
  QString title = mw_one->m_NotesList->noteTitle;

  QFuture<void> future = QtConcurrent::run([=]() {
    mw_one->m_NotesList->refreshRecentOpen(title);
    mw_one->m_NotesList->saveRecentOpen();
    MD2Html(currentMDFile);
  });

  // 使用 QFutureWatcher 监控进度
  QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
  connect(watcher, &QFutureWatcher<void>::finished, this, [=]() {
    mw_one->closeProgress();

    if (isAndroid) {
      m_Method->setMDTitle(title);

      m_Method->setMDFile(currentMDFile);
      // openMDWindow();

      openLocalHtmlFileInAndroid();

      setAndroidNoteConfig("/cpos/currentMDFile",
                           QFileInfo(currentMDFile).baseName());

      return;
    } else {
      openBrowserOnce(htmlFileName);
    }

    qDebug() << "Preview note completed";
    watcher->deleteLater();
  });
  watcher->setFuture(future);
}

void Notes::appendToSyncList(QString file) {
  notes_sync_files.removeOne(file);
  notes_sync_files.append(file);
  qDebug() << "Add to Notes Sync List ====>>>>" << file;
}

// 调用Android方法打开本地HTML文件
void Notes::openLocalHtmlFileInAndroid() {
#ifdef Q_OS_ANDROID
  // 调用主Activity的静态方法启动WebView
  QJniObject::callStaticMethod<void>("com/x/MyActivity",  // 替换为实际包名
                                     "launchWebView",  // 主Activity中的方法名
                                     "()V"             // 方法签名
  );

  // 检查异常
  QJniEnvironment env;
  if (env->ExceptionCheck()) {
    qDebug() << "启动WebView失败";
    env->ExceptionClear();
  }
#endif
}
