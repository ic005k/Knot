#include "src/Notes/Notes.h"
#ifdef Q_OS_ANDROID
#include <QJniObject>

#endif

void Notes::openAndroidNoteEditor() {
#ifdef Q_OS_ANDROID
  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  activity.callMethod<void>("openNoteEditor", "()V");
#endif
  mw_one->isOpenSearchResult = false;
}

void Notes::openMDWindow() {
#ifdef Q_OS_ANDROID
  QJniObject::callStaticMethod<void>("com.x/MyActivity", "openMDWindow", "()V");
#endif
}

void Notes::appendNote(QString str) {
#ifdef Q_OS_ANDROID
  QJniObject jTitle = QJniObject::fromString(str);
  QJniObject m_activity = QNativeInterface::QAndroidApplication::context();
  m_activity.callStaticMethod<void>("com.x/NoteEditor", "appendNote",
                                    "(Ljava/lang/String;)V",
                                    jTitle.object<jstring>());
#endif
}

void Notes::insertNote(QString str) {
#ifdef Q_OS_ANDROID
  QJniObject jTitle = QJniObject::fromString(str);
  QJniObject m_activity = QNativeInterface::QAndroidApplication::context();
  m_activity.callStaticMethod<void>("com.x/NoteEditor", "insertNote",
                                    "(Ljava/lang/String;)V",
                                    jTitle.object<jstring>());
#endif
}

void Notes::setOpenSearchResultForAndroid(bool isValue, QString strSearchText) {
#ifdef Q_OS_ANDROID
  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  activity.callMethod<void>("setOpenSearchResult", "(Z)V", isValue);
  QJniObject jFile = QJniObject::fromString(strSearchText);
  activity.callMethod<void>("setSearchText", "(Ljava/lang/String;)V",
                            jFile.object<jstring>());
#endif
}

void Notes::openLocalHtmlFileInAndroid() {
#ifdef Q_OS_ANDROID
  QJniObject::callStaticMethod<void>(ANDROID_MAIN_ACTIVITY, "launchWebView",
                                     "()V");
#endif
}

void Notes::refreshLocalHtmlFileInAndroid() {
#ifdef Q_OS_ANDROID
  QJniObject::callStaticMethod<void>("com/x/WebViewActivity",
                                     "refreshWebViewContent", "()V");
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

void Notes::javaNoteToQMLNote() {
  newText = loadText(currentMDFile);
  updateDiff(oldText, newText);

  if (isSetNewNoteTitle()) {
    TitleGenerator generator;
    new_title = generator.genNewTitle(newText);
    renameTitle(true);
  }

  zipNoteToSyncList();

  startBackgroundTaskUpdateNoteIndex(currentMDFile);
}

void Notes::refreshNote() {
  QFuture<void> future = QtConcurrent::run([=]() { MD2Html(currentMDFile); });

  // 使用 QFutureWatcher 监控进度
  QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
  connect(watcher, &QFutureWatcher<void>::finished, this, [=]() {
    refreshLocalHtmlFileInAndroid();

    qDebug() << "Refresh note completed";
    watcher->deleteLater();
  });
  watcher->setFuture(future);
}