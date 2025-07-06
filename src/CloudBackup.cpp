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
#include "src/MainWindow.h"
#include "ui_CloudBackup.h"
#include "ui_MainWindow.h"

extern MainWindow *mw_one;
extern Method *m_Method;
extern QString iniFile, iniDir, zipfile, privateDir, bakfileDir;

extern bool isZipOK, isMenuImport, isDownData, isAndroid, isUpData;

WebDavHelper *listWebDavFiles(const QString &url, const QString &username,
                              const QString &password);
QList<QPair<QString, QDateTime>> parseWebDavResponse(const QByteArray &data);

CloudBackup::CloudBackup(QWidget *parent)
    : QDialog(parent), ui(new Ui::CloudBackup) {
  ui->setupUi(this);

  this->installEventFilter(this);

  init();
  // initQuick();

  QString secret;
  // 先从环境变量读取，便于CI运行
  secret = qEnvironmentVariable("ONEDRIVE_SECRET");
  if (secret.isEmpty()) {
    QSettings settings(iniDir + "config.ini", QSettings::IniFormat);
    secret = settings.value("OneDrive/Secret").toString();
  }
}

QString CloudBackup::initUserInfo(QString info) {
  QTextEdit *edit = new QTextEdit;
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
  ui->lineEdit_fileID->setFocus();
  this->setGeometry(mw_one->geometry().x(), mw_one->geometry().y(),
                    mw_one->width(), mw_one->height());
  ui->frameOne->setMaximumHeight(100);

  this->setModal(true);
}

CloudBackup::~CloudBackup() { delete ui; }

bool CloudBackup::eventFilter(QObject *obj, QEvent *evn) {
  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
      on_btnBack_clicked();
      return true;
    }
  }

  return QWidget::eventFilter(obj, evn);
}

void CloudBackup::on_pushButton_SignIn_clicked() {}

void CloudBackup::on_pushButton_SingOut_clicked() {}

void CloudBackup::on_pushButton_GetUserInfo_clicked() {}

void CloudBackup::on_pushButton_clicked() {}

void CloudBackup::on_lineEdit_fileID_textChanged(const QString &arg1) {
  ui->pushButton_deleteFile->setEnabled(!arg1.isEmpty());
  // ui->pushButton_downloadFile->setEnabled(  !arg1.isEmpty() );
  ui->pushButton_traserveFolder->setEnabled(!arg1.isEmpty());
  // ui->pushButton_upload2->setEnabled(  !arg1.isEmpty() );
  ui->pushButton_createFolder->setEnabled(!arg1.isEmpty());
}

void CloudBackup::on_pushButton_getFiles_clicked() {}

void CloudBackup::on_pushButton_traserveFolder_clicked() {}

void CloudBackup::on_pushButton_getFolders_clicked() {}

void CloudBackup::on_pushButton_downloadFile_clicked() {}

void CloudBackup::on_pushButton_createFolder_clicked() {}

void CloudBackup::on_pushButton_deleteFile_clicked() {}

void CloudBackup::uploadData() {
  QString strFlag;
  if (mw_one->ui->chkOneDrive->isChecked())
    strFlag = "OneDrive";
  else
    strFlag = "WebDAV";
  ShowMessage *m_ShowMsg = new ShowMessage(this);
  if (!m_ShowMsg->showMsg(
          strFlag,
          tr("Uploading data?") + "\n\n" +
              tr("This action updates the data in the cloud.") + "\n\n" +
              mw_one->m_Reader->getUriRealPath(zipfile) +
              "\n\nSIZE: " + m_Method->getFileSize(QFile(zipfile).size(), 2),
          2))
    return;

  if (mw_one->ui->chkWebDAV->isChecked()) {
    QString url = getWebDAVArgument();
    createDirectory(url, "Knot/");
    uploadFileToWebDAV(url, zipfile, "Knot/memo.zip");
  }
}

QString CloudBackup::getWebDAVArgument() {
  QString url = mw_one->ui->editWebDAV->text().trimmed();
  USERNAME = mw_one->ui->editWebDAVUsername->text().trimmed();
  APP_PASSWORD = mw_one->ui->editWebDAVPassword->text().trimmed();
  return url;
}

void CloudBackup::on_pushButton_storageInfo_clicked() {}

void CloudBackup::on_btnBack_clicked() {}

void CloudBackup::loadLogQML() {}

void CloudBackup::loadText(QString str) {}

void CloudBackup::initQuick() {}

int CloudBackup::getProg() { return 0; }

void CloudBackup::startBakData() {
  isUpData = true;
  mw_one->showProgress();

  mw_one->ui->progressBar->setValue(0);
  mw_one->ui->progBar->show();
  mw_one->ui->progBar->setMaximum(100);
  mw_one->ui->progBar->setMinimum(0);
  mw_one->ui->progBar->setValue(0);

  mw_one->myBakDataThread->start();
}

// 上传文件到WebDAV,默认坚果云
void CloudBackup::uploadFileToWebDAV(QString webdavUrl, QString localFilePath,
                                     QString remoteFileName) {
  QNetworkAccessManager *manager = new QNetworkAccessManager();
  QUrl url(webdavUrl + remoteFileName);
  QNetworkRequest request(url);

  // 认证头
  QString auth = USERNAME + ":" + APP_PASSWORD;
  request.setRawHeader("Authorization", "Basic " + auth.toUtf8().toBase64());

  // 调试输出
  qDebug() << "上传URL：" << url.toString();
  qDebug() << "认证头：" << request.rawHeader("Authorization");

  QFile *file = new QFile(localFilePath);
  if (!file->open(QIODevice::ReadOnly)) {
    qDebug() << "无法打开本地文件：" << localFilePath;
    delete manager;
    delete file;
    return;
  }

  mw_one->ui->progBar->show();
  mw_one->ui->progBar->setValue(0);
  mw_one->ui->progressBar->setValue(0);

  QNetworkReply *reply = manager->put(request, file);

  connect(reply, &QNetworkReply::uploadProgress, this,
          &CloudBackup::updateUploadProgress);

  QObject::connect(reply, &QNetworkReply::finished, [=]() {
    const int statusCode =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "HTTP状态码：" << statusCode;
    if (reply->error() == QNetworkReply::NoError) {
      qDebug() << "上传成功！";

      ShowMessage *m_ShowMsg = new ShowMessage(this);
      m_ShowMsg->showMsg(
          "WebDAV",
          QString(tr("Success Upload File:") + "\n\nPath: %1\n\nID: %2" +
                  "\n\n" + QDateTime::currentDateTime().toString())
              .arg(localFilePath, webdavUrl + remoteFileName),
          1);

      mw_one->ui->progBar->hide();

    } else {
      qDebug() << "上传失败：" << reply->errorString();
      qDebug() << "服务器响应：" << reply->readAll();

      mw_one->ui->progBar->hide();

      if (statusCode == 401) {
        ShowMessage *m_ShowMsg = new ShowMessage(mw_one);
        m_ShowMsg->showMsg("WebDAV", tr("Authentication failed."), 1);
      } else {
        ShowMessage *m_ShowMsg = new ShowMessage(mw_one);
        m_ShowMsg->showMsg(
            "WebDAV", tr("Upload error") + " : " + reply->errorString(), 1);
      }
    }
    file->close();
    reply->deleteLater();
    manager->deleteLater();
  });
}

void CloudBackup::updateUploadProgress(qint64 bytesSent, qint64 bytesTotal) {
  if (bytesTotal > 0) {
    int percent = static_cast<int>((bytesSent * 100) / bytesTotal);
    mw_one->ui->progBar->setValue(percent);
    mw_one->ui->progressBar->setValue(percent);
  }
}

void CloudBackup::createDirectory(QString webdavUrl, QString remoteDirPath) {
  QNetworkAccessManager *m_manager = new QNetworkAccessManager();
  QEventLoop loop;
  QUrl url(webdavUrl + remoteDirPath);
  QNetworkRequest request(url);

  // 认证头
  QString auth = USERNAME + ":" + APP_PASSWORD;
  request.setRawHeader("Authorization", "Basic " + auth.toUtf8().toBase64());

  QNetworkReply *reply = m_manager->sendCustomRequest(request, "MKCOL");

  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();  // 阻塞直到请求完成

  int statusCode =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (statusCode == 201 || statusCode == 405) {  // 405表示目录已存在
    qDebug() << "目录已就绪:" << remoteDirPath;
  } else {
    qDebug() << "目录创建失败，状态码:" << statusCode;
  }

  delete reply;
}

void CloudBackup::downloadFile(QString remoteFileName, QString localSavePath) {
  m_manager = new QNetworkAccessManager();

  QUrl url(WEBDAV_URL + remoteFileName);
  QNetworkRequest request(url);
  request.setTransferTimeout(30000);

  // 设置认证头
  QString auth = QString("%1:%2").arg(USERNAME).arg(APP_PASSWORD);
  request.setRawHeader("Authorization", "Basic " + auth.toUtf8().toBase64());

  QFile *localFile = new QFile(localSavePath);
  if (!localFile->open(QIODevice::WriteOnly)) {
    qDebug() << "无法创建本地文件：" << localSavePath;
    delete localFile;
    // ShowMessage::showError("WebDAV", tr("Failed to create local file"));
    return;
  }

  QNetworkReply *reply = m_manager->get(request);
  m_activeDownloads.insert(reply, localFile);

  // 进度更新
  QObject::connect(
      reply, &QNetworkReply::downloadProgress,
      [this](qint64 bytesReceived, qint64 bytesTotal) {  // 显式捕获this
        QMetaObject::invokeMethod(this, [this, bytesReceived, bytesTotal]() {
          int percent =
              (bytesTotal > 0)
                  ? static_cast<int>((bytesReceived * 100) / bytesTotal)
                  : 0;
          mw_one->ui->progressBar->setValue(percent);
        });
      });

  // 数据写入
  QObject::connect(reply, &QNetworkReply::readyRead,
                   [this, reply]() {  // 显式捕获this
                     if (QFile *file = m_activeDownloads.value(reply)) {
                       file->write(reply->readAll());
                     }
                   });

  // 完成处理
  QObject::connect(
      reply, &QNetworkReply::finished,
      [this, reply, localSavePath]() {  // 显式捕获this
        QFile *file = m_activeDownloads.take(reply);
        const int statusCode =
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        if (file) {
          if (reply->error() == QNetworkReply::NoError && statusCode >= 200 &&
              statusCode < 300) {
            file->write(reply->readAll());
            file->close();

            zipfile = localSavePath;
            ShowMessage *showbox = new ShowMessage(this);
            showbox->showMsg(
                "WebDAV",
                tr("Successfully downloaded file,File saved to") + " : " +
                    localSavePath + "\n\nSize: " +
                    m_Method->getFileSize(QFile(localSavePath).size(), 2),
                1);

            if (QFile(localSavePath).exists()) {
              if (!localSavePath.isNull()) {
                ShowMessage *m_ShowMsg = new ShowMessage(mw_one);
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

              if (isZipOK) mw_one->on_btnBack_One_clicked();
            }
          } else {
            file->remove();
            if (statusCode == 401) {
              ShowMessage *m_ShowMsg = new ShowMessage(mw_one);
              m_ShowMsg->showMsg("WebDAV", tr("Authentication failed."), 1);
            } else {
              ShowMessage *m_ShowMsg = new ShowMessage(mw_one);
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

void CloudBackup::uploadFilesToWebDAV(QStringList files) {
  QNetworkAccessManager *manager = new QNetworkAccessManager();
  QString url = getWebDAVArgument();

  foreach (QString m_file, files) {
    QString localFile = m_file;
    QString remoteFile = m_file;
    remoteFile = remoteFile.replace(privateDir, "");
    qDebug() << "remoteFile=" << remoteFile;
    QString remoteUrl = url + remoteFile;

    QString remotePath = "KnotData/";

    qDebug() << "remotePath=" << remotePath;

    QFile *file = new QFile(localFile);
    if (!file->open(QIODevice::ReadOnly)) {
      qDebug() << "Failed to open file:" << localFile;
      delete file;
      continue;
    }

    QNetworkRequest request;
    request.setUrl(QUrl(remoteUrl));
    QString auth = QString("%1:%2").arg(USERNAME).arg(APP_PASSWORD);
    request.setRawHeader("Authorization",
                         "Basic " + auth.toLocal8Bit().toBase64());

    QNetworkReply *reply = manager->put(request, file);
    file->setParent(reply);  // 确保文件在请求完成后被释放

    // 可选：跟踪上传进度
    QObject::connect(reply, &QNetworkReply::uploadProgress,
                     [=](qint64 bytesSent, qint64 bytesTotal) {
                       qDebug() << "Uploading" << m_file << bytesSent << "/"
                                << bytesTotal;
                     });

    // 处理完成/错误
    QObject::connect(reply, &QNetworkReply::finished, [=]() {
      if (reply->error() == QNetworkReply::NoError) {
        qDebug() << "Upload succeeded:" << m_file;
      } else {
        qDebug() << "Error uploading" << m_file << ":" << reply->errorString();
      }
      reply->deleteLater();
    });
  }
}

// 核心函数：列出目录文件（认证信息直接作为参数）
WebDavHelper *listWebDavFiles(const QString &url, const QString &username,
                              const QString &password

) {
  // 创建信号发射器对象
  WebDavHelper *helper = new WebDavHelper();

  // 每个请求使用独立的NetworkManager
  QNetworkAccessManager *manager = new QNetworkAccessManager(helper);

  // 连接认证信号（Lambda捕获当前请求的账号密码）
  QObject::connect(
      manager, &QNetworkAccessManager::authenticationRequired,
      [username, password](QNetworkReply *reply, QAuthenticator *auth) {
        qDebug() << "正在认证:" << reply->url().toString();
        auth->setUser(username);
        auth->setPassword(password);
      });

  // 构造请求
  QNetworkRequest request;
  request.setUrl(QUrl(url));
  request.setRawHeader("Depth", "1");  // 仅获取当前目录的直接子项
  // request.setRawHeader("Depth",
  //                      "infinity");  // 递归获取子目录，但有的webdav不支持
  if (url.contains(mw_one->ui->editWebDAV->text().trimmed()))
    request.setRawHeader("Brief", "t");  // 坚果云需要此头
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    "text/xml; charset=utf-8");

  // 严格格式的XML请求体
  const QByteArray body = R"(<?xml version="1.0" encoding="utf-8"?>
        <d:propfind xmlns:d="DAV:">
            <d:prop>
                <d:displayname/>
                <d:getlastmodified/>
                <d:resourcetype/>
            </d:prop>
        </d:propfind>)";

  // 发送请求
  QNetworkReply *reply = manager->sendCustomRequest(request, "PROPFIND", body);

  // 处理响应
  QObject::connect(reply, &QNetworkReply::finished, [manager, helper, reply]() {
    if (reply->error() != QNetworkReply::NoError) {
      const QString error =
          QString("[HTTP %1] %2")
              .arg(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute)
                       .toInt())
              .arg(reply->errorString());
      emit helper->errorOccurred(error);
    } else {
      QByteArray responseData = reply->readAll();
      QList<QPair<QString, QDateTime>> files =
          parseWebDavResponse(responseData);  // 调用解析函数
      emit helper->listCompleted(files);
    }

    // 清理资源
    reply->deleteLater();
    manager->deleteLater();
  });

  return helper;
}

QList<QPair<QString, QDateTime>> parseWebDavResponse(const QByteArray &data) {
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
        // currentHref = QUrl::fromPercentEncoding(currentHref.toUtf8());
        // 一次性完成编码转换
        currentHref = QString::fromUtf8(
            QByteArray::fromPercentEncoding(currentHref.toLatin1()));

      } else if (xml.name() == QLatin1String("getlastmodified")) {
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

            // xml.skipCurrentElement();
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

WebDavDownloader::WebDavDownloader(const QString &username,
                                   const QString &password, QObject *parent)
    : QObject(parent), m_username(username), m_password(password) {
  // 连接认证信号
  connect(&manager, &QNetworkAccessManager::authenticationRequired, this,
          &WebDavDownloader::handleAuthentication);
}

void WebDavDownloader::downloadFiles(const QList<QString> &remotePaths,
                                     const QString &localBaseDir,
                                     int maxConcurrent) {
  // 清空队列
  downloadQueue.clear();
  activeDownloads.clear();

  // 初始化队列
  for (const QString &remotePath : remotePaths) {
    QString localPath = QDir(localBaseDir).absoluteFilePath(remotePath);
    downloadQueue.enqueue({remotePath, localPath});
  }

  totalFiles = downloadQueue.size();
  completedFiles = 0;
  this->maxConcurrent = qMax(1, maxConcurrent);

  // 启动初始下载
  for (int i = 0; i < qMin(this->maxConcurrent, downloadQueue.size()); ++i) {
    startNextDownload();
  }
}

void WebDavDownloader::handleAuthentication(QNetworkReply *reply,
                                            QAuthenticator *auth) {
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
  QUrl url("https://dav.jianguoyun.com/dav/" + remotePath);
  if (!url.isValid()) {
    qWarning() << "无效的URL:" << url.toString();
    return;
  }

  QNetworkRequest request(url);
  QNetworkReply *reply = manager.get(request);

  // 记录活动下载
  activeDownloads[reply] = localPath;

  // 连接信号
  connect(reply, &QNetworkReply::finished, this,
          [this, reply]() { onDownloadFinished(reply); });
  connect(reply, &QNetworkReply::downloadProgress, this,
          [this, reply](qint64 bytesReceived, qint64 bytesTotal) {
            // 下面未使用的两个变量用来计算下载进度（目前未实现）
            Q_UNUSED(bytesReceived);
            Q_UNUSED(bytesTotal);
            emit progressChanged(completedFiles, totalFiles,
                                 activeDownloads[reply]);
          });

  qDebug() << "开始下载:" << url.toString() << "->" << localPath;
}

void WebDavDownloader::onDownloadFinished(QNetworkReply *reply) {
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

  // 启动下一个下载
  if (!downloadQueue.isEmpty()) {
    startNextDownload();
  }

  // 全部完成
  if (completedFiles >= totalFiles) {
    emit downloadFinished(success, error);
    qDebug() << "所有下载任务完成";
  }
}

void CloudBackup::getRemoteFileList(QString url) {
  webdavFileList.clear();
  webdavDateTimeList.clear();
  isGetRemoteFileListEnd = false;

  WebDavHelper *helper = listWebDavFiles(url, USERNAME, APP_PASSWORD);
  // 连接信号
  QObject::connect(
      helper, &WebDavHelper::listCompleted,
      [=](const QList<QPair<QString, QDateTime>> &files) {
        qDebug() << "获取到文件列表:";
        qDebug() << "共找到" << files.size() << "个文件:";
        for (const auto &[path, mtime] : files) {
          qDebug() << "路径:" << path
                   << "修改时间:" << mtime.toString("yyyy-MM-dd hh:mm:ss");
          QString remoteFile = path;
          remoteFile = remoteFile.replace("/dav/", "");  // 此处需注意
          webdavFileList.append(remoteFile);
          webdavDateTimeList.append(mtime);
        }
        isGetRemoteFileListEnd = true;
      });

  QObject::connect(helper, &WebDavHelper::errorOccurred,
                   [=](const QString &error) {
                     qDebug() << "操作失败:" << error;
                     isGetRemoteFileListEnd = true;
                   });
}

void CloudBackup::createRemoteWebDAVDir() {
  QString url = getWebDAVArgument();
  createDirectory(url, "KnotData/");
  createDirectory(url, "KnotData/memo/");
  createDirectory(url, "KnotData/memo/images");
}

void CloudBackup::deleteWebDAVFiles(QStringList filesToDelete) {
  if (filesToDelete.count() > 0) {
    QString url = getWebDAVArgument();
    CloudDeleter deleter(USERNAME, APP_PASSWORD);
    deleter.baseUrl = url;
    deleter.deleteFiles(filesToDelete);
  }
}
