#include "Todo.h"
#include "src/MainWindow.h"
#include "ui_Todo.h"

void Todo::showAlarmWindow(const QString& strTime, const QString& strText,
                           const QString& strTodoAlarmActiveTime) {
  if (!mw_one) {
    qWarning() << "主窗口指针mw_one为空，无法创建提醒窗口！";
    return;
  }

  // ========== 核心：自定义QDialog ==========
  class AlarmDialog : public QDialog {
   public:
    explicit AlarmDialog(QWidget* parent = nullptr) : QDialog(parent) {
      setModal(false);
      setWindowTitle("");
      setAttribute(Qt::WA_DeleteOnClose);
      m_titleFontSize = 26;
      m_contentFontSize = 25;
    }

    int m_titleFontSize;
    int m_contentFontSize;
    QLabel* titleLabel = nullptr;
    QLabel* contentLabel = nullptr;

   protected:
    bool event(QEvent* e) override {
      if (e->type() == QEvent::PaletteChange) {
        update();
      }
      return QDialog::event(e);
    }
  };

  AlarmDialog* alarmDialog = new AlarmDialog(mw_one);
  alarmDialog->resize(mw_one->size());
  alarmDialog->move(mw_one->pos());

  // ✅ 合并所有样式表，只设置一次（避免字体规则被覆盖）
  QString styleSheet = QString(R"(
    QDialog {
        color: palette(window-text);
    }
    QLabel#titleLabel {
        font-size: %1pt;
        font-weight: bold;
        color: palette(window-text);
    }
    QLabel#contentLabel {
        font-size: %2pt;
        color: palette(window-text);
    }
    QLabel {
        color: palette(window-text);
    }
    QPushButton {
        color: white;
        border: none;
        border-radius: 8px;
        font-size: 18px;
        padding: 8px 16px;
        outline: none;
    }
    QPushButton#playBtn {
        background-color: #3498db;
    }
    QPushButton#playBtn:pressed {
        background-color: #1f618d;
    }
    QPushButton#closeBtn {
        background-color: #e74c3c;
    }
    QPushButton#closeBtn:pressed {
        background-color: #a93226;
    }
  )")
                           .arg(alarmDialog->m_titleFontSize)
                           .arg(alarmDialog->m_contentFontSize);

  alarmDialog->setStyleSheet(styleSheet);

  // ========== 布局逻辑 ==========
  QVBoxLayout* mainLayout = new QVBoxLayout(alarmDialog);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  QLabel* titleLabel = new QLabel(strTime);
  titleLabel->setObjectName("titleLabel");  // ✅ 必须在addWidget前设置
  titleLabel->setAlignment(Qt::AlignCenter);
  titleLabel->setContentsMargins(0, 20, 0, 20);
  mainLayout->addWidget(titleLabel);
  alarmDialog->titleLabel = titleLabel;

  QLabel* contentLabel =
      new QLabel(strText + "\n\n(" + strTodoAlarmActiveTime + ")");
  contentLabel->setObjectName("contentLabel");  // ✅ 必须在addWidget前设置
  contentLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  contentLabel->setWordWrap(true);
  contentLabel->setContentsMargins(40, 0, 40, 0);
  mainLayout->addWidget(contentLabel, 1);
  alarmDialog->contentLabel = contentLabel;

  QWidget* btnWidget = new QWidget();
  QHBoxLayout* btnLayout = new QHBoxLayout(btnWidget);
  btnLayout->setContentsMargins(50, 20, 50, 40);
  btnLayout->setSpacing(20);

  QPushButton* playBtn = new QPushButton(tr("Play"));
  playBtn->setObjectName("playBtn");
  playBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  playBtn->setMinimumHeight(48);
  btnLayout->addWidget(playBtn);

  QPushButton* closeBtn = new QPushButton(tr("Close"));
  closeBtn->setObjectName("closeBtn");
  closeBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  closeBtn->setMinimumHeight(48);
  btnLayout->addWidget(closeBtn);

  mainLayout->addWidget(btnWidget);

  // ========== 事件绑定 ==========
  bool isVoice = mw_one->m_Todo->isVoice(strText);
  connect(closeBtn, &QPushButton::pressed, alarmDialog, [=]() {
    alarmDialog->close();
    if (mui->frameMain->isHidden()) {
      m_Method->openMainEntranceWindow();
    }
  });

  connect(closeBtn, &QPushButton::pressed, this, &Todo::clearJavaNotify);

  connect(playBtn, &QPushButton::pressed, this, [=]() {
    qDebug() << "触发播放提醒：" << strText;
    if (isVoice) {
      QString voiceFile = mw_one->m_Todo->getVoiceFile(strText);
      if (m_Method) m_Method->playRecord(voiceFile);
    } else {
      isPlayBook = false;
      if (m_Method) {
        m_Method->stopPlayMyText();
        m_Method->playMyText(strText);
      }
    }
  });

  // ✅ 移除 BypassWindowManagerHint（这是透明层遮挡的元凶）
  // ✅ 移除 inputMethod 操作（避免键盘弹出/焦点抢夺）
  alarmDialog->setAttribute(Qt::WA_ShowWithoutActivating, false);
  alarmDialog->setWindowFlags(alarmDialog->windowFlags() |
                              Qt::WindowStaysOnTopHint);

  // ========== 显示窗口 ==========
  alarmDialog->show();

  // ✅ 延迟激活：确保 Native Window 已完成注册
  QTimer::singleShot(100, alarmDialog, [alarmDialog]() {
    if (!alarmDialog || !alarmDialog->isVisible()) return;
    alarmDialog->raise();
    alarmDialog->activateWindow();
    alarmDialog->setFocus(Qt::ActiveWindowFocusReason);

    // 安全地清除焦点代理，确保对话框自身接收事件
    if (auto* fw = alarmDialog->focusWidget()) {
      fw->clearFocus();
    }
    alarmDialog->setFocus();
  });
}

