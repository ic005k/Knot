#include "ReceiveShare.h"

#include "src/MainWindow.h"
#include "ui_MainWindow.h"

extern MainWindow* mw_one;
extern Ui::MainWindow* mui;
extern Method* m_Method;

extern QString currentMDFile, privateDir;

extern int deleteDirfile(QString dirName);
extern QString loadText(QString textFile);
extern QString getTextEditLineText(QTextEdit* txtEdit, int i);
extern void TextEditToFile(QTextEdit* txtEdit, QString fileName);
extern bool StringToFile(QString buffers, QString fileName);

ReceiveShare::ReceiveShare(QWidget* parent) : QDialog(parent) {}

ReceiveShare::~ReceiveShare() {}

void ReceiveShare::closeEvent(QCloseEvent* event) {
  Q_UNUSED(event)
  QClipboard* m_Clip = QApplication::clipboard();
  m_Clip->setText(strReceiveShareData);

  m_Method->closeGrayWindows();
}

bool ReceiveShare::eventFilter(QObject* watch, QEvent* evn) {
  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
      Close();

      return true;
    }
  }

  return QWidget::eventFilter(watch, evn);
}

void ReceiveShare::setShareDone(QString strDone) {
  QSettings Reg("/storage/emulated/0/.Knot/myshare.ini", QSettings::IniFormat);

  Reg.setValue("/share/shareDone", strDone);
}

QString ReceiveShare::getShareDone() {
  QSettings Reg("/storage/emulated/0/.Knot/myshare.ini", QSettings::IniFormat);

  return Reg.value("/share/shareDone", "true").toString();
}

QString ReceiveShare::getShareType() {
  QSettings Reg("/storage/emulated/0/.Knot/myshare.ini", QSettings::IniFormat);

  return Reg.value("/share/shareType", "text/plain").toString();
}

QString ReceiveShare::getShareString() {
  QString file = privateDir + "share_text.txt";
  return loadText(file);
}

QString ReceiveShare::getShareMethod() {
  QSettings Reg("/storage/emulated/0/.Knot/myshare.ini", QSettings::IniFormat);

  QString method = Reg.value("/share/method", "").toString();
  return method;
}

int ReceiveShare::getImgCount() {
  QSettings Reg("/storage/emulated/0/.Knot/myshare.ini", QSettings::IniFormat);

  return Reg.value("/share/imgCount", 0).toInt();
}

int ReceiveShare::getCursorPos() {
  QSettings Reg("/storage/emulated/0/.Knot/note_text.ini",
                QSettings::IniFormat);

  return Reg.value("/cpos/" + QFileInfo(currentMDFile).baseName(), "").toInt();
}

void ReceiveShare::setCursorPos(int pos) {
  QSettings Reg("/storage/emulated/0/.Knot/note_text.ini",
                QSettings::IniFormat);

  Reg.setValue("/cpos/" + QFileInfo(currentMDFile).baseName(), pos);
}

void ReceiveShare::Close() {
  m_Method->closeGrayWindows();
  close();
}

QString ReceiveShare::addToNote_Java() {
  strReceiveShareData = getShareString();
  shareType = getShareType();
  QString strData;

  if (shareType == "text/plain") {
    strData = strReceiveShareData;
  }

  if (shareType == "image/*") {
    int imgCount = getImgCount();
    for (int i = 0; i < imgCount; i++) {
      QString imgFile =
          "/storage/emulated/0/.Knot/img" + QString::number(i) + ".png";
      QString strImg = mw_one->m_Notes->insertImage(imgFile, false);
      strData = strData + strImg;
    }
  }

  qDebug() << "strReceiveShareData=" << strReceiveShareData;
  StringToFile(strData, privateDir + "share_text.txt");

  QSettings Reg("/storage/emulated/0/.Knot/myshare.ini", QSettings::IniFormat);

  if (isInsertToNote)
    Reg.setValue("/share/on_create", "insert");
  else
    Reg.setValue("/share/on_create", "append");

  return strData;
}

