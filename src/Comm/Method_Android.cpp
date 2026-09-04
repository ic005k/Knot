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

  for (int i = 0; i < tabData->count(); i++) {
    QString tabText = tabData->tabText(i);
    QTreeWidget* tw = mw_one->get_tw(i);
    int isFlagToday = m_Method->getFlagToday(tw);
    list1.append(tabText + "|==|" + QString::number(isFlagToday));
  }

  int index = tabData->currentIndex();
  setMainTabLastSelectedPos(index);

  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  QJniObject jArrayList("java/util/ArrayList", "()V");

  for (const QString& item : list1) {
    QJniObject jItem = QJniObject::fromString(item);
    jArrayList.callMethod<bool>("add", "(Ljava/lang/Object;)Z", jItem.object());
  }

  activity.callMethod<void>("openMainEntranceWindow",
                            "(Ljava/util/ArrayList;)V", jArrayList.object());
#endif
}

void Method::closeMainEntranceWindow() {
#ifdef Q_OS_ANDROID
  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  activity.callMethod<void>("closeMainEntranceWindow", "()V");
#endif
}

float Method::getSystemFontScale() {
#ifdef Q_OS_ANDROID

  QJniObject ctx = QNativeInterface::QAndroidApplication::context();
  if (!ctx.isValid()) return 1.0f;

  jfloat scale = QJniObject::callStaticMethod<jfloat>(
      "com/x/MyActivity", "getSystemFontScale", "(Landroid/content/Context;)F",
      ctx.object());
  return static_cast<qreal>(scale);
#else
  // 桌面端固定缩放系数1.0
  return 1.0f;
#endif
}

void Method::setMainTabLastSelectedPos(int index) {
#ifdef Q_OS_ANDROID
  QJniObject::callStaticMethod<void>("com/x/MyActivity",
                                     "setMainTabLastSelectedPos", "(I)V",
                                     static_cast<jint>(index));
#endif
}

int Method::getMainTabLastSelectedPos() {
#ifdef Q_OS_ANDROID
  return QJniObject::callStaticMethod<jint>("com/x/MyActivity",
                                            "getMainTabLastSelectedPos", "()I");
#else
  return -1;
#endif
}

void Method::refreshMainEntranceCards() {
#ifdef Q_OS_ANDROID
  // 1. 构建与打开时相同格式的数据列表
  QStringList list1;
  for (int i = 0; i < tabData->count(); i++) {
    QString tabText = tabData->tabText(i);
    QTreeWidget* tw = mw_one->get_tw(i);
    int isFlagToday = m_Method->getFlagToday(tw);
    list1.append(tabText + "|==|" + QString::number(isFlagToday));
  }

  int index = tabData->currentIndex();
  setMainTabLastSelectedPos(index);

  // 2. 构造 Java ArrayList<String>
  QJniObject jArrayList("java/util/ArrayList", "()V");
  for (const QString& item : list1) {
    QJniObject jItem = QJniObject::fromString(item);
    jArrayList.callMethod<bool>("add", "(Ljava/lang/Object;)Z", jItem.object());
  }

  // 3. 通过静态实例调用 refreshCardList
  QJniObject instance = QJniObject::getStaticObjectField(
      "com/x/MainEntrance", "mInstance", "Lcom/x/MainEntrance;");

  if (instance.isValid()) {
    instance.callMethod<void>("refreshCardList", "(Ljava/util/ArrayList;)V",
                              jArrayList.object());
  }

  qInfo() << "Tab Text List=" << list1 << index;
#endif
}

void Method::openMyEventWindow() {
#ifdef Q_OS_ANDROID
  QStringList list1, list2, list3;
  list1 = mw_one->listMyEventTitle;
  list2 = mw_one->listMainDate;
  list3 = mw_one->listMainDateDetail;

  QJniObject activity = QNativeInterface::QAndroidApplication::context();

  QJniObject jArrayList("java/util/ArrayList", "()V");

  for (const QString& item : list1) {
    QJniObject jItem = QJniObject::fromString(item);
    jArrayList.callMethod<bool>("add", "(Ljava/lang/Object;)Z", jItem.object());
  }

  activity.callMethod<void>("openMyEventWindow", "(Ljava/util/ArrayList;)V",
                            jArrayList.object());
#endif
}

