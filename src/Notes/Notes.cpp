#include "src/Notes/Notes.h"

#include "cmark-gfm-core-extensions.h"
#include "cmark_wrapper.h"
#include "lib/md4c/md4c-html.h"
#include "src/MainWindow.h"
#include "subscript.h"
#include "superscript.h"
#include "ui_MainWindow.h"
#include "ui_Notes.h"

extern MainWindow *mw_one;
extern Method *m_Method;
extern QString iniFile, iniDir, privateDir, currentMDFile, imgFileName, appName,
    encPassword, errorInfo;
extern bool isAndroid, isIOS, isDark, isNeedSync, isPasswordError;
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

QString markdownToHtml(const QString &markdown, int options);
QString markdownToHtmlWithMath(const QString &md);
TextSelector *m_TextSelector = nullptr;

NoteIndexManager::NoteIndexManager(QObject *parent) : QObject{parent} {}

Notes::Notes(QWidget *parent) : QDialog(parent), ui(new Ui::Notes) {
  ui->setupUi(this);
  m_NoteIndexManager = new NoteIndexManager();
  m_EditSource1 = new QTextEditHighlighter();

  m_TextSelector = new TextSelector(this);

  initEditor();
  m_EditSource->setUtf8(true);
  init_md();

  QString path = iniDir + "memo/";
  QDir dir(path);
  if (!dir.exists()) dir.mkdir(path);

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

  mw_one->set_ToolButtonStyle(this);
}

void Notes::initEditor() {
  m_EditSource = new QsciScintilla(this);

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
}

