#include "CloudBackup.h"

#include <QAuthenticator>
#include <QFile>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QRandomGenerator>
#include <QTimer>
#include <QXmlStreamReader>

#include "Comm/qaesencryption.h"
#include "DelWebDAVFiles.h"
#include "src/MainWindow.h"
#include "src/defines.h"
#include "ui_CloudBackup.h"
#include "ui_MainWindow.h"

QList<QPair<QString, QDateTime>> parseWebDavResponse(const QByteArray& data);

CloudBackup::CloudBackup(QWidget* parent)
    : QDialog(parent), ui(new Ui::CloudBackup) {
  ui->setupUi(this);

  this->installEventFilter(this);

  init();

  m_networkManager = new QNetworkAccessManager(this);  // 只创建一次！
                                                       // 默认并发 2，全平台兼容

  QString secret;
  // 先从环境变量读取，便于CI运行
  secret = qEnvironmentVariable("ONEDRIVE_SECRET");
  if (secret.isEmpty()) {
    QSettings settings(iniDir + "config.ini", QSettings::IniFormat);
    secret = settings.value("OneDrive/Secret").toString();
  }
}

QString CloudBackup::initUserInfo(QString info) {
  QTextEdit* edit = new QTextEdit;
  edit->setPlainText(info);
  int lineCount = edit->document()->lineCount();

  QString str1;
  for (int i = 0; i < lineCount; i++) {
    QString str = edit->document()->findBlockByLineNumber(i).text().trimmed();
    if (str.contains(":")) {
      str = str.replace(",", "");
      str = str.replace("[", "");
      str = str.replace("{", "");
      str = str.replace("\"", "");

      if (str.contains("remaining", Qt::CaseInsensitive) ||
          str.contains("total") || str.contains("used")) {
        QStringList list = str.split(":");
        QString s_size = list.at(1);
        qint64 size = s_size.toLongLong();
        str = list.at(0) + ": " + m_Method->getFileSize(size, 2);
      }

      str1 = str1 + "\n" + str;
    }
  }

  return str1.trimmed();
}

void CloudBackup::init() {
  ui->frame->hide();

  this->setGeometry(mw_one->geometry().x(), mw_one->geometry().y(),
                    mw_one->width(), mw_one->height());
  ui->frameOne->setMaximumHeight(100);

  this->setModal(true);
}

CloudBackup::~CloudBackup() {
  delete ui;
  if (m_manager) {
    m_manager->deleteLater();  // 释放成员变量m_manager
  }
}

bool CloudBackup::eventFilter(QObject* obj, QEvent* evn) {
  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
      on_btnBack_clicked();
      return true;
    }
  }

  return QWidget::eventFilter(obj, evn);
}

void CloudBackup::on_pushButton_downloadFile_clicked() {}

void CloudBackup::uploadData() {
  QString strFlag;
  strFlag = "WebDAV";
  auto m_ShowMsg = std::make_unique<ShowMessage>(this);
  if (!m_ShowMsg->showMsg(
          strFlag,
          tr("Uploading data?") + "\n\n" +
              tr("This action updates the data in the cloud.") + "\n\n" +
              mw_one->m_Reader->getUriRealPath(zipfile) +
              "\n\nSIZE: " + m_Method->getFileSize(QFile(zipfile).size(), 2),
          2))
    return;

  mui->btnWebDAVBackup->setEnabled(false);
  mui->btnWebDAVRestore->setEnabled(false);

  if (mui->chkWebDAV->isChecked()) {
    QString url = getWebDAVArgument();
    createDirectory(url, "Knot/");
    uploadFileToWebDAV(url, zipfile, "Knot/memo.zip");
  }
}

QString CloudBackup::getWebDAVArgument() {
  QString url = mui->cboxWebDAV->currentText().trimmed();
  url = unifyWebDAVBaseUrlToDavEnd(url);
  USERNAME = mui->editWebDAVUsername->text().trimmed();
  APP_PASSWORD = mui->editWebDAVPassword->text().trimmed();
  return url;
}

void CloudBackup::on_btnBack_clicked() {}

void CloudBackup::loadLogQML() {}

void CloudBackup::initQuick() {}

int CloudBackup::getProg() { return 0; }

void CloudBackup::startBakData() {
  isUpData = true;
  mw_one->showProgress();

  mui->progressBar->setValue(0);
  mui->progBar->show();
  mui->progBar->setMaximum(100);
  mui->progBar->setMinimum(0);
  mui->progBar->setValue(0);

  mw_one->myBakDataThread->start();
}

// 上传文件到WebDAV
void CloudBackup::uploadFileToWebDAV(QString webdavUrl, QString localFilePath,
                                     QString remoteFileName) {
  QNetworkAccessManager* manager = new QNetworkAccessManager();
  QUrl url(webdavUrl + remoteFileName);
  QNetworkRequest request(url);

  // 认证头
  QString auth = USERNAME + ":" + APP_PASSWORD;
  request.setRawHeader("Authorization", "Basic " + auth.toUtf8().toBase64());

  // 🔥 必须加这一行：伪装成 Zotero
  request.setRawHeader("User-Agent", "Zotero/5.0");

  // 调试输出
  qDebug() << "上传URL：" << url.toString();
  qDebug() << "认证头：" << request.rawHeader("Authorization");

  QFile* file = new QFile(localFilePath);
  if (!file->open(QIODevice::ReadOnly)) {
    qDebug() << "无法打开本地文件：" << localFilePath;
    delete manager;
    delete file;
    return;
  }

  mui->progBar->show();
  mui->progBar->setValue(0);
  mui->progressBar->setValue(0);

  QNetworkReply* reply = manager->put(request, file);

  connect(reply, &QNetworkReply::uploadProgress, this,
          &CloudBackup::updateUploadProgress);

  QObject::connect(reply, &QNetworkReply::finished, [=]() {
    const int statusCode =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "HTTP状态码：" << statusCode;
    if (reply->error() == QNetworkReply::NoError) {
      qDebug() << "上传成功！";

      auto m_ShowMsg = std::make_unique<ShowMessage>(this);
      m_ShowMsg->showMsg(
          "WebDAV",
          QString(tr("Success Upload File:") + "\n\nPath: %1\n\nID: %2" +
                  "\n\n" + QDateTime::currentDateTime().toString())
              .arg(localFilePath, webdavUrl + remoteFileName),
          1);

      mui->progBar->hide();

    } else {
      qDebug() << "上传失败：" << reply->errorString();
      qDebug() << "服务器响应：" << reply->readAll();

      mui->progBar->hide();

      if (statusCode == 401) {
        auto m_ShowMsg = std::make_unique<ShowMessage>(this);
        m_ShowMsg->showMsg("WebDAV", tr("Authentication failed."), 1);
      } else {
        auto m_ShowMsg = std::make_unique<ShowMessage>(this);
        m_ShowMsg->showMsg(
            "WebDAV", tr("Upload error") + " : " + reply->errorString(), 1);
      }
    }
    file->close();
    reply->deleteLater();
    manager->deleteLater();

    mui->btnWebDAVBackup->setEnabled(true);
    mui->btnWebDAVRestore->setEnabled(true);
  });
}

void CloudBackup::updateUploadProgress(qint64 bytesSent, qint64 bytesTotal) {
  if (bytesTotal > 0) {
    int percent = static_cast<int>((bytesSent * 100) / bytesTotal);
    mui->progBar->setValue(percent);
    mui->progressBar->setValue(percent);
  }
}

void CloudBackup::createDirectory(QString webdavUrl, QString remoteDirPath) {
  QNetworkAccessManager* m_manager = new QNetworkAccessManager();
  QEventLoop loop;
  QUrl url(webdavUrl + remoteDirPath);
  QNetworkRequest request(url);

  // 认证头
  QString auth = USERNAME + ":" + APP_PASSWORD;
  request.setRawHeader("Authorization", "Basic " + auth.toUtf8().toBase64());

  // 🔥 必须加这一行：伪装成 Zotero
  request.setRawHeader("User-Agent", "Zotero/5.0");

  QNetworkReply* reply = m_manager->sendCustomRequest(request, "MKCOL");

  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();  // 阻塞直到请求完成

  int statusCode =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (statusCode == 201 || statusCode == 405) {  // 405表示目录已存在
    qDebug() << "目录已就绪:" << remoteDirPath;
    m_Method->setInfoText(">> " + remoteDirPath);
  } else {
    qDebug() << "目录创建失败，状态码:" << statusCode;
  }

  delete reply;
  m_manager->deleteLater();
}

void CloudBackup::downloadFile(QString remoteFileName, QString localSavePath) {
  m_manager = new QNetworkAccessManager();

  QUrl url(WEBDAV_URL + remoteFileName);
  QNetworkRequest request(url);
  request.setTransferTimeout(30000);

  // 设置认证头
  QString auth = QString("%1:%2").arg(USERNAME).arg(APP_PASSWORD);
  request.setRawHeader("Authorization", "Basic " + auth.toUtf8().toBase64());

  request.setRawHeader("User-Agent", "Zotero/5.0");

  QFile* localFile = new QFile(localSavePath);
  if (!localFile->open(QIODevice::WriteOnly)) {
    qDebug() << "无法创建本地文件：" << localSavePath;
    delete localFile;
    // ShowMessage::showError("WebDAV", tr("Failed to create local file"));
    return;
  }

  QNetworkReply* reply = m_manager->get(request);
  m_activeDownloads.insert(reply, localFile);

  // 进度更新
  QObject::connect(
      reply, &QNetworkReply::downloadProgress,
      [this](qint64 bytesReceived, qint64 bytesTotal) {  // 显式捕获this
        QMetaObject::invokeMethod(this, [bytesReceived, bytesTotal]() {
          int percent =
              (bytesTotal > 0)
                  ? static_cast<int>((bytesReceived * 100) / bytesTotal)
                  : 0;

          QString strReceived = m_Method->getFileSize(bytesReceived, 2);
          // qDebug() << "bytesTotal=" << bytesTotal
          //          << "bytesReceived=" << strReceived;
          mui->lblReceivedBytes->setText(strReceived);
          if (bytesTotal < 0) {
            mui->progressBar->setMinimum(0);
            mui->progressBar->setMaximum(0);
          }
          mui->progressBar->setValue(percent);
        });
      });

  // 数据写入
  QObject::connect(reply, &QNetworkReply::readyRead,
                   [this, reply]() {  // 显式捕获this
                     if (QFile* file = m_activeDownloads.value(reply)) {
                       file->write(reply->readAll());
                     }
                   });

  // 完成处理
  QObject::connect(
      reply, &QNetworkReply::finished,
      [this, reply, localSavePath]() {  // 显式捕获this
        QFile* file = m_activeDownloads.take(reply);
        const int statusCode =
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        if (file) {
          if (reply->error() == QNetworkReply::NoError && statusCode >= 200 &&
              statusCode < 300) {
            file->write(reply->readAll());
            file->close();

            resetProgBar();

            zipfile = localSavePath;
            auto showbox = std::make_unique<ShowMessage>(this);
            showbox->showMsg(
                "WebDAV",
                tr("Successfully downloaded file,File saved to") + " : " +
                    localSavePath + "\n\nSize: " +
                    m_Method->getFileSize(QFile(localSavePath).size(), 2),
                1);

            if (QFile(localSavePath).exists()) {
              if (!localSavePath.isNull()) {
                auto m_ShowMsg = std::make_unique<ShowMessage>(this);
                if (!m_ShowMsg->showMsg(
                        "Kont",
                        tr("Import this data?") + "\n" +
                            mw_one->m_Reader->getUriRealPath(localSavePath),
                        2)) {
                  isZipOK = false;
                  return;
                }
              }
              isZipOK = true;

              mw_one->showProgress();

              isMenuImport = false;

              isDownData = true;
              mw_one->myImportDataThread->start();

              if (isZipOK) mw_one->on_btnBack_One_pressed();
            }
          } else {
            resetProgBar();
            mui->progressBar->setValue(0);
            mui->lblReceivedBytes->setText("0");

            file->remove();
            if (statusCode == 401) {
              auto m_ShowMsg = std::make_unique<ShowMessage>(this);
              m_ShowMsg->showMsg("WebDAV", tr("Authentication failed."), 1);
            } else {
              auto m_ShowMsg = std::make_unique<ShowMessage>(this);
              m_ShowMsg->showMsg(
                  "WebDAV", tr("Download error") + " : " + reply->errorString(),
                  1);
            }
          }
          delete file;
        }

        reply->deleteLater();

      });
}

