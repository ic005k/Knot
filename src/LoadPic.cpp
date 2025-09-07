#include "LoadPic.h"

#include "MainWindow.h"
#include "ui_MainWindow.h"
extern MainWindow* mw_one;
extern Method* m_Method;
extern QString picfile, imgFileName;

extern Ui::MainWindow* mui;

LoadPic::LoadPic(QWidget* parent) : QDialog(parent) {
  QFont font = this->font();
  font.setPointSize(13);
  mui->lblImgInfo->setFont(font);
  mui->lblImgInfo->adjustSize();
  mui->lblImgInfo->setWordWrap(true);
  mui->lblImgInfo->setText("");

#ifdef Q_OS_ANDROID
  mui->btnShareImage->show();
#else
  mui->btnShareImage->hide();
#endif

  this->installEventFilter(this);
}

LoadPic::~LoadPic() {}

void LoadPic::initMain(QString imgFile) {
  imgFileName = imgFile;
  // mui->lblImgInfo->setText(imgFile + "  " +
  //                         m_Method->getFileSize(QFile(imgFile).size(), 2));

  if (mui->frameReader->isVisible()) mui->frameReader->hide();
  if (mui->frameNotesGraph->isVisible()) mui->frameNotesGraph->hide();

  mui->qw_Img->rootContext()->setContextProperty("myW", mw_one->width());
  mui->qw_Img->rootContext()->setContextProperty("myH", mw_one->height());

  mui->frameImgView->show();

  mui->qw_Img->rootContext()->setContextProperty("imgW",
                                                 mui->qw_Img->width() * 10);
  mui->qw_Img->rootContext()->setContextProperty("imgH",
                                                 mui->qw_Img->height() * 10);
  mui->qw_Img->rootContext()->setContextProperty("imgFile", imgFile);
  mui->qw_Img->setSource(QUrl(QStringLiteral("qrc:/src/qmlsrc/imgview.qml")));
}

bool LoadPic::eventFilter(QObject* watch, QEvent* evn) {
  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
      return true;
    }
  }

  return QWidget::eventFilter(watch, evn);
}
