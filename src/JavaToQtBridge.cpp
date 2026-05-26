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

static void JavaNotify_3() {
#ifdef Q_OS_ANDROID
  // 时间处理（无风险）
  QDateTime now = QDateTime::currentDateTime();
  QString datePart = now.toString("yyyy-MM-dd");
  QString timePart = now.toString("HH:mm:ss");
  QString strTodoAlarmActiveTime = datePart + "  " + timePart;

  // 1. 先切到Qt主线程执行所有逻辑
  QMetaObject::invokeMethod(
      QCoreApplication::instance(),
      [strTodoAlarmActiveTime]() {
        // ========== 所有逻辑移到Qt主线程内执行 ==========
        // 2. 空指针校验（避免野指针访问）
        if (mw_one == nullptr || mw_one->m_Todo == nullptr) {
          qDebug() << "JavaNotify_3: mw_one/m_Todo 为空，跳过执行";
          return;
        }

        mw_one->m_ReceiveShare->bringAppToForeground();

        // 3. 读取UI属性（主线程安全）
        QString strTime = mw_one->m_Todo->strAlarmTime;
        QString strText = mw_one->m_Todo->strAlarmText;

        try {
          // 5. UI方法调用（主线程安全）
          mw_one->m_Todo->refreshAlarm();

          // 6. 音频/文件操作（加异常捕获）
          bool isVoice = mw_one->m_Todo->isVoice(strText);
          QString ini_file = privateDir + "msg.ini";
          QSettings Reg(ini_file, QSettings::IniFormat);
          bool isPlayText = Reg.value("voice", 0).toBool();

          if (isVoice || isPlayText) {
            if (isVoice) {
              QString voiceFile = mw_one->m_Todo->getVoiceFile(strText);
              // 音频播放建议异步（如果playRecord是同步的，改用线程/信号槽）
              if (m_Method != nullptr) {
                m_Method->playRecord(voiceFile);
              }
            } else {
              QString txt = strText;
              if (m_Method != nullptr) {
                isPlayBook = false;
                m_Method->stopPlayMyText();
                m_Method->playMyText(txt);
              }
            }
          }

          // 7. 弹窗显示（✅ 安卓Qt终极安全方案，不崩溃+不丢点击）
          QMetaObject::invokeMethod(
              mw_one->m_Todo,
              [=]() {
                if (mw_one && mw_one->m_Todo) {
                  mw_one->m_Todo->showAlarmWindow(strTime, strText,
                                                  strTodoAlarmActiveTime);
                }
              },
              Qt::QueuedConnection);

          qDebug() << "C++ JavaNotify_3 执行完成";
        } catch (const std::exception& e) {
          // 捕获所有异常，避免卡死
          qDebug() << "JavaNotify_3 执行异常：" << e.what();
        } catch (...) {
          qDebug() << "JavaNotify_3 执行未知异常";
        }
      },
      Qt::QueuedConnection);  // 关键：QueuedConnection 确保在Qt主线程执行
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
  // 延迟到Qt主线程空闲时执行，解决QML不可视崩溃问题
  QTimer::singleShot(100, mw_one, [=]() {
    // 双重安全校验，杜绝空指针
    if (m_Notes != nullptr && m_NotesList != nullptr &&
        m_NotesList->getNoteBookCurrentIndex() >= 0) {
      m_Notes->javaNoteToQMLNote();
    }
  });

  qDebug() << "C++ JavaNotify_6 executed safely";
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
  mw_one->on_btnOpen_clicked();

  qDebug() << "C++ JavaNotify_10";
}

static void JavaNotify_11() {
  // Books List
  mui->btnReadList->click();

  qDebug() << "C++ JavaNotify_11";
}

static void JavaNotify_12() {
  if (isPDF && isAndroid) mw_one->m_Reader->openMyPDF(fileName);

  qDebug() << "C++ JavaNotify_12";
}

static void JavaNotify_13() {
  mw_one->m_Reader->openMyPDF(fileName);

  qDebug() << "C++ JavaNotify_13";
}

static void JavaNotify_14() {
  if (m_Method->getDateTimeFlag() == "todo") {
    mw_one->m_TodoAlarm->setDateTime();
  } else if (m_Method->getDateTimeFlag() == "gpslist") {
    mui->btnGetGpsListData->click();
  } else {
    mw_one->m_DateSelector->ui->btnOk->click();
  }
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
  // 屏幕熄了
  QTimer::singleShot(100, mw_one, []() {
    if (mui->frameReader->isVisible()) {
      mw_one->on_btnAutoStop_clicked();
      mw_one->m_Reader->saveReader("", false);
      mw_one->m_Reader->savePageVPos();
    }
  });

  qDebug() << "C++ JavaNotify_18";
}

static void JavaNotify_19() {
  // TTS播放长文本完成
  mui->btnStopSpeak->click();
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

    if (mw_one && mw_one->m_Reader) {
      mw_one->m_Reader->setTtsCurrentSentence(currentSentence);
    }
  });
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

#endif

//=========================================================================