void Todo::playAlarmVoice() {}

void Todo::clearJavaNotify() {
#ifdef Q_OS_ANDROID
  try {
    // ========== 使用Qt6标准方式获取上下文 ==========
    QJniObject context;
    context = QNativeInterface::QAndroidApplication::context();

    if (context == nullptr) {
      qDebug() << "[清除通知] 获取Android上下文失败（context=null）！";
      return;
    }

    // ========== 传递context参数 ==========
    QJniObject::callStaticMethod<void>("com/x/MyService", "clearNotify",
                                       "(Landroid/content/Context;)V",
                                       context.object());

    // ========== 可选：检查Java异常（调试用） ==========
    QJniEnvironment env;
    if (env->ExceptionCheck()) {
      env->ExceptionDescribe();
      env->ExceptionClear();
      qDebug() << "[清除通知] 调用Java方法失败，捕获异常！";
      return;
    }

    qDebug() << "[清除通知] 调用clearNotify成功！";
  } catch (...) {
    qDebug() << "[清除通知] C++端捕获未知异常！";
  }
#endif
}

void Todo::openTodoListWindow(QStringList list) {
#ifdef Q_OS_ANDROID
  QJniObject activity = QNativeInterface::QAndroidApplication::context();

  QJniObject jArrayList("java/util/ArrayList", "()V");

  for (const QString& item : list) {
    QJniObject jItem = QJniObject::fromString(item);
    jArrayList.callMethod<bool>("add", "(Ljava/lang/Object;)Z", jItem.object());
  }

  activity.callMethod<void>("openTodoListWindow", "(Ljava/util/ArrayList;)V",
                            jArrayList.object());
#endif
}

void Todo::cppRefreshTodoCardList() {
#ifdef Q_OS_ANDROID
  QStringList list = listTodo;

  QJniObject activity = QNativeInterface::QAndroidApplication::context();

  QJniObject jArrayList("java/util/ArrayList", "()V");

  for (const QString& item : list) {
    QJniObject jItem = QJniObject::fromString(item);
    jArrayList.callMethod<bool>("add", "(Ljava/lang/Object;)Z", jItem.object());
  }

  activity.callMethod<void>("cppRefreshTodoCardList",
                            "(Ljava/util/ArrayList;)V", jArrayList.object());
#endif
}

void Todo::openTodoRecycleWindow(QStringList list) {
#ifdef Q_OS_ANDROID
  QJniObject activity = QNativeInterface::QAndroidApplication::context();

  QJniObject jArrayList("java/util/ArrayList", "()V");

  for (const QString& item : list) {
    QJniObject jItem = QJniObject::fromString(item);
    jArrayList.callMethod<bool>("add", "(Ljava/lang/Object;)Z", jItem.object());
  }

  activity.callMethod<void>("openTodoRecycleWindow", "(Ljava/util/ArrayList;)V",
                            jArrayList.object());
#endif
}

void Todo::openTodoAlarmWindow(QStringList list) {
#ifdef Q_OS_ANDROID
  QJniObject activity = QNativeInterface::QAndroidApplication::context();

  QJniObject jArrayList("java/util/ArrayList", "()V");

  for (const QString& item : list) {
    QJniObject jItem = QJniObject::fromString(item);
    jArrayList.callMethod<bool>("add", "(Ljava/lang/Object;)Z", jItem.object());
  }

  activity.callMethod<void>("openTodoAlarmWindow", "(Ljava/util/ArrayList;)V",
                            jArrayList.object());
#endif
}

void Todo::openClockActivity(const QString& content) {
  Q_UNUSED(content);
#ifdef Q_OS_ANDROID
  QJniObject activity =
      QJniObject(QNativeInterface::QAndroidApplication::context());
  if (activity.isValid()) {
    QJniObject jContent = QJniObject::fromString(content);
    activity.callMethod<void>("openClockActivityWithContent",
                              "(Ljava/lang/String;)V",
                              jContent.object<jstring>());
  }
#endif
}