void CloudBackup::resetProgBar() {
  mui->progressBar->setMinimum(0);
  mui->progressBar->setMaximum(100);
  mui->btnWebDAVRestore->setEnabled(true);
  mui->btnWebDAVBackup->setEnabled(true);
}

// 加密函数（返回Base64编码字符串）
QString CloudBackup::aesEncrypt(QString plainText, QByteArray key,
                                QByteArray iv) {
  QAESEncryption encryption(QAESEncryption::AES_256, QAESEncryption::CBC);

  // 处理密钥和IV长度（AES-256需要32字节，CBC模式需要16字节IV）
  QByteArray adjustedKey =
      key.leftJustified(32, '\0', true);  // 截断或填充到32字节
  QByteArray adjustedIv = iv.leftJustified(16, '\0', true);  // 调整IV到16字节

  // 加密
  QByteArray encrypted =
      encryption.encode(plainText.toUtf8(), adjustedKey, adjustedIv);

  // 转换为Base64方便传输
  return encrypted.toBase64();
}

// 解密函数
QString CloudBackup::aesDecrypt(QString cipherText, QByteArray key,
                                QByteArray iv) {
  QAESEncryption encryption(QAESEncryption::AES_256, QAESEncryption::CBC);

  QByteArray adjustedKey = key.leftJustified(32, '\0', true);
  QByteArray adjustedIv = iv.leftJustified(16, '\0', true);

  // 解码Base64并解密
  QByteArray decoded = QByteArray::fromBase64(cipherText.toUtf8());
  QByteArray decrypted = encryption.decode(decoded, adjustedKey, adjustedIv);

  // 移除PKCS#7填充
  decrypted = encryption.removePadding(decrypted);

  return QString::fromUtf8(decrypted);
}

///////////////////////////////////////////////////////////////////////////////////

void CloudBackup::uploadFilesToWebDAV(const QStringList& files) {
  // 将新文件添加到队列
  for (const QString& file : files) {
    uploadQueue.enqueue(file);
  }

  // 启动上传处理
  startNextUpload();
}

// 采用全局网络管理变量
/*void CloudBackup::startNextUpload() {
  // 核心：严格控制并发数量
  while (activeReplies.size() < maxNetConcurrent && !uploadQueue.isEmpty()) {
    QString filePath = uploadQueue.dequeue();
    QString remoteFile = filePath;
    remoteFile.replace(privateDir, "");

    QUrl fullUrl = QUrl(getWebDAVArgument()).resolved(remoteFile);

    QFile* file = new QFile(filePath);
    if (!file->open(QIODevice::ReadOnly)) {
      qWarning() << "Failed to open file:" << filePath;
      delete file;
      continue;
    }

    QNetworkRequest request(fullUrl);
    QString auth = QString("%1:%2").arg(USERNAME, APP_PASSWORD);
    request.setRawHeader("Authorization",
                         "Basic " + auth.toLocal8Bit().toBase64());

    request.setRawHeader("User-Agent", "Zotero/5.0");

    // ✅ 全局唯一 m_networkManager
    QNetworkReply* reply = m_networkManager->put(request, file);
    file->setParent(reply);
    activeReplies.insert(reply);

    connect(reply, &QNetworkReply::finished, this, [this, reply, filePath]() {
      handleUploadFinished(reply, filePath);
    });
  }
}*/

