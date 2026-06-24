#include "LoadPic.h"

#include "MainWindow.h"
#include "src/defines.h"
#include "ui_MainWindow.h"

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
  mui->btnShareImage->show();
#endif

  this->installEventFilter(this);
}

LoadPic::~LoadPic() {}

void LoadPic::initMain(QString imgFile) {
  bookimgFileName = privateDir + "bookimg.png";
  saveBase64ToPng(imgFile, bookimgFileName);

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

/**
 * @brief base64图片字符串转PNG本地文件
 * @param base64Str 输入 data:image/jpeg;base64,xxx 完整字符串
 * @param savePath 输出png完整路径，如 "./book.png"
 * @return 成功true / 失败false
 */
bool LoadPic::saveBase64ToPng(const QString& base64Str,
                              const QString& savePath) {
  // 1. 分割前缀与纯base64内容
  const QString prefix = "data:image/jpeg;base64,";
  if (!base64Str.startsWith(prefix)) {
    qWarning() << "Base64字符串格式错误，缺少前缀";
    return false;
  }
  QString pureBase64 = base64Str.mid(prefix.length());

  // 2. Base64解码二进制数据
  QByteArray imgBin = QByteArray::fromBase64(pureBase64.toLatin1());
  if (imgBin.isEmpty()) {
    qWarning() << "Base64解码失败";
    return false;
  }

  // 3. 二进制数据载入QImage（自动识别JPG）
  QImage img;
  if (!img.loadFromData(imgBin, "JPG")) {
    qWarning() << "二进制数据无法解析为图片";
    return false;
  }

  // 4. 创建文件并保存为PNG格式
  QFile file(savePath);
  if (!file.open(QIODevice::WriteOnly)) {
    qWarning() << "文件打开失败:" << file.errorString();
    return false;
  }
  // save第二个参数指定格式PNG，参数2为压缩质量(0-100，PNG无损忽略)
  bool ok = img.save(&file, "PNG", 100);
  file.close();

  if (ok)
    qDebug() << "PNG图片保存成功:" << savePath;
  else
    qWarning() << "图片保存PNG失败";
  return ok;
}
