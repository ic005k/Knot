#include <QDebug>
#include <QFile>
#include <QObject>
#include <QSettings>

#include "MainWindow.h"
#include "defines.h"

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

// 安卓笔记编辑调用C++获取链接内容进行预览
static jstring nativeParsePreview(JNIEnv* env, jclass clazz, jstring lineText,
                                  jint cursorPos);

static void sendQuestionToCpp(JNIEnv* env, jclass clazz, jstring questionText);

static void PublicJavaCallCpp(JNIEnv* env, jclass clazz, jstring type);

#endif

#ifdef Q_OS_ANDROID
static void JavaNotify_0() {
  // pdf关闭后退出全屏

  QTimer::singleShot(100, mw_one, []() {
    m_Method->exitSystemFullscreen();

    if (isAndroid) {
      m_Reader->openReadListWindow(m_Reader->bookList);
    } else
      m_Reader->getReadList();
  });

  qDebug() << "C++ JavaNotify_0：退出全屏模式";
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
    m_Method->closeMainEntranceWindow();

    QTimer::singleShot(100, mw_one, []() { mw_one->execDeskShortcut(); });

  } else {
    isNeedExecDeskShortcut = true;
  }

  qDebug() << "C++ JavaNotify_8";
}

static void JavaNotify_9() {
  // 安卓打开书籍和缺省打开书籍调用
  QTimer::singleShot(100, mw_one,
                     []() { mw_one->m_ReceiveShare->callJavaNotify9(); });

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
  // QMetaObject::invokeMethod(
  //    qApp, []() { mw_one->ui->btnReadList->click(); }, Qt::QueuedConnection);

  qDebug() << "C++ JavaNotify_11";
}

static void JavaNotify_12() { qDebug() << "C++ JavaNotify_12"; }

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
          mw_one->ui->btnGetGpsListData->click();
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

  });

  qDebug() << "C++ JavaNotify_18";
}

static void JavaNotify_19() {
  // TTS播放长文本完成
  QMetaObject::invokeMethod(
      qApp,
      []() {  // mw_one->ui->btnStopSpeak->click();
      },
      Qt::QueuedConnection);
  qDebug() << "C++ JavaNotify_19";
}