// 使用局部网络管理变量
void CloudBackup::startNextUpload() {
  while (activeReplies.size() < maxNetConcurrent && !uploadQueue.isEmpty()) {
    QString filePath = uploadQueue.dequeue();
    QString remoteFile = filePath;
    remoteFile.replace(privateDir, "");

    QUrl fullUrl = QUrl(getWebDAVArgument() + remoteFile);

    QFile* file = new QFile(filePath);
    if (!file->open(QIODevice::ReadOnly)) {
      qWarning() << "Failed to open file:" << filePath;
      delete file;
      continue;
    }

    QNetworkRequest request(fullUrl);
    QString auth = QString("%1:%2").arg(USERNAME, APP_PASSWORD);
    request.setRawHeader("Authorization",
                         "Basic " + auth.toLocal8Bit().toBase64());
    request.setRawHeader("User-Agent", "Zotero/5.0");

    // ==============================
    // ✅ 【终极稳定】局部独立 QNAM
    // ==============================
    QNetworkAccessManager* nam = new QNetworkAccessManager();
    QNetworkReply* reply = nam->put(request, file);

    // 绑定上传进度
    connect(reply, &QNetworkReply::uploadProgress, this,
            [this, filePath](qint64 sent, qint64 total) {
              if (total > 0) {
                int percent = static_cast<int>(sent * 100.0 / total);
                qDebug() << "上传进度：" << filePath << percent << "%";
                // 同步更新界面进度条
                mui->progBar->show();
                mui->progBar->setMinimum(0);
                mui->progBar->setMaximum(100);
                mui->progBar->setValue(percent);
              }
            });

    // 信号连接
    connect(reply, &QNetworkReply::finished, this, [=]() {
      handleUploadFinished(reply, filePath);
      reply->deleteLater();
      nam->deleteLater();  // 用完释放
      file->close();
      file->deleteLater();

      mui->progBar->setValue(0);
    });

    activeReplies.insert(reply);
  }
}

void CloudBackup::handleUploadFinished(QNetworkReply* reply,
                                       const QString& filePath) {
  activeReplies.remove(reply);

  if (reply->error() == QNetworkReply::NoError) {
    if (mw_one && m_Notes) {
      m_Notes->notes_sync_files.removeOne(filePath);
      mw_one->saveNeedSyncNotes();
      m_Method->delayDelFile(filePath);
    }
  } else {
    qWarning() << "Upload error:" << filePath << reply->errorString();
    // 失败重试可在这里加
  }

  reply->deleteLater();

  // ✅ 一个任务完成，立即启动下一个 → 严格保持并发数
  startNextUpload();

  // 全部完成
  if (activeReplies.isEmpty() && uploadQueue.isEmpty()) {
    if (mw_one) {
      mw_one->closeProgress();
      mui->progBar->hide();
      emit uploadAllFinished();
    }
  }
}

/// /////////////////////////////////////////////////////////////////////////////////

// 核心函数：列出目录文件（100% 兼容 坚果云 + 中科院 + 所有标准WebDAV）
/*WebDavHelper* listWebDavFiles(const QString& url, const QString& username,
                              const QString& password) {
  WebDavHelper* helper = new WebDavHelper();
  QNetworkAccessManager* manager = new QNetworkAccessManager(helper);

  // ==============================================
  // 精准判断：是否为 中科院数据胶囊
  // ==============================================
  bool isCstCloud = url.contains("data.cstcloud.cn", Qt::CaseInsensitive);
  QString fixedUrl = url;

  // 🔥 中科院专属：强制 URL 以 / 结尾（不影响其他网盘）
  if (isCstCloud && !fixedUrl.endsWith("/")) {
    fixedUrl += "/";
  }

  QNetworkRequest request;
  request.setUrl(QUrl(fixedUrl));

  // ==============================================
  // 🔥 中科院专属：直接手动加 Basic 认证
  // 其他网盘继续使用自动认证，保持原有逻辑不变
  // ==============================================
  if (isCstCloud) {
    QString auth = username + ":" + password;
    request.setRawHeader("Authorization", "Basic " + auth.toUtf8().toBase64());
  } else {
    // 原有逻辑：其他网盘使用 Qt 自动认证（完全不动）
    QObject::connect(
        manager, &QNetworkAccessManager::authenticationRequired,
        [username, password](QNetworkReply* reply, QAuthenticator* auth) {
          Q_UNUSED(reply);
          auth->setUser(username);
          auth->setPassword(password);
        });
  }

  // 标准请求头
  request.setRawHeader("Depth", "1");

  // ==============================================
  // Brief 头：保持原有逻辑
  // 坚果云 = t
  // 中科院 = t（兼容）
  // 其他 = 不加
  // ==============================================
  if (url.contains("jianguoyun.com") || isCstCloud) {
    request.setRawHeader("Brief", "t");
  }

  // ==============================================
  // 🔥 中科院专属：强制 Zotero UA
  // 其他网盘：不设置，使用 Qt 默认（完全不影响原有行为）
  // ==============================================
  if (isCstCloud) {
    request.setRawHeader("User-Agent", "Zotero/5.0");
  }

  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    "text/xml; charset=utf-8");

  // 标准 PROPFIND XML
  QByteArray body;
  if (isGetWebDavModiTime) {
    body = R"(<?xml version="1.0" encoding="utf-8"?>
        <d:propfind xmlns:d="DAV:">
            <d:prop>
                <d:displayname/>
                <d:getlastmodified/>
                <d:resourcetype/>
            </d:prop>
        </d:propfind>)";
  } else {
    body = R"(<?xml version="1.0" encoding="utf-8"?>
      <d:propfind xmlns:d="DAV:">
          <d:prop>
              <d:displayname/>
              <d:resourcetype/>
          </d:prop>
      </d:propfind>)";
  }

  QNetworkReply* reply = manager->sendCustomRequest(request, "PROPFIND", body);

  QObject::connect(reply, &QNetworkReply::finished, [manager, helper, reply]() {
    if (reply->error() != QNetworkReply::NoError) {
      const QString error =
          QString("[HTTP %1] %2")
              .arg(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute)
                       .toInt())
              .arg(reply->errorString());
      emit helper->errorOccurred(error);
    } else {
      helper->setResponseHeaders(reply->rawHeaderPairs());
      QByteArray responseData = reply->readAll();
      QList<QPair<QString, QDateTime>> files =
          parseWebDavResponse(responseData);
      emit helper->listCompleted(files);
    }

    reply->deleteLater();
    manager->deleteLater();
  });

  return helper;
}*/

