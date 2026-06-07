#include <QAuthenticator>
#include <QEventLoop>
#include <QList>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QUrl>
#include <functional>

#include "src/MainWindow.h"
#include "src/defines.h"

class CloudDeleter : public QObject {
  Q_OBJECT
 public:
  explicit CloudDeleter(const QString& user, const QString& appPassword,
                        QObject* parent = nullptr)
      : QObject(parent), username(user), password(appPassword) {}

  QString baseUrl = "https://dav.jianguoyun.com/dav/";

  // maxConcurrent=1 表示串行删除
  void deleteFiles(const QList<QString>& filePaths, int maxConcurrent = 1) {
    maxConcurrent = maxNetConcurrent;
    QEventLoop loop;
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QMutex mutex;
    int active = 0;
    int index = 0;
    int total = filePaths.count();

    std::function<void()> startNext;
    startNext = [&]() mutable {
      QMutexLocker locker(&mutex);
      while (active < maxConcurrent && index < total) {
        int i = index++;
        const QString& path = filePaths.at(i);
        QUrl url(baseUrl + path);
        QNetworkRequest request(url);

        request.setRawHeader("User-Agent", "Zotero/5.0");

        QNetworkReply* reply = manager->deleteResource(request);
        connect(reply, &QNetworkReply::sslErrors,
                [](const QList<QSslError>& errors) {
                  qWarning() << "SSL Errors:" << errors;
                });

        // 30秒超时
        QTimer::singleShot(30000, reply, &QNetworkReply::abort);

        connect(reply, &QNetworkReply::finished, reply, [&, reply]() mutable {
          if (reply->error() == QNetworkReply::NoError) {
            QString file = reply->request().url().path();
            qInfo() << "Deleted:" << file;

            QMetaObject::invokeMethod(
                qApp,
                [=]() {
                  // 这里面所有代码，都会在主线程安全执行
                  mw_one->saveNeedDelWebDAVFiles(file);
                },
                Qt::DirectConnection);

          } else {
            qWarning() << "Failed to delete:" << reply->request().url().path()
                       << ":" << reply->errorString();
          }
          reply->deleteLater();

          {
            QMutexLocker locker(&mutex);
            active--;
          }

          startNext();

          QMutexLocker locker(&mutex);
          if (index >= total && active == 0) {
            QMetaObject::invokeMethod(
                qApp,
                [=]() {
                  // 这里面所有代码，都会在主线程安全执行
                  isDelWebDAVFilesEnd = true;
                },
                Qt::DirectConnection);

            loop.quit();
          }
        });

        active++;
      }
    };

    connect(manager, &QNetworkAccessManager::authenticationRequired,
            [this](QNetworkReply*, QAuthenticator* auth) {
              auth->setUser(this->username);
              auth->setPassword(this->password);
            });

    startNext();
    loop.exec();

    manager->deleteLater();
  }

 private:
  QString username;
  QString password;
};