#include "NotesList.h"

void NotesList::genRecentOpenMenu() {
  menuRecentOpen = new QMenu(this);
  menuRecentOpen->setMaximumWidth(mw_one->width());
  int count = listRecentOpen.count();
  for (int i = 0; i < count; i++) {
    QString name, file, item;
    item = listRecentOpen.at(i);
    QStringList list = item.split("===");
    name = list.at(0);
    file = iniDir + list.at(1);

    if (QFile::exists(file)) {
      QFontMetrics fm(mw_one->font());
      QString txt = QString::number(i + 1) + " . " + name;
      QString menuTitle =
          fm.elidedText(txt, Qt::ElideRight, mw_one->width() - 30);
      QAction* act = new QAction(menuTitle, menuRecentOpen);
      menuRecentOpen->addAction(act);

      connect(act, &QAction::triggered, this, [=]() {
        currentMDFile = file;
        noteTitle = name;

        saveCurrentNoteInfo();
        isExecRecentOpen = true;
        setCurrentItemFromMDFile(currentMDFile);

        /*QTimer::singleShot(200, this, [=]() {
#ifdef Q_OS_ANDROID
          mw_one->on_btnOpenNote_clicked();
#else
              mw_one->on_btnEditNote_clicked();
#endif
        });*/
      });
    }
  }

  menuRecentOpen->setStyleSheet(m_Method->qssMenu);

  int x = 0;
  x = mw_one->geometry().x() + 2;
  int y = mw_one->geometry().y() + mui->btnRecentOpen->height() + 4;
  QPoint pos(x, y);
  menuRecentOpen->exec(pos);
}

void NotesList::on_actionAdd_NoteBook_triggered() {
  QInputDialog* dlg = m_Method->inputDialog(
      tr("New Notebook"), tr("Please enter notebook name:"), "");

  connect(dlg, &QDialog::accepted, this, [=]() {
    // 点击确定：拿到输入内容
    QString inputName = dlg->textValue();
    if (!inputName.isEmpty()) {
      notebookName = inputName;

      QTreeWidgetItem* item = new QTreeWidgetItem();
      item->setText(0, notebookName);
      item->setText(2, "#FF0000");
      tw->addTopLevelItem(item);
      tw->setCurrentItem(item);

      loadAllNoteBook();

      int count = m_Method->getCountFromQW(mui->qwNoteBook);
      int index = 0;
      for (int i = 0; i < count; i++) {
        if (pNoteBookItems.at(i) == tw->currentItem()) {
          index = i;
          break;
        }
      }
      setNoteBookCurrentIndex(index);
      clickNoteBook();

      saveNotesList();
    }
    dlg->deleteLater();  // 销毁对象
  });

  connect(dlg, &QDialog::rejected, this, [=]() { dlg->deleteLater(); });
}

void NotesList::on_actionDel_NoteBook_triggered() {
  int index = getNoteBookCurrentIndex();
  if (index < 0) return;

  tw->setCurrentItem(pNoteBookItems.at(index));
  on_btnDel_clicked();

  loadAllNoteBook();

  int count = getNoteBookCount();
  for (int i = 0; i < count; i++) {
    if (tw->currentItem() == pNoteBookItems.at(i)) {
      index = i;
      break;
    }
  }
  setNoteBookCurrentIndex(index);

  setNoteLabel();

  saveNotesList();

  if (count == 0) {
    m_Notes->loadEmptyNote();
  }
}

void NotesList::on_actionRename_NoteBook_triggered() {
  int index = getNoteBookCurrentIndex();
  if (index < 0) return;

  on_btnRename_clicked();
}

