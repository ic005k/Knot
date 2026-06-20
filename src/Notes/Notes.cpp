#include "src/Notes/Notes.h"

#include "cmark-gfm-core-extensions.h"
#include "cmark_wrapper.h"
// #include "src/MainWindow.h"
// #include "src/defines.h"
#include "subscript.h"
#include "superscript.h"
// #include "ui_MainWindow.h"
#include "ui_Notes.h"

static QAtomicInt n_Files = 0;

NoteIndexManager1::NoteIndexManager1(QObject* parent) : QObject{parent} {}

Notes::Notes(QWidget* parent) : QDialog(parent), ui(new Ui::Notes) {
  ui->setupUi(this);
  m_NoteIndexManager = new NoteIndexManager();

  initEditor();
  init_md();

  QString path = iniDir + "memo/";
  QDir dir(path);
  if (!dir.exists()) dir.mkdir(path);
  htmlFileName = iniDir + "memo.html";

  this->installEventFilter(this);
  this->setModal(true);
  this->layout()->setContentsMargins(5, 5, 5, 5);
  ui->frameEdit->layout()->setContentsMargins(0, 0, 0, 0);

  timerEditNote = new QTimer(this);
  connect(timerEditNote, SIGNAL(timeout()), this, SLOT(on_editNote()));

  ui->btnColor->hide();
  ui->lblCount->hide();
  ui->btnFind->hide();
  ui->editFind->setMinimumWidth(65);

  if (isAndroid) {
    mui->btnBack_NotesSearchResult->hide();
  }

  m_Method->set_ToolButtonStyle(this);

  // 笔记链接自动补全
  m_popupList = new QListWidget(this);
  m_popupList->setWindowFlags(Qt::FramelessWindowHint | Qt::Window | Qt::Tool |
                              Qt::WindowStaysOnTopHint);
  m_popupList->setFocusPolicy(Qt::NoFocus);
  m_popupList->hide();

  connect(m_popupList, &QListWidget::itemClicked, this,
          &Notes::onPopupItemClicked);

  // Ctrl+F 快捷键
  QShortcut* shortcut1 = new QShortcut(QKeySequence("Ctrl+F"), this);
  connect(shortcut1, &QShortcut::activated, this, [this]() {
    ui->editFind->setFocus();
    ui->editFind->selectAll();
  });

  // 清理旧备份
  QDir memoDir(iniDir + "memo/");
  memoDir.setNameFilters(QStringList() << "*.md.bak" << "*.json.bak");
  foreach (const QString& bakFile, memoDir.entryList()) {
    memoDir.remove(bakFile);
  }
}

Notes::~Notes() {
  delete m_NoteIndexManager;
  delete ui;
}

void Notes::showEvent(QShowEvent* event) {
  QWidget::showEvent(event);
  if (!m_initialized) {
    int btn_h = ui->btnNext->height();
    ui->btnDone->setFixedSize(btn_h, btn_h);
    ui->btnView->setFixedSize(btn_h, btn_h);

#ifndef Q_OS_ANDROID
    QFont font = mw_one->font();
    m_EditSource->setFont(font);
    markdownLexer->setFont(font);
#endif

    m_initialized = true;
  }
}

void Notes::init() {
  int w = this->width();
  if (mw_one->width() > w) w = mw_one->width();
  this->setGeometry(this->x(), mw_one->geometry().y(), w, mw_one->height());
}

void Notes::wheelEvent(QWheelEvent* e) { Q_UNUSED(e); }
void Notes::keyReleaseEvent(QKeyEvent* event) { event->accept(); }
void Notes::editVSBarValueChanged() {}
void Notes::resizeEvent(QResizeEvent* event) { Q_UNUSED(event); }

bool Notes::eventFilter(QObject* obj, QEvent* evn) {
  QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evn);
  if (evn->type() == QEvent::KeyRelease) {
    if (keyEvent->key() == Qt::Key_Back) {
    }
  }

  if (evn->type() == QEvent::KeyPress) {
    if (keyEvent->key() == Qt::Key_Escape) {
      close();
      evn->accept();
    }
  }

