#include "ReceiveShare.h"

#include "src/MainWindow.h"
#include "ui_MainWindow.h"

extern MainWindow* mw_one;
extern Method* m_Method;

extern QString currentMDFile, privateDir;

extern int deleteDirfile(QString dirName);
extern QString loadText(QString textFile);
extern QString getTextEditLineText(QTextEdit* txtEdit, int i);
extern void TextEditToFile(QTextEdit* txtEdit, QString fileName);
extern void StringToFile(QString buffers, QString fileName);

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
  mw_one->ui->btnNotes->click();
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
    while (!mw_one->ui->frameTodo->isVisible())
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    mw_one->ui->btnBackTodo->click();
  }

  if (mw_one->m_AboutThis->isVisible()) {
    mw_one->m_AboutThis->ui->btnBack_About->click();
  }

  if (mw_one->ui->frameSetTab->isVisible()) mw_one->ui->btnBackSetTab->click();

  if (mw_one->ui->frameNoteRecycle->isVisible()) {
    mw_one->ui->btnBackNoteRecycle->click();
    mw_one->ui->btnBackNoteList->click();
  }

  if (mw_one->ui->frameNotesSearchResult->isVisible()) {
    mw_one->ui->btnBack_NotesSearchResult->click();
    mw_one->ui->btnBackNoteList->click();
  }

  if (mw_one->ui->frameNoteList->isVisible()) {
    mw_one->ui->btnBackNoteList->click();
  }

  if (mw_one->ui->frameNotes->isVisible()) mw_one->ui->btnBackNotes->click();

  if (mw_one->ui->frameTodoRecycle->isVisible()) {
    mw_one->ui->btnReturnRecycle->click();
    mw_one->ui->btnBackTodo->click();
  }

  if (mw_one->ui->frameTodo->isVisible()) mw_one->ui->btnBackTodo->click();

  if (mw_one->m_StepsOptions->isVisible()) {
    mw_one->m_StepsOptions->ui->btnBack->click();
    mw_one->ui->btnBackSteps->click();
  }

  if (mw_one->ui->frameSteps->isVisible()) mw_one->ui->btnBackSteps->click();

  if (mw_one->ui->frameBookList->isVisible()) {
    mw_one->ui->btnBackBookList->click();
    mw_one->ui->btnBackReader->click();
  }

  if (mw_one->ui->frameReader->isVisible()) {
    mw_one->ui->btnBackReader->click();
  }

  if (mw_one->m_Preferences->isVisible())
    mw_one->m_Preferences->ui->btnBack->click();

  if (mw_one->ui->frameOne->isVisible()) mw_one->ui->btnBack_One->click();

  if (mw_one->ui->frameBakList->isVisible())
    mw_one->ui->btnBackBakList->click();

  if (mw_one->ui->frameCategory->isVisible()) {
    mw_one->ui->btnCancelType->click();
    mw_one->ui->btnBackEditRecord->click();
  }

  if (mw_one->ui->frameEditRecord->isVisible())
    mw_one->ui->btnBackEditRecord->click();

  if (mw_one->ui->frameTabRecycle->isVisible())
    mw_one->ui->btnBackTabRecycle->click();

  if (mw_one->ui->frameViewCate->isVisible()) {
    mw_one->ui->btnOkViewCate->click();
    mw_one->ui->btnBack_Report->click();
  }

  if (mw_one->ui->frameReport->isVisible()) mw_one->ui->btnBack_Report->click();

  if (mw_one->ui->frameSearch->isVisible()) mw_one->ui->btnBackSearch->click();
}

void ReceiveShare::closeAllActiveWindows() {
  if (mw_one->m_TodoAlarm->isVisible()) {
    mw_one->m_TodoAlarm->ui->btnBack->click();
    mw_one->ui->btnBackTodo->click();
  }

  if (mw_one->m_AboutThis->isVisible()) {
    mw_one->m_AboutThis->on_btnBack_About_clicked();
  }

  if (mw_one->ui->frameMain->isVisible()) return;

  QObjectList frameList;
  frameList = getAllFrame(m_Method->getAllUIControls(mw_one));
  for (int i = 0; i < frameList.count(); i++) {
    QFrame* frame = (QFrame*)frameList.at(i);
    if (frame->parent() == mw_one->ui->centralwidget &&
        frame->objectName() != "frameMain") {
      qDebug() << frame->objectName();
      if (frame->isVisible()) {
        frame->hide();
      }
    }
  }
  mw_one->ui->frameMain->show();
}

void ReceiveShare::closeAllActiveWindowsKeep(QString frameName) {
  if (mw_one->ui->frameMain->isVisible()) return;

  if (mw_one->m_TodoAlarm->isVisible()) {
    mw_one->m_TodoAlarm->ui->btnBack->click();
  }

  if (mw_one->m_AboutThis->isVisible()) {
    mw_one->m_AboutThis->on_btnBack_About_clicked();
  }

  QObjectList frameList;
  frameList = getAllFrame(m_Method->getAllUIControls(mw_one));
  for (int i = 0; i < frameList.count(); i++) {
    QFrame* frame = (QFrame*)frameList.at(i);
    if (frame->parent() == mw_one->ui->centralwidget &&
        frame->objectName() != frameName) {
      qDebug() << frame->objectName();
      if (frame->isVisible()) {
        frame->hide();
      }
    }
  }
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

    if (mw_one->ui->frameTodo->isHidden() && mw_one->ui->frameMain->isHidden())
      closeAllChildWindows();

    if (mw_one->ui->frameTodo->isHidden()) {
      mw_one->ui->btnTodo->click();
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
    mw_one->ui->btnNotes->click();
  }
}