void NotesList::on_actionAdd_Note_triggered() {
  int notebookIndex = getNoteBookCurrentIndex();
  if (notebookIndex < 0) {
    auto msg = std::make_unique<ShowMessage>(mw_one);
    msg->showMsg("Knot",
                 tr("Please create a new notebook first, and then create "
                    "new notes."),
                 1);
    return;
  }

  if (tw->topLevelItemCount() == 0) return;

  tw->setCurrentItem(pNoteBookItems.at(notebookIndex));

  QString noteFile = "memo/" + m_Notes->getDateTimeStr() + "_" +
                     m_Method->generateRandom3() + ".md";
  QTreeWidgetItem* parentitem = tw->currentItem();

  QTreeWidgetItem* item1 = new QTreeWidgetItem(parentitem);
  item1->setText(0, tr("Untitled Note"));
  item1->setText(1, noteFile);

  QTextEdit edit;
  edit.append("");
  TextEditToFile(&edit, iniDir + noteFile);

  pNoteItems.append(item1);

  tw->setCurrentItem(item1);
  noteName = item1->text(0);

  m_Method->addItemToQW(mui->qwNoteList, noteName, "", "", noteFile, 0);

  int count = getNotesListCount();
  setNotesListCurrentIndex(count - 1);

  clickNoteList();
  m_Notes->updateMDFileToSyncLists();

  setNoteLabel();

  saveNotesList();
  updateAllNoteIndexManager();

  mw_one->on_btnEditNote_clicked();
}

void NotesList::on_actionDel_Note_triggered() {
  if (getNotesListCount() == 0) return;

  int notebookIndex = getNoteBookCurrentIndex();
  int notelistIndex = getNotesListCurrentIndex();

  if (notebookIndex < 0) return;
  if (notelistIndex < 0) return;

  on_btnDel_clicked();

  int count = getNotesListCount();

  setNoteLabel();

  saveNotesList();

  if (count == 0) {
    m_Notes->loadEmptyNote();
  }
}

void NotesList::on_actionRename_Note_triggered() {
  int notebookIndex = getNoteBookCurrentIndex();
  int noteIndex = getNotesListCurrentIndex();
  if (notebookIndex < 0) return;
  if (noteIndex < 0) return;

  on_btnRename_clicked();
}

void NotesList::on_actionMoveUp_Note_triggered() {
  int indexBook = getNoteBookCurrentIndex();
  int indexNote = getNotesListCurrentIndex();
  if (indexBook < 0) return;
  if (indexNote <= 0) return;

  setNoteBookCurrentItem();
  tw->setCurrentItem(tw->currentItem()->child(indexNote));
  on_btnUp_clicked();

  clickNoteBook();
  setNotesListCurrentIndex(indexNote - 1);
}

void NotesList::on_actionMoveDown_Note_triggered() {
  int indexBook = getNoteBookCurrentIndex();
  int indexNote = getNotesListCurrentIndex();
  if (indexBook < 0) return;
  if (indexNote < 0) return;
  if (indexNote + 1 == getNotesListCount()) return;

  setNoteBookCurrentItem();
  tw->setCurrentItem(tw->currentItem()->child(indexNote));
  on_btnDown_clicked();

  clickNoteBook();
  setNotesListCurrentIndex(indexNote + 1);
}

void NotesList::on_actionImport_Note_triggered() {
  int indexBook = getNoteBookCurrentIndex();
  if (indexBook < 0) return;

  QTreeWidgetItem* oldItem = tw->currentItem();
  bool isNoteBook = pNoteBookItems.contains(oldItem);
  if (!isNoteBook) {
    tw->setCurrentItem(oldItem->parent());
  }

  int fileCount = on_btnImport_clicked();

  while (!isImportFilesEnd) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    QThread::msleep(1);
  }

  mw_one->safeCloseProgress();

  if (fileCount > 0) {
    clickNoteBook();

    clickNoteList();
    saveNotesList();

    updateAllNoteIndexManager();
  }
}

void NotesList::on_actionExport_Note_triggered() {
  int indexBook = getNoteBookCurrentIndex();
  int indexNote = getNotesListCurrentIndex();

  if (indexBook < 0) return;
  if (indexNote < 0) return;

  on_btnExport_clicked();
}

void NotesList::show_NoteBookPopMenu(int qmlIndex) {
  QMenu* mainMenu = new QMenu(this);
  QAction* actNew = new QAction(tr("New Sub NoteBook"));

  connect(actNew, &QAction::triggered, this,
          [this, qmlIndex]() { slotCreateSubNotebook(qmlIndex); });

  mainMenu->addAction(actNew);
  mainMenu->setStyleSheet(m_Method->qssMenu);

  QPoint pos(mw_one->geometry().x() + mui->qwNoteBook->geometry().x() + 2,
             mw_one->geometry().y() + 45);

  mainMenu->exec(pos);
}

void NotesList::init_NoteBookMenu(QMenu* mainMenu) {
  QAction* actNew = new QAction(tr("New NoteBook"));
  QAction* actNewSub = new QAction(tr("New Sub NoteBook"));
  QAction* actDel = new QAction(tr("Del NoteBook"));
  QAction* actRename = new QAction(tr("Rename NoteBook"));
  QAction* actMoveUp = new QAction(tr("Move Up"));
  QAction* actMoveDown = new QAction(tr("Move Down"));
  QAction* actSetColorFlag = new QAction(tr("Set Color Marker"));
  actSetColorFlag->setEnabled(isActColorFlagStatus);
  QAction* actStatistics = new QAction(tr("Statistics"));
  QAction* actRebuildSearchIndex =
      new QAction(tr("Rebuild Search Database Index"));

  connect(actNew, &QAction::triggered, this,
          &NotesList::on_actionAdd_NoteBook_triggered);

  connect(actNewSub, &QAction::triggered, this, [this]() {
    int qmlIndex = getNoteBookCurrentIndex();
    if (qmlIndex < 0) return;

    slotCreateSubNotebook(qmlIndex);
  });

  connect(actDel, &QAction::triggered, this,
          &NotesList::on_actionDel_NoteBook_triggered);
  connect(actRename, &QAction::triggered, this,
          &NotesList::on_actionRename_NoteBook_triggered);

  connect(actMoveUp, &QAction::triggered, this,
          &NotesList::on_actionMoveUp_NoteBook_triggered);
  connect(actMoveDown, &QAction::triggered, this,
          &NotesList::on_actionMoveDown_NoteBook_triggered);
  connect(actSetColorFlag, &QAction::triggered, this,
          &NotesList::on_actionSetColorFlag);

  connect(actStatistics, &QAction::triggered, this,
          &NotesList::on_actionStatistics);

  connect(actRebuildSearchIndex, &QAction::triggered, this, [this]() {
    auto msg = std::make_unique<ShowMessage>(mw_one);
    if (msg->showMsg(appName,
                     tr("Rebuilding the index will take some time. Click OK "
                        "to start."),
                     2)) {
      mw_one->showProgress();

      if (isLocalAIModel) {
        rebuilderNotesVector();
      }
    }
  });

  mainMenu->addAction(actNew);
  mainMenu->addAction(actNewSub);
  mainMenu->addAction(actRename);
  mainMenu->addAction(actDel);

  mainMenu->addAction(actMoveUp);
  mainMenu->addAction(actMoveDown);

  mainMenu->addAction(actStatistics);
  mainMenu->addAction(actRebuildSearchIndex);
  mainMenu->addAction(actSetColorFlag);

  actRename->setVisible(false);
  actDel->setVisible(false);
  actMoveUp->setVisible(false);
  actMoveDown->setVisible(false);
  actStatistics->setVisible(true);
  actRebuildSearchIndex->setVisible(false);

  mainMenu->setStyleSheet(m_Method->qssMenu);
}