void ReceiveShare::addToNote(bool isInsert) {
  QString imgFile = "/storage/emulated/0/.Knot/receive_share_pic.png";
  strReceiveShareData = getShareString();
  shareType = getShareType();

  QTextEdit* edit = new QTextEdit;
  QString strBuffer = loadText(currentMDFile);
  edit->setPlainText(strBuffer);
  int curPos = getCursorPos();
  if (curPos < 0) curPos = 0;
  if (curPos > strBuffer.length()) curPos = strBuffer.length();
  QTextCursor tmpCursor = edit->textCursor();
  tmpCursor.setPosition(curPos);
  edit->setTextCursor(tmpCursor);

  if (isInsert) {
    if (shareType == "text/plain") {
      edit->insertPlainText(strReceiveShareData);
    }
    if (shareType == "image/*") {
      QString strImg = mw_one->m_Notes->insertImage(imgFile, false);
      edit->insertPlainText(strImg);
    }
  } else {
    // append
    if (shareType == "text/plain") {
      edit->append(strReceiveShareData);
      int newPos = strBuffer.length() + strReceiveShareData.length();
      setCursorPos(newPos);
    }
    if (shareType == "image/*") {
      QString strImg = mw_one->m_Notes->insertImage(imgFile, false);
      edit->append(strImg);
      int newPos = strBuffer.length() + strImg.length();
      setCursorPos(newPos);
    }
  }

  TextEditToFile(edit, currentMDFile);

  qDebug() << "strReceiveShareData=" << strReceiveShareData;
}

void ReceiveShare::on_btnAppendToNote_clicked() {
  if (nMethod == 1) {
    isInsertToNote = false;
    addToNote(isInsertToNote);

    openNoteEditor();
  }

  if (nMethod == 2) {
    isInsertToNote = false;
    addToNote_Java();

    openNoteEditor();
  }
}

void ReceiveShare::on_btnInsertToNote_clicked() {
  if (nMethod == 1) {
    isInsertToNote = true;
    addToNote(isInsertToNote);

    openNoteEditor();
  }

  if (nMethod == 2) {
    isInsertToNote = true;
    addToNote_Java();

    openNoteEditor();
  }
}

void ReceiveShare::openNoteEditor() {
  closeAllChildWindows();
  mw_one->m_Notes->isRequestOpenNoteEditor = true;
  mui->btnNotes->click();
}

QObjectList ReceiveShare::getAllFrame(QObjectList lstUIControls) {
  QObjectList lst;
  foreach (QObject* obj, lstUIControls) {
    if (obj->metaObject()->className() == QStringLiteral("QFrame")) {
      lst.append(obj);
    }
  }
  return lst;
}

void ReceiveShare::closeAllChildWindows() {
  mw_one->m_Reader->closeMyPDF();

  if (mw_one->m_TodoAlarm->isVisible()) {
    mw_one->m_TodoAlarm->ui->btnBack->click();
    while (!mui->frameTodo->isVisible())
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    mui->btnBackTodo->click();
  }

  if (mw_one->m_AboutThis->isVisible()) {
    mw_one->m_AboutThis->ui->btnBack_About->click();
  }

  if (mui->frameSetTab->isVisible()) mui->btnBackSetTab->click();

  if (mui->frameNoteRecycle->isVisible()) {
    mui->btnBackNoteRecycle->click();
    mui->btnBackNoteList->click();
  }

  if (mui->frameNotesSearchResult->isVisible()) {
    mui->btnBack_NotesSearchResult->click();
    mui->btnBackNoteList->click();
  }

  if (mui->frameNoteList->isVisible()) {
    mui->btnBackNoteList->click();
  }

  if (mui->frameNotesGraph->isVisible()) {
    mui->btnBackNotesGraph->click();
    mui->btnBackNoteList->click();
  }

  if (mui->frameTodoRecycle->isVisible()) {
    mui->btnReturnRecycle->click();
    mui->btnBackTodo->click();
  }

  if (mui->frameTodo->isVisible()) mui->btnBackTodo->click();

  if (mw_one->m_StepsOptions->isVisible()) {
    mw_one->m_StepsOptions->ui->btnBack->click();
    mui->btnBackSteps->click();
  }

  if (mui->frameSteps->isVisible()) mui->btnBackSteps->click();

  if (mui->frameBookList->isVisible()) {
    mui->btnBackBookList->click();
    mui->btnBackReader->click();
  }

  if (mui->frameReader->isVisible()) {
    mui->btnBackReader->click();
  }

  if (mw_one->m_Preferences->isVisible())
    mw_one->m_Preferences->ui->btnBack->click();

  if (mui->frameOne->isVisible()) mui->btnBack_One->click();

  if (mui->frameBakList->isVisible()) mui->btnBackBakList->click();

  if (mui->frameCategory->isVisible()) {
    mui->btnCancelType->click();
    mui->btnBackEditRecord->click();
  }

  if (mui->frameEditRecord->isVisible()) mui->btnBackEditRecord->click();

  if (mui->frameTabRecycle->isVisible()) mui->btnBackTabRecycle->click();

  if (mui->frameViewCate->isVisible()) {
    mui->btnOkViewCate->click();
    mui->btnBack_Report->click();
  }

  if (mui->frameReport->isVisible()) mui->btnBack_Report->click();

  if (mui->frameSearch->isVisible()) mui->btnBackSearch->click();
}

