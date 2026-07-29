#include <QDebug>
#include <QFile>
#include <QObject>
#include <QSettings>

#include "MainWindow.h"
#include "src/defines.h"
#include "ui_MainWindow.h"

#ifdef Q_OS_ANDROID
static void JavaNotify_0();
static void JavaNotify_1();
static void JavaNotify_2();
static void JavaNotify_3();
static void JavaNotify_4();
static void JavaNotify_5();
static void JavaNotify_6();
static void JavaNotify_7();
static void JavaNotify_8();
static void JavaNotify_9();
static void JavaNotify_10();
static void JavaNotify_11();
static void JavaNotify_12();
static void JavaNotify_13();
static void JavaNotify_14();
static void JavaNotify_15();
static void JavaNotify_16();
static void JavaNotify_17();
static void JavaNotify_18();
static void JavaNotify_19();
static void JavaNotify_20(JNIEnv* env, jclass clazz, jstring sentence);
// AI Link Native方法声明（标准JNI静态方法参数）
static void generateAiLinkMetadata(JNIEnv* env, jclass clazz,
                                   jstring noteFullText, jint cursorPos);
#endif

#ifdef Q_OS_ANDROID
static void JavaNotify_0() {
  // onResume

  if (mw_one->initMain) return;

  qDebug() << "C++ JavaNotify_0";
}

static void JavaNotify_1() {
  // onPause

  if (mw_one->initMain) return;

  qDebug() << "C++ JavaNotify_1";
}

static void JavaNotify_2() {
  // When the screen lights up

  m_Steps->updateHardSensorSteps();

  qDebug() << "C++ JavaNotify_2";
}

// Todo闹钟时间到，显示弹窗
static void JavaNotify_3() {
#ifdef Q_OS_ANDROID
  // ✅ JNI 入口级防护，防止 Qt 未初始化时崩溃
  if (!QCoreApplication::instance() || !QCoreApplication::eventDispatcher()) {
    qDebug() << "JavaNotify_3: Qt未就绪，丢弃本次闹钟通知";
    return;
  }

  QDateTime now = QDateTime::currentDateTime();
  QString strTodoAlarmActiveTime = now.toString("yyyy-MM-dd  HH:mm:ss");

  // ✅ 先切到主线程，但只做轻量级数据准备和前台唤醒
  QMetaObject::invokeMethod(
      QCoreApplication::instance(),
      [strTodoAlarmActiveTime]() {
        if (!mw_one || !mw_one->m_Todo) {
          qDebug() << "JavaNotify_3: mw_one/m_Todo 为空，跳过";
          return;
        }

        // 1. 先唤醒 App 到前台（这是触发 Surface 恢复的关键）
        if (mw_one->m_ReceiveShare) {
          mw_one->m_ReceiveShare->bringAppToForeground();
        }

        // 2. 在主线程安全读取 UI 数据（轻量操作，不阻塞）
        QString strTime = mw_one->m_Todo->strAlarmTime;
        QString strText = mw_one->m_Todo->strAlarmText;

        // ✅ 将耗时的文件IO和音频播放移到弹窗之后或异步执行
        // 这里只收集配置，不执行播放
        bool shouldPlayAudio = false;
        bool isVoice = false;
        QString voiceFile;

        try {
          mw_one->m_Todo->refreshAlarm();
          isVoice = mw_one->m_Todo->isVoice(strText);

          QString ini_file = privateDir + "msg.ini";
          QSettings Reg(ini_file, QSettings::IniFormat);
          bool isPlayText = Reg.value("voice", 0).toBool();
          shouldPlayAudio = (isVoice || isPlayText);

          if (isVoice) {
            voiceFile = mw_one->m_Todo->getVoiceFile(strText);
          }
        } catch (...) {
          qDebug() << "JavaNotify_3: 读取配置异常";
        }

        // ✅ 延迟显示弹窗，等待 Surface 稳定
        // 不再嵌套 QueuedConnection，改用 QTimer 给 WM 足够时间
        QTimer::singleShot(300, mw_one->m_Todo,
                           [strTime, strText, strTodoAlarmActiveTime,
                            shouldPlayAudio, isVoice, voiceFile]() {
                             if (!mw_one || !mw_one->m_Todo) return;

                             // 同步显示弹窗（此时已在主线程，且 Surface
                             // 已稳定）
                             mw_one->m_Todo->showAlarmWindow(
                                 strTime, strText, strTodoAlarmActiveTime);

                             // ✅ 弹窗显示后再异步播放音频，避免阻塞 UI
                             // 渲染
                             if (shouldPlayAudio && m_Method) {
                               QTimer::singleShot(50, [=]() {
                                 if (!m_Method) return;
                                 if (isVoice) {
                                   m_Method->playRecord(voiceFile);
                                 } else {
                                   isPlayBook = false;
                                   m_Method->stopPlayMyText();
                                   m_Method->playMyText(strText);
                                 }
                               });
                             }

                             qDebug() << "C++ JavaNotify_3 弹窗+音频调度完成";
                           });
      },
      Qt::QueuedConnection);
#endif
}