#ifndef Q_OS_ANDROID
  if (obj == m_EditSource) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evn);

    // Ctrl+F 处理
    if (keyEvent->matches(QKeySequence::Find)) {
      QString text = m_EditSource->selectedText();
      if (!text.isEmpty()) {
        ui->editFind->setText(text);
        ui->editFind->setFocus();
      }
      keyEvent->accept();
      return true;  // 事件过滤器返回true = 拦截事件
    }

    if (evn->type() == QEvent::KeyPress) {
      if (keyEvent->key() != Qt::Key_Back) {
      }
    }
  }

  if (obj == m_EditSource->viewport()) {
    if (evn->type() == QEvent::MouseButtonPress) {
    }
  }
#endif

  return QWidget::eventFilter(obj, evn);
}

void Notes::closeEvent(QCloseEvent* event) {
  Q_UNUSED(event);
  m_popupList->close();
  saveEditorState(currentMDFile);

#ifndef Q_OS_ANDROID
  newText = m_EditSource->text().trimmed();
#endif

  m_Method->Sleep(100);

  if (isTextChange) {
    auto msg = std::make_unique<ShowMessage>(this);
    msg->ui->btnOk->setText(tr("Yes") + " (Y)");
    msg->ui->btnCancel->setText(tr("No") + " (N)");
    if (msg->showMsg(tr("Notes"), tr("Do you want to save the notes?"), 2)) {
      saveMainNotes();
    }
  }

  if (isSetNewNoteTitle()) {
    TitleGenerator generator;
    new_title = generator.genNewTitle(newText);

    int index = m_NotesList->getNotesListCurrentIndex();
    tw->setCurrentItem(m_NotesList->pNoteItems.at(index));

    renameTitle(true);
  }
}

void Notes::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);
  return;
}

bool Notes::eventFilterQwNote(QObject* watch, QEvent* event) {
  return QWidget::eventFilter(watch, event);
}

void Notes::openEditUI() {
  int count = m_NotesList->getNoteBookCount();
  if (count == 0) return;

  qDebug() << "currentMDFile=" << currentMDFile;
  if (!QFile::exists(currentMDFile)) {
    auto msg = std::make_unique<ShowMessage>(mw_one);
    msg->showMsg(appName,
                 tr("The current note does not exist. Please select another "
                    "note or create a new note."),
                 0);

    return;
  }

  oldText = loadText(currentMDFile);

  m_NotesList->refreshRecentOpen(m_NotesList->noteTitle);
  m_NotesList->saveRecentOpen();
  m_NotesList->moveToFirst();

  if (isAndroid) {
    m_Method->setMDFile(currentMDFile);
    setAndroidNoteConfig("/cpos/currentMDFile",
                         QFileInfo(currentMDFile).baseName());

    openAndroidNoteEditor();

    return;
  }

#ifndef Q_OS_ANDROID

  mw_one->mainHeight = mw_one->height();

  init();

  m_EditSource->setWrapMode(QsciScintilla::WrapNone);
  m_EditSource->setText(oldText);
  m_EditSource->setWrapMode(QsciScintilla::WrapWord);  // 按单词换行

  show();

  m_Method->Sleep(100);

  restoreEditorState(currentMDFile);
  m_EditSource->setFocus();

  m_Method->Sleep(200);

  isTextChange = false;

  if (mw_one->isOpenSearchResult) {
    QString findText = mw_one->mySearchText;
    if (findText.length() > 0) {
      m_EditSource->SendScintilla(QsciScintilla::SCI_SETANCHOR, 0);
      m_EditSource->SendScintilla(QsciScintilla::SCI_SETCURRENTPOS, 0);

      ui->editFind->setText(findText);
      on_btnNext_clicked();
    }
  }

  mw_one->isOpenSearchResult = false;

#endif
}