// 核心函数：列出目录文件（100% 兼容 坚果云 + 中科院 + 所有标准WebDAV）
WebDavHelper* listWebDavFiles(const QString& url, const QString& username,
                              const QString& password) {
  WebDavHelper* helper = new WebDavHelper();
  QNetworkAccessManager* manager = new QNetworkAccessManager(helper);

  // ==============================================
  // 精准判断：是否为 中科院数据胶囊
  // ==============================================
  bool isCstCloud = url.contains("data.cstcloud.cn", Qt::CaseInsensitive);
  QString fixedUrl = url;

  // 🔥 中科院专属：强制 URL 以 / 结尾（不影响其他网盘）
  if (isCstCloud && !fixedUrl.endsWith("/")) {
    fixedUrl += "/";
  }

  QNetworkRequest request;
  request.setUrl(QUrl(fixedUrl));

  // ==============================================
  // 核心认证优化
  // 所有网盘统一：直接预生成并发送 Basic 认证头
  // 每次都是最新账号密码，无缓存，支持换服务器
  // ==============================================
  QString auth = username + ":" + password;
  request.setRawHeader("Authorization", "Basic " + auth.toUtf8().toBase64());

  // 【删除了旧的 authenticationRequired 连接，彻底消除 401 往返】

  // 标准请求头
  request.setRawHeader("Depth", "1");

  // ==============================================
  // Brief 头：保持原有逻辑
  // 坚果云 = t
  // 中科院 = t（兼容）
  // 其他 = 不加
  // ==============================================
  if (url.contains("jianguoyun.com") || isCstCloud) {
    request.setRawHeader("Brief", "t");
  }

  // ==============================================
  // 🔥 中科院专属：强制 Zotero UA
  // 其他网盘：不设置，使用 Qt 默认（完全不影响原有行为）
  // ==============================================
  if (isCstCloud) {
    request.setRawHeader("User-Agent", "Zotero/5.0");
  }

  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    "text/xml; charset=utf-8");

  // 标准 PROPFIND XML
  QByteArray body;
  if (isGetWebDavModiTime) {
    body = R"(<?xml version="1.0" encoding="utf-8"?>
        <d:propfind xmlns:d="DAV:">
            <d:prop>
                <d:displayname/>
                <d:getlastmodified/>
                <d:resourcetype/>
            </d:prop>
        </d:propfind>)";
  } else {
    body = R"(<?xml version="1.0" encoding="utf-8"?>
      <d:propfind xmlns:d="DAV:">
          <d:prop>
              <d:displayname/>
              <d:resourcetype/>
          </d:prop>
      </d:propfind>)";
  }

  QNetworkReply* reply = manager->sendCustomRequest(request, "PROPFIND", body);

  QObject::connect(reply, &QNetworkReply::finished, [manager, helper, reply]() {
    if (reply->error() != QNetworkReply::NoError) {
      const QString error =
          QString("[HTTP %1] %2")
              .arg(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute)
                       .toInt())
              .arg(reply->errorString());
      emit helper->errorOccurred(error);
    } else {
      helper->setResponseHeaders(reply->rawHeaderPairs());
      QByteArray responseData = reply->readAll();
      QList<QPair<QString, QDateTime>> files =
          parseWebDavResponse(responseData);
      emit helper->listCompleted(files);
    }

    reply->deleteLater();
    manager->deleteLater();
  });

  return helper;
}

// 解析函数
QList<QPair<QString, QDateTime>> parseWebDavResponse(const QByteArray& data) {
  QList<QPair<QString, QDateTime>> files;
  QXmlStreamReader xml(data);
  QString currentHref;
  QDateTime currentModified;
  bool isDirectory = false;

  while (!xml.atEnd()) {
    xml.readNext();

    // 开始处理每个资源项
    if (xml.isStartElement() && xml.name() == QLatin1String("response")) {
      currentHref.clear();
      currentModified = QDateTime();
      isDirectory = false;
    }

    // 提取关键属性
    if (xml.isStartElement()) {
      if (xml.name() == QLatin1String("href")) {
        currentHref = xml.readElementText();
        // 规范化路径（去除URL编码）
        currentHref = QString::fromUtf8(
            QByteArray::fromPercentEncoding(currentHref.toLatin1()));

      } else if (isGetWebDavModiTime &&
                 xml.name() == QLatin1String("getlastmodified")) {
        QString rawTime = xml.readElementText();

        // Qt5
        // currentModified = QDateTime::fromString(rawTime, Qt::RFC2822Date);

        // Qt6
        QString timeWithoutTZ =
            rawTime.left(rawTime.lastIndexOf(' '));  // 移除 "GMT"
        QLocale locale(QLocale::C);
        currentModified =
            locale.toDateTime(timeWithoutTZ, "ddd, dd MMM yyyy hh:mm:ss");
        currentModified.setTimeZone(QTimeZone::utc());  // 手动设置为 UTC

        if (!currentModified.isValid()) {
          qWarning() << "时间解析失败:" << rawTime;
        }
      } else if (xml.name() == QLatin1String("resourcetype")) {
        // 检测是否为目录
        while (xml.readNextStartElement()) {
          if (xml.name() == QLatin1String("collection")) {
            isDirectory = true;
          }
          xml.skipCurrentElement();  // 直接跳过剩余内容
        }
      }
    }

    // 结束处理资源项
    if (xml.isEndElement() && xml.name() == QLatin1String("response")) {
      // 排除目录项
      if (!isDirectory && !currentHref.isEmpty()) {
        files.append({currentHref, currentModified});
      }
    }
  }

  return files;
}

///////////////////////////////////////////////////////////////////////////////////

WebDavDownloader::WebDavDownloader(const QString& username,
                                   const QString& password, QObject* parent)
    : QObject(parent), m_username(username), m_password(password) {
  // 连接认证信号
  connect(&manager, &QNetworkAccessManager::authenticationRequired, this,
          &WebDavDownloader::handleAuthentication);
}

void WebDavDownloader::downloadFiles(const QList<QString>& remotePaths,
                                     const QString& localBaseDir,
                                     int maxConcurrent) {
  // 清空队列
  downloadQueue.clear();
  activeDownloads.clear();

  // 初始化队列
  QString baseUrl = m_CloudBackup->getWebDAVArgument();
  QString dataDir = m_CloudBackup->getWebDAVDataDir(baseUrl);

  for (const QString& remotePath : remotePaths) {
    QString localPath = QDir(localBaseDir).absoluteFilePath(remotePath);

    if (!dataDir.isEmpty()) {
      qDebug() << "WebDAV 数据目录是:" << dataDir;
      localPath = localPath.replace("/" + dataDir + "/", "/");
    }

    downloadQueue.enqueue({remotePath, localPath});
  }

  totalFiles = downloadQueue.size();
  completedFiles = 0;
  this->maxConcurrent = qMax(1, maxConcurrent);

  // 启动初始下载，每个任务之间加入延迟（毫秒）
  const int INITIAL_DELAY = 50;  // 时间间隔
  for (int i = 0; i < qMin(this->maxConcurrent, downloadQueue.size()); ++i) {
    // 第i个任务延迟i*INITIAL_DELAY毫秒启动，避免同时发起
    QTimer::singleShot(i * INITIAL_DELAY, this,
                       &WebDavDownloader::startNextDownload);
  }
}

void WebDavDownloader::handleAuthentication(QNetworkReply* reply,
                                            QAuthenticator* auth) {
  Q_UNUSED(reply);
  auth->setUser(m_username);
  auth->setPassword(m_password);
}

void WebDavDownloader::startNextDownload() {
  if (downloadQueue.isEmpty()) return;

  auto [remotePath, localPath] = downloadQueue.dequeue();

  // 创建本地目录
  QFileInfo fileInfo(localPath);
  if (!QDir().mkpath(fileInfo.absolutePath())) {
    qWarning() << "无法创建目录:" << fileInfo.absolutePath();
    emit downloadFinished(false, "目录创建失败");
    return;
  }

  // 构造请求URL
  QString strUrl = m_CloudBackup->getWebDAVArgument();
  QUrl url(strUrl + remotePath);
  if (!url.isValid()) {
    qWarning() << "无效的URL:" << url.toString();
    return;
  }

  QNetworkRequest request(url);

  request.setRawHeader("User-Agent", "Zotero/5.0");

  QNetworkReply* reply = manager.get(request);

  // 记录活动下载
  activeDownloads[reply] = localPath;

  // 连接信号
  connect(reply, &QNetworkReply::finished, this,
          [this, reply]() { onDownloadFinished(reply); });
  connect(reply, &QNetworkReply::downloadProgress, this,
          [this, reply](qint64 bytesReceived, qint64 bytesTotal) {
            // 下面两个变量用来计算下载进度（目前未实现）

            qDebug() << "bytesReceived=" << bytesReceived << "->"
                     << "bytesTotal=" << bytesTotal;

            if (m_Method->infoWindow && m_Method->infoWindow->isVisible()) {
              QString rece = m_Method->getFileSize(bytesReceived, 2);
              QString showText = "[" + QString::number(completedFiles) + "/" +
                                 QString::number(totalFiles) + "] [" + rece +
                                 "] " + activeDownloads[reply];
              m_Method->emit sigUpdateProgressAndText(showText, totalFiles,
                                                      completedFiles);
            }

            emit progressChanged(completedFiles, totalFiles,
                                 activeDownloads[reply]);
          });

  qDebug() << "开始下载:" << url.toString() << "->" << localPath;
}

void WebDavDownloader::onDownloadFinished(QNetworkReply* reply) {
  QString localPath = activeDownloads.value(reply);
  bool success = false;
  QString error;

  if (reply->error() == QNetworkReply::NoError) {
    QFile file(localPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
      file.write(reply->readAll());
      file.close();
      success = true;
      emit fileSaved(localPath);
      qDebug() << "文件保存成功:" << localPath;
    } else {
      error = QString("文件写入失败: %1").arg(file.errorString());
    }
  } else {
    error = QString("网络错误: %1").arg(reply->errorString());
  }

  // 清理
  activeDownloads.remove(reply);
  reply->deleteLater();
  completedFiles++;

  // 进度更新
  emit progressChanged(completedFiles, totalFiles, localPath);

  // 启动下一个下载（加入延迟）
  if (!downloadQueue.isEmpty()) {
    // 设置延迟时间（毫秒），可根据需要调整
    const int DOWNLOAD_DELAY = 50;  // 延迟
    QTimer::singleShot(DOWNLOAD_DELAY, this,
                       &WebDavDownloader::startNextDownload);
  }

  // 全部完成
  if (completedFiles >= totalFiles) {
    emit downloadFinished(success, error);
    qDebug() << "所有下载任务完成";
  }
}

///////////////////////////////////////////////////////////////////////

void CloudBackup::getRemoteFileList(QString url) {
  webdavFileList.clear();
  webdavDateTimeList.clear();
  isGetRemoteFileListEnd = false;
  nextPageUrl = url;
  allFiles.clear();

  qDebug() << "获取文件列表：" << url;
  m_Method->setInfoText(">>" + url);

  m_currentRemoteDir = url;

  // 开始获取第一页
  fetchNextPage();
}