void Notes::showEvent(QShowEvent *event) {
  QWidget::showEvent(event);
  if (!m_initialized) {
    // 调试输出
    qDebug() << "当前字体是否有效："
             << m_EditSource->lexer()
                    ->font(QsciLexerMarkdown::CodeBlock)
                    .exactMatch();

    int btn_h = ui->btnNext->height();
    ui->btnDone->setFixedHeight(btn_h);
    ui->btnDone->setFixedWidth(btn_h);
    int m_size = btn_h * 0.8;
    ui->btnDone->setIconSize(QSize(m_size, m_size));

    ui->btnView->setFixedHeight(btn_h);
    ui->btnView->setFixedWidth(btn_h);
    ui->btnView->setIconSize(QSize(m_size, m_size));

    QFont font = mw_one->font();
    m_EditSource->setFont(font);
    markdownLexer->setFont(font);

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

void Notes::on_btnDone_clicked() {
  saveMainNotes();

  if (!mw_one->ui->frameNotes->isHidden()) {
    MD2Html(currentMDFile);
    loadNoteToQML();
  }
}

void Notes::MD2Html(QString mdFile) {
  QString htmlFileName = privateDir + "memo.html";

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
  mw_one->isNeedAutoBackup = true;
  mw_one->strLatestModify = tr("Modi Notes");

  saveQMLVPos();

  if (isTextChange) {
    QString text = m_EditSource->text();
    text = formatMDText(text);
    StringToFile(text, currentMDFile);

    qDebug() << "Save Note: " << currentMDFile;

    updateMDFileToSyncLists(currentMDFile);

    mw_one->m_NotesList->m_dbManager.updateFileIndex(currentMDFile);
  }

  isTextChange = false;
  isNeedSync = true;
}

void Notes::updateMDFileToSyncLists(QString currentMDFile) {
  QString zipMD = privateDir + "KnotData/memo/" +
                  QFileInfo(currentMDFile).fileName() + ".zip";

  // m_Method->compressFile(zipMD, currentMDFile, encPassword);

  if (!m_Method->compressFileWithZlib(currentMDFile, zipMD,
                                      Z_DEFAULT_COMPRESSION)) {
    errorInfo = tr("An error occurred while compressing the file.");
    ShowMessage *msg = new ShowMessage(this);
    msg->showMsg("Knot", errorInfo, 1);
    return;
  }

  QString enc_file = m_Method->useEnc(zipMD);
  if (enc_file != "") zipMD = enc_file;

  notes_sync_files.append(zipMD);
}

void Notes::getEditPanel(QTextEdit *textEdit, QEvent *evn) {
  QMouseEvent *event = static_cast<QMouseEvent *>(evn);
  byTextEdit = textEdit;

  QPalette pt = palette();
  pt.setBrush(QPalette::Text, Qt::black);
  pt.setBrush(QPalette::Base, QColor(255, 255, 255));
  pt.setBrush(QPalette::Highlight, QColor(90, 90, 255));
  pt.setBrush(QPalette::HighlightedText, Qt::white);
  textEdit->setPalette(pt);

  if (event->type() == QEvent::MouseButtonPress) {
    if (event->button() == Qt::LeftButton) {
      isMousePress = true;
      isMouseMove = false;

      if (!isAndroid) {
        m_TextSelector->on_btnClose_clicked();
      }

      px = event->globalX();
      py = event->globalY();

      int a = 100;
      int hy = py - a - m_TextSelector->height();
      if (hy >= 0)
        y1 = hy;
      else
        y1 = py + a;

#ifdef Q_OS_ANDROID
      m_TextSelector->setFixedWidth(mw_one->width() - 6);

      if (textEdit == mw_one->ui->editTodo)
        y1 = mw_one->geometry().y() + mw_one->ui->editTodo->y() +
             mw_one->ui->editTodo->height() + 2;

      if (textEdit == mw_one->ui->editDetails)
        y1 = mw_one->geometry().y() + mw_one->ui->editDetails->y() +
             mw_one->ui->editDetails->height() + 2;
#else

#endif

      textEdit->cursor().setPos(event->globalPos());

      if (m_TextSelector->isHidden()) {
        if (isAndroid) {
          if (!pAndroidKeyboard->isVisible()) {
            pAndroidKeyboard->setVisible(true);
          }
          timerEditPanel->start(1000);
        }
      }
    }
  }

  if (event->type() == QEvent::MouseButtonRelease) {
    isMouseRelease = true;
    isMousePress = false;
    isMouseMove = false;

    if (m_TextSelector->ui->lineEdit->text() != "") {
      m_TextSelector->show();
      if (isFunShow) {
        isFunShow = false;

        m_TextSelector->init(y1);
        textEdit->setFocus();

        QTextCursor cursor = textEdit->textCursor();
        cursor.setPosition(start);
        cursor.setPosition(end, QTextCursor::KeepAnchor);
        textEdit->setTextCursor(cursor);
      }
    }
  }

  if (event->type() == QEvent::MouseMove) {
    isMouseMove = true;
    if (isMousePress) {
      textEdit->cursor().setPos(event->globalPos());

      mx = event->globalX();
      my = event->globalY();

      if (mx <= px) {
        m_TextSelector->on_btnClose_clicked();
        return;
      }

      QString str = textEdit->textCursor().selectedText().trimmed();

      end = textEdit->textCursor().position();
      start = end - textEdit->textCursor().selectedText().length();

      QTextCursor cursor;
      cursor = byTextEdit->textCursor();
      cursor.setPosition(start);
      cursor.setPosition(end, QTextCursor::KeepAnchor);
      byTextEdit->setTextCursor(cursor);

      m_TextSelector->ui->lineEdit->setText(str);
      if (str != "") {
        m_TextSelector->init(y1);
      }
    }
  }
}

bool Notes::eventFilter(QObject *obj, QEvent *evn) {
  if (obj == m_EditSource->viewport()) {
    // getEditPanel(m_EditSource, evn);
  }

  QKeyEvent *keyEvent = static_cast<QKeyEvent *>(evn);
  if (evn->type() == QEvent::KeyRelease) {
    if (keyEvent->key() == Qt::Key_Back) {
      if (!m_TextSelector->isHidden()) {
        m_TextSelector->close();
        return true;
      }

      if (pAndroidKeyboard->isVisible()) {
        pAndroidKeyboard->hide();
        setGeometry(mw_one->geometry().x(), mw_one->geometry().y(), width(),
                    mw_one->mainHeight);
        return true;
      }

      close();

      return true;
    }
  }

  // Keyboard
  if (obj == m_EditSource) {
    if (evn->type() == QEvent::KeyPress) {
      if (keyEvent->key() != Qt::Key_Back) {
      }
    }
  }

  // Mouse
  if (obj == m_EditSource->viewport()) {
    if (evn->type() == QEvent::MouseButtonPress) {
    }
  }

#ifdef Q_OS_ANDROID
  if (obj == m_EditSource->viewport()) {
    if (evn->type() == QEvent::MouseButtonDblClick) {
      y1 = 2;
      isMousePress = true;
      on_showEditPanel();
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

      if (!m_TextSelector->isHidden()) {
        m_TextSelector->setGeometry(m_TextSelector->geometry().x(), 10,
                                    m_TextSelector->width(),
                                    m_TextSelector->height());
      }
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

    int w0 = mw_one->width();
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
      m_EditSource->insert(strImage);
    } else {
      if (isToAndroidView) insertNote(strImage);
    }

    qDebug() << "pic=" << strTar << nLeftMargin;
  }

  QString zipImg =
      privateDir + "KnotData/memo/images/" + QFileInfo(tarImageFile).fileName();
  QFile::copy(tarImageFile, zipImg);
  zipImg = m_Method->useEnc(zipImg);
  notes_sync_files.append(zipImg);

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
  QString htmlFileName = privateDir + "memo.html";

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

void Notes::loadNoteToQML() {
  if (isAndroid) {
    return;
  }

  QString htmlFileName = privateDir + "memo.html";

  // new method
  setWebViewFile(htmlFileName);
}

void Notes::refreshQMLVPos(qreal newPos) {
  QSettings Reg(privateDir + "notes.ini", QSettings::IniFormat);

  if (QFile(currentMDFile).exists()) {
    Reg.setValue("/MainNotes/SlidePos" + currentMDFile, newPos);
  }
}

void Notes::saveQMLVPos() {
  QSettings Reg(privateDir + "notes.ini", QSettings::IniFormat);

  QString strTag = currentMDFile;
  strTag.replace(iniDir, "");

  Reg.setValue("/MainNotes/editVPos" + strTag,
               m_EditSource->verticalScrollBar()->value());
  int cursorPos = m_EditSource->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS);
  Reg.setValue("/MainNotes/editCPos" + strTag, cursorPos);

  if (QFile(currentMDFile).exists()) {
    sliderPos = getVPos();
    Reg.setValue("/MainNotes/SlidePos" + currentMDFile, sliderPos);
  }
}

void Notes::setVPos() {
  QSettings Reg(privateDir + "notes.ini", QSettings::IniFormat);

  sliderPos = Reg.value("/MainNotes/SlidePos" + currentMDFile).toReal();
  qreal m_pos = sliderPos;

  qreal textHeight = getVHeight();
  qDebug() << "textHeight=" << textHeight << "m_pos=" << m_pos;
}

qreal Notes::getVPos() { return sliderPos; }

qreal Notes::getVHeight() { return textHeight; }

void Notes::on_btnInsertTable_clicked() {
  QString table1 = "|Title1|Title2|\n";
  QString table2 = "|------|------|\n";
  QString table = table1 + table2;
  m_EditSource->insert(table);
}

void Notes::on_btnS1_clicked() {
  QString str = m_EditSource->selectedText();
  if (str == "") str = tr("Bold Italic");
  if (!m_EditSource->hasSelectedText())
    m_EditSource->insert("_**" + str + "**_");
  else
    m_EditSource->replaceSelectedText("_**" + str + "**_");
}

void Notes::on_btnS2_clicked() {
  QString str = m_EditSource->selectedText();
  if (str == "") str = tr("Italic");
  if (!m_EditSource->hasSelectedText())
    m_EditSource->insert("_" + str + "_");
  else
    m_EditSource->replaceSelectedText("_" + str + "_");
}

void Notes::on_btnS3_clicked() {
  QString str = m_EditSource->selectedText();
  if (str == "") str = tr("Underline");

  if (!m_EditSource->hasSelectedText())
    m_EditSource->insert("<u>" + str + "</u>");
  else
    m_EditSource->replaceSelectedText("<u>" + str + "</u>");
}

void Notes::on_btnS4_clicked() {
  QString str = m_EditSource->selectedText();
  if (str == "") str = tr("Strickout");

  if (!m_EditSource->hasSelectedText())
    m_EditSource->insert("~~" + str + "~~");
  else
    m_EditSource->replaceSelectedText("~~" + str + "~~");
}

void Notes::on_btnColor_clicked() {
  QString strColor = m_Method->getCustomColor();
  if (strColor.isEmpty()) return;

  QString str = m_EditSource->selectedText();
  if (str == "") str = tr("Color");
  m_EditSource->insert("<font color=" + strColor + ">" + str + "</font>");
}

QColor Notes::StringToColor(QString mRgbStr) {
  QColor color(mRgbStr.toUInt(NULL, 16));
  return color;
}

void Notes::on_btnS5_clicked() {
  QString str = m_EditSource->selectedText();
  if (str == "") str = tr("Bold");
  if (!m_EditSource->hasSelectedText())
    m_EditSource->insert("**" + str + "**");
  else
    m_EditSource->replaceSelectedText("**" + str + "**");
}

void Notes::on_btnPaste_clicked() {
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
}

bool Notes::eventFilterQwNote(QObject *watch, QEvent *event) {
  return QWidget::eventFilter(watch, event);
}

bool Notes::eventFilterEditTodo(QObject *watch, QEvent *evn) {
  if (watch == mw_one->ui->editTodo->viewport()) {
    getEditPanel(mw_one->ui->editTodo, evn);

    if (evn->type() == QEvent::MouseButtonDblClick) {
      y1 = mw_one->geometry().y() + mw_one->ui->editTodo->y() +
           mw_one->ui->editTodo->height() + 2;
      isMousePress = true;
      on_showEditPanel();
    }
  }

  return QWidget::eventFilter(watch, evn);
}

bool Notes::eventFilterEditRecord(QObject *watch, QEvent *evn) {
  if (watch == mw_one->ui->editDetails->viewport()) {
    mw_one->m_Notes->getEditPanel(mw_one->ui->editDetails, evn);

    if (evn->type() == QEvent::MouseButtonDblClick) {
      y1 = mw_one->geometry().y() + mw_one->ui->editDetails->y() +
           mw_one->ui->editDetails->height() + 2;
      isMousePress = true;
      on_showEditPanel();
    }
  }

  return QWidget::eventFilter(watch, evn);
}

void Notes::on_showEditPanel() {
  timerEditPanel->stop();
  if (isMousePress) {
    isFunShow = true;

    QTextCursor cursor;
    start = byTextEdit->textCursor().position();
    end = start + 2;
    cursor = byTextEdit->textCursor();
    cursor.setPosition(start);
    cursor.setPosition(end, QTextCursor::KeepAnchor);
    byTextEdit->setTextCursor(cursor);

    m_TextSelector->ui->lineEdit->setText(cursor.selectedText());

    m_TextSelector->init(y1);
  }
}

void Notes::selectText(int start, int end) {
  QTextCursor cursor;
  cursor = byTextEdit->textCursor();
  cursor.setPosition(start);
  cursor.setPosition(end, QTextCursor::KeepAnchor);
  byTextEdit->setTextCursor(cursor);
  m_TextSelector->ui->lineEdit->setText(cursor.selectedText());
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

  strNoteText = m_EditSource->text().trimmed();

  if (!m_TextSelector->isHidden()) {
    m_TextSelector->close();
  }

  m_Method->Sleep(100);

  if (isTextChange) {
    ShowMessage *msg = new ShowMessage(this);
    if (msg->showMsg(tr("Notes"), tr("Do you want to save the notes?"), 2)) {
      saveMainNotes();
    }
  }

  if (isSetNewNoteTitle()) {
    if (strNoteText.length() > 20)
      new_title = strNoteText.mid(0, 20).trimmed() + "...";
    else
      new_title = strNoteText;
    mw_one->ui->btnRename->click();
  }
}

void Notes::syncToWebDAV() {
  if (isNeedSync && mw_one->ui->chkAutoSync->isChecked() &&
      mw_one->ui->chkWebDAV->isChecked()) {
    if (notes_sync_files.count() > 0) {
      m_CloudBackup->uploadFilesToWebDAV(notes_sync_files);
    }
    isNeedSync = false;
  }
}

bool Notes::isSetNewNoteTitle() {
  QString title = mw_one->ui->lblNoteName->text();
  if (title.trimmed() == "") {
    return true;
  }

  return false;
}

void Notes::on_editSource_textChanged() { isTextChange = true; }

void Notes::show_findText() {
  QString findtext = ui->editFind->text().trimmed().toLower();
  if (findtext == "") return;
  // 获得对话框的内容
  if (m_EditSource1->find(findtext, QTextDocument::FindCaseSensitively))
  // 查找后一个
  {
    // 查找到后高亮显示
    QPalette palette = m_EditSource->palette();
    palette.setColor(QPalette::Highlight,
                     palette.color(QPalette::Active, QPalette::Highlight));
    m_EditSource1->setPalette(palette);
  } else {
    m_Method->m_widget = new QWidget(this);
    ShowMessage *m_ShowMsg = new ShowMessage(this);
    m_ShowMsg->showMsg("Knot", tr("The end of the document has been reached."),
                       0);
  }
}

void Notes::show_findTextBack() {
  QString findtext = ui->editFind->text().trimmed().toLower();
  if (findtext == "") return;
  // 获得对话框的内容
  if (m_EditSource1->find(findtext, QTextDocument::FindBackward))
  // 查找后一个
  {
    // 查找到后高亮显示
    QPalette palette = m_EditSource1->palette();
    palette.setColor(QPalette::Highlight,
                     palette.color(QPalette::Active, QPalette::Highlight));
    m_EditSource1->setPalette(palette);
  } else {
    m_Method->m_widget = new QWidget(this);
    ShowMessage *m_ShowMsg = new ShowMessage(this);
    m_ShowMsg->showMsg(
        "Knot", tr("The beginning of the document has been reached."), 0);
  }
}

void Notes::findText() {
  QString search_text = ui->editFind->text().trimmed().toLower();
  if (search_text.trimmed().isEmpty()) {
    return;
  } else {
    QTextDocument *document = m_EditSource1->document();
    bool found = false;
    QTextCursor highlight_cursor(document);
    QTextCursor cursor(document);
    // 开始
    cursor.beginEditBlock();
    QTextCharFormat color_format(highlight_cursor.charFormat());
    color_format.setForeground(Qt::red);
    while (!highlight_cursor.isNull() && !highlight_cursor.atEnd()) {
      // 查找指定的文本，匹配整个单词
      highlight_cursor = document->find(search_text, highlight_cursor,
                                        QTextDocument::FindCaseSensitively);
      if (!highlight_cursor.isNull()) {
        if (!found) found = true;
        highlight_cursor.mergeCharFormat(color_format);
      }
    }
    cursor.endEditBlock();
    // 结束
    if (found == false) {
      QMessageBox::information(this, tr("Word not found"),
                               tr("Sorry,the word cannot be found."));
    }
  }
}

void Notes::on_btnFind_clicked() {
  if (ui->editFind->text().trimmed() == "") return;
  show_findText();
}

void Notes::on_btnPrev_clicked() { searchPrevious(); }

void Notes::on_btnNext_clicked() { searchNext(); }

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
                mw_one->ui->lblNoteName->text() + QStringLiteral(".pdf");
#else
  QFileDialog dialog(NULL, QStringLiteral("NotePDFExport"));
  dialog.setFileMode(QFileDialog::AnyFile);
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  dialog.setNameFilter(tr("PDF files") + QStringLiteral(" (*.pdf)"));
  dialog.setWindowTitle(tr("Export current note as PDF"));
  dialog.selectFile(mw_one->ui->lblNoteName->text() + QStringLiteral(".pdf"));
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
  QString html = loadText(privateDir + "memo.html");
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

void Notes::editNote() { mw_one->on_btnEdit_clicked(); }

void Notes::showNoteList() { mw_one->on_btnNotesList_clicked(); }

void Notes::on_editNote() {
  timerEditNote->stop();
  mw_one->on_btnEdit_clicked();
}

void Notes::setEditorVPos() {
  QSettings Reg(privateDir + "notes.ini", QSettings::IniFormat);

  qreal pos = 0;
  if (QFile(currentMDFile).exists()) {
    pos = Reg.value("/MainNotes/Editor" + currentMDFile, 0).toReal();
  }
}

void Notes::showTextSelector() {
  m_TextSelector->setGeometry(mw_one->geometry().x(), mw_one->geometry().y(),
                              mw_one->width(), m_TextSelector->height());
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
  activity.callMethod<void>("openMDWindow", "()V");

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
    QString mdString = loadText(currentMDFile).trimmed();
    if (mdString.length() > 20)
      new_title = mdString.mid(0, 20).trimmed() + "...";
    else
      new_title = mdString;
    mw_one->ui->btnRename->click();
  }

#ifdef Q_OS_ANDROID
  QJniObject m_activity =
      QJniObject(QNativeInterface::QAndroidApplication::context());
  if (m_activity.callMethod<jdouble>("getEditStatus", "()D") == 1) {
    mw_one->ui->btnOpenNote->click();
  }
#endif

  QString zipMD = privateDir + "KnotData/memo/" +
                  QFileInfo(currentMDFile).fileName() + ".zip";

  // m_Method->compressFile(zipMD, currentMDFile, encPassword);

  if (!m_Method->compressFileWithZlib(currentMDFile, zipMD,
                                      Z_DEFAULT_COMPRESSION)) {
    errorInfo = tr("An error occurred while compressing the file.");
    ShowMessage *msg = new ShowMessage(this);
    msg->showMsg("Knot", errorInfo, 1);
    return;
  }

  QString enc_file = m_Method->useEnc(zipMD);
  if (enc_file != "") zipMD = enc_file;

  notes_sync_files.append(zipMD);

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

  setVPos();
}

void Notes::loadEmptyNote() {
  currentMDFile = "";
  MD2Html(currentMDFile);

  mw_one->ui->lblNoteName->setText("");
}

void Notes::setWebViewFile(QString htmlfile) {}

void Notes::saveWebScrollPos(QString mdfilename) {}

QString markdownToHtml(const QString &markdown, int options) {
  // 处理空输入
  if (markdown.isEmpty()) {
    return QString();
  }

  // 转换为 UTF-8 字节数组（自动管理内存）
  QByteArray mdUtf8 = markdown.toUtf8();

  // 执行转换（使用字节数组的实际长度，避免 C 字符串截断问题）
  char *html_cstr = cmark_markdown_to_html(
      mdUtf8.constData(),  // 原始数据指针
      mdUtf8.size(),       // 数据实际长度（重要！不用 strlen）
      options);

  // 检查转换结果
  if (!html_cstr) {
    return QString();  // 返回空表示失败
  }

  // 转换为 Qt 字符串
  QString html = QString::fromUtf8(html_cstr);

  // 释放 CMARK 分配的内存
  free(html_cstr);

  return html;
}

QString markdownToHtmlWithMath(const QString &md) {
  // 初始化所有 GitHub 扩展
  cmark_gfm_core_extensions_ensure_registered();

  // 创建解析器并启用关键选项
  cmark_parser *parser = cmark_parser_new(
      CMARK_OPT_TABLE_PREFER_STYLE_ATTRIBUTES |  // 表格样式优化
      CMARK_OPT_UNSAFE                           // 保留原始字符（如 $）
  );

  // 附加所有需要的扩展
  const char *extensions[] = {"table", "strikethrough", "tasklist", "autolink",
                              "tagfilter"};
  for (const char *ext_name : extensions) {
    if (cmark_syntax_extension *ext = cmark_find_syntax_extension(ext_name)) {
      cmark_parser_attach_syntax_extension(parser, ext);
    }
  }

  // 解析 Markdown
  QByteArray utf8 = md.toUtf8();
  cmark_parser_feed(parser, utf8.constData(), utf8.size());
  cmark_node *doc = cmark_parser_finish(parser);
  cmark_parser_free(parser);

  // 渲染 HTML（保留原始内容）
  char *html_cstr = cmark_render_html(doc, CMARK_OPT_UNSAFE, nullptr);
  QString html = QString::fromUtf8(html_cstr);

  // --- 新增：处理 Mermaid 代码块 ---
  QRegularExpression mermaidCodeBlock(
      R"(<pre><code class="language-mermaid">(.*?)</code></pre>)",
      QRegularExpression::DotMatchesEverythingOption);
  html.replace(mermaidCodeBlock, R"(<div class="mermaid">\1</div>)");

  // --- 新增：定义 Mermaid 脚本 ---
  QString mermaid_js = R"(
      <script
  src="https://cdn.jsdelivr.net/npm/mermaid@9/dist/mermaid.min.js"></script>
      <script>
          document.addEventListener('DOMContentLoaded', function() {
              mermaid.initialize({ startOnLoad: true, theme: 'neutral' });
              mermaid.init();
          });
      </script>
  )";

  // 处理转义字符
  // html.replace("\\~", "~");
  // html.replace("\\^", "^");

  // 处理下标
  // html.replace(QRegularExpression(R"((?<!\\)~([^~]|\\~)+~)"),
  // "<sub>\\1</sub>");
  // 处理上标
  // html.replace(QRegularExpression(R"((?<!\\)\^([^^]|\\\^)+\^)"),
  //             "<sup>\\1</sup>");

  // 插入 MathJax 和语法高亮支持
  QString mathjax_config = R"(
        <script>
            MathJax = {
                tex: {
                    inlineMath: [['$', '$'], ['\\(', '\\)']],
                    displayMath: [['$$', '$$'], ['\\[', '\\]']],
                    processEscapes: true,
                    packages: {'[+]': ['base', 'ams', 'newcommand']}
                },
                options: {
                    ignoreHtmlClass: 'tex-ignore',
                    processHtmlClass: 'tex-process'
                },
                startup: {
                    typeset: false
                }
            };
        </script>
        <script src="https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-mml-chtml.js"></script>
        <script>MathJax.startup.document.state(0); MathJax.startup.defaultReady();</script>
    )";

  QString highlight_js = R"(
        <link rel="stylesheet"
    href="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.7.0/styles/vs.min.css">
        <script
    src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.7.0/highlight.min.js"></script>
        <script>
            document.addEventListener('DOMContentLoaded', (event) => {
                document.querySelectorAll('pre code').forEach((el) => {
                    hljs.highlightElement(el);
                });
                if (typeof MathJax !== 'undefined') {
                    MathJax.typesetPromise();
                }
            });
        </script>
    )";

  // 添加 CSS 样式（表格边框、代码块背景等）
  QString custom_css = R"(
        <style>
            table {
                border-collapse: collapse;
                margin: 1em 0;
                border: 1px solid #dee2e6;
            }
            th, td {
                padding: 0.75em;
                border: 1px solid #dee2e6;
            }
            th {
                background-color: #f8f9fa;
                font-weight: 600;
            }
            pre code {
                display: block;
                padding: 1em;
                background: #f8f9fa;
                border-radius: 4px;
                overflow-x: auto;
            }
            .tex-process {
                color: #d63384; /* 数学公式预览颜色 */
            }
            code { /* 新增行内代码样式 */
                background: #f8f9fa;
                padding: 0.2em 0.4em;
                border-radius: 3px;
                font-family: monospace;
            }
            pre code { /* 保持代码块样式 */ }
            code { /* 行内代码样式 */ }
            sup {
                vertical-align: super;
                font-size: smaller;
            }
            sub {
                vertical-align: sub;
                font-size: smaller;
            }

            /* 新增块引用样式 */
            blockquote {
                border-left: 4px solid #dee2e6;  /* 左侧竖线颜色（与表格边框一致） */
                padding-left: 1.5em;             /* 左侧缩进距离 */
                margin-left: 0;                  /* 取消默认外边距 */
                color: #6c757d;                  /* 文字颜色（与副文本颜色一致） */
                margin: 1em 0;                   /* 上下外边距 */
            }

        </style>
    )";

  // 组合完整 HTML
  html =
      "<!DOCTYPE html><html><head>"
      "<meta charset='utf-8'>" +
      mathjax_config + mermaid_js + highlight_js + custom_css +
      "</head><body>" + html + "</body></html>";

  // 清理资源
  free(html_cstr);
  cmark_node_free(doc);

  return html;
}