void ReceiveShare::shareString(const QString& title, const QString& content) {
  Q_UNUSED(title);
  Q_UNUSED(content);
#ifdef Q_OS_ANDROID

  QJniObject jTitle = QJniObject::fromString(title);
  QJniObject jPath = QJniObject::fromString(content);
  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  activity.callMethod<void>(
      "shareString",
      "(Ljava/lang/String;Ljava/lang/String;Lorg/qtproject/qt/android/"
      "bindings/QtActivity;)V",
      jTitle.object<jstring>(), jPath.object<jstring>(),
      activity.object<jobject>());

#endif
}

void ReceiveShare::shareImage(const QString& title, const QString& path,
                              const QString& fileType) {
  Q_UNUSED(title);
  Q_UNUSED(path);
  Q_UNUSED(fileType);
#ifdef Q_OS_ANDROID

  QJniObject jTitle = QJniObject::fromString(title);
  QJniObject jPath = QJniObject::fromString(path);
  QJniObject jType = QJniObject::fromString(fileType);
  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  activity.callMethod<void>("shareImage",
                            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/"
                            "String;Lorg/qtproject/qt/android/"
                            "bindings/QtActivity;)V",
                            jTitle.object<jstring>(), jPath.object<jstring>(),
                            jType.object<jstring>(),
                            activity.object<jobject>());

#endif
}

void ReceiveShare::shareImages(const QString& title,
                               const QStringList& imagesPathList) {
  Q_UNUSED(title);
  Q_UNUSED(imagesPathList);
#ifdef Q_OS_ANDROID

  QString imagesPath;
  foreach (QString str, imagesPathList) {
    imagesPath += str + "|";
  }
  imagesPath = imagesPath.remove(imagesPath.size() - 1, 1).trimmed();

  QJniObject jTitle = QJniObject::fromString(title);
  QJniObject jPathList = QJniObject::fromString(imagesPath);
  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  QJniObject::callStaticMethod<void>(
      "com.x/MyActivity", "shareImages",
      "(Ljava/lang/String;Ljava/lang/String;Lorg/qtproject/qt/android/"
      "bindings/QtActivity;)V",
      jTitle.object<jstring>(), jPathList.object<jstring>(),
      activity.object<jobject>());

#endif
}

void ReceiveShare::moveTaskToFront() {
#ifdef Q_OS_ANDROID

  QJniObject m_activity = QNativeInterface::QAndroidApplication::context();
  m_activity.callStaticMethod<void>("com.x/MyActivity", "bringToFront", "()V");

#endif
}

void ReceiveShare::goReceiveShare() {
  m_Method->showTempActivity();
  QString method = mw_one->m_ReceiveShare->getShareMethod();
  if (method == "todo") {
    strReceiveShareData = getShareString();

    mw_one->m_Todo->isNeedAddToTodoList = true;
    mw_one->m_Todo->strNeedAddToTodoText = strReceiveShareData;

    if (mui->frameTodo->isHidden() && mui->frameMain->isHidden())
      closeAllChildWindows();

    if (mui->frameTodo->isHidden()) {
      mui->btnTodo->click();
    }
  }

  if (method == "appendNote") {
    on_btnAppendToNote_clicked();
  }

  if (method == "insertNote") {
    on_btnInsertToNote_clicked();
  }

  if (method == "freePaste") {
    shareType = getShareType();
    if (shareType == "text/plain") {
      strReceiveShareData = getShareString();
      QClipboard* pClip = QApplication::clipboard();
      pClip->setText(strReceiveShareData);
    }

    if (shareType == "image/*") {
      QClipboard* clip = QApplication::clipboard();
      int imgCount = getImgCount();
      for (int i = 0; i < imgCount; i++) {
        QString imgFile =
            "/storage/emulated/0/.Knot/img" + QString::number(i) + ".png";
        QImage* image = new QImage();
        image->load(imgFile);
        clip->setPixmap(QPixmap::fromImage(*image));
        qDebug() << "imgFile=" << imgFile;
      }
    }

    closeAllChildWindows();
    mui->btnNotes->click();
  }
}

void ReceiveShare::callJavaNotify9() {
  QSettings Reg(privateDir + "choice_book.ini", QSettings::IniFormat);

  QString file = Reg.value("book/file", "").toString();
  QString type = Reg.value("book/type", "filepicker").toString();
  if (QFile::exists(file)) {
    if (type == "defaultopen") {
      closeAllChildWindows();
      moveTaskToFront();
    }
    mw_one->m_Reader->startOpenFile(file);
  }
}