void CloudBackup::fetchNextPage() {
  if (nextPageUrl.isEmpty()) {
    // 所有页面处理完成
    processFileList();
    return;
  }

  WebDavHelper* helper = listWebDavFiles(nextPageUrl, USERNAME, APP_PASSWORD);
  helper->setParent(this);

  QObject::connect(
      helper, &WebDavHelper::listCompleted, this,
      [this, helper](const QList<QPair<QString, QDateTime>>& files) {
        allFiles.append(files);

        // 解析下一页URL
        nextPageUrl = parseNextPageUrl(helper->getResponseHeaders());

        // 删除当前 helper
        helper->deleteLater();

        if (!nextPageUrl.isEmpty()) {
          // 继续获取下一页
          // QTimer::singleShot(100, this, &CloudBackup::fetchNextPage);
          fetchNextPage();
        } else {
          // 所有页面处理完成
          processFileList();
        }
      });

  QObject::connect(helper, &WebDavHelper::errorOccurred, this,
                   [this, helper](const QString& error) {
                     qDebug() << "获取文件列表出错:" << error;
                     helper->deleteLater();
                     processFileList();  // 即使出错也处理已获取的文件
                   });
}

QString CloudBackup::parseNextPageUrl(
    const QList<QNetworkReply::RawHeaderPair>& headers) {
  // 查找 Link 头
  for (const auto& header : headers) {
    if (header.first.compare("Link", Qt::CaseInsensitive) == 0) {
      QByteArray linkValue = header.second;

      // 坚果云分页格式: <URL>; rel="next"
      QRegularExpression nextPageRegex(
          "<([^>]*)>\\s*;\\s*rel\\s*=\\s*\"?next\"?");
      QRegularExpressionMatch match = nextPageRegex.match(linkValue);
      if (match.hasMatch()) {
        return match.captured(1);
      }
    }
  }
  return QString();  // 没有找到下一页
}