void Notes::previewNote() {
  int count = m_NotesList->getNoteBookCount();
  if (count == 0) return;

  if (!QFile::exists(currentMDFile)) return;

  mw_one->showProgress();
  QString title = m_NoteIndexManager->getNoteTitle(currentMDFile);

  QFuture<void> future = QtConcurrent::run([=]() {
    m_NotesList->refreshRecentOpen(title);
    m_NotesList->saveRecentOpen();
    MD2Html(currentMDFile);
  });

  // 使用 QFutureWatcher 监控进度
  QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
  connect(watcher, &QFutureWatcher<void>::finished, this, [=]() {
    mw_one->closeProgress();

    m_NotesList->moveToFirst();

    if (isAndroid) {
      m_Method->setMDTitle(title);
      oldText = loadText(currentMDFile);
      m_Method->setMDFile(currentMDFile);
      // openMDWindow();

      openLocalHtmlFileInAndroid();

      setAndroidNoteConfig("/cpos/currentMDFile",
                           QFileInfo(currentMDFile).baseName());

    } else {
      openBrowserOnce(htmlFileName);
    }

    qDebug() << "Preview note completed";
    watcher->deleteLater();
  });
  watcher->setFuture(future);
}

//===========================================================
// 检查是否需要清理 + 自动更新下次清理日期（二合一）
// 返回：true=需要清理  false=不需要
//===========================================================
bool Notes::checkAndUpdateCleanDate() {
  QSettings cfg(iniDir + "config.ini", QSettings::IniFormat);
  QDate today = QDate::currentDate();

  QString lastDateStr = cfg.value("Clean/LastCleanDate", "").toString();
  QDate lastClean = QDate::fromString(lastDateStr, "yyyyMMdd");

  // 无记录/无效日期 → 初始化，不清理
  if (!lastClean.isValid()) {
    cfg.setValue("Clean/LastCleanDate", today.toString("yyyyMMdd"));
    cfg.sync();
    return false;
  }

  QDate nextClean = lastClean.addDays(90);
  bool needClean = (today >= nextClean);

  // 需要清理 → 立即更新日期
  if (needClean) {
    cfg.setValue("Clean/LastCleanDate", today.toString("yyyyMMdd"));
    cfg.sync();
  }

  if (needClean) buildCleanFileList();

  return needClean;
}

//===========================================================
// 构建自动清理列表（追加模式，不清空原有列表）
// 传统at(i)遍历，无clazy警告，最安全
//===========================================================
void Notes::buildCleanFileList() {
  QDate today = QDate::currentDate();
  // 匹配开头 14 位数字，提取前 8 位日期数字
  QRegularExpression reg("^(\\d{8})\\d{6}_");

  // 传统下标遍历，无detach，无clazy警告
  int count = orgRemoteFiles.size();
  for (int i = 0; i < count; ++i) {
    const QString& fullPath = orgRemoteFiles.at(i);

    // 跳过包含 mainnotes / todo 的备份文件（永不删除）
    if (fullPath.contains("mainnotes") || fullPath.contains("todo")) {
      continue;
    }

    bool istest = false;
    if (istest) {
      // test
      if (i == 0) {
        needDelWebDAVFiles.append(fullPath);
        qDebug() << "测试自动清理服务器文件：" << fullPath;
      }
    }

    // 提取日期
    QRegularExpressionMatch match = reg.match(fullPath);
    if (!match.hasMatch()) continue;

    QDate fileDate = QDate::fromString(match.captured(1), "yyyyMMdd");
    if (!fileDate.isValid()) continue;

    // ==============================
    // ✅ 新安全策略：只删除超过 60 天的旧文件
    // ✅ 90 天触发一次清理
    // ✅ 保留最近 30 天，给足多端同步时间
    if (fileDate.daysTo(today) > 60) {
      needDelWebDAVFiles.append(fullPath);
    }
  }
}

void Notes::init_all_notes() {
  m_NotesList->initNotesList();
  m_NotesList->initRecycle();
  m_NotesList->initUnclassified();

  // load note
  if (!isReceiveRemoteFile) {
    currentMDFile = m_NotesList->getCurrentMDFile();
  } else
    isReceiveRemoteFile = false;
  qDebug() << "currentMDFile=" << currentMDFile;
  if (QFile::exists(currentMDFile)) {
  } else {
    loadEmptyNote();
  }
}

