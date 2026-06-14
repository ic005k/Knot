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

void Reader::initTTS() {
  // 调用静态方法 com.x.MyService.checkAndInitTts()，返回 int
  int state = QJniObject::callStaticMethod<int>("com/x/MyService",
                                                "checkAndInitTts", "()I");

  // 按状态码分支处理
  switch (state) {
    case 0:
      // TTS 空闲就绪
      break;
    case 1:
      // TTS 正在初始化
      break;
    case 2:
      // 当前正在播放语音
      break;
    case -1:
    default:
      // 实例无效/异常
      break;
  }
}
