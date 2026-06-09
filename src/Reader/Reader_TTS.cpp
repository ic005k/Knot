#include "Reader.h"

void Reader::startSpeak() {
  stopSpeak();

  QString text = getBookSpeakTextFromQML();
  m_Method->playMyText(text);
}

void Reader::stopSpeak() { m_Method->stopPlayMyText(); }

void Reader::setAutoStopPlayTime() {
  m_autoStopDeadline = QDateTime::currentDateTime().addSecs(
      mui->editAutoStopTTS->text().toInt() * 60);
}

QString Reader::getBookSpeakTextFromQML() {
  QVariant item;
  QQuickItem* root = mui->qwReader->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "getBookSpeakText",
                            Q_RETURN_ARG(QVariant, item));
  QString txt = item.toString();

  return txt;
}