void Notes::openNotesUI() {
  init_all_notes();

  mw_one->isMemoVisible = true;
  mw_one->isReaderVisible = false;

  mw_one->ui->frameMain->hide();
  mw_one->ui->frameNotes->show();
  setVPos();

  mw_one->ui->btnNotesList->click();

  mw_one->closeProgress();

  mainnotesLastModi = QFileInfo(iniDir + "mainnotes.ini").lastModified();
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
    mw_one->on_btnNotesList_clicked();
    return;
  }

  if (isAndroid) {
    m_Method->setMDFile(currentMDFile);
    setAndroidNoteConfig("/cpos/currentMDFile",
                         QFileInfo(currentMDFile).baseName());

    openAndroidNoteEditor();
    return;
  }

  m_TextSelector->close();
  delete m_TextSelector;
  m_TextSelector = new TextSelector(mw_one->m_Notes);

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
}

void Notes::openNotes() {
  notes_sync_files.clear();
  mw_one->m_NotesList->needDelWebDAVFiles.clear();
  isPasswordError = false;

  if (mw_one->ui->chkAutoSync->isChecked() &&
      mw_one->ui->chkWebDAV->isChecked()) {
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

    // 连接信号
    QObject::connect(
        helper, &WebDavHelper::listCompleted,
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
                  m_CloudBackup->USERNAME, m_CloudBackup->APP_PASSWORD);

              // 连接信号
              QObject::connect(downloader, &WebDavDownloader::progressChanged,
                               [](int current, int total, QString file) {
                                 qDebug()
                                     << QString("进度: %1/%2  当前文件: %3")
                                            .arg(current)
                                            .arg(total)
                                            .arg(file);
                               });

              QObject::connect(
                  downloader, &WebDavDownloader::downloadFinished,
                  [=](bool success, QString error) {
                    qDebug() << (success ? "下载成功" : "下载失败: " + error);

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

                        // m_Method->decompressWithPassword(
                        //     zFile, pDir, encPassword);

                        if (isPasswordError == false) {
                          if (QFileInfo(pFile).lastModified() >
                              QFileInfo(kFile).lastModified()) {
                            QFile::remove(kFile);
                            if (QFile::copy(pFile, kFile)) {
                              qDebug() << "kFile:" << kFile
                                       << " Update successfully. ";
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
                            QFile::remove(privateDir +
                                          "KnotData/mainnotes.ini.zip");
                            return;
                          }
                        }

                        // m_Method->decompressWithPassword(
                        //     zFile, pDir, encPassword);

                        if (isPasswordError == false) {
                          if (QFileInfo(pFile).lastModified() >
                              QFileInfo(kFile).lastModified()) {
                            QFile::remove(kFile);
                            QFile::copy(pFile, kFile);
                            mw_one->m_NotesList->m_dbManager.updateFileIndex(
                                kFile);
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

                        if (QFileInfo(pFile).lastModified() >
                            QFileInfo(kFile).lastModified()) {
                          QFile::remove(kFile);
                          QFile::copy(pFile, kFile);
                        }
                      }
                    }

                    mw_one->m_Notes->openNotesUI();
                  });

              // 开始下载（1并发,根据文件的下载个数）
              QString lf = privateDir;
              qDebug() << "lf=" << lf;
              downloader->downloadFiles(remoteFiles, lf, remoteFiles.count());
            }

            if (remoteFiles.count() == 0) mw_one->m_Notes->openNotesUI();
            break;
          }
        });

    QObject::connect(helper, &WebDavHelper::errorOccurred,
                     [=](const QString &error) {
                       qDebug() << "操作失败:" << error;
                       mw_one->m_Notes->openNotesUI();
                     });
  } else

    mw_one->m_Notes->openNotesUI();
}

