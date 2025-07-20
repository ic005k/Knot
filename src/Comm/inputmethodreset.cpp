#include "inputmethodreset.h"

#include <QCoreApplication>
#include <QDebug>
#include <QGuiApplication>
#include <QInputMethod>
#include <QInputMethodEvent>

#ifdef Q_OS_ANDROID
#include <QJniEnvironment>
#include <QJniObject>
#endif

#include <QWindow>

InputMethodReset &InputMethodReset::instance() {
  static InputMethodReset instance;
  return instance;
}

InputMethodReset::InputMethodReset(QObject *parent) : QObject(parent) {}

void InputMethodReset::fullReset() {
  if (m_resetting) return;
  m_resetting = true;

  qDebug() << "Performing full input method reset";

  resetQtContext();
  resetAndroidContext();
  resetNativeBridge();
  resetWindowSystem();

  m_resetting = false;
}

void InputMethodReset::resetQtContext() {
  // 重置输入法
  QInputMethod *im = QGuiApplication::inputMethod();
  if (im) {
    im->reset();
    im->hide();
  }

  // 替代 platformIntegration() 的方法
  // 通过焦点窗口获取输入上下文
  QWindow *focusWindow = QGuiApplication::focusWindow();
  if (focusWindow) {
    // 发送重置事件
    QInputMethodEvent event;
    QCoreApplication::sendEvent(focusWindow, &event);
  }

  // 强制更新输入法状态
  if (im) {
    im->update(Qt::ImQueryInput);
  }
}

void InputMethodReset::resetAndroidContext() {
#ifdef Q_OS_ANDROID

  // 获取 Android Activity
  QJniObject activity = QJniObject::callStaticObjectMethod(
      "org/qtproject/qt/android/QtNative", "activity",
      "()Landroid/app/Activity;");

  if (!activity.isValid()) {
    qWarning() << "Failed to get Android activity";
    return;
  }

  // 调用 Java 层深度重置
  QJniObject::callStaticMethod<void>(
      "com/x/InputMethodHelper", "deepResetInputMethod",
      "(Landroid/app/Activity;)V", activity.object());

#endif
}

void InputMethodReset::resetNativeBridge() {
#ifdef Q_OS_ANDROID
  // 重置 JNI 引用
  QJniObject::callStaticMethod<void>("com/x/InputMethodHelper",
                                     "resetNativeReferences");
#endif
}

void InputMethodReset::resetWindowSystem() {
  // 重置所有窗口的输入法状态
  for (QWindow *window : QGuiApplication::allWindows()) {
    // 通过隐藏再显示来刷新窗口
    bool wasVisible = window->isVisible();
    if (wasVisible) {
      window->hide();
      window->show();
    }
  }
}
