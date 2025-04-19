#include "AutoUpdate.h"

#include "MainWindow.h"
#include "ui_AutoUpdate.h"

extern MainWindow* mw_one;
extern Method* m_Method;
extern QString iniDir, privateDir;
extern bool isDark;

AutoUpdate::AutoUpdate(QWidget* parent)
    : QDialog(parent), ui(new Ui::AutoUpdate) {
  ui->setupUi(this);

  setWindowFlag(Qt::FramelessWindowHint);
  setAttribute(Qt::WA_TranslucentBackground);
  if (isDark)
    ui->frame->setStyleSheet(
        "#frame{background-color: #19232D; border-radius:10px; "
        "border:1px solid gray;}");
  else
    ui->frame->setStyleSheet(
        "#frame{background-color: rgb(250, 250, 250); border-radius:10px; "
        "border:1px solid gray;}");

  mw_one->set_ToolButtonStyle(this);
  setModal(true);
  ui->lblTxt->adjustSize();
  ui->lblTxt->setText(tr("Download Progress") + " : \n" + "");

  myfile = new QFile(this);
  manager = new QNetworkAccessManager(this);
}

bool AutoUpdate::eventFilter(QObject* watch, QEvent* evn) {
  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
      return false;
    }
  }

  return QWidget::eventFilter(watch, evn);
}

AutoUpdate::~AutoUpdate() { delete ui; }

void AutoUpdate::doProcessReadyRead()  // 读取并写入
{
  while (!reply->atEnd()) {
    QByteArray ba = reply->readAll();
    myfile->write(ba);
  }
}

void AutoUpdate::doProcessFinished() {
  myfile->close();
  this->close();
  m_Method->closeGrayWindows();
  if (isCancel) return;

    // install apk
#ifdef Q_OS_ANDROID
    // "/storage/emulated/0/KnotBak/"

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QAndroidJniObject jo = QAndroidJniObject::fromString(tarFile);
  // jo.callStaticMethod<void>("com.x/MyActivity", "setAPKFile",
  //                          "(Ljava/lang/String;)V", jo.object<jstring>());

  QAndroidJniObject m_activity = QtAndroid::androidActivity();
  // m_activity.callMethod<void>("installApk");
  m_activity.callMethod<void>("installApk", "(Ljava/lang/String;)V",
                              jo.object<jstring>());
#else
  QJniObject jo = QJniObject::fromString(tarFile);
  QJniObject m_activity = QNativeInterface::QAndroidApplication::context();
  m_activity.callMethod<void>("installApk", "(Ljava/lang/String;)V",
                              jo.object<jstring>());
#endif

#endif
}

void AutoUpdate::doProcessDownloadProgress(qint64 recv_total,
                                           qint64 all_total) {
  ui->progressBar->setMaximum(all_total);
  ui->progressBar->setValue(recv_total);
  ui->lblTxt->setText(tr("Download Progress") + " : \n" +
                      GetFileSize(recv_total, 2) + " -> " +
                      GetFileSize(all_total, 2));

  if (recv_total == all_total) {
    if (recv_total < 10000) {
      return;
    }

    this->repaint();
  }
}

void AutoUpdate::startDownload(QString strLink) {
  isCancel = false;

  QNetworkRequest request;
  request.setUrl(QUrl(strLink));

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  // github redirects the request, so this attribute must be set to true,
  // otherwise returns nothing from qt5.6
  request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
#endif

  reply = manager->get(request);  // 发送请求
  connect(reply, &QNetworkReply::readyRead, this,
          &AutoUpdate::doProcessReadyRead);  // 可读
  connect(reply, &QNetworkReply::finished, this,
          &AutoUpdate::doProcessFinished);
  connect(reply, &QNetworkReply::downloadProgress, this,
          &AutoUpdate::doProcessDownloadProgress);  // 大小

  filename = "Knot.apk";

#ifdef Q_OS_MAC
  tarFile = privateDir + filename;
#endif

#ifdef Q_OS_ANDROID
  // "/storage/emulated/0/KnotBak/"
  tarFile = privateDir + filename;
#endif
  QFile apk(tarFile);
  apk.remove();

  myfile->setFileName(tarFile);
  bool ret =
      myfile->open(QIODevice::WriteOnly | QIODevice::Truncate);  // 创建文件
  if (!ret) {
    QMessageBox::warning(this, "warning", "Failed to open.");
    return;
  }
  ui->progressBar->setValue(0);
  ui->progressBar->setMinimum(0);
}