void Notes::openNotes() {
  if (!mw_one->m_Preferences->devMode)
    mui->btnManagement->hide();
  else
    mui->btnManagement->show();

  isPasswordError = false;
  isWebDAVError = false;
  isGetWebDavModiTime = false;

  m_Method->showInfoWindow(tr("Processing..."));

  if (mui->chkAutoSync->isChecked() && mui->chkWebDAV->isChecked()) {
    if (!m_CloudBackup->checkWebDAVConnection()) {
      m_Method->closeInfoWindow();
      isWebDAVError = true;
      auto msg = std::make_unique<ShowMessage>(this);
      msg->showMsg(appName,
                   tr("WebDAV connection failed. Please check the network, "
                      "website address or login information."),
                   1);

      QTimer::singleShot(100, mw_one, [this]() { openNotesUI(); });
      return;
    }

    orgRemoteDateTime.clear();
    orgRemoteFiles.clear();
    remoteFiles.clear();
    QString url = m_CloudBackup->getWebDAVArgument();

    // get md files
    m_CloudBackup->getRemoteFileList(url + "KnotData/memo/");
    while (!m_CloudBackup->isGetRemoteFileListEnd) {
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
      QThread::msleep(1);
    }

    // qDebug() << m_CloudBackup->webdavFileList
    //          << m_CloudBackup->webdavDateTimeList;

    for (int i = 0; i < m_CloudBackup->webdavFileList.count(); i++) {
      orgRemoteFiles.append(m_CloudBackup->webdavFileList.at(i));
      orgRemoteDateTime.append(m_CloudBackup->webdavDateTimeList.at(i));

      if (i == 0) {
        qDebug() << "远程原始文件取样：orgRemoteFiles=" << orgRemoteFiles.at(0);
      }
    }

    // get md image files
    m_CloudBackup->getRemoteFileList(url + "KnotData/memo/images/");
    while (!m_CloudBackup->isGetRemoteFileListEnd) {
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
      QThread::msleep(1);
    }

    // qDebug() << m_CloudBackup->webdavFileList
    //          << m_CloudBackup->webdavDateTimeList;

    for (int i = 0; i < m_CloudBackup->webdavFileList.count(); i++) {
      orgRemoteFiles.append(m_CloudBackup->webdavFileList.at(i));
      orgRemoteDateTime.append(m_CloudBackup->webdavDateTimeList.at(i));
    }

    WebDavHelper* helper =
        listWebDavFiles(url + "KnotData/", m_CloudBackup->USERNAME,
                        m_CloudBackup->APP_PASSWORD);
    helper->setParent(this);

    m_Method->setAccessCount(6);

    // 连接信号
    QObject::connect(
        helper, &WebDavHelper::listCompleted, this,
        [=](const QList<QPair<QString, QDateTime>>& files) {
          qDebug() << "获取KnotData根目录文件列表:" << url + "KnotData/";
          m_Method->setInfoText(">>" + url + "KnotData/");
          QString info = "共找到 " + QString::number(files.size()) + " 文件";
          qDebug() << info;

          for (const auto& [path, mtime] : files) {
            qDebug() << "路径:" << path
                     << "修改时间:" << mtime.toString("yyyy-MM-dd hh:mm:ss");
            QString remote_f = path;

            remote_f = m_CloudBackup->getRemoteKnotDataFullPath(remote_f);

            qDebug() << "处理之后的远程文件：" << remote_f;

            if (path.contains("mainnotes.json.zip")) {
              orgRemoteFiles.append(remote_f);
              orgRemoteDateTime.append(mtime);
            }
          }

          for (int j = 0; j < orgRemoteFiles.count(); j++) {
            QString or_file = orgRemoteFiles.at(j);

            // qDebug() << "or_file=" << or_file;

            QDateTime or_datetime = orgRemoteDateTime.at(j);
            or_datetime.toTimeZone(QTimeZone::utc());
            or_datetime = or_datetime.toLocalTime();

            QString local_file = privateDir + or_file;

            QString local_realfile, localLastModi;
            QFileInfo fi(local_file);
            QString fn = fi.fileName();
            fn = fn.replace(".zip", "");
            if (local_file.contains("mainnotes.json"))
              local_realfile = iniDir + fn;
            if (local_file.contains(".md"))
              local_realfile = iniDir + "memo/" + fn;
            if (local_file.contains(".json") &&
                !local_file.contains("mainnotes.json"))
              local_realfile = iniDir + "memo/" + fn;
            if (local_file.contains("images"))
              local_realfile = iniDir + "memo/images/" + fn;

            QString remoteLastModi;
            QStringList list = fn.split("_");
            if (list.count() == 4 || or_file.contains("_mainnotes.json")) {
              remoteLastModi = list.at(0).trimmed();

              local_realfile = local_realfile.replace(remoteLastModi + "_", "");
              localLastModi = m_Method->getFileUTCString(local_realfile);

              if (remoteLastModi > localLastModi) {
                remoteFiles.append(or_file);
                qDebug() << "Remote time: " << remoteLastModi
                         << "Local time: " << localLastModi << or_file
                         << local_realfile;
              }
            }
          }

          if (remoteFiles.count() > 0) {
            // 初始化下载器
            WebDavDownloader* downloader = new WebDavDownloader(
                m_CloudBackup->USERNAME, m_CloudBackup->APP_PASSWORD, this);

            // 连接信号
            QObject::connect(downloader, &WebDavDownloader::progressChanged,
                             [](int current, int total, QString file) {
                               QString info = QString("prog: %1/%2  cur: %3")
                                                  .arg(current)
                                                  .arg(total)
                                                  .arg(file);
                               qDebug() << info;
                             });

            QObject::connect(
                downloader, &WebDavDownloader::downloadFinished, this,
                [=](bool success, QString error) {
                  qDebug() << (success ? "下载成功" : "下载失败: " + error);
                  if (success) {
                    startBackgroundProcessRemoteFiles_MultiThread();
                  } else {
                    qDebug() << "下载失败：" << error;
                    auto msg = std::make_unique<ShowMessage>(this);
                    msg->showMsg(
                        appName,
                        tr("Synchronization failed. Please try again later."),
                        1);
                    QTimer::singleShot(100, mw_one,
                                       [this]() { openNotesUI(); });
                  }
                });

            // 开始下载（2并发,根据webdav供应商，一般2会很安全）
            QString lf = privateDir;
            qDebug() << "lf=" << lf;
            int maxConcurrentDownloads = maxNetConcurrent;
            downloader->downloadFiles(remoteFiles, lf, maxConcurrentDownloads);
            m_Method->setAccessCount(remoteFiles.count());
          }

          if (remoteFiles.count() == 0)
            QTimer::singleShot(100, mw_one, [this]() { openNotesUI(); });
        });

    QObject::connect(
        helper, &WebDavHelper::errorOccurred, this, [=](const QString& error) {
          qDebug() << "操作失败:" << error;
          QTimer::singleShot(100, mw_one, [this]() { openNotesUI(); });
        });
  } else

    QTimer::singleShot(100, mw_one, [this]() { openNotesUI(); });
}

