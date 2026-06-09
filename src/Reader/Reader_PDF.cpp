#include "Reader.h"

void Reader::openMyPDF(QString uri) {
  Q_UNUSED(uri);
#ifdef Q_OS_ANDROID

  QJniObject jPath = QJniObject::fromString(uri);
  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  activity.callMethod<void>("openMyPDF", "(Ljava/lang/String;)V",
                            jPath.object<jstring>());

#endif
}

void Reader::closeMyPDF() {
#ifdef Q_OS_ANDROID

  // QJniObject activity = QNativeInterface::QAndroidApplication::context();
  QJniObject::callStaticMethod<void>("com.xhh.pdfui/PDFActivity", "closeMyPDF",
                                     "()V");

#endif
}
