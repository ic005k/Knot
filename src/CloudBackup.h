#ifndef CLOUDBACKUP_H
#define CLOUDBACKUP_H

#include <QAtomicInt>
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
#include <functional>

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

  void getRemoteFileList(QString url);
  void createRemoteWebDAVDir();
  void deleteWebDAVFiles(QStringList filesToDelete);
  void uploadFilesToWebDAV(const QStringList &files);
  void backExit();
  void init_CloudBacup();
  void webDAVRestoreData();

  void changeComBoxWebDAV(const QString &arg1);
  bool checkWebDAVConnection();
  void on_pushButton_downloadFile_clicked();
  void on_btnBack_clicked();

  void uploadFilesToWebDAV_old(QStringList files);

  QString unifyWebDAVBaseUrlToDavEnd(QString url);

  QString getWebDAVDataDir(const QString &url);
 signals:

 protected:
  bool eventFilter(QObject *obj, QEvent *evn) override;
 public slots:

 private slots:
  void updateUploadProgress(qint64 bytesSent, qint64 bytesTotal);
  void startNextUpload();

  void handleUploadFinished(QNetworkReply *reply, const QString &filePath);

 private:
  QtOneDrive *oneDrive = nullptr;
  QString initUserInfo(QString info);

  QNetworkAccessManager *m_manager = nullptr;
  QHash<QNetworkReply *, QFile *> m_activeDownloads;  // 必须声明为类成员

  QByteArray aes_key = "MySuperSecretKey1234567890";  // 长度不足32会自动处理
  QByteArray aes_iv = "InitializationVe";             // 16字节
  void resetProgBar();

  QNetworkAccessManager *manager = nullptr;
  QQueue<QString> uploadQueue;
  QSet<QNetworkReply *> activeReplies;
  int maxConcurrentUploads = 1;  // 并发数

  QString nextPageUrl;
  QList<QPair<QString, QDateTime>> allFiles;

  void fetchNextPage();
  void processFileList();
  QString parseNextPageUrl(const QList<QNetworkReply::RawHeaderPair> &headers);
};

// 声明一个轻量级信号发射器,列出WebDAV上某个目录下的所有文件
class WebDavHelper : public QObject {
  Q_OBJECT
 public:
  explicit WebDavHelper(QObject *parent = nullptr) : QObject(parent) {}

  void setResponseHeaders(const QList<QNetworkReply::RawHeaderPair> &headers) {
    responseHeaders = headers;
  }

  QList<QNetworkReply::RawHeaderPair> getResponseHeaders() const {
    return responseHeaders;
  }

 signals:
  void listCompleted(const QList<QPair<QString, QDateTime>> &files);
  void errorOccurred(const QString &error);

 private:
  QList<QNetworkReply::RawHeaderPair> responseHeaders;
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

#endif  // CLOUDBACKUP_H