void NotesList::init_NotesListMenu(QMenu* mainMenu) {
  QAction* actNew = new QAction(tr("New Note"));
  QAction* actDel = new QAction(tr("Del Note"));
  QAction* actRename = new QAction(tr("Rename Note"));
  QAction* actMoveUp = new QAction(tr("Move Up"));
  QAction* actMoveDown = new QAction(tr("Move Down"));
  QAction* actImport = new QAction(tr("Import"));
  QAction* actExport = new QAction(tr("Export"));
  QAction* actShare = new QAction(tr("Share"));
  QAction* actCopyLink = new QAction(tr("Copy Note Link"));
  QAction* actRelationshipGraph = new QAction(tr("Relationship Graph"));
  QAction* actModificationHistory = new QAction(tr("Modification History"));

  connect(actNew, &QAction::triggered, this,
          &NotesList::on_actionAdd_Note_triggered);
  connect(actDel, &QAction::triggered, this,
          &NotesList::on_actionDel_Note_triggered);
  connect(actRename, &QAction::triggered, this,
          &NotesList::on_actionRename_Note_triggered);

  connect(actMoveUp, &QAction::triggered, this,
          &NotesList::on_actionMoveUp_Note_triggered);
  connect(actMoveDown, &QAction::triggered, this,
          &NotesList::on_actionMoveDown_Note_triggered);

  connect(actImport, &QAction::triggered, this,
          &NotesList::on_actionImport_Note_triggered);
  connect(actExport, &QAction::triggered, this,
          &NotesList::on_actionExport_Note_triggered);
  connect(actShare, &QAction::triggered, this,
          &NotesList::on_actionShareNoteFile);
  connect(actCopyLink, &QAction::triggered, this,
          &NotesList::on_actionCopyNoteLink);
  connect(actRelationshipGraph, &QAction::triggered, this,
          &NotesList::on_actionRelationshipGraph);
  connect(actModificationHistory, &QAction::triggered, this,
          &NotesList::on_actionModificationHistory);

  mainMenu->addAction(actNew);
  mainMenu->addAction(actRename);
  mainMenu->addAction(actDel);

  mainMenu->addAction(actImport);
  mainMenu->addAction(actExport);
  mainMenu->addAction(actShare);

  mainMenu->addAction(actMoveUp);
  mainMenu->addAction(actMoveDown);

  mainMenu->addAction(actCopyLink);
  mainMenu->addAction(actRelationshipGraph);
  mainMenu->addAction(actModificationHistory);

  actRename->setVisible(false);
  actDel->setVisible(false);
  actMoveUp->setVisible(false);
  actMoveDown->setVisible(false);

#ifdef Q_OS_ANDROID
  actShare->setVisible(true);
#else
  actShare->setVisible(false);
#endif

  mainMenu->setStyleSheet(m_Method->qssMenu);
}

void NotesList::on_actionModificationHistory() {
  // 读取差异记录
  QList<QJsonObject> allDiffs =
      m_Notes->loadAllDiffs(m_Notes->getCurrentJSON(currentMDFile));

  // 按修改时间排序（最新的在前面）
  std::sort(allDiffs.begin(), allDiffs.end(),
            [](const QJsonObject& a, const QJsonObject& b) {
              return a["modifyTime"].toString() > b["modifyTime"].toString();
            });

  // 获取 QML 根对象（即 NoteVersionList.qml 中的 Rectangle）
  QQuickItem* rootItem = mui->qwNoteVersion->rootObject();
  if (!rootItem) {
    qWarning() << "获取 QML 根对象失败";
    return;
  }

  mui->frameNoteList->hide();
  mui->frameDiff->show();

  // 获取 QML 中的 ListModel（id: noteVersionModel）
  QObject* versionModel = rootItem->findChild<QObject*>("noteVersionModel");
  if (!versionModel) {
    qWarning() << "获取 noteVersionModel 失败";
    return;
  }

  // 清空旧数据（调用 QML 中的 clearModel() 方法）
  QMetaObject::invokeMethod(versionModel, "clearModel");

  noteDiffTime.clear();
  noteDiffHtml.clear();
  noteDiffPatch.clear();

  // 遍历展示（例如在QListView中显示版本列表）
  foreach (const QJsonObject& diff, allDiffs) {
    QString version = diff["version"].toString();  // 版本（笔记修改时间）
    QString time = diff["modifyTime"].toString();  // 修改时间
    QString html = diff["htmlDiff"].toString();    // 可视化内容
    QString patch = diff["patch"].toString();

    noteDiffTime.append(time);
    noteDiffHtml.append(html);
    noteDiffPatch.append(patch);

    QMetaObject::invokeMethod(
        versionModel, "addRecord",
        Q_ARG(QVariant, version)  // 传入修改时间（QVariant 适配 QML 类型）
    );
  }

  setNoteDiffHtmlToQML("");
}

void NotesList::on_actionCopyNoteLink() {
  int index = m_Method->getCurrentIndexFromQW(mui->qwNoteList);
  QString file = noteModel->getText3(index);
  QString name = noteModel->getText0(index);
  QString strlink = "[" + name + "](" + file + ")";
  QClipboard* clipboard = QApplication::clipboard();
  clipboard->setText(strlink);

  QString res = m_Method->escapeAllHtml(strlink);

  auto msg = std::make_unique<ShowMessage>(mw_one);
  msg->showMsg(appName, res, 1);
}

void NotesList::on_actionShareNoteFile() {
  if (QFile::exists(currentMDFile)) {
    mw_one->m_ReceiveShare->shareImage(tr("Share to"), currentMDFile, "*/*");
  }
}

void NotesList::on_actionMoveDown_NoteBook_triggered() {
  int index = getNoteBookCurrentIndex();

  if (index < 0) return;

  if (index == getNoteBookCount() - 1) return;

  setNoteBookCurrentItem();
  on_btnDown_clicked();

  int newIndex = getNoteBookIndex_twToqml();

  loadAllNoteBook();
  setNoteBookCurrentIndex(newIndex);
  clickNoteBook();
}

void NotesList::on_actionMoveUp_NoteBook_triggered() {
  int index = getNoteBookCurrentIndex();
  if (index <= 0) return;

  setNoteBookCurrentItem();
  on_btnUp_clicked();

  int newIndex = getNoteBookIndex_twToqml();

  loadAllNoteBook();
  setNoteBookCurrentIndex(newIndex);
  clickNoteBook();
}

void NotesList::on_actionRelationshipGraph() {
  QFileInfo fi(currentMDFile);
  if (!fi.exists()) return;

  mui->frameNotesGraph->show();
  mui->frameNoteList->hide();

  QTimer::singleShot(100, this, []() { mw_one->showProgress(); });

  if (m_graphController) {
    m_graphController->setCurrentNotePath(currentMDFile);

  } else {
    mw_one->safeCloseProgress();
  }
}

void NotesList::on_actionSetColorFlag() {
  QString color_0;
  color_0 = m_Method->getCustomColor();
  if (color_0.isNull()) return;

  QTreeWidgetItem* itemTop;
  QTreeWidgetItem* item = tw->currentItem();
  bool isNoteBook = pNoteBookItems.contains(item);
  if (!isNoteBook) {
    itemTop = item->parent();
  } else
    itemTop = item;

  itemTop->setText(2, color_0);
  setColorFlag(color_0);
  saveNotesList();
}

void NotesList::on_actionStatistics() {
  mw_one->showProgress();
  int countNoteBook = getNoteBookCount();

  int webDAVCount = m_Method->getAccessCount();
  QString memoDir = iniDir + "memo/images/";
  QString localAppName = appName;  // 单独赋值，便于值捕获

  // 定义一个可被Lambda捕获的变量（用于存储后台统计结果）
  int* imgCountPtr = new int(0);
  int* notesCountPtr = new int(0);

  QFuture<void> future = QtConcurrent::run([=]() {
    *imgCountPtr = countMdFilesImages(memoDir);
    *notesCountPtr = getAllNotePaths().count();
  });

  QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
  connect(watcher, &QFutureWatcher<void>::finished, this, [=]() {
    qDebug() << "Statistics task completed.";

    QString strAccessCount =
        tr("Access WebDAV:") + QString::number(webDAVCount) + "t/30min";

    mw_one->safeCloseProgress();

    auto msg = std::make_unique<ShowMessage>(mw_one);
    msg->showMsg(localAppName,
                 tr("NoteBook:") + QString::number(countNoteBook) + "\n\n" +
                     tr("Local Notes:") + QString::number(*notesCountPtr) +
                     "\n\n" + tr("Remote Notes:") +
                     QString::number(m_CloudBackup->m_currentRemoteNotesCount) +
                     "\n\n" + tr("Images:") + QString::number(*imgCountPtr) +
                     "\n\n" + strAccessCount,
                 1);

    delete imgCountPtr;
    watcher->deleteLater();
  });
  watcher->setFuture(future);
}

void NotesList::showNoteBookMenu(int x, int y) {
  menuNoteBook = new QMenu(this);
  init_NoteBookMenu(menuNoteBook);

  QPoint pos(mw_one->geometry().x() + x, mw_one->geometry().y() + y);
  menuNoteBook->exec(pos);
}

void NotesList::showNotsListMenu(int x, int y) {
  menuNoteList = new QMenu(this);
  init_NotesListMenu(menuNoteList);

  QPoint pos(mw_one->geometry().x() + x, mw_one->geometry().y() + y);
  menuNoteList->exec(pos);
}

void NotesList::initRecentOpen() {
  QSettings iniFile(iniDir + "recentopen.ini", QSettings::IniFormat);

  listRecentOpen.clear();
  int count = iniFile.value("/RecentOpen/count", 0).toInt();
  for (int i = 0; i < count; i++) {
    QString list =
        iniFile.value("/RecentOpen/list" + QString::number(i)).toString();
    QFile file(iniDir + list.split("===").at(1));
    if (file.exists()) {
      listRecentOpen.append(list);
    }
  }

  saveRecentOpen();
}

void NotesList::saveRecentOpen() {
  QSettings iniFile(iniDir + "recentopen.ini", QSettings::IniFormat);

  iniFile.setValue("/RecentOpen/count", listRecentOpen.count());
  for (int i = 0; i < listRecentOpen.count(); i++) {
    iniFile.setValue("/RecentOpen/list" + QString::number(i),
                     listRecentOpen.at(i));
  }
}

void NotesList::refreshRecentOpen(QString name) {
  QString strmd = currentMDFile;
  strmd = strmd.replace(iniDir, "").trimmed();

  listRecentOpen.insert(0, name + "===" + strmd);
  listRecentOpen = m_Method->removeDuplicatesFromQStringList(listRecentOpen);

  int count = listRecentOpen.count();
  if (count > 15) {
    listRecentOpen.removeAt(count - 1);
  }
}

void NotesList::slotCreateSubNotebook(int qmlIndex) {
  // 校验：拿到长按的父笔记本
  if (qmlIndex < 0 || qmlIndex >= pNoteBookItems.size()) return;

  QTreeWidgetItem* parentItem = pNoteBookItems.at(qmlIndex);
  if (!parentItem) return;

  QInputDialog* dlg = m_Method->inputDialog(
      tr("New Sub Notebook"), tr("Please enter sub notebook name:"), "");

  connect(dlg, &QDialog::accepted, this, [=]() {
    // 点击确定：拿到输入内容
    QString inputName = dlg->textValue();
    if (!inputName.isEmpty()) {
      //  在当前笔记本下 创建子笔记本
      QTreeWidgetItem* newItem = new QTreeWidgetItem(parentItem);
      newItem->setText(0, inputName.trimmed());
      newItem->setText(2, "#e5e1e1");
      newItem->setForeground(0, Qt::red);

      // 展开父节点，保证能看到新建的子笔记本
      parentItem->setExpanded(true);
      tw->setCurrentItem(newItem);

      //  刷新 QML 笔记本列表
      loadAllNoteBook();

      //  自动选中新建的项
      int newIndex = pNoteBookItems.indexOf(newItem);
      if (newIndex >= 0) {
        setNoteBookCurrentIndex(newIndex);
        clickNoteBook();
      }

      //  保存数据
      saveNotesList();
    }
    dlg->deleteLater();  // 销毁对象
  });

  connect(dlg, &QDialog::rejected, this, [=]() { dlg->deleteLater(); });
}

