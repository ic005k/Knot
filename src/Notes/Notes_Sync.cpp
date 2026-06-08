#include <QFutureWatcher>
#include <QtConcurrent>

#include "src/Notes/Notes.h"

void Notes::syncToWebDAV() {
  if (mui->chkAutoSync->isChecked() && mui->chkWebDAV->isChecked()) {
    if (notes_sync_files.count() > 0) {
      mw_one->showProgress();
      m_CloudBackup->createRemoteWebDAVDir();
      delRemoteFile(notes_sync_files);
      connect(
          m_CloudBackup, &CloudBackup::uploadAllFinished, this,
          [this]() { emit syncFinished(); },
          Qt::ConnectionType(Qt::QueuedConnection | Qt::SingleShotConnection));
      m_CloudBackup->uploadFilesToWebDAV(notes_sync_files);
      m_Method->setAccessCount(notes_sync_files.count());
    }
  }
}

void Notes::delRemoteFile(const QStringList& Files) {
  QStringList delFiles;
  for (int j = 0; j < Files.count(); j++) {
    QString syncFile = Files.at(j);
    QString baseFlag = m_Method->getBaseFlag(syncFile);
    if (!baseFlag.isEmpty()) {
      for (int i = 0; i < orgRemoteFiles.count(); i++) {
        QString orgFile = orgRemoteFiles.at(i);
        if (orgFile.contains(baseFlag)) {
          delFiles.append(orgFile);
        }
      }
    }
  }
  m_CloudBackup->deleteWebDAVFiles(delFiles);
}

void Notes::zipNoteToSyncList() {
  QFileInfo fi(currentMDFile);
  if (currentMDFile.endsWith(".md", Qt::CaseInsensitive)) {
    if (fi.exists() && fi.size() == 0) {
      QSaveFile saveFile(currentMDFile);
      if (saveFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        saveFile.write(" ");
        saveFile.commit();
      }
    }
  }

  QString lastModi = m_Method->getFileUTCString(currentMDFile);
  QString zipMD =
      privateDir + "KnotData/memo/" + lastModi + "_" + fi.fileName() + ".zip";
  QString zipJSON = privateDir + "KnotData/memo/" + lastModi + "_" +
                    fi.baseName() + ".json.zip";

  if (!m_Method->compressFileWithZlib(currentMDFile, zipMD,
                                      Z_DEFAULT_COMPRESSION)) {
    errorInfo = tr("An error occurred while compressing the file.");
    auto msg = std::make_unique<ShowMessage>(this);
    msg->showMsg("Knot", errorInfo, 1);
    return;
  }

  QString enc_file = m_Method->useEnc(zipMD);
  if (enc_file != "") zipMD = enc_file;
  appendToSyncList(zipMD);

  QString json = getCurrentJSON(currentMDFile);
  if (QFile::exists(json)) {
    if (!m_Method->compressFileWithZlib(json, zipJSON, Z_DEFAULT_COMPRESSION)) {
      errorInfo = tr("An error occurred while compressing the file.");
      auto msg = std::make_unique<ShowMessage>(this);
      msg->showMsg("Knot", errorInfo, 1);
      return;
    }
    QString enc_json = m_Method->useEnc(zipJSON);
    if (enc_json != "") zipJSON = enc_json;
    appendToSyncList(zipJSON);
  }
}

QString Notes::getCurrentJSON(const QString& md) {
  QFileInfo fi(md);
  return iniDir + "memo/" + fi.baseName() + ".json";
}

void Notes::updateMDFileToSyncLists() { zipNoteToSyncList(); }
void Notes::updateMainnotesIniToSyncLists() { /* 原有逻辑 */ }

void Notes::appendToSyncList(QString file) {
  QString baseFlag = m_Method->getBaseFlag(file);
  notes_sync_files.removeIf([&baseFlag](const QString& f) {
    return m_Method->getBaseFlag(f) == baseFlag;
  });
  notes_sync_files.append(file);
}

// 后台任务
void Notes::startBackgroundTaskUpdateNoteIndex(QString mdFile) {
  QFuture<void> future = QtConcurrent::run([=]() {
    m_NotesList->m_dbManager.updateFileIndex(mdFile);
    m_NotesList->m_graphController->parser()->updateNoteCache(mdFile);
  });
  QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
  connect(watcher, &QFutureWatcher<void>::finished, this,
          [=]() { watcher->deleteLater(); });
  watcher->setFuture(future);
}

void Notes::startBackgroundTaskUpdateNoteIndexes(QStringList mdFileList) {
  QFuture<void> future = QtConcurrent::run(
      [=]() { m_NotesList->m_dbManager.updateFileIndexes(mdFileList); });
  QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
  connect(watcher, &QFutureWatcher<void>::finished, this,
          [=]() { watcher->deleteLater(); });
  watcher->setFuture(future);
}

void Notes::startBackgroundTaskDelAndClear() {
  if (m_NotesList->isDelNoteRecycle) mw_one->showProgress();
  QString fullPath = iniDir + "memo";
  QString dbFile = privateDir + "md_database_v3.db";

  QFuture<void> future = QtConcurrent::run([=]() {
    m_NotesList->needDelNotes();
    DatabaseManager localDbManager;
    localDbManager.initDatabase(dbFile);
    localDbManager.cleanMissingFileRecords(fullPath);
    localDbManager.closeDatabase();
  });

  QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
  connect(watcher, &QFutureWatcher<void>::finished, this, [=]() {
    if (!m_NotesList->isDelNoteRecycle) {
      m_NotesList->delRemoteWebDAVFiles();
      loadNotesToUI();
    } else
      m_NotesList->isDelNoteRecycle = false;
    watcher->deleteLater();
    mw_one->closeProgress();
  });
  watcher->setFuture(future);
}