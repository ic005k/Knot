#include "Reader.h"
#include "src/MainWindow.h"

void Reader::startSpeak() {
  stopSpeak();

  QString text = getBookSpeakTextFromQML();
  m_Method->playMyText(text);
}

void Reader::stopSpeak() { m_Method->stopPlayMyText(); }

void Reader::setAutoStopPlayTime() {}

QString Reader::getBookSpeakTextFromQML() { return txt; }

void Reader::initTTS() {
#ifdef Q_OS_ANDROID
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
#endif
}

void Reader::setTtsCurrentSentence(const QString& currentSentence) {
  bool isLockScreen = m_Method->getLockScreenStatus();

  if (QDateTime::currentDateTime() > m_autoStopDeadline) {
    m_autoStopDeadline = QDateTime();  // 清空超时时间
    savePageVPos();
    return;
  }

  // 清理空白字符（避免换行/空格导致匹配失败）
  QString sentence = currentSentence.trimmed();

  if (sentence == "__TTS_PLAY_FINISHED__") {
    qDebug() << "🎉 TTS 全部文本播放完成！";

    stopSpeak();

    // 自动播放下一章
    if (isPlayBook) {
      if (cPage < tPage) {
        goNextPage();
        startSpeak();
      }
    }

    return;  // 直接返回，不执行后面的高亮
  }

  // 同时满足：非锁屏 + APP前台活跃 才执行高亮
  if (!isLockScreen && m_isAppForeground) {
    // C++ 直接调用 QML 的高亮方法：highlightCurrentSentence(string)

    qDebug() << "✅ 已通知 QML 高亮句子：" << sentence;
  }

  savePageVPos();
}
