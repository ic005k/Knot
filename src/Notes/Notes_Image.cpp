#include <QFileDialog>
#include <QRandomGenerator>

#include "src/Notes/Notes.h"

void Notes::on_btnPic_clicked() {
  QString fileName = QFileDialog::getOpenFileName(NULL, tr("Knot"), "",
                                                  tr("Picture Files (*.*)"));
  insertImage(fileName, false);
}

QString Notes::insertImage(QString fileName, bool isToAndroidView) {
  QFileInfo fi(fileName);
  QString strImage, tarImageFile;
  if (fi.exists()) {
    QDir dir;
    dir.mkpath(iniDir + "memo/images/");

    QString strTar = iniDir + "memo/images/" + getDateTimeStr() + "_" +
                     m_Method->generateRandom3() + ".png";
    if (QFile(strTar).exists()) QFile(strTar).remove();

    int nLeftMargin = 9 + 9 + 6;
    QImage img(fileName);
    double w = img.width(), h = img.height();
    int new_w, new_h;
    int w0 = 1024;
    double r = w / h;
    if (w > w0 - nLeftMargin) {
      new_w = w0 - nLeftMargin;
      new_h = new_w / r;
    } else {
      new_w = w;
      new_h = h;
    }

    if (!isAndroid) {
      auto msg = std::make_unique<ShowMessage>(this);
      msg->ui->btnCancel->setText(tr("No"));
      msg->ui->btnOk->setText(tr("Yes"));
      bool isYes = msg->showMsg(
          "Knot", tr("Is the original size of the image used?"), 2);
      if (isYes) {
        new_w = w;
        new_h = h;
      }
    }

    QPixmap pix = QPixmap::fromImage(img);
    pix =
        pix.scaled(new_w, new_h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pix.save(strTar);
    tarImageFile = strTar;
    strTar = strTar.replace(iniDir + "memo/", "");
    strImage = "\n\n![image](" + strTar + ")\n\n";

    if (!isAndroid) {
#ifndef Q_OS_ANDROID
      m_EditSource->insert(strImage);
#endif
    } else {
      if (isToAndroidView) insertNote(strImage);
    }
  }

  QString lastModi = m_Method->getFileUTCString(tarImageFile);
  QString zipImg = privateDir + "KnotData/memo/images/" + lastModi + "_" +
                   QFileInfo(tarImageFile).fileName() + ".zip";
  QFile::copy(tarImageFile, zipImg);
  zipImg = m_Method->useEnc(zipImg);
  appendToSyncList(zipImg);
  return strImage;
}

QString Notes::imageToBase64(const QString& path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) return "";
  QByteArray data = file.readAll();
  return "data:image/png;base64," + data.toBase64();
}