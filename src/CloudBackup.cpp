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
#include "src/defines.h"
#include "ui_CloudBackup.h"
#include "ui_MainWindow.h"

QList<QPair<QString, QDateTime>> parseWebDavResponse(const QByteArray &data);

CloudBackup::CloudBackup(QWidget *parent)
    : QDialog(parent), ui(new Ui::CloudBackup) {
  ui->setupUi(this);

  this->installEventFilter(this);

  int mh = mui->cboxWebDAV->height();
  mui->btnShowCboxList->setFixedSize(mh, mh);

  init();

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

  if (manager) {
    manager->deleteLater();  // 释放成员变量manager
  }
}

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

void CloudBackup::on_pushButton_downloadFile_clicked() {}

void CloudBackup::uploadData() {
  QString strFlag;
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

  if (mui->chkWebDAV->isChecked()) {
    QString url = getWebDAVArgument();
    createDirectory(url, "Knot/");
    uploadFileToWebDAV(url, zipfile, "Knot/memo.zip");
  }
}

QString CloudBackup::getWebDAVArgument() {
  QString url = mui->cboxWebDAV->currentText().trimmed();
  USERNAME = mui->editWebDAVUsername->text().trimmed();
  APP_PASSWORD = mui->editWebDAVPassword->text().trimmed();
  return url;
}

void CloudBackup::on_btnBack_clicked() {}

void CloudBackup::loadLogQML() {}

void CloudBackup::loadText(QString str) {}

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

  mui->progBar->show();
  mui->progBar->setValue(0);
  mui->progressBar->setValue(0);

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

      mui->progBar->hide();

    } else {
      qDebug() << "上传失败：" << reply->errorString();
      qDebug() << "服务器响应：" << reply->readAll();

      mui->progBar->hide();

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
    mui->progBar->setValue(percent);
    mui->progressBar->setValue(percent);
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

            resetProgBar();

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
            resetProgBar();
            mui->progressBar->setValue(0);
            mui->lblReceivedBytes->setText("0");

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

void CloudBackup::resetProgBar() {
  mui->progressBar->setMinimum(0);
  mui->progressBar->setMaximum(100);
}

void CloudBackup::downloadFile_Old(QString remoteFileName,
                                   QString localSavePath) {
  // 初始化网络管理器
  if (m_manager != nullptr) {
    delete m_manager;
    m_manager = nullptr;
  }
  m_manager = new QNetworkAccessManager(this);

  // 1. 构建并校验URL
  QUrl url(WEBDAV_URL + remoteFileName);
  if (!url.isValid()) {
    ShowMessage *m_ShowMsg = new ShowMessage(mw_one);
    m_ShowMsg->showMsg("WebDAV", tr("Invalid URL: ") + url.toString(), 1);
    return;
  }

  // 2. 构建网络请求
  QNetworkRequest request(url);
  request.setTransferTimeout(30000);  // 30秒超时
  request.setRawHeader("User-Agent", "CloudBackup/1.0");

  // 设置认证信息
  QString auth = QString("%1:%2").arg(USERNAME).arg(APP_PASSWORD);
  request.setRawHeader("Authorization", "Basic " + auth.toUtf8().toBase64());

  // 3. 准备本地文件
  QFile *localFile = new QFile(localSavePath);
  if (!localFile->open(QIODevice::WriteOnly)) {
    QString errorMsg =
        tr("Failed to create local file: ") + localFile->errorString();
    qDebug() << errorMsg;

    ShowMessage *m_ShowMsg = new ShowMessage(mw_one);
    m_ShowMsg->showMsg("WebDAV", errorMsg, 1);

    delete localFile;
    return;
  }

  // 4. 关键变量定义
  qint64 bytesReceived = 0;         // 已接收字节数
  qint64 fileTotalSize = -1;        // 文件总大小
  bool serviceChecked = false;      // 是否已完成服务类型检查
  bool useBusyProgress = true;      // 是否使用忙碌状态（默认是）
  QTimer *progressTimer = nullptr;  // 进度更新定时器

  // 5. 发起下载请求
  QNetworkReply *reply = m_manager->get(request);
  m_activeDownloads.insert(reply, localFile);

  // 6. 服务类型检查与处理（完全分离的逻辑）
  auto checkServiceType = [&]() {
    // 检查是否有总大小信息
    QByteArray contentLength = reply->rawHeader("Content-Length");
    QByteArray contentRange = reply->rawHeader("Content-Range");

    // 解析总大小
    if (!contentLength.isEmpty()) {
      bool conversionOk = false;
      fileTotalSize = contentLength.toLongLong(&conversionOk);
      if (conversionOk && fileTotalSize > 0) {
        useBusyProgress = false;  // 标准服务，使用百分比
      }
    } else if (!contentRange.isEmpty()) {
      QString rangeStr = contentRange;
      int totalSepIndex = rangeStr.lastIndexOf('/');
      if (totalSepIndex != -1) {
        bool conversionOk = false;
        fileTotalSize =
            rangeStr.mid(totalSepIndex + 1).toLongLong(&conversionOk);
        if (conversionOk && fileTotalSize > 0) {
          useBusyProgress = false;  // 标准服务，使用百分比
        }
      }
    }

    // 根据服务类型初始化进度条（完全分离的初始化）
    QMetaObject::invokeMethod(this, [this, useBusyProgress]() {
      if (useBusyProgress) {
        // 非标准服务：忙碌状态（min = max）
        mui->progressBar->setRange(0, 0);
        mui->progressBar->setValue(0);
      } else {
        // 标准服务：百分比状态
        mui->progressBar->setRange(0, 100);
        mui->progressBar->setValue(0);
      }
    });

    // 初始化对应类型的进度更新逻辑（完全分离的更新机制）
    if (!useBusyProgress) {
      // 标准服务：使用字节计数更新百分比
      connect(reply, &QNetworkReply::readyRead,
              [this, reply, &bytesReceived, &fileTotalSize]() {
                if (!reply || reply->isFinished()) return;

                QFile *file = m_activeDownloads.value(reply);
                if (file && file->isOpen()) {
                  QByteArray data = reply->readAll();
                  qint64 writeSize = file->write(data);
                  if (writeSize > 0) {
                    bytesReceived += writeSize;
                    int percent = static_cast<int>((bytesReceived * 100.0) /
                                                   fileTotalSize);
                    percent = qMin(percent, 100);

                    QMetaObject::invokeMethod(this, [this, percent]() {
                      mui->progressBar->setValue(percent);
                    });
                  }
                }
              });
    } else {
      // 非标准服务：仅需保持忙碌状态，无需额外更新逻辑
      qDebug() << "Using busy progress - no additional updates needed";
    }

    serviceChecked = true;
  };

  // 立即检查一次服务类型（可能此时元数据还未就绪）
  checkServiceType();

  // 元数据就绪后再次检查（确保准确判断服务类型）
  connect(reply, &QNetworkReply::metaDataChanged, [&]() {
    if (!serviceChecked) {
      checkServiceType();
    }
  });

  // 7. 非标准服务的数据接收处理（仅写入文件，不更新进度）
  connect(reply, &QNetworkReply::readyRead,
          [this, reply, localFile, &useBusyProgress]() {
            if (useBusyProgress && reply && !reply->isFinished()) {
              QFile *file = m_activeDownloads.value(reply);
              if (file && file->isOpen()) {
                // 仅写入数据，不处理进度（忙碌状态由Qt自动维护）
                QByteArray data = reply->readAll();
                file->write(data);
              }
            }
          });

  // 8. 下载完成处理 - 关键修复：将progressTimer改为按引用捕获
  connect(
      reply, &QNetworkReply::finished,
      [this, reply, localFile, localSavePath, &useBusyProgress,
       &progressTimer]()  // 这里使用&progressTimer按引用捕获
      {
        // 清理定时器（如果存在）
        if (progressTimer != nullptr) {
          progressTimer->stop();
          delete progressTimer;
          progressTimer = nullptr;
        }

        // 处理剩余数据
        if (localFile->isOpen()) {
          QByteArray remainingData = reply->readAll();
          if (!remainingData.isEmpty()) {
            localFile->write(remainingData);
          }
          localFile->close();
        }

        // 获取响应状态
        const int statusCode =
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QNetworkReply::NetworkError error = reply->error();

        // 重置进度条为正常范围
        QMetaObject::invokeMethod(
            this, [this]() { mui->progressBar->setRange(0, 100); });

        // 下载成功
        if (error == QNetworkReply::NoError && statusCode >= 200 &&
            statusCode < 300) {
          // 进度条设为100%
          QMetaObject::invokeMethod(
              this, [this]() { mui->progressBar->setValue(100); });

          // 成功提示
          zipfile = localSavePath;
          ShowMessage *showbox = new ShowMessage(this);
          showbox->showMsg(
              "WebDAV",
              tr("Successfully downloaded file, saved to: ") + localSavePath +
                  "\n\n" + tr("Final Size: ") +
                  m_Method->getFileSize(QFile(localSavePath).size(), 2),
              1);

          // 导入逻辑
          if (QFile::exists(localSavePath)) {
            ShowMessage *m_ShowMsg = new ShowMessage(mw_one);
            bool importConfirmed = m_ShowMsg->showMsg(
                "Kont",
                tr("Import this data?") + "\n" +
                    mw_one->m_Reader->getUriRealPath(localSavePath),
                2);

            if (!importConfirmed) {
              isZipOK = false;
            } else {
              isZipOK = true;
              mw_one->showProgress();
              isMenuImport = false;
              isDownData = true;
              mw_one->myImportDataThread->start();

              if (isZipOK) {
                mw_one->on_btnBack_One_clicked();
              }
            }
          }
        }
        // 下载失败
        else {
          localFile->remove();
          QString errorMsg =
              tr("Download failed (Error %1): ").arg(statusCode) +
              reply->errorString();
          qDebug() << errorMsg;

          ShowMessage *m_ShowMsg = new ShowMessage(mw_one);
          if (statusCode == 401) {
            m_ShowMsg->showMsg("WebDAV", tr("Authentication failed."), 1);
          } else {
            m_ShowMsg->showMsg("WebDAV", errorMsg, 1);
          }
          isZipOK = false;

          // 重置进度条为0
          QMetaObject::invokeMethod(
              this, [this]() { mui->progressBar->setValue(0); });
        }

        // 清理资源
        m_activeDownloads.remove(reply);
        delete localFile;
        reply->deleteLater();
      });

  // 错误处理
  connect(reply, &QNetworkReply::errorOccurred,
          [this, reply](QNetworkReply::NetworkError code) {
            qDebug() << "Download error - Code:" << code
                     << "Message:" << reply->errorString();
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

void CloudBackup::uploadFilesToWebDAV_old(QStringList files) {
  QNetworkAccessManager *manager = new QNetworkAccessManager();
  QString url = getWebDAVArgument();

  // 使用QAtomicInt确保线程安全
  QAtomicInt *activeReplyCount = new QAtomicInt(files.size());

  // 处理空文件列表的情况
  if (files.isEmpty()) {
    manager->deleteLater();
    delete activeReplyCount;
    return;
  }

  foreach (QString m_file, files) {
    QString localFile = m_file;
    QString remoteFile = m_file;
    remoteFile = remoteFile.replace(privateDir, "");

    // 规范URL拼接
    QUrl baseUrl(url);
    QUrl fullUrl = baseUrl.resolved(remoteFile);

    QFile *file = new QFile(localFile);
    if (!file->open(QIODevice::ReadOnly)) {
      qDebug() << "Failed to open file:" << localFile;
      delete file;

      // 安全处理计数器
      if (activeReplyCount->fetchAndSubRelaxed(1) == 1) {
        manager->deleteLater();
        delete activeReplyCount;
      }
      continue;
    }

    QNetworkRequest request;
    request.setUrl(fullUrl);
    QString auth = QString("%1:%2").arg(USERNAME, APP_PASSWORD);
    request.setRawHeader("Authorization",
                         "Basic " + auth.toLocal8Bit().toBase64());

    QNetworkReply *reply = manager->put(request, file);
    file->setParent(reply);  // 文件随reply释放

    // 弱引用this指针，避免悬垂指针
    QPointer<CloudBackup> thisPtr(this);

    // 上传进度跟踪
    QObject::connect(reply, &QNetworkReply::uploadProgress,
                     [=](qint64 bytesSent, qint64 bytesTotal) {
                       Q_UNUSED(bytesSent);
                       Q_UNUSED(bytesTotal);

                       // qDebug() << "Uploading" << m_file << bytesSent << "/"
                       //          << bytesTotal;
                     });

    // 处理完成/错误
    QObject::connect(reply, &QNetworkReply::finished, this, [=]() {
      if (!thisPtr) {  // 检查对象是否已销毁
        reply->deleteLater();
        return;
      }

      if (reply->error() == QNetworkReply::NoError) {
        mw_one->m_Notes->notes_sync_files.removeOne(m_file);
        qDebug() << "Upload succeeded:" << m_file
                 << "Rema:" << mw_one->m_Notes->notes_sync_files.count();

        mw_one->saveNeedSyncNotes();
      } else {
        qDebug() << "Error uploading" << m_file << ":" << reply->errorString();
      }
      reply->deleteLater();

      // 原子操作减少计数器并检查是否为最后一个任务
      if (activeReplyCount->fetchAndSubRelaxed(1) == 1) {
        mw_one->closeProgress();

        manager->deleteLater();
        delete activeReplyCount;
      }
    });
  }
}

///////////////////////////////////////////////////////////////////////////////////

void CloudBackup::uploadFilesToWebDAV(const QStringList &files) {
  // 将新文件添加到队列
  for (const QString &file : files) {
    uploadQueue.enqueue(file);
  }

  // 启动上传处理
  startNextUpload();
}

void CloudBackup::startNextUpload() {
  // 检查是否达到最大并发数
  while (activeReplies.size() < maxConcurrentUploads &&
         !uploadQueue.isEmpty()) {
    QString filePath = uploadQueue.dequeue();
    QString remoteFile = filePath;
    remoteFile.replace(privateDir, "");
    QUrl fullUrl = QUrl(getWebDAVArgument()).resolved(remoteFile);

    QFile *file = new QFile(filePath);
    if (!file->open(QIODevice::ReadOnly)) {
      qWarning() << "Failed to open file:" << filePath;
      delete file;
      continue;
    }

    QNetworkRequest request(fullUrl);
    QString auth = QString("%1:%2").arg(USERNAME, APP_PASSWORD);
    request.setRawHeader("Authorization",
                         "Basic " + auth.toLocal8Bit().toBase64());

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager->put(request, file);
    file->setParent(reply);
    activeReplies.insert(reply);

    // 使用 Qt 的信号槽机制安全处理完成事件
    connect(reply, &QNetworkReply::finished, this, [this, reply, filePath]() {
      handleUploadFinished(reply, filePath);
    });
  }
}

void CloudBackup::handleUploadFinished(QNetworkReply *reply,
                                       const QString &filePath) {
  // 确保从活动集合中移除
  activeReplies.remove(reply);

  if (reply->error() == QNetworkReply::NoError) {
    // 处理成功上传
    if (mw_one && mw_one->m_Notes) {
      mw_one->m_Notes->notes_sync_files.removeOne(filePath);
      qDebug() << "Upload succeeded:" << filePath
               << "Remaining:" << mw_one->m_Notes->notes_sync_files.count();
      mw_one->saveNeedSyncNotes();
    }
  } else {
    qWarning() << "Error uploading" << filePath << ":" << reply->errorString();
    // 可选：将失败的文件重新加入队列重试
    // uploadQueue.enqueue(filePath);
  }

  // 清理资源
  reply->deleteLater();

  // 检查是否所有上传都已完成
  if (activeReplies.isEmpty() && uploadQueue.isEmpty()) {
    if (mw_one) {
      mw_one->closeProgress();
    }
  } else {
    // 启动下一个上传
    startNextUpload();
  }
}
/// /////////////////////////////////////////////////////////////////////////////////

// 核心函数：列出目录文件（支持坚果云分页）
WebDavHelper *listWebDavFiles(const QString &url, const QString &username,
                              const QString &password) {
  WebDavHelper *helper = new WebDavHelper();
  QNetworkAccessManager *manager = new QNetworkAccessManager(helper);

  // 连接认证信号
  QObject::connect(
      manager, &QNetworkAccessManager::authenticationRequired,
      [username, password](QNetworkReply *reply, QAuthenticator *auth) {
        Q_UNUSED(reply);
        auth->setUser(username);
        auth->setPassword(password);
      });

  QNetworkRequest request;
  request.setUrl(QUrl(url));
  request.setRawHeader("Depth", "1");
  if (url.contains("jianguoyun.com"))  // 坚果云特定头
    request.setRawHeader("Brief", "t");
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    "text/xml; charset=utf-8");

  const QByteArray body = R"(<?xml version="1.0" encoding="utf-8"?>
        <d:propfind xmlns:d="DAV:">
            <d:prop>
                <d:displayname/>
                <d:getlastmodified/>
                <d:resourcetype/>
            </d:prop>
        </d:propfind>)";

  QNetworkReply *reply = manager->sendCustomRequest(request, "PROPFIND", body);

  QObject::connect(reply, &QNetworkReply::finished, [manager, helper, reply]() {
    if (reply->error() != QNetworkReply::NoError) {
      const QString error =
          QString("[HTTP %1] %2")
              .arg(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute)
                       .toInt())
              .arg(reply->errorString());
      emit helper->errorOccurred(error);
    } else {
      // 保存响应头
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

  // 启动初始下载，每个任务之间加入延迟（毫秒）
  const int INITIAL_DELAY = 50;  // 时间间隔
  for (int i = 0; i < qMin(this->maxConcurrent, downloadQueue.size()); ++i) {
    // 第i个任务延迟i*INITIAL_DELAY毫秒启动，避免同时发起
    QTimer::singleShot(i * INITIAL_DELAY, this,
                       &WebDavDownloader::startNextDownload);
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

  // 构造请求URL "https://dav.jianguoyun.com/dav/"
  QString strUrl = mui->cboxWebDAV->currentText().trimmed();
  QUrl url(strUrl + remotePath);
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
            // 下面两个变量用来计算下载进度（目前未实现）

            qDebug() << "bytesReceived=" << bytesReceived << "->"
                     << "bytesTotal=" << bytesTotal;

            if (m_Method->lblInfo != nullptr) {
              QString rece = m_Method->getFileSize(bytesReceived, 2);
              m_Method->lblInfo->setText("[" + QString::number(completedFiles) +
                                         "/" + QString::number(totalFiles) +
                                         "] [" + rece + "] " +
                                         activeDownloads[reply]);

              infoProgBarMax = totalFiles;
              infoProgBarValue = completedFiles;
            }

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

  // 开始获取第一页
  fetchNextPage();
}

void CloudBackup::fetchNextPage() {
  if (nextPageUrl.isEmpty()) {
    // 所有页面处理完成
    processFileList();
    return;
  }

  WebDavHelper *helper = listWebDavFiles(nextPageUrl, USERNAME, APP_PASSWORD);
  helper->setParent(this);

  QObject::connect(
      helper, &WebDavHelper::listCompleted, this,
      [this, helper](const QList<QPair<QString, QDateTime>> &files) {
        allFiles.append(files);

        // 解析下一页URL
        nextPageUrl = parseNextPageUrl(helper->getResponseHeaders());

        // 删除当前 helper
        helper->deleteLater();

        if (!nextPageUrl.isEmpty()) {
          // 继续获取下一页
          QTimer::singleShot(100, this, &CloudBackup::fetchNextPage);
        } else {
          // 所有页面处理完成
          processFileList();
        }
      });

  QObject::connect(helper, &WebDavHelper::errorOccurred, this,
                   [this, helper](const QString &error) {
                     qDebug() << "获取文件列表出错:" << error;
                     helper->deleteLater();
                     processFileList();  // 即使出错也处理已获取的文件
                   });
}

QString CloudBackup::parseNextPageUrl(
    const QList<QNetworkReply::RawHeaderPair> &headers) {
  // 查找 Link 头
  for (const auto &header : headers) {
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
  qDebug() << "获取到文件列表:";
  qDebug() << "共找到" << allFiles.size() << "个文件:";

  for (const auto &[path, mtime] : allFiles) {
    QString remoteFile = path;
    remoteFile = remoteFile.replace("/dav/", "");  // 坚果云路径处理

    webdavFileList.append(remoteFile);
    webdavDateTimeList.append(mtime);
  }

  isGetRemoteFileListEnd = true;
  // emit fileListReady();  // 通知其他部分文件列表已准备好
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

  QString strWebDAV = mui->cboxWebDAV->currentText().trimmed();
  QString strUserName = mui->editWebDAVUsername->text().trimmed();
  iniPreferences->setValue("/webdav/url", strWebDAV);
  iniPreferences->setValue("/webdav/username", strUserName);
  QString password = mui->editWebDAVPassword->text().trimmed();
  QString aesStr = aesEncrypt(password, aes_key, aes_iv);
  iniPreferences->setValue("/webdav/password", aesStr);

  iniPreferences->setValue("/webdav/username_" + strWebDAV, strUserName);
  iniPreferences->setValue("/webdav/password_" + strWebDAV, aesStr);

  int count = mui->cboxWebDAV->count();
  iniPreferences->setValue("/webdav/count", count);
  for (int i = 0; i < count; i++) {
    iniPreferences->setValue("/webdav/text" + QString::number(i),
                             mui->cboxWebDAV->itemText(i));
  }

  iniPreferences->setValue("/cloudbak/webdav", mui->chkWebDAV->isChecked());
  iniPreferences->setValue("/cloudbak/autosync", mui->chkAutoSync->isChecked());

  mw_one->m_Preferences->setEncSyncStatusTip();

  mui->frameOne->hide();
  mui->frameMain->show();
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

void CloudBackup::changeComBoxWebDAV(const QString &arg1) {
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

  ShowMessage *m_ShowMsg = new ShowMessage(this);
  if (!m_ShowMsg->showMsg(
          "WebDAV",
          tr("Downloading data?") + "\n\n" +
              tr("This action overwrites local files with files in the cloud."),
          2))
    return;
  WEBDAV_URL = mui->cboxWebDAV->currentText().trimmed();
  USERNAME = mui->editWebDAVUsername->text().trimmed();
  APP_PASSWORD = mui->editWebDAVPassword->text().trimmed();
  downloadFile("Knot/memo.zip", filePath);
  mui->progressBar->setValue(0);
}

bool CloudBackup::checkWebDAVConnection() {
  // 获取并处理URL
  QString urlText = mui->cboxWebDAV->currentText().trimmed();
  if (!urlText.endsWith("/")) {
    urlText += "/";  // 确保URL以斜杠结尾，符合WebDAV规范
  }
  QUrl url(urlText);

  QString m_errorMsg;
  // 检查URL有效性
  if (!url.isValid() || url.scheme() != "https") {
    m_errorMsg = "URL无效，必须使用https协议";
    return false;
  }

  QString username = mui->editWebDAVUsername->text().trimmed();
  QString password = mui->editWebDAVPassword->text().trimmed();

  // 创建网络管理器和请求
  QNetworkAccessManager manager;
  QNetworkRequest request(url);

  // 设置必要的请求头
  request.setRawHeader("User-Agent", "MyApp/1.0");
  request.setRawHeader("Depth", "0");  // PROPFIND需要的深度头
  request.setRawHeader("Content-Type", "application/xml");

  // 准备PROPFIND请求的XML内容
  QString xml =
      "<?xml version=\"1.0\"?>"
      "<propfind xmlns=\"DAV:\">"
      "  <prop>"
      "    <current-user-principal />"
      "  </prop>"
      "</propfind>";
  QByteArray data = xml.toUtf8();

  // 发送PROPFIND请求（WebDAV标准方法）
  QNetworkReply *reply = manager.sendCustomRequest(request, "PROPFIND", data);

  // 等待响应
  QEventLoop loop;
  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();

  // 处理可能的认证挑战
  QVariant statusCode =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
  int status = statusCode.toInt();

  if (status == 401) {  // 需要认证
    // 提取认证挑战信息
    QByteArray authHeader = reply->rawHeader("WWW-Authenticate");
    reply->deleteLater();

    // 构建认证信息
    if (!authHeader.isEmpty() && authHeader.contains("Basic")) {
      QByteArray auth = username.toUtf8() + ":" + password.toUtf8();
      QNetworkRequest authRequest(url);

      // 设置认证头和其他必要头信息
      authRequest.setRawHeader("Authorization", "Basic " + auth.toBase64());
      authRequest.setRawHeader("User-Agent", "MyApp/1.0");
      authRequest.setRawHeader("Depth", "0");
      authRequest.setRawHeader("Content-Type", "application/xml");

      // 重新发送带认证信息的请求
      QNetworkReply *authReply =
          manager.sendCustomRequest(authRequest, "PROPFIND", data);
      QEventLoop authLoop;
      QObject::connect(authReply, &QNetworkReply::finished, &authLoop,
                       &QEventLoop::quit);
      authLoop.exec();

      // 检查认证后的响应
      QVariant authStatusCode =
          authReply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
      int authStatus = authStatusCode.toInt();

      if (authReply->error() == QNetworkReply::NoError &&
          (authStatus == 207 ||
           authStatus == 200)) {  // 207是WebDAV的多状态响应
        m_errorMsg = "连接成功";
        authReply->deleteLater();
        return true;
      } else {
        m_errorMsg = QString("认证失败: %1 (状态码: %2)")
                         .arg(authReply->errorString())
                         .arg(authStatus);
        authReply->deleteLater();
        return false;
      }
    } else {
      m_errorMsg = "不支持的认证方式";
      return false;
    }
  }
  // 处理无需认证或直接成功的情况
  else if (reply->error() == QNetworkReply::NoError &&
           (status == 207 || status == 200)) {
    m_errorMsg = "连接成功";
    reply->deleteLater();
    return true;
  }
  // 处理其他错误
  else {
    m_errorMsg = QString("连接失败: %1 (状态码: %2)")
                     .arg(reply->errorString())
                     .arg(status);
    reply->deleteLater();
    return false;
  }
}