void CloudBackup::processFileList() {
  qDebug() << "共找到" << allFiles.size() << "个文件:";

  if (m_currentRemoteDir.contains("KnotData/memo") &&
      !m_currentRemoteDir.contains("images")) {
    m_currentRemoteNotesCount = allFiles.size();
  }

  for (int i = 0; i < allFiles.size(); ++i) {
    const auto& filePair = allFiles.at(i);  // .at() 绝对安全，不触发 detach
    const QString& path = filePair.first;
    const QDateTime& mtime = filePair.second;

    QString remoteFile = path;
    remoteFile = remoteFile.replace("/dav/", "");

    webdavFileList.append(remoteFile);
    webdavDateTimeList.append(mtime);
  }

  isGetRemoteFileListEnd = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void CloudBackup::createRemoteWebDAVDir() {
  QString url = getWebDAVArgument();
  createDirectory(url, "KnotData/");
  createDirectory(url, "KnotData/memo/");
  createDirectory(url, "KnotData/memo/images/");
}

void CloudBackup::deleteWebDAVFiles(QStringList filesToDelete) {
  if (filesToDelete.count() > 0) {
    QString url = getWebDAVArgument();
    CloudDeleter deleter(USERNAME, APP_PASSWORD);
    deleter.baseUrl = url;
    deleter.deleteFiles(filesToDelete);
  }
}

void CloudBackup::backExit() {
  mw_one->clearWidgetFocus();

  save_WebDav();
  mw_one->m_Preferences->saveOptions();
  mw_one->m_Preferences->setEncSyncStatusTip();

  mui->frameMain->show();
  mui->frameOne->hide();
}

void CloudBackup::save_WebDav() {
  QString strWebDAV = mui->cboxWebDAV->currentText().trimmed();
  QString strUserName = mui->editWebDAVUsername->text().trimmed();
  QString password = mui->editWebDAVPassword->text().trimmed();
  QString aesStr = aesEncrypt(password, aes_key, aes_iv);

  // ==========================
  // 1. 保存当前默认账号
  // ==========================
  iniPreferences->setValue("/webdav/url", strWebDAV);
  iniPreferences->setValue("/webdav/username", strUserName);
  iniPreferences->setValue("/webdav/password", aesStr);

  // ==========================
  // 2. 按网址独立保存账号密码
  // ==========================
  iniPreferences->setValue("/webdav/username_" + strWebDAV, strUserName);
  iniPreferences->setValue("/webdav/password_" + strWebDAV, aesStr);

  // ==========================
  // 3. 先把当前地址添加到ComboBox（自动去重）
  // ==========================
  int idx = mui->cboxWebDAV->findText(strWebDAV);
  if (idx == -1) {
    mui->cboxWebDAV->addItem(strWebDAV);
  }

  // ==========================
  // 4. 去重并保存最终列表
  // ==========================
  QStringList items;
  for (int i = 0; i < mui->cboxWebDAV->count(); ++i) {
    QString text = mui->cboxWebDAV->itemText(i).trimmed();
    if (!text.isEmpty()) {
      items.append(text);
    }
  }
  items.removeDuplicates();  // 强制去重

  // 清空旧配置，重新保存
  iniPreferences->remove("/webdav/count");
  iniPreferences->remove("/webdav/text");
  iniPreferences->setValue("/webdav/count", items.size());
  for (int i = 0; i < items.size(); ++i) {
    iniPreferences->setValue("/webdav/text" + QString::number(i), items[i]);
  }

  // ==========================
  // 5. 保存选项
  // ==========================
  iniPreferences->setValue("/cloudbak/webdav", mui->chkWebDAV->isChecked());
  iniPreferences->setValue("/cloudbak/autosync", mui->chkAutoSync->isChecked());

  // 立即写入磁盘
  iniPreferences->sync();
}

void CloudBackup::init_CloudBacup() {
  int count = iniPreferences->value("/webdav/count", 0).toInt();
  mui->cboxWebDAV->clear();
  QStringList txtList;
  txtList.append("https://dav.jianguoyun.com/dav/");
  txtList.append("https://soya.infini-cloud.net/dav/");
  txtList.append("https://app.koofr.net/dav/Koofr/");
  for (int i = 0; i < count; i++) {
    QString text =
        iniPreferences->value("/webdav/text" + QString::number(i), "")
            .toString();
    txtList.append(text);
  }
  txtList.removeDuplicates();
  mui->cboxWebDAV->addItems(txtList);

  mui->cboxWebDAV->setCurrentText(
      iniPreferences->value("/webdav/url", "https://dav.jianguoyun.com/dav/")
          .toString());

  mui->editWebDAVUsername->setText(
      iniPreferences->value("/webdav/username").toString());

  QString aesStr = iniPreferences->value("/webdav/password").toString();
  QString password = aesDecrypt(aesStr, aes_key, aes_iv);
  mui->editWebDAVPassword->setText(password);

  mui->chkWebDAV->setChecked(
      iniPreferences->value("/cloudbak/webdav", 1).toBool());
  mui->chkAutoSync->setChecked(
      iniPreferences->value("/cloudbak/autosync", 0).toBool());
}

void CloudBackup::changeComBoxWebDAV(const QString& arg1) {
  QString username, password;
  username = iniPreferences->value("/webdav/username_" + arg1).toString();
  QString aesStr = iniPreferences->value("/webdav/password_" + arg1).toString();
  password = aesDecrypt(aesStr, aes_key, aes_iv);

  mui->editWebDAVUsername->setText(username);
  mui->editWebDAVPassword->setText(password);
}

void CloudBackup::webDAVRestoreData() {
  QString filePath;
  filePath = bakfileDir + "memo.zip";

  if (QFile(filePath).exists()) QFile(filePath).remove();
  if (filePath.isEmpty()) return;

  auto m_ShowMsg = std::make_unique<ShowMessage>(this);
  if (!m_ShowMsg->showMsg(
          "WebDAV",
          tr("Downloading data?") + "\n\n" +
              tr("This action overwrites local files with files in the cloud."),
          2))
    return;
  WEBDAV_URL = m_CloudBackup->getWebDAVArgument();
  USERNAME = mui->editWebDAVUsername->text().trimmed();
  APP_PASSWORD = mui->editWebDAVPassword->text().trimmed();
  downloadFile("Knot/memo.zip", filePath);

  mui->progressBar->setValue(0);
  mui->btnWebDAVRestore->setEnabled(false);
  mui->btnWebDAVBackup->setEnabled(false);
}

bool CloudBackup::checkWebDAVConnection() {
  QString urlText = m_CloudBackup->getWebDAVArgument();
  QUrl url(urlText);

  if (!url.isValid() || url.scheme() != "https") {
    qDebug() << "URL无效，必须使用https协议";
    return false;
  }

  QString username = mui->editWebDAVUsername->text().trimmed();
  QString password = mui->editWebDAVPassword->text().trimmed();

  QNetworkAccessManager manager;
  QNetworkRequest request(url);

  // 🔥 关键：中科院必须直接带认证，不等待401
  QByteArray auth = username.toUtf8() + ":" + password.toUtf8();
  request.setRawHeader("Authorization", "Basic " + auth.toBase64());

  request.setRawHeader("User-Agent", "Zotero/5.0");
  request.setRawHeader("Depth", "0");

  // 🔥 关键：极简 PROPFIND，中科院只认这个
  QString xml =
      "<?xml version=\"1.0\"?>"
      "<d:propfind xmlns:d=\"DAV:\">"
      "<d:allprop/>"
      "</d:propfind>";

  QNetworkReply* reply =
      manager.sendCustomRequest(request, "PROPFIND", xml.toUtf8());

  QEventLoop loop;
  connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();

  int status =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  bool ok = (reply->error() == QNetworkReply::NoError &&
             (status == 207 || status == 200));

  QString strError =
      ok ? "连接成功" : QString("连接失败，状态码：%1").arg(status);
  qDebug() << strError;
  reply->deleteLater();

  if (ok) save_WebDav();

  return ok;
}

QString CloudBackup::unifyWebDAVBaseUrlToDavEnd(QString url) {
  // 找到最后一个 /dav/
  int davIndex = url.lastIndexOf("/dav/");

  // 统一截取到 /dav/ 为止（关键：+5 才是完整的 /dav/）
  if (davIndex != -1) {
    url = url.left(davIndex + 5);
  }

  // 无论如何，最后必须以 / 结尾（WebDAV 强制要求）
  if (!url.endsWith("/")) {
    url += "/";
  }

  return url;
}

QString CloudBackup::getWebDAVDataDir(const QString& url) {
  // 找到最后一个 "/dav/" 的位置
  int davIndex = url.lastIndexOf("/dav/");
  if (davIndex == -1) {
    return QString();  // 没有 /dav/，返回空
  }

  // 提取 "/dav/" 之后的全部内容
  QString afterDav = url.mid(davIndex + 5);  // "/dav/" 长度为 5

  // 去掉前后的空白和斜杠
  afterDav = afterDav.trimmed();
  while (!afterDav.isEmpty() && afterDav.front() == '/') {
    afterDav.remove(0, 1);
  }
  while (!afterDav.isEmpty() && afterDav.back() == '/') {
    afterDav.chop(1);
  }

  return afterDav;
}