void AutoUpdate::startUpdate() {
  QString strZip, strPath, strExec;
  QFileInfo appInfo(qApp->applicationDirPath());
  strZip = privateDir + filename;

  QDir dir;
  dir.setCurrent(privateDir);

  qApp->exit();

#ifdef Q_OS_MAC

  QString fileName = tempDir + "up.sh";
  QTextEdit* txtEdit = new QTextEdit();
  QString strTarget = appInfo.path().replace("Contents", "");
  strTarget = strTarget + ".";
  strTarget = "\"" + strTarget + "\"";

  txtEdit->append("hdiutil mount -mountpoint /Volumes/Knot " + strZip);
  txtEdit->append(
      "cp -R -p -f "
      "/Volumes/Knot/Knot.app/. " +
      strTarget);

  txtEdit->append("hdiutil eject /Volumes/Knot");

  strPath = appInfo.path().replace("Contents", "");
  strExec = strPath.mid(0, strPath.length() - 1);
  strExec = "\"" + strExec + "\"";
  txtEdit->append("open " + strExec);

  TextEditToFile(txtEdit, fileName);

  QProcess::startDetached("bash", QStringList() << fileName);

#endif

#ifdef Q_OS_WIN
  QString strUnzip;
  QString fileName = privateDir + "up.bat";
  strPath = appInfo.filePath();

  QTextEdit* txtEdit = new QTextEdit();
  strUnzip = strPath + "/unzip.exe";
  strUnzip = "\"" + strUnzip + "\"";
  strZip = "\"" + strZip + "\"";
  strPath = "\"" + strPath + "\"";
  strExec = qApp->applicationFilePath();
  strExec = "\"" + strExec + "\"";
  QString strCommand1, strCommand2;
  QString strx = "\"" + privateDir + "\"";
  strCommand1 = strUnzip + " -o " + strZip + " -d " + strx;
  QString stry = privateDir + QFileInfo(filename).baseName();
  stry = "\"" + stry + "\"";
  strCommand2 = "xcopy " + stry + " " + strPath + " /s/y";
  txtEdit->append(strCommand1 + " && " + strCommand2 + " && " + strExec);

  TextEditToFile(txtEdit, fileName);

  QProcess::startDetached("cmd.exe", QStringList() << "/c" << fileName);
#endif

#ifdef Q_OS_LINUX
  QString fileName = privateDir + "up.sh";
  QTextEdit* txtEdit = new QTextEdit();
  strZip = "\"" + strZip + "\"";
  strLinuxTargetFile = "\"" + strLinuxTargetFile + "\"";
  txtEdit->append("cp -f " + strZip + " " + strLinuxTargetFile);
  txtEdit->append(strLinuxTargetFile);

  TextEditToFile(txtEdit, fileName);

  QProcess::execute("chmod", QStringList() << "+x" << fileName);
  QProcess::startDetached("bash", QStringList() << fileName);
#endif
}

void AutoUpdate::closeEvent(QCloseEvent* event) {
  Q_UNUSED(event);
  reply->close();
  myfile->close();
}

QString AutoUpdate::GetFileSize(qint64 size) {
  if (size < 0) return "0";
  if (!size) {
    return "0 Bytes";
  }
  static QStringList SizeNames;
  if (SizeNames.empty()) {
    SizeNames << " Bytes"
              << " KB"
              << " MB"
              << " GB"
              << " TB"
              << " PB"
              << " EB"
              << " ZB"
              << " YB";
  }
  int i = qFloor(qLn(size) / qLn(1024));
  return QString::number(size * 1.0 / qPow(1000, qFloor(i)), 'f',
                         (i > 1) ? 2 : 0) +
         SizeNames.at(i);
}

QString AutoUpdate::GetFileSize(const qint64& size, int precision) {
  double sizeAsDouble = size;
  static QStringList measures;
  if (measures.isEmpty())
    measures << QCoreApplication::translate("QInstaller", "bytes")
             << QCoreApplication::translate("QInstaller", "KiB")
             << QCoreApplication::translate("QInstaller", "MiB")
             << QCoreApplication::translate("QInstaller", "GiB")
             << QCoreApplication::translate("QInstaller", "TiB")
             << QCoreApplication::translate("QInstaller", "PiB")
             << QCoreApplication::translate("QInstaller", "EiB")
             << QCoreApplication::translate("QInstaller", "ZiB")
             << QCoreApplication::translate("QInstaller", "YiB");
  QStringListIterator it(measures);
  QString measure(it.next());
  while (sizeAsDouble >= 1024.0 && it.hasNext()) {
    measure = it.next();
    sizeAsDouble /= 1024.0;
  }
  return QString::fromLatin1("%1 %2")
      .arg(sizeAsDouble, 0, 'f', precision)
      .arg(measure);
}

void AutoUpdate::TextEditToFile(QTextEdit* txtEdit, QString fileName) {
  QFile* file;
  file = new QFile;
  file->setFileName(fileName);
  bool ok = file->open(QIODevice::WriteOnly);
  if (ok) {
    QTextStream out(file);
    out << txtEdit->toPlainText();
    file->close();
    delete file;
  }
}

void AutoUpdate::keyPressEvent(QKeyEvent* event) {
  switch (event->key()) {
    case Qt::Key_Escape:
      // reply->close();
      // close();
      break;

    case Qt::Key_Return:

      break;

    case Qt::Key_Backspace:

      break;

    case Qt::Key_Space:

      break;

    case Qt::Key_F1:

      break;
  }

  if (event->modifiers() == Qt::ControlModifier) {
    if (event->key() == Qt::Key_M) {
      this->setWindowState(Qt::WindowMaximized);
    }
  }
}

void AutoUpdate::on_btnCancel_clicked() {
  isCancel = true;
  close();
}