void Method::refreshMainDate() {
#ifdef Q_OS_ANDROID
  // 1. 构建与打开时相同格式的数据列表
  QStringList list1 = mw_one->listMainDate;

  QJniObject jStr = QJniObject::fromString(strStats);

  // 2. 构造 Java ArrayList<String>
  QJniObject jArrayList("java/util/ArrayList", "()V");
  for (const QString& item : list1) {
    QJniObject jItem = QJniObject::fromString(item);
    jArrayList.callMethod<bool>("add", "(Ljava/lang/Object;)Z", jItem.object());
  }

  // 3. 通过静态实例调用
  QJniObject instance = QJniObject::getStaticObjectField(
      "com/x/MyEventActivity", "mInstance", "Lcom/x/MyEventActivity;");

  if (instance.isValid()) {
    instance.callMethod<void>("refreshLeftGroupList",
                              "(Ljava/util/ArrayList;)V", jArrayList.object());

    instance.callMethod<void>("refreshTotalValue", "(Ljava/lang/String;)V",
                              jStr.object());
  }

  qInfo() << "MainData=" << list1;

#endif
}

void Method::refreshMainDateDetail() {
#ifdef Q_OS_ANDROID

  QStringList list1 = mw_one->listMainDateDetail;

  QJniObject jArrayList("java/util/ArrayList", "()V");
  for (const QString& item : list1) {
    QJniObject jItem = QJniObject::fromString(item);
    jArrayList.callMethod<bool>("add", "(Ljava/lang/Object;)Z", jItem.object());
  }

  QJniObject instance = QJniObject::getStaticObjectField(
      "com/x/MyEventActivity", "mInstance", "Lcom/x/MyEventActivity;");

  if (instance.isValid()) {
    instance.callMethod<void>("refreshRightDetailList",
                              "(Ljava/util/ArrayList;)V", jArrayList.object());
  }

  qInfo() << "MainDataDetail=" << list1;

#endif
}

void Method::openTabRecycleBinActivity(QStringList list) {
#ifdef Q_OS_ANDROID

  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  QJniObject jArrayList("java/util/ArrayList", "()V");

  for (const QString& item : list) {
    QJniObject jItem = QJniObject::fromString(item);
    jArrayList.callMethod<bool>("add", "(Ljava/lang/Object;)Z", jItem.object());
  }

  activity.callMethod<void>("openTabRecycleBinActivity",
                            "(Ljava/util/ArrayList;)V", jArrayList.object());
#endif
}

void Method::openActivity(QString callJavaName, QStringList list) {
#ifdef Q_OS_ANDROID

  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  QJniObject jArrayList("java/util/ArrayList", "()V");

  for (const QString& item : list) {
    QJniObject jItem = QJniObject::fromString(item);
    jArrayList.callMethod<bool>("add", "(Ljava/lang/Object;)Z", jItem.object());
  }

  activity.callMethod<void>(callJavaName.toUtf8().constData(),
                            "(Ljava/util/ArrayList;)V", jArrayList.object());

  qInfo() << callJavaName << "=" << list;
#endif
}

void Method::refreshJavaData(QString callJavaName, QString className,
                             QStringList list) {
#ifdef Q_OS_ANDROID

  QJniObject jArrayList("java/util/ArrayList", "()V");
  for (const QString& item : list) {
    QJniObject jItem = QJniObject::fromString(item);
    jArrayList.callMethod<bool>("add", "(Ljava/lang/Object;)Z", jItem.object());
  }

  QString c1, c2;
  c1 = "com/x/" + className;
  c2 = "Lcom/x/" + className + ";";

  QJniObject instance = QJniObject::getStaticObjectField(
      c1.toUtf8().constData(), "mInstance", c2.toUtf8().constData());

  if (instance.isValid()) {
    instance.callMethod<void>(callJavaName.toUtf8().constData(),
                              "(Ljava/util/ArrayList;)V", jArrayList.object());
  }

  qInfo() << callJavaName << "=" << list;

#endif
}