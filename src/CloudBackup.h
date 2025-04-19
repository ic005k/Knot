#ifndef CLOUDBACKUP_H
#define CLOUDBACKUP_H

#include <QAuthenticator>
#include <QCoreApplication>
#include <QDebug>
#include <QDialog>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QQueue>
#include <QQuickWidget>

namespace Ui {
class CloudBackup;
}

class WebDavHelper;
class WebDavDownloader;
class CloudDeleter;

class QtOneDrive;
class CloudBackup : public QDialog {
  Q_OBJECT
 public:
  explicit CloudBackup(QWidget *parent = 0);
  ~CloudBackup();
  Ui::CloudBackup *ui;

  bool isGetRemoteFileListEnd = false;
  QStringList webdavFileList;
  QList<QDateTime> webdavDateTimeList;
  QQuickWidget *quickWidget;
  void init();

  QString WEBDAV_URL = "";
  QString USERNAME = "";
  QString APP_PASSWORD = "";

  void loadLogQML();

  void initQuick();

  int getProg();

  void loadText(QString str);

  void uploadData();

  void uploadFileToWebDAV(QString webdavUrl, QString localFilePath,
                          QString remoteFileName);
  void createDirectory(QString webdavUrl, QString remoteDirPath);
  void startBakData();
  void downloadFile(QString remoteFileName, QString localSavePath);

  QString aesEncrypt(QString plainText, QByteArray key, QByteArray iv);
  QString aesDecrypt(QString cipherText, QByteArray key, QByteArray iv);

  QString getWebDAVArgument();
  void uploadFilesToWebDAV(QStringList files);

  void getRemoteFileList(QString url);
  void createRemoteWebDAVDir();
  void deleteWebDAVFiles(QStringList filesToDelete);
 signals:

 protected:
  bool eventFilter(QObject *obj, QEvent *evn) override;
 public slots:
  void on_pushButton_SignIn_clicked();
  void on_pushButton_SingOut_clicked();
  void on_pushButton_downloadFile_clicked();

  void on_pushButton_GetUserInfo_clicked();

  void on_pushButton_clicked();

  void on_lineEdit_fileID_textChanged(const QString &arg1);

  void on_pushButton_getFiles_clicked();

  void on_pushButton_traserveFolder_clicked();

  void on_pushButton_getFolders_clicked();

  void on_pushButton_createFolder_clicked();

  void on_pushButton_deleteFile_clicked();

  void on_pushButton_storageInfo_clicked();

  void on_btnBack_clicked();

 private slots:
  void updateUploadProgress(qint64 bytesSent, qint64 bytesTotal);

 private:
  QtOneDrive *oneDrive = nullptr;
  QString initUserInfo(QString info);

  QNetworkAccessManager *m_manager = nullptr;
  QHash<QNetworkReply *, QFile *> m_activeDownloads;  // 必须声明为类成员
};

// 声明一个轻量级信号发射器,列出WebDAV上某个目录下的所有文件
class WebDavHelper : public QObject {
  Q_OBJECT
 public:
  explicit WebDavHelper(QObject *parent = nullptr) : QObject(parent) {}

 signals:
  void listCompleted(const QList<QPair<QString, QDateTime>> &files);
  void errorOccurred(const QString &error);
};

class WebDavDownloader : public QObject {
  Q_OBJECT
 public:
  explicit WebDavDownloader(const QString &username, const QString &password,
                            QObject *parent = nullptr);

  void downloadFiles(const QList<QString> &remotePaths,
                     const QString &localBaseDir, int maxConcurrent = 3);

 signals:
  void progressChanged(int current, int total, QString currentFile);
  void downloadFinished(bool success, const QString &error);
  void fileSaved(QString localPath);

 private slots:
  void handleAuthentication(QNetworkReply *reply, QAuthenticator *auth);
  void startNextDownload();
  void onDownloadFinished(QNetworkReply *reply);

 private:
  QNetworkAccessManager manager;
  QQueue<QPair<QString, QString>> downloadQueue;
  QHash<QNetworkReply *, QString> activeDownloads;
  QString m_username;
  QString m_password;
  int maxConcurrent;
  int totalFiles;
  int completedFiles;
};

class CloudDeleter : public QObject {
  Q_OBJECT
 public:
  CloudDeleter(const QString &user, const QString &appPassword,
               QObject *parent = nullptr)
      : QObject(parent), username(user), password(appPassword) {}

  QString baseUrl = "https://dav.jianguoyun.com/dav/";

  void deleteFiles(const QList<QString> &filePaths) {
    QEventLoop loop;
    int remaining = filePaths.count();

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    connect(
        manager, &QNetworkAccessManager::finished, [&](QNetworkReply *reply) {
          remaining--;
          if (reply->error() == QNetworkReply::NoError) {
            qInfo() << "Deleted:" << reply->request().url().path();
          } else {
            qWarning() << "Failed to delete" << reply->request().url().path()
                       << ":" << reply->errorString();
          }
          reply->deleteLater();
          if (remaining <= 0) loop.quit();
        });

    connect(manager, &QNetworkAccessManager::authenticationRequired,
            [this](QNetworkReply *, QAuthenticator *auth) {
              auth->setUser(this->username);
              auth->setPassword(this->password);
            });

    foreach (const QString &path, filePaths) {
      QUrl url(baseUrl + path);
      QNetworkRequest request(url);
      request.setAttribute(QNetworkRequest::User, path);  // 保存原始路径

      QNetworkReply *reply = manager->deleteResource(request);
      connect(reply, &QNetworkReply::sslErrors,
              [](const QList<QSslError> &errors) {
                qWarning() << "SSL Errors:" << errors;
              });
    }

    loop.exec();
    manager->deleteLater();
  }

 private:
  QString username;
  QString password;
};

#endif  // CLOUDBACKUP_H