static void JavaNotify_4() {
  bool isBackMain = false;
  QJniObject activity =
      QJniObject(QNativeInterface::QAndroidApplication::context());
  if (activity.isValid()) {
    jboolean result = activity.callMethod<jboolean>("getIsBackMainUI", "()Z");
    activity.callMethod<void>("setIsBackMainUI", "(Z)V", false);
    isBackMain = result;
  }

  if (!isBackMain) {
    mw_one->setMini();
  }

  qDebug() << "C++ JavaNotify_4";
}

static void JavaNotify_5() {
  mw_one->m_ReceiveShare->goReceiveShare();

  qDebug() << "C++ JavaNotify_5";
}

static void JavaNotify_6() {
  QTimer::singleShot(100, mw_one, [=]() {
    if (m_Notes != nullptr && m_NotesList != nullptr &&
        m_NotesList->getNoteBookCurrentIndex() >= 0) {
      m_Notes->javaNoteToQMLNote();
    }
  });

  qDebug() << "JavaNotify_6 已执行";
}

static void JavaNotify_7() {
  m_Notes->insertImage(privateDir + "receive_share_pic.png", true);

  qDebug() << "C++ JavaNotify_7";
}

static void JavaNotify_8() {
  if (isInitThemeEnd) {
    QTimer::singleShot(100, mw_one, []() { mw_one->execDeskShortcut(); });

  } else {
    isNeedExecDeskShortcut = true;
  }

  qDebug() << "C++ JavaNotify_8";
}

static void JavaNotify_9() {
  mw_one->m_ReceiveShare->callJavaNotify9();

  qDebug() << "C++ JavaNotify_9";
}

static void JavaNotify_10() {
  // Open Book
  QMetaObject::invokeMethod(
      qApp, []() { mw_one->on_btnOpen_clicked(); }, Qt::QueuedConnection);

  qDebug() << "C++ JavaNotify_10";
}

static void JavaNotify_11() {
  // Books List
  QMetaObject::invokeMethod(
      qApp, []() { mui->btnReadList->click(); }, Qt::QueuedConnection);

  qDebug() << "C++ JavaNotify_11";
}

static void JavaNotify_12() {
  if (isPDF && isAndroid) m_Reader->openMyPDF(fileName);

  qDebug() << "C++ JavaNotify_12";
}

static void JavaNotify_13() {
  m_Reader->openMyPDF(fileName);

  qDebug() << "C++ JavaNotify_13";
}

static void JavaNotify_14() {
  QString flag = m_Method->getDateTimeFlag();
  // UI逻辑通过invokeMethod抛到UI主线程执行
  QMetaObject::invokeMethod(
      qApp,
      [=]() {
        if (flag == "todo") {
          mw_one->m_TodoAlarm->setDateTime();
        } else if (flag == "gpslist") {
          mui->btnGetGpsListData->click();
        } else {
          mw_one->m_DateSelector->ui->btnOk->click();
        }
      },
      Qt::QueuedConnection);

  qDebug() << "C++ JavaNotify_14";
}

static void JavaNotify_15() {
  if (mw_one != nullptr) {
    mw_one->emitAndroidBackSignal();
  }

  qDebug() << "C++ JavaNotify_15";
}

static void JavaNotify_16() {
  QTimer::singleShot(100, mw_one, []() { m_Notes->refreshNote(); });

  qDebug() << "C++ JavaNotify_16";
}

static void JavaNotify_17() {
  QTimer::singleShot(100, mw_one, []() { m_NotesList->clickNoteList(); });
  qDebug() << "C++ JavaNotify_17";
}

static void JavaNotify_18() {
  // 屏幕熄了（锁屏）
  QTimer::singleShot(100, mw_one, []() {
    if (mui->frameReader->isVisible()) {
      if (mui->btnAutoStop->isVisible()) {
        mw_one->on_btnAutoStop_clicked();
        m_Reader->saveReader("", false);
        m_Reader->savePageVPos();
      }
    }
  });

  qDebug() << "C++ JavaNotify_18";
}

static void JavaNotify_19() {
  // TTS播放长文本完成
  QMetaObject::invokeMethod(
      qApp, []() { mui->btnStopSpeak->click(); }, Qt::QueuedConnection);
  qDebug() << "C++ JavaNotify_19";
}

