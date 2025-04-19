#ifndef QTONEDRIVE_H
#define QTONEDRIVE_H

#include <QDateTime>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QSet>
#include <QUrl>
#include <QUrlQuery>

#include "qtonedrivelib_global.h"

class QtOneDriveAuthorizationDialog;
class QSettings;
class QNetworkAccessManager;
class QNetworkReply;
class QFile;

class QTONEDRIVELIBSHARED_EXPORT QtOneDrive : public QObject {
  Q_OBJECT

  enum OneDriveState {
    Empty = 0,

    SingIn,
    SingOut,
    GetUserInfo,
    RefreshToken,

    TraverseFolder,

    UploadFile,
    DownloadFile,

    DeleteItem,
    GetStorageInfo,
    CreateFolder
  };

 public:
  explicit QtOneDrive(const QString& clientID, const QString& secret,
                      const QString& userID, QObject* parent = 0);

  explicit QtOneDrive(const QString& clientID, const QString& secret,
                      const QString& redirectUri, const QString& userID,
                      QObject* parent = 0);

  ~QtOneDrive();

 public:
  void signIn();
  bool isSingIn() const { return isSignIn_; }

  void signOut();
  void getUserInfo();
  void refreshToken();
  void traverseFolder(const QString& rootFolderID = "");

  void getStorageInfo();

  void uploadFile(const QString& localFilePath, const QString& remoteFileName,
                  const QString& folderID = "");
  void uploadFile(QFile* file, const QString& remoteFileName,
                  const QString& folderID = "");
  void downloadFile(const QString& localFilePath, const QString& fileID);

  void deleteItem(const QString& fileOrFolderID);
  void createFolder(const QString& folderName,
                    const QString& parentFolderId = "");

  void getTokenRequest();
  void closeAuthorizationForm();

  QString debugInfo() const;
  bool isBusy() const { return state_ != Empty; }

 signals:
  void errorSignIn(const QString error);
  void errorSignOut(const QString error);
  void errorUploadFile(const QString error);
  void errorDownloadFile(const QString error);
  void errorGetUserInfo(const QString error);
  void errorRefreshToken(const QString error);
  void errorTraverseFolder(const QString error);
  void errorDeleteItem(const QString error);
  void errorCreateFolder(const QString error);
  void errorGetStorageInfo(const QString error);
  void error(const QString error);

  void progressUploadFile(const QString localFilePath, int percent);
  void progressDownloadFile(const QString fileID, int percent);

  void successSignIn();
  void successSingOut();

  void successUploadFile(const QString localFilePath, const QString fileId);
  void successDownloadFile(const QString fileID);
  void successDeleteItem(const QString id);
  void successCreateFolder(const QString folderId);

  void successRefreshToken();
  void successGetUserInfo(const QJsonObject json);
  void successTraverseFolder(const QJsonObject json,
                             const QString rootFolderID);
  void successGetStorageInfo(const QJsonObject json);

 private:
  QUrl urlSingIn() const;
  QUrl urlSignOut() const;
  QUrl urlStorageInfo() const;
  QUrl urlGetToken() const;
  QUrl urlGetUserInfo() const;
  QUrl urlDeleteItem(const QString& id) const;
  QUrl urlTraverseFolder(const QString& parentFolderId) const;
  QUrl urlUploadFile(const QString& remoteFileName,
                     const QString& folderId) const;
  QUrl urlDownloadFile(const QString& fileId) const;
  QUrl urlCreateFolder(const QString& id) const;

  QUrlQuery postGetToken() const;
  QUrlQuery postRefreshToken() const;

  void saveProperties();
  void loadProperties();
  void onRefreshTokenFinished();

 private:
  void emitError(const QString& desc);
  bool isNeedRefreshToken() const;
  void refreshTokenRequest();
  void downloadFile(const QUrl& url);

  QJsonObject checkReplyJson(QNetworkReply* reply);

 private:
  QString clientID_;
  QString secret_;
  QString authorizationCode_;
  QString redirectURI_;
  QString accessToken_;
  QString refreshToken_;
  QString authenticationToken_;

  QDateTime tokenTime_;
  QDateTime expiredTokenTime_;

  bool isSignIn_ = false;

  QSettings* properties_;
  OneDriveState state_ = Empty;
  QNetworkAccessManager* networkManager_ = nullptr;

  QString tmp_parentFolderId_;
  QString tmp_fileId_;
  QString tmp_localFileName_;
  QString tmp_remoteName_;

  QFile* tmp_file_ = nullptr;

  static QSet<QString> exisitInstances_;
  QString hash_;
};

#endif  // QTONEDRIVE_H
