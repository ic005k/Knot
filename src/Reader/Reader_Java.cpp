#include "Reader.h"

void Reader::openReadListWindow(QStringList list) {
#ifdef Q_OS_ANDROID
  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  // 构造 Java ArrayList<String>
  QJniObject jArrayList("java/util/ArrayList", "()V");

  for (const QString& item : list) {
    QJniObject jItem = QJniObject::fromString(item);
    jArrayList.callMethod<bool>("add", "(Ljava/lang/Object;)Z", jItem.object());
  }

  // 调用Java方法 openReadListWindow(ArrayList<String>)
  activity.callMethod<void>("openReadListWindow", "(Ljava/util/ArrayList;)V",
                            jArrayList.object());
#endif
}
