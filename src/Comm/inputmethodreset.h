// inputmethodreset.h
#pragma once

#include <qpa/qplatforminputcontext.h>
#include <qpa/qplatformintegration.h>

#include <QGuiApplication>
#include <QInputMethod>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif

#include <QObject>
#include <QWindow>

class InputMethodReset : public QObject {
  Q_OBJECT

 public:
  static InputMethodReset& instance();

  void fullReset();

 private:
  InputMethodReset(QObject* parent = nullptr);

  void resetQtContext();
  void resetAndroidContext();
  void resetNativeBridge();
  void resetWindowSystem();

  bool m_resetting = false;
};