void NotesList::rebuilderNotesVector() {
  if (!isLocalAIModel) {
    mui->lblVectorStatus->hide();
    return;
  }

  // ✅ 防重入：如果已经在重建，直接返回
  if (!m_rebuildMutex.tryLock()) {
    qInfo() << "[Embedding] 重建任务已在运行，跳过本次触发";
    return;
  }

  // ✅ 每次新任务开始前重置计数器
  m_Notes->m_skipCount.storeRelaxed(0);

  // ✅ 重置取消标志
  m_rebuildCancelled.store(false, std::memory_order_release);

  mw_one->setVectorStatus(1);

  auto watcher = new QFutureWatcher<void>(this);
  connect(watcher, &QFutureWatcher<void>::finished, this, [watcher, this]() {
    qInfo() << "[Embedding] 重建向量任务结束, cancelled:"
            << m_rebuildCancelled.load();

    // 检查后台线程是否抛出了异常
    if (watcher->isCanceled()) {
      // QtConcurrent 异常会通过 watcher 传递
      mw_one->setVectorStatus(2);
    } else {
      mw_one->setVectorStatus(0);
    }

    mw_one->safeCloseProgress();
    watcher->deleteLater();
    m_rebuildMutex.unlock();  // ✅ 释放锁，允许下次触发
  });

  auto future = QtConcurrent::run([this]() {
    try {
      QStringList allNotes = getAllNotePaths();
      for (int i = 0; i < allNotes.size(); ++i) {
        // ✅ 每个笔记处理前检查取消标志
        if (m_rebuildCancelled.load(std::memory_order_acquire)) {
          qInfo() << "[Embedding] 重建被取消, 已完成:" << i << "/"
                  << allNotes.size();
          break;
        }
        m_Notes->syncNoteVectorsBatchToDb(allNotes.at(i));
      }
    } catch (const std::exception& e) {
      qCritical() << "[Embedding] Rebuild failed:" << e.what();
    }
  });
  watcher->setFuture(future);
}

void NotesList::cancelRebuildNotesVector() {
  m_rebuildCancelled.store(true, std::memory_order_release);
  qInfo() << "[Embedding] 已发送重建取消信号";

  mw_one->setVectorStatus(0);
}

bool NotesList::waitForRebuildFinished(int timeoutMs) {
  QElapsedTimer t;
  t.start();
  while (true) {
    if (m_rebuildMutex.tryLock()) {
      m_rebuildMutex.unlock();
      return true;  // 锁已释放 = finished 回调已执行 = 线程安全退出
    }
    // timeoutMs == -1 表示无限等待，否则检查是否超时
    if (timeoutMs >= 0 && t.elapsed() >= timeoutMs) {
      return false;
    }

    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    QThread::msleep(50);
  }
}

void NotesList::safeExitLlama() {
  mw_one->showProgress();
  cancelRebuildNotesVector();

  // ⭐ 阻塞等待后台线程完全退出（-1：不带超时保护）
  if (!waitForRebuildFinished(-1)) {
    qWarning() << "Vector rebuild force-stopped on close";
  }
  mw_one->safeCloseProgress();
}