static void JavaNotify_20(JNIEnv* env, jclass clazz, jstring sentence) {
  Q_UNUSED(clazz);

  if (isPDF) return;

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

// Java: public static native String nativeParsePreview(String lineText);
// JNI签名：(Ljava/lang/String;)Ljava/lang/String;
static jstring nativeParsePreview(JNIEnv* env, jclass clazz, jstring lineText,
                                  jint cursorPos) {
  Q_UNUSED(clazz);

  // JNI 字符串转换必须在当前 JNI 线程完成
  QString inputText;
  if (lineText != nullptr) {
    const char* utf8Raw = env->GetStringUTFChars(lineText, nullptr);
    inputText = QString::fromUtf8(utf8Raw);
    env->ReleaseStringUTFChars(lineText, utf8Raw);
  }

  int pos = static_cast<int>(cursorPos);

  qDebug() << "[Preview-JNI] IN:" << inputText << "cursorPos:" << pos;

  // 强制在主线程执行，使用值捕获避免悬垂引用
  QString parseResult;
  bool invoked = QMetaObject::invokeMethod(
      m_Notes,  // ✅ 直接使用全局 m_Notes 作为目标对象
      [inputText, pos]() -> QString {
        try {
          return m_Notes->parsePreviewData(inputText, pos);
        } catch (const std::exception& ex) {  // ✅ 改名避免混淆，且 ex 是 catch
                                              // 块局部变量，无需捕获
          qDebug() << "[Preview-JNI] EXCEPTION:" << ex.what();
        } catch (...) {
          qDebug() << "[Preview-JNI] UNKNOWN EXCEPTION";
        }
        return QString();
      },
      Qt::BlockingQueuedConnection,
      &parseResult  // ✅ 通过返回值参数接收结果
  );

  if (!invoked) {
    qWarning() << "[Preview-JNI] invokeMethod FAILED!";
    parseResult = QString();
  }

  qDebug() << "[Preview-JNI] OUT:" << parseResult;

  if (parseResult.isEmpty()) {
    return nullptr;
  }
  return env->NewStringUTF(parseResult.toUtf8().constData());
}

static void sendQuestionToCpp(JNIEnv* env, jclass clazz, jstring questionText) {
  Q_UNUSED(env);
  Q_UNUSED(clazz);
  QString question;

  if (questionText != nullptr) {
    const char* str = env->GetStringUTFChars(questionText, nullptr);
    question = QString::fromUtf8(str);
    env->ReleaseStringUTFChars(questionText, str);
  }

  qDebug() << "[JNI] sendQuestionToCpp:" << question;

  isAndroidAIQA = true;

  // ========== 业务入口 =========================
  // 异步投递到主线程，JNI 线程立即返回，不阻塞
  bool invoked = QMetaObject::invokeMethod(
      mw_one,
      [question]() {
        try {
          mw_one->aiChatQuery(question);
        } catch (const std::exception& ex) {
          qDebug() << "[AI-JNI] EXCEPTION:" << ex.what();
        } catch (...) {
          qDebug() << "[AI-JNI] UNKNOWN EXCEPTION";
        }
      },
      Qt::QueuedConnection  // ← 关键：异步，不阻塞 JNI 线程
  );

  if (!invoked) {
    qWarning() << "[AI-JNI] invokeMethod FAILED!";
  }
  // =======================================================
}

static void PublicJavaCallCpp(JNIEnv* env, jclass clazz, jstring type) {
  Q_UNUSED(clazz);
  QString strType;
  if (type != nullptr) {
    const char* raw = env->GetStringUTFChars(type, nullptr);
    strType = QString::fromUtf8(raw);
    env->ReleaseStringUTFChars(type, raw);
  }
  qDebug() << "[JNI] PublicJavaCallCpp:" << strType;

  // JNI线程立即返回，业务切Qt主线程执行
  bool invoked = QMetaObject::invokeMethod(
      mw_one,
      [strType]() {
        try {
          // 打开书籍文件 ===========================================
          if (strType == "open_book_file") {
            QTimer::singleShot(100, mw_one, [=]() {
              QString bookfile = m_Method->getTempSwapStr();
              QFileInfo fi(bookfile);
              QString suffix = fi.suffix().toLower();
              if (suffix != "pdf" && suffix != "mobi") {
                mw_one->ui->frameMain->hide();
              }
              m_Reader->startOpenFile(bookfile);
            });
          }

          if (strType == "clear_reader_records") {
            QString c_name = m_Method->getTempSwapStr();
            m_Reader->clearReaderRecords(c_name);
          }

          // 增加事件记录 ==============================================
          if (strType == "add_event_record") {
            QTimer::singleShot(100, mw_one, [=]() {
              if (mw_one && mw_one->m_EditRecord) {
                mw_one->ui->frameMain->show();
                mw_one->m_EditRecord->on_btnOk_clicked();
              }
            });
          }

          if (strType == "cancel_add_event_record") {
            QTimer::singleShot(100, mw_one, [=]() {
              if (mw_one->ui->frameMain->isHidden()) {
                m_Method->openMainEntranceWindow();
              }
            });
          }

          if (strType == "open_category_dialog") {
            QTimer::singleShot(100, mw_one, [=]() {
              mw_one->m_EditRecord->on_btnType_clicked();
            });
          }

          if (strType == "select_tab") {
            QTimer::singleShot(100, mw_one,
                               [=]() { mw_one->m_MainHelper->selectTab(); });
          }

          // 主入口 ==============================================
          if (strType == "mainentrance_destroy") {
            QTimer::singleShot(100, mw_one, [=]() {

            });
          }

          if (strType.contains("maintab_selected|==|")) {
            QTimer::singleShot(100, mw_one, [=]() {
              QStringList list = strType.split("|==|");

              if (list.count() == 2) {
                int index = 0;
                index = list.at(1).toInt();
                mw_one->clickMainTab(index);

                qInfo() << "当前被点击的卡片：" << index;
              }
            });
          }

          if (strType.contains("get_maindatedetail|==|")) {
            QTimer::singleShot(100, mw_one, [=]() {
              QStringList list = strType.split("|==|");

              if (list.count() == 2) {
                int index = 0;
                index = list.at(1).toInt();
                m_Method->clickMainDate(index);
                m_Method->clickMainDateData(index);
              }
            });
          }

          if (strType.contains("refresh_alldata")) {
            QTimer::singleShot(100, mw_one, [=]() {
              m_Method->refreshMainDate();
              m_Method->refreshMainDateDetail();
            });
          }

          if (strType.contains("edit_datadetail|==|")) {
            QTimer::singleShot(100, mw_one, [=]() {
              QStringList list = strType.split("|==|");

              if (list.count() == 3) {
                int index0 = 0;
                int index1 = 0;
                index0 = list.at(1).toInt();
                index1 = list.at(2).toInt();
                m_Method->reeditMainEventData(index0, index1);
              }
            });
          }

          if (strType.contains("add_datadetail")) {
            QTimer::singleShot(100, mw_one,
                               [=]() { mw_one->on_btnAdd_clicked(); });
          }

          if (strType.contains("del_datadetail")) {
            QTimer::singleShot(100, mw_one,
                               [=]() { mw_one->on_btnAdd_clicked(); });
          }

          // 主工具栏按钮=============================================
          if (strType == "topbtn_add") {
            QTimer::singleShot(100, mw_one,
                               [=]() { mw_one->ui->btnAdd->click(); });
          }

          if (strType == "topbtn_upload") {
            QTimer::singleShot(100, mw_one,
                               [=]() { mw_one->ui->btnSync->click(); });
          }

          if (strType == "topbtn_search") {
            QTimer::singleShot(100, mw_one,
                               [=]() { mw_one->ui->btnFind->click(); });
          }

          if (strType == "tab_reader") {
            QTimer::singleShot(100, mw_one,
                               [=]() { mw_one->ui->btnReader->click(); });
          }

          if (strType == "tab_todo") {
            QTimer::singleShot(100, mw_one,
                               [=]() { mw_one->ui->btnTodo->click(); });
          }

          if (strType == "tab_notes") {
            QTimer::singleShot(100, mw_one,
                               [=]() { mw_one->ui->btnNotes->click(); });
          }

          if (strType == "tab_steps") {
            QTimer::singleShot(100, mw_one,
                               [=]() { mw_one->ui->btnSteps->click(); });
          }

          // 菜单==========================================================
          if (strType == "menu_id_add_tab") {
            QTimer::singleShot(100, mw_one,
                               [=]() { mw_one->on_actionAdd_Tab_triggered(); });
          }
          if (strType == "menu_id_delete_tab") {
            QTimer::singleShot(100, mw_one,
                               [=]() { mw_one->on_actionDel_Tab_triggered(); });
          }
          if (strType == "menu_id_rename_tab") {
            QTimer::singleShot(100, mw_one,
                               [=]() { mw_one->on_actionRename_triggered(); });
          }
          if (strType == "menu_id_export_data") {
            QTimer::singleShot(100, mw_one, [=]() {
              mw_one->on_actionExport_Data_triggered();
            });
          }
          if (strType == "menu_id_import_data") {
            QTimer::singleShot(100, mw_one, [=]() {
              mw_one->on_actionImport_Data_triggered();
            });
          }
          if (strType == "menu_id_preference") {
            QTimer::singleShot(100, mw_one, [=]() {
              mw_one->on_actionPreferences_triggered();
            });
          }
          if (strType == "menu_id_cloud_backup_restore") {
            QTimer::singleShot(
                100, mw_one, [=]() { mw_one->on_actionOneDriveBackupData(); });
          }
          if (strType == "menu_id_backup_file_list") {
            QTimer::singleShot(100, mw_one,
                               [=]() { mw_one->on_actionBakFileList(); });
          }
          if (strType == "menu_id_tab_recycle_bin") {
            QTimer::singleShot(100, mw_one,
                               [=]() { mw_one->on_actionTabRecycle(); });
          }
          if (strType == "menu_id_about") {
            QTimer::singleShot(100, mw_one,
                               [=]() { mw_one->on_actionAbout(); });
          }

          // Todo ===================================================
          if (strType == "back_todo") {
            QTimer::singleShot(100, mw_one, [=]() {
              if (mw_one->ui->frameMain->isHidden()) {
                mw_one->m_Todo->closeTodo();
              }
            });
          }

          if (strType == "todo_recycle") {
            QTimer::singleShot(100, mw_one,
                               [=]() { mw_one->m_Todo->on_btnRecycle(); });
          }

          if (strType == "todo_recycle_clearall") {
            QTimer::singleShot(100, mw_one,
                               [=]() { mw_one->m_Todo->clearAllRecycle(); });
          }

          if (strType.startsWith("todo_recycle_del|==|")) {
            QStringList list = strType.split("|==|");
            if (list.size() == 2) {
              int index = list.at(1).toInt();
              QTimer::singleShot(0, mw_one, [index]() {
                mw_one->m_Todo->delItemRecycle(index);
              });
            }
          }

          if (strType.startsWith("todo_recycle_restore|==|")) {
            QStringList list = strType.split("|==|");
            if (list.size() == 2) {
              QString todoContent = list.at(1);
              QTimer::singleShot(0, mw_one, [todoContent]() {
                mw_one->m_Todo->addToList(todoContent, false);
              });
            }
          }

          if (strType.startsWith("todo_add|==|")) {
            QStringList list = strType.split("|==|");
            if (list.size() == 2) {
              QString todoContent = list.at(1);
              QTimer::singleShot(0, mw_one, [todoContent]() {
                mw_one->m_Todo->addToList(todoContent, true);
              });
            }
          }

          if (strType.startsWith("todo_high|==|")) {
            QStringList list = strType.split("|==|");
            if (list.size() == 2) {
              int index = list.at(1).toInt();
              QTimer::singleShot(
                  0, mw_one, [index]() { mw_one->m_Todo->on_btnHigh(index); });
            }
          }

          if (strType.startsWith("todo_low|==|")) {
            QStringList list = strType.split("|==|");
            if (list.size() == 2) {
              int index = list.at(1).toInt();
              QTimer::singleShot(
                  0, mw_one, [index]() { mw_one->m_Todo->on_btnLow(index); });
            }
          }

          if (strType.startsWith("todo_confirm_edit|==|")) {
            QStringList list = strType.split("|==|");
            if (list.size() == 3) {
              int index = list.at(1).toInt();
              QString strText = list.at(2);
              QTimer::singleShot(0, mw_one, [index, strText]() {
                mw_one->m_Todo->modifyTodoText(index, strText);
              });
            }
          }

          if (strType.startsWith("todo_done|==|")) {
            QStringList list = strType.split("|==|");
            if (list.size() == 2) {
              int index = list.at(1).toInt();
              QTimer::singleShot(0, mw_one, [index]() {
                mw_one->m_Todo->addToRecycle(index);
              });
            }
          }

          if (strType.startsWith("todo_alarm|==|")) {
            QStringList list = strType.split("|==|");
            if (list.size() == 2) {
              int index = list.at(1).toInt();
              QTimer::singleShot(0, mw_one, [index]() {
                mw_one->m_Todo->on_btnSetTime(index);
              });
            }
          }

          if (strType.startsWith("todo_alarm_set|==|")) {
            QStringList parts = strType.split("|==|");
            // 校验参数数量：todo_alarm_set + w1~w7(7个) + y mon d h m(5个)
            // 一共13项
            if (parts.size() == 13) {
              bool w1 = parts[1].toInt() != 0;
              bool w2 = parts[2].toInt() != 0;
              bool w3 = parts[3].toInt() != 0;
              bool w4 = parts[4].toInt() != 0;
              bool w5 = parts[5].toInt() != 0;
              bool w6 = parts[6].toInt() != 0;
              bool w7 = parts[7].toInt() != 0;

              int y = parts[8].toInt();
              int mon = parts[9].toInt();
              int d = parts[10].toInt();
              int h = parts[11].toInt();
              int m = parts[12].toInt();

              // 切到UI线程执行业务接口
              QTimer::singleShot(0, mw_one, [=]() {
                mw_one->m_Todo->on_SetAlarm(w1, w2, w3, w4, w5, w6, w7, y, mon,
                                            d, h, m);
              });
            }
          }

          if (strType == "todo_alarm_delete") {
            QTimer::singleShot(0, mw_one,
                               []() { mw_one->m_Todo->on_DelAlarm(); });
          }

          // Notes=====================================================

          if (strType.startsWith("note_book_click|==|")) {
            QTimer::singleShot(100, mw_one, [=]() {
              QStringList list = strType.split("|==|");

              if (list.count() == 2) {
                int index = 0;
                index = list.at(1).toInt();
                m_NotesList->clickNoteBook(index);
              }
            });
          }

          //===========================================================

          qDebug() << "[PublicJavaCallCpp main thread] type:" << strType;
        } catch (const std::exception& ex) {
          qDebug() << "[PublicJavaCallCpp] EXCEPTION:" << ex.what();
        } catch (...) {
          qDebug() << "[PublicJavaCallCpp] UNKNOWN EXCEPTION";
        }
      },
      Qt::QueuedConnection);
  if (!invoked) {
    qWarning() << "[PublicJavaCallCpp] invokeMethod FAILED!";
  }
}

//============== JNI 方法注册数组  ===========================

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

static const JNINativeMethod gMethodsParsePreview[] = {
    {"nativeParsePreview", "(Ljava/lang/String;I)Ljava/lang/String;",
     (void*)nativeParsePreview}};

static const JNINativeMethod gMethodsSendQuestion[] = {
    {"sendQuestionToCpp", "(Ljava/lang/String;)V", (void*)sendQuestionToCpp}};

static const JNINativeMethod gMethodsPublicJavaCallCpp[] = {
    {"PublicJavaCallCpp", "(Ljava/lang/String;)V", (void*)PublicJavaCallCpp}};

///// 注册函数 //////////////////////////////////////////////////////////////

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

void RegJniParsePreview(const char* myClassName) {
  QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
    QJniEnvironment Environment;
    const char* mClassName = myClassName;
    jclass j_class;
    j_class = Environment->FindClass(mClassName);
    if (j_class == nullptr) {
      qDebug() << "erro clazz ParsePreview";
      return;
    }
    jint mj = Environment->RegisterNatives(
        j_class, gMethodsParsePreview,
        sizeof(gMethodsParsePreview) / sizeof(gMethodsParsePreview[0]));
    if (mj != JNI_OK) {
      qDebug() << "register nativeParsePreview failed!";
      return;
    } else {
      qDebug() << "RegisterNatives ParsePreview success!";
    }
  });
  qDebug() << "++++++++++++++++++++++++";
}

void RegJniSendQuestion(const char* myClassName) {
  QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
    QJniEnvironment Environment;
    jclass j_class = Environment->FindClass(myClassName);
    if (j_class == nullptr) {
      qDebug() << "error: find class failed sendQuestionToCpp";
      return;
    }

    jint ret = Environment->RegisterNatives(
        j_class, gMethodsSendQuestion,
        sizeof(gMethodsSendQuestion) / sizeof(gMethodsSendQuestion[0]));

    if (ret != JNI_OK) {
      qDebug() << "register sendQuestionToCpp failed";
    } else {
      qDebug() << "RegisterNatives sendQuestionToCpp success!";
    }
  });
}

void RegJniPublicJavaCallCpp(const char* myClassName) {
  QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
    QJniEnvironment Environment;
    jclass j_class = Environment->FindClass(myClassName);
    if (j_class == nullptr) {
      qDebug() << "error: find class failed PublicJavaCallCpp";
      return;
    }
    jint ret =
        Environment->RegisterNatives(j_class, gMethodsPublicJavaCallCpp,
                                     sizeof(gMethodsPublicJavaCallCpp) /
                                         sizeof(gMethodsPublicJavaCallCpp[0]));
    if (ret != JNI_OK) {
      qDebug() << "register PublicJavaCallCpp failed";
    } else {
      qDebug() << "RegisterNatives PublicJavaCallCpp success!";
    }
  });
}

#endif

//=========================================================================