static void JavaNotify_20(JNIEnv* env, jclass clazz, jstring sentence) {
  Q_UNUSED(clazz);

  // 👇 【安全第一步】先在当前JNI线程把字符串转好
  if (!sentence) {
    qDebug() << "JavaNotify_20: 空句子";
    return;
  }

  const char* utf8 = env->GetStringUTFChars(sentence, nullptr);
  QString currentSentence = QString::fromUtf8(utf8);
  env->ReleaseStringUTFChars(sentence, utf8);

  // 👇 【安全第二步】只把 QString 抛到主线程
  QTimer::singleShot(100, mw_one, [=]() {
    qDebug() << "TTS 朗读句子：" << currentSentence;

    if (mw_one && m_Reader) {
      m_Reader->setTtsCurrentSentence(currentSentence);
    }
  });
}

// ===================== AI Link 生成入口 =====================
static void generateAiLinkMetadata(JNIEnv* env, jclass clazz,
                                   jstring noteFullText, jint cursorPos) {
  Q_UNUSED(clazz);
  // 1. 安全转换Java字符串为Qt字符串，释放JNI引用
  QString fullText;
  if (noteFullText != nullptr) {
    const char* utf8Text = env->GetStringUTFChars(noteFullText, nullptr);
    fullText = QString::fromUtf8(utf8Text);
    env->ReleaseStringUTFChars(noteFullText, utf8Text);
  }
  int cursorIndex = (int)cursorPos;
  qDebug() << "收到AI链接生成请求，光标位置：" << cursorIndex << "文本长度："
           << fullText.size();

  // 2. 启动独立子线程执行AI耗时推理（阻塞JNI会导致ANR）
  std::thread aiWorkThread([fullText, cursorIndex]() {
    QString result = m_Notes->getCursorPosText(fullText, cursorIndex, 10);
    qDebug() << "Android前后各取10各字词：" << result;

    m_Notes->isAndroidAILinkGen = true;
    m_NotesList->startVectorSerach(result);
  });
  // 分离线程，自动回收资源
  aiWorkThread.detach();
}

static const JNINativeMethod gMethods[] = {
    {"CallJavaNotify_0", "()V", (void*)JavaNotify_0},
    {"CallJavaNotify_1", "()V", (void*)JavaNotify_1},
    {"CallJavaNotify_2", "()V", (void*)JavaNotify_2},
    {"CallJavaNotify_3", "()V", (void*)JavaNotify_3},
    {"CallJavaNotify_4", "()V", (void*)JavaNotify_4},
    {"CallJavaNotify_5", "()V", (void*)JavaNotify_5},
    {"CallJavaNotify_6", "()V", (void*)JavaNotify_6},
    {"CallJavaNotify_7", "()V", (void*)JavaNotify_7},
    {"CallJavaNotify_8", "()V", (void*)JavaNotify_8},
    {"CallJavaNotify_9", "()V", (void*)JavaNotify_9},
    {"CallJavaNotify_10", "()V", (void*)JavaNotify_10},
    {"CallJavaNotify_11", "()V", (void*)JavaNotify_11},
    {"CallJavaNotify_12", "()V", (void*)JavaNotify_12},
    {"CallJavaNotify_13", "()V", (void*)JavaNotify_13},
    {"CallJavaNotify_14", "()V", (void*)JavaNotify_14}

};

static const JNINativeMethod gMethods15[] = {
    {"CallJavaNotify_15", "()V", (void*)JavaNotify_15}};

static const JNINativeMethod gMethods16[] = {
    {"CallJavaNotify_16", "()V", (void*)JavaNotify_16}};

static const JNINativeMethod gMethods17[] = {
    {"CallJavaNotify_17", "()V", (void*)JavaNotify_17}};

static const JNINativeMethod gMethods18[] = {
    {"CallJavaNotify_18", "()V", (void*)JavaNotify_18}};

static const JNINativeMethod gMethods19[] = {
    {"CallJavaNotify_19", "()V", (void*)JavaNotify_19}};

static const JNINativeMethod gMethods20[] = {
    {"CallJavaNotify_20", "(Ljava/lang/String;)V", (void*)JavaNotify_20}};

static const JNINativeMethod gMethodsAiLink[] = {
    {"generateAiLinkMetadata", "(Ljava/lang/String;I)V",
     (void*)generateAiLinkMetadata}};

void RegJni(const char* myClassName) {
  QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
    QJniEnvironment Environment;
    const char* mClassName = myClassName;
    jclass j_class;
    j_class = Environment->FindClass(mClassName);
    if (j_class == nullptr) {
      qDebug() << "erro clazz";
      return;
    }
    jint mj = Environment->RegisterNatives(
        j_class, gMethods, sizeof(gMethods) / sizeof(gMethods[0]));
    if (mj != JNI_OK) {
      qDebug() << "register native method failed!";
      return;
    } else {
      qDebug() << "RegisterNatives success!";
    }
  });
  qDebug() << "++++++++++++++++++++++++";
}

void RegJni15(const char* myClassName) {
  QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
    QJniEnvironment Environment;
    const char* mClassName = myClassName;
    jclass j_class;
    j_class = Environment->FindClass(mClassName);
    if (j_class == nullptr) {
      qDebug() << "erro clazz";
      return;
    }
    jint mj = Environment->RegisterNatives(
        j_class, gMethods15, sizeof(gMethods15) / sizeof(gMethods15[0]));
    if (mj != JNI_OK) {
      qDebug() << "register native method failed!";
      return;
    } else {
      qDebug() << "RegisterNatives15 success!";
    }
  });
  qDebug() << "++++++++++++++++++++++++";
}

void RegJni16(const char* myClassName) {
  QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
    QJniEnvironment Environment;
    const char* mClassName = myClassName;
    jclass j_class;
    j_class = Environment->FindClass(mClassName);
    if (j_class == nullptr) {
      qDebug() << "erro clazz";
      return;
    }
    jint mj = Environment->RegisterNatives(
        j_class, gMethods16, sizeof(gMethods16) / sizeof(gMethods16[0]));
    if (mj != JNI_OK) {
      qDebug() << "register native method failed!";
      return;
    } else {
      qDebug() << "RegisterNatives16 success!";
    }
  });
  qDebug() << "++++++++++++++++++++++++";
}

void RegJni17(const char* myClassName) {
  QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
    QJniEnvironment Environment;
    const char* mClassName = myClassName;
    jclass j_class;
    j_class = Environment->FindClass(mClassName);
    if (j_class == nullptr) {
      qDebug() << "erro clazz";
      return;
    }
    jint mj = Environment->RegisterNatives(
        j_class, gMethods17, sizeof(gMethods17) / sizeof(gMethods17[0]));
    if (mj != JNI_OK) {
      qDebug() << "register native method failed!";
      return;
    } else {
      qDebug() << "RegisterNatives17 success!";
    }
  });
  qDebug() << "++++++++++++++++++++++++";
}

void RegJni18(const char* myClassName) {
  QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
    QJniEnvironment Environment;
    const char* mClassName = myClassName;
    jclass j_class;
    j_class = Environment->FindClass(mClassName);
    if (j_class == nullptr) {
      qDebug() << "erro clazz";
      return;
    }
    jint mj = Environment->RegisterNatives(
        j_class, gMethods18, sizeof(gMethods18) / sizeof(gMethods18[0]));
    if (mj != JNI_OK) {
      qDebug() << "register native method failed!";
      return;
    } else {
      qDebug() << "RegisterNatives18 success!";
    }
  });
  qDebug() << "++++++++++++++++++++++++";
}

void RegJni19(const char* myClassName) {
  QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
    QJniEnvironment Environment;
    const char* mClassName = myClassName;
    jclass j_class;
    j_class = Environment->FindClass(mClassName);
    if (j_class == nullptr) {
      qDebug() << "erro clazz";
      return;
    }
    jint mj = Environment->RegisterNatives(
        j_class, gMethods19, sizeof(gMethods19) / sizeof(gMethods19[0]));
    if (mj != JNI_OK) {
      qDebug() << "register native method failed!";
      return;
    } else {
      qDebug() << "RegisterNatives19 success!";
    }
  });
  qDebug() << "++++++++++++++++++++++++";
}

void RegJni20(const char* myClassName) {
  QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
    QJniEnvironment env;
    jclass cls = env->FindClass(myClassName);
    env->RegisterNatives(cls, gMethods20, 1);
  });
}

void RegJniAiLink(const char* myClassName) {
  QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
    QJniEnvironment Environment;
    const char* mClassName = myClassName;
    jclass j_class;
    j_class = Environment->FindClass(mClassName);
    if (j_class == nullptr) {
      qDebug() << "erro clazz AiLink";
      return;
    }
    jint mj = Environment->RegisterNatives(
        j_class, gMethodsAiLink,
        sizeof(gMethodsAiLink) / sizeof(gMethodsAiLink[0]));
    if (mj != JNI_OK) {
      qDebug() << "register AiLink native method failed!";
      return;
    } else {
      qDebug() << "RegisterNatives AiLink success!";
    }
  });
  qDebug() << "++++++++++++++++++++++++";
}

#endif

//=========================================================================