void Notes::updateMainnotesIniToSyncLists() {
  QDateTime cDT = QFileInfo(iniDir + "mainnotes.ini").lastModified();
  qDebug() << "cDT=" << cDT << "mainnotesLastModi=" << mainnotesLastModi;
  if (cDT > mainnotesLastModi) {
    QString zipMainnotes = privateDir + "KnotData/mainnotes.ini.zip";

    // m_Method->compressFile(zipMainnotes, iniDir + "mainnotes.ini",
    // encPassword);

    if (!m_Method->compressFileWithZlib(iniDir + "mainnotes.ini", zipMainnotes,
                                        Z_DEFAULT_COMPRESSION)) {
      errorInfo = tr("An error occurred while compressing the file.");
      ShowMessage *msg = new ShowMessage(this);
      msg->showMsg("Knot", errorInfo, 1);
      return;
    }

    QString enc_file = m_Method->useEnc(zipMainnotes);
    if (enc_file != "") zipMainnotes = enc_file;

    mw_one->m_Notes->notes_sync_files.removeOne(zipMainnotes);
    mw_one->m_Notes->notes_sync_files.append(zipMainnotes);
  }
}

void Notes::initMarkdownLexer() {
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
}

void Notes::initMarkdownLexerDark() {
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
}

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

// 查找关键词
void Notes::searchText(const QString &text, bool forward) {
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
}

void Notes::jumpToPrevMatch() {
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
}

// 获取搜索结果的匹配总数
int Notes::getSearchMatchCount(const QString &text) {
  if (text.isEmpty()) return 0;

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
}

void Notes::openBrowserOnce(const QString &htmlPath) {
  QDesktopServices::openUrl(QUrl::fromLocalFile(htmlPath));

  // QUrl url("http://localhost:8000/memo.html");
  // QDesktopServices::openUrl(url);
}

void Notes::on_btnView_clicked() { mw_one->ui->btnOpenNote->click(); }

void Notes::init_md() {
  if (isDark) {
    initMarkdownLexerDark();
    m_EditSource->verticalScrollBar()->setStyleSheet(
        m_Method->darkScrollbarStyle);
  } else {
    initMarkdownLexer();
    m_EditSource->verticalScrollBar()->setStyleSheet(
        m_Method->lightScrollbarStyle);
  }
  initMarkdownEditor(m_EditSource);
}

#include <QSettings>

void Notes::saveEditorState(const QString &filePath) {
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
}

void Notes::restoreEditorState(const QString &filePath) {
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
}
