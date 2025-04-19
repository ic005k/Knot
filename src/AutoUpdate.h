#ifndef AUTOUPDATE_H
#define AUTOUPDATE_H

#include <QDebug>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QTextEdit>
#include <QTextStream>
#include <QtMath>

namespace Ui {
class AutoUpdate;
}

class AutoUpdate : public QDialog {
  Q_OBJECT

 public:
  explicit AutoUpdate(QWidget* parent = nullptr);
  ~AutoUpdate();
  Ui::AutoUpdate* ui;

  QString strLinuxTargetFile;
  QString tempDir;
  QString filename;
  QNetworkAccessManager* manager;
  QNetworkReply* reply;
  QFile* myfile;

  void doProcessReadyRead();
  void doProcessFinished();
  void doProcessDownloadProgress(qint64, qint64);

  void startUpdate();

  QString GetFileSize(qint64 size);

  void TextEditToFile(QTextEdit* txtEdit, QString fileName);

  QString GetFileSize(const qint64& size, int precision);

  void startDownload(QString strLink);

 protected:
  void closeEvent(QCloseEvent* event);
  void keyPressEvent(QKeyEvent* event);
  bool eventFilter(QObject* watch, QEvent* evn);
 private slots:

  void on_btnCancel_clicked();

 private:
  QString tarFile;
  bool isCancel = false;
};

#endif  // AUTOUPDATE_H
