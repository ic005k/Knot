#include "Reader.h"

extern EpubReader* reader;

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