void Notes::startBackgroundProcessRemoteFiles_MultiThread() {
  if (remoteFiles.isEmpty()) {
    QTimer::singleShot(100, mw_one, [this]() { openNotesUI(); });
    return;
  }

  n_Files = 0;

  // 后台异步，但 串行执行本地处理（安全、稳定、不崩溃）
  QThreadPool::globalInstance()->start([this]() {
    for (const QString& file : std::as_const(remoteFiles)) {
      processSingleRemoteFile(file);
    }

    QMetaObject::invokeMethod(this, [this]() { openNotesUI(); });
  });
}

void Notes::processSingleRemoteFile(const QString& file) {
  QString temp_f = file;

  QString pDir, pFile, kFile, asFile, zFile;
  pFile = privateDir + temp_f;
  zFile = pFile;
  asFile = temp_f;

  QFileInfo fi(file);
  QString fn = fi.fileName();
  QStringList list = fn.split("_");
  QString remoteLastModi;
  if (list.count() == 4 || list.count() == 2)
    remoteLastModi = list.at(0).trimmed();

  // ================================
  // 处理 mainnotes.json.zip
  // ================================
  if (file.contains("mainnotes.json.zip")) {
    pDir = privateDir + "KnotData";
    pFile = pFile.replace(".zip", "");
    kFile = iniDir + asFile.replace("KnotData/", "");
    kFile = kFile.replace(".zip", "");
    kFile = kFile.replace(remoteLastModi + "_", "");

    QString dec_file = m_Method->useDec(zFile);
    if (dec_file != "") zFile = dec_file;

    if (!m_Method->decompressFileWithZlib(zFile, pFile)) {
      mw_one->closeProgress();
      errorInfo =
          tr("Decompression failed. Please check in Preferences that the "
             "passwords are consistent across all platforms.");

      QMetaObject::invokeMethod(mw_one, [this]() {
        auto msg = std::make_unique<ShowMessage>(this);
        msg->showMsg("Knot", errorInfo, 1);
      });

      isPasswordError = true;
      QFile::remove(zFile);
      return;
    }

    if (!isPasswordError) {
      QFileInfo pFileInfo(pFile);
      QFileInfo kFileInfo(kFile);
      if (pFileInfo.lastModified() > kFileInfo.lastModified()) {
        QString tempFile = iniDir + "temp_notes_ini.tmp";
        if (QFile::exists(tempFile)) QFile::remove(tempFile);
        if (QFile::copy(pFile, tempFile)) {
          m_Method->upIniFile(tempFile, kFile);
          m_Method->delayDelFile(pFile);
          m_Method->delayDelFile(zFile);
        }
      }
    }
  }

  // ================================
  // 处理 .md.zip
  // ================================
  else if (file.contains(".md.zip")) {
    pDir = privateDir + "KnotData/memo";
    pFile = pFile.replace(".zip", "");
    kFile = iniDir + asFile.replace("KnotData/", "");
    kFile = kFile.replace(".zip", "");
    kFile = kFile.replace(remoteLastModi + "_", "");

    QString dec_file = m_Method->useDec(zFile);
    if (dec_file != "") zFile = dec_file;

    if (QFile::exists(zFile)) {
      if (!m_Method->decompressFileWithZlib(zFile, pFile)) {
        mw_one->closeProgress();
        errorInfo =
            tr("Decompression failed. Please check in Preferences that the "
               "passwords are consistent across all platforms.");

        QMetaObject::invokeMethod(mw_one, [this]() {
          auto msg = std::make_unique<ShowMessage>(this);
          msg->showMsg("Knot", errorInfo, 1);
        });

        isPasswordError = true;
        QFile::remove(zFile);
        QFile::remove(privateDir + "KnotData/mainnotes.json.zip");
        return;
      }
    }

    if (!isPasswordError) {
      QFileInfo pFileInfo(pFile);
      QFileInfo kFileInfo(kFile);
      if (pFileInfo.lastModified() > kFileInfo.lastModified()) {
        QFile::remove(kFile);
        QFile::copy(pFile, kFile);
        m_NotesList->m_dbManager.updateFileIndex(kFile);
        m_NotesList->m_graphController->parser()->updateNoteCache(kFile);
        m_Method->delayDelFile(pFile);
        m_Method->delayDelFile(zFile);
      }
    }
  }

  // ================================
  // 处理 .json.zip（非 mainnotes）
  // ================================
  else if (file.contains(".json.zip") && !file.contains("mainnotes.json.zip")) {
    pDir = privateDir + "KnotData/memo";
    pFile = pFile.replace(".zip", "");
    kFile = iniDir + asFile.replace("KnotData/", "");
    kFile = kFile.replace(".zip", "");
    kFile = kFile.replace(remoteLastModi + "_", "");

    QString dec_file = m_Method->useDec(zFile);
    if (dec_file != "") zFile = dec_file;

    if (QFile::exists(zFile)) {
      if (!m_Method->decompressFileWithZlib(zFile, pFile)) {
        mw_one->closeProgress();
        errorInfo =
            tr("Decompression failed. Please check in Preferences that the "
               "passwords are consistent across all platforms.");

        QMetaObject::invokeMethod(mw_one, [this]() {
          auto msg = std::make_unique<ShowMessage>(this);
          msg->showMsg("Knot", errorInfo, 1);
        });

        isPasswordError = true;
        QFile::remove(zFile);
        QFile::remove(privateDir + "KnotData/mainnotes.json.zip");
        return;
      }
    }

    if (!isPasswordError) {
      QFileInfo pFileInfo(pFile);
      QFileInfo kFileInfo(kFile);
      if (pFileInfo.lastModified() > kFileInfo.lastModified()) {
        QFile::remove(kFile);
        QFile::copy(pFile, kFile);
        m_Method->delayDelFile(pFile);
        m_Method->delayDelFile(zFile);
      }
    }
  }

  // ================================
  // 处理 .png 图片
  // ================================
  else if (file.contains(".png")) {
    pFile = m_Method->useDec(pFile);
    kFile = iniDir + asFile.replace("KnotData/", "");
    kFile = kFile.replace(remoteLastModi + "_", "");

    if (kFile.endsWith(".png.zip")) {
      kFile.replace(".png.zip", ".png");
    }

    QFileInfo pFileInfo(pFile);
    QFileInfo kFileInfo(kFile);
    if (pFileInfo.lastModified() > kFileInfo.lastModified()) {
      QFile::remove(kFile);
      QFile::copy(pFile, kFile);
      m_Method->delayDelFile(pFile);
    }
  }

  // 进度更新（安全跨线程）

  QString showText = "[" + QString::number(n_Files.fetchAndAddOrdered(1) + 1) +
                     "/" + QString::number(remoteFiles.size()) + "] " + pFile;
  QMetaObject::invokeMethod(m_Method, [=]() {
    m_Method->emit sigUpdateProgressAndText(showText, remoteFiles.size(),
                                            n_Files);
  });
}

void Notes::loadEmptyNote() {
  currentMDFile = "";
  MD2Html(currentMDFile);

  m_NotesList->noteTitle = "";
}

void Notes::on_editNote() {
  timerEditNote->stop();
  openEditUI();
}

void Notes::showNoteList() {
  QTimer::singleShot(100, mw_one, [this]() { openNotesUI(); });
}

void Notes::show_findText() {
#ifndef Q_OS_ANDROID

#endif
}

void Notes::loadNotesToUI() {
  init_all_notes();

  mw_one->isMemoVisible = true;
  mw_one->isReaderVisible = false;

  m_NotesList->set_memo_dir();

  if (tw->topLevelItemCount() == 0) {
    mui->lblNoteBook->setText(tr("Note Book"));
    mui->lblNoteList->setText(tr("Note List"));
    m_Method->closeInfoWindow();
    return;
  }

  m_NotesList->loadAllNoteBook();
  if (m_NotesList->getNoteBookCount() > 0) {
    if (!m_NotesList->setCurrentItemFromMDFile(currentMDFile)) {
      m_NotesList->setNoteBookCurrentIndex(0);
      m_NotesList->clickNoteBook();
    }

    m_NotesList->setNoteLabel();
  }

  m_Method->closeInfoWindow();

  if (isRequestOpenNoteEditor) {
    isRequestOpenNoteEditor = false;
    openEditUI();
  }
}

void Notes::openNotesUI() {
  if (mui->chkAutoSync->isChecked() && mui->chkWebDAV->isChecked()) {
    // 先清空旧连接，避免重复触发
    disconnect(m_Notes, &Notes::syncFinished, this, nullptr);
    // 绑定：等 sync 全部结束 → 再删除
    disconnect(m_Notes, &Notes::syncFinished, this, nullptr);
    connect(
        m_Notes, &Notes::syncFinished, this,
        [this]() { startBackgroundTaskDelAndClear(); },
        Qt::ConnectionType(Qt::QueuedConnection | Qt::SingleShotConnection));
    mw_one->execNeedSyncNotes();
  } else {
    loadNotesToUI();
  }

  mui->frameNoteList->show();
  mui->frameMain->hide();
}

void Notes::editNote() { openEditUI(); }
