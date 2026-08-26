#include <QKeyEvent>

#include "Method.h"
#include "src/Comm/IOSCircularProgress.h"
#include "src/MainWindow.h"

void Method::exitSystemFullscreen() {
#ifdef Q_OS_ANDROID

  QJniObject activity =
      QJniObject(QCoreApplication::instance()
                     ->nativeInterface<QNativeInterface::QAndroidApplication>()
                     ->context());
  activity.callMethod<void>("exitSystemFullscreen", "()V");

#endif
}

QString Method::getTempSwapStr() {
#ifdef Q_OS_ANDROID
  QJniObject activity =
      QJniObject(QCoreApplication::instance()
                     ->nativeInterface<QNativeInterface::QAndroidApplication>()
                     ->context());
  // JNI签名 ()Ljava/lang/String;
  QJniObject jRet =
      activity.callObjectMethod("getTempSwapStr", "()Ljava/lang/String;");
  if (jRet.isValid()) {
    return jRet.toString();
  }
  return QString();
#else
  // 非安卓平台返回空字符串
  return QString();
#endif
}

void Method::setTempSwapStr(const QString& str) {
  Q_UNUSED(str);

#ifdef Q_OS_ANDROID
  auto* androidApp =
      QCoreApplication::instance()
          ->nativeInterface<QNativeInterface::QAndroidApplication>();
  if (!androidApp) return;

  QJniObject activity = QJniObject(androidApp->context());
  if (!activity.isValid()) return;

  // 将QString转为JNI字符串对象
  QJniObject jStr = QJniObject::fromString(str);

  // Java: public void setTempSwapStr(String tempSwapStr)
  // 签名：(Ljava/lang/String;)V
  activity.callMethod<void>("setTempSwapStr", "(Ljava/lang/String;)V",
                            jStr.object());
#endif
}

void Method::openMainEntranceWindow() {
#ifdef Q_OS_ANDROID
  QStringList list1;
  int count = mui->tabWidget->count();
  for (int i = 0; i < count; i++) {
    list1.append(mui->tabWidget->tabText(i));
  }

  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  // 构造 Java ArrayList<String>
  QJniObject jArrayList("java/util/ArrayList", "()V");

  for (const QString& item : list1) {
    QJniObject jItem = QJniObject::fromString(item);
    jArrayList.callMethod<bool>("add", "(Ljava/lang/Object;)Z", jItem.object());
  }

  activity.callMethod<void>("openMainEntranceWindow",
                            "(Ljava/util/ArrayList;)V", jArrayList.object());
#endif
}
