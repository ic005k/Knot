#include "NotesList.h"

// 节点双击事件处理（打开对应的笔记）
void NotesList::onNoteNodeDoubleClicked(const QString& filePath) {
  qDebug() << "打开笔记：" << filePath;
  QFileInfo fi(filePath);
  if (fi.exists()) {
    currentMDFile = filePath;
  } else {
    currentMDFile = iniDir + filePath;
  }

  m_Notes->previewNote();
}

void NotesList::onSearchFinished() {
  if (!watcher) return;

  // ▶️ 正式获取结果
  const ResultsMap results = watcher->result();

  // ▶️ 数据安全检查
  if (results.isEmpty()) {
    mw_one->closeProgress();
    searchResultList.clear();
    mui->btnFindNextNote->setEnabled(false);
    mui->btnFindPreviousNote->setEnabled(false);
    mui->lblShowLineSn->setText("0");
    mui->lblFindNoteCount->setText("0");
    auto msg = std::make_unique<ShowMessage>(this);
    msg->showMsg("Knot", tr("No match was found."), 1);

    return;
  }

  m_Method->clearAllBakList(mui->qwNotesSearchResult);

  // ▶️ 处理搜索结果

  for (auto it = results.constBegin(); it != results.constEnd(); ++it) {
    const QString& filePath = it.key();
    const QList<int> lines = it.value().lineNumbers;

    if (!isAndroid) {
      qDebug() << "文件：" << it.key();
      qDebug() << "匹配行号：" << it.value().lineNumbers;
    }

    QString strLineSn;
    int linesCount = lines.count();
    for (int i = 0; i < linesCount; i++) {
      strLineSn = strLineSn + " " + QString::number(lines.at(i));
    }
    strLineSn = strLineSn.trimmed();

    if (!recycleNotesList.contains(filePath))
      searchResultList.append(filePath + "-==-" + strLineSn + "-==-" +
                              QString::number(linesCount));
  }

  if (searchResultList.count() > 0) {
    mui->btnFindNextNote->setEnabled(true);
    mui->btnFindPreviousNote->setEnabled(true);
    mui->lblFindNoteCount->setText(QString::number(searchResultList.count()));

    goNext();
  }

  mw_one->closeProgress();
}

void NotesList::onSearchTextChanged(const QString& text) {
  QTimer::singleShot(300, this, [this, text]() {  // 防抖处理
    auto results =
        m_dbManager.searchDocuments(text, m_Notes->m_NoteIndexManager);
    m_searchModel.setResults(results);
    mui->lblNoteSearchResult->setText(tr("Note Search Results:") +
                                      QString::number(results.count()));
  });
}

void NotesList::on_btnRecycle() {
  ui->frame0->hide();
  ui->frame1->show();
  setWinPos();
  twrb->setFocus();
  twrb->setCurrentItem(twrb->topLevelItem(0));
}

void NotesList::on_btnBack_clicked() {
  ui->frame1->hide();
  ui->frame0->show();
  setWinPos();
  tw->setFocus();
}

void NotesList::on_btnRestore_clicked() {
  QTreeWidgetItem* curItem = twrb->currentItem();
  if (curItem->parent() == NULL) return;

  if (moveItem(twrb)) {
    resetQML_List();
    clickNoteList();
    if (!ui->frame1->isHidden()) {
      on_btnBack_clicked();
    }
    if (!mui->frameNoteRecycle->isHidden()) {
      mw_one->on_btnBackNoteRecycle_clicked();
    }
  } else
    return;

  saveNotesList();
}

void NotesList::on_btnBatchRestore_clicked() {
  // ***************** 第一步：完整复用QML选中索引获取逻辑 *****************
  if (!mui || !mui->qwNoteRecycle) {
    qWarning() << "QQuickWidget（qwNoteRecycle）无效，无法执行批量移动";
    return;
  }

  QQuickWidget* quickWidget = mui->qwNoteRecycle;
  QObject* qmlRootObj = quickWidget->rootObject();
  if (!qmlRootObj) {
    qWarning() << "获取QML根对象失败，请检查QML是否正确加载";
    return;
  }

  // 调用QML接口getSelectedIndexes()，获取选中索引列表
  QVariant selectedIndexesVar;
  bool invokeSuccess =
      QMetaObject::invokeMethod(qmlRootObj, "getSelectedIndexes",
                                Q_RETURN_ARG(QVariant, selectedIndexesVar));

  if (!invokeSuccess || !selectedIndexesVar.canConvert<QVariantList>()) {
    qWarning() << "调用QML方法getSelectedIndexes失败";
    return;
  }

  QVariantList selectedIndexes = selectedIndexesVar.toList();
  if (selectedIndexes.isEmpty()) {
    auto msg = std::make_unique<ShowMessage>(this);
    msg->showMsg("Knot", tr("Please select at least one item to restore."), 1);
    return;
  }

  // ***************** 第二步：Qt 6兼容版，设置twrb多选状态（最简洁）
  // 1. 清空twrb原有选中状态，避免残留干扰
  twrb->clearSelection();

  // 2. 获取twrb的唯一顶级项目
  QTreeWidgetItem* twrbTopItem = twrb->topLevelItem(0);
  if (!twrbTopItem) {
    qWarning() << "twrb无顶级项目，无法执行批量移动";
    return;
  }

  // 3. 遍历QML选中索引，一一映射设置twrb子项目的多选状态
  foreach (QVariant indexVar, selectedIndexes) {
    int qmlIndex = indexVar.toInt();
    if (qmlIndex >= 0 && qmlIndex < twrbTopItem->childCount()) {
      QTreeWidgetItem* targetTWRBItem = twrbTopItem->child(qmlIndex);
      if (targetTWRBItem) {
        // 直接操作QTreeWidgetItem，兼容Qt 5/6
        targetTWRBItem->setSelected(true);
      }
    }
  }

  // ***************** 第三步：复用原有批量移动逻辑 *****************
  QList<QTreeWidgetItem*> selectedItems = twrb->selectedItems();
  if (selectedItems.isEmpty()) return;
  if (getNoteBookCount() == 0) return;

  bool allMoveSuccess = true;
  foreach (QTreeWidgetItem* item, selectedItems) {
    if (isStopMoveNote) {
      isStopMoveNote = false;
      allMoveSuccess = false;
      break;
    }

    if (item == NULL || item->parent() == NULL) {
      allMoveSuccess = false;
      continue;
    }

    twrb->setCurrentItem(item);
    if (!moveItem(twrb)) {
      allMoveSuccess = false;
      continue;
    }
  }

  if (allMoveSuccess) {
    // 清空QML选中状态
    QObject* qmlRootObj = quickWidget->rootObject();
    if (qmlRootObj) {
      QMetaObject::invokeMethod(qmlRootObj, "resetQMLToNewState",
                                Qt::DirectConnection);
    }
    resetQML_List();
    clickNoteList();
    if (!ui->frame1->isHidden()) {
      on_btnBack_clicked();
    }
    if (!mui->frameNoteRecycle->isHidden()) {
      mw_one->on_btnBackNoteRecycle_clicked();
    }
  }

  saveNotesList();
}

void NotesList::on_btnBatchDel_Recycle_clicked() {
  // 1. 通过QQuickWidget直接获取QML根对象
  if (!mui || !mui->qwNoteRecycle) {
    qWarning() << "QQuickWidget（qwNoteRecycle）无效，无法执行批量删除";
    return;
  }

  QQuickWidget* quickWidget = mui->qwNoteRecycle;
  QObject* qmlRootObj = quickWidget->rootObject();
  if (!qmlRootObj) {
    qWarning() << "获取QML根对象失败，请检查QML是否正确加载";
    return;
  }

  // 2. 调用QML接口getSelectedIndexes()，获取选中索引列表
  QVariant selectedIndexesVar;
  bool invokeSuccess =
      QMetaObject::invokeMethod(qmlRootObj, "getSelectedIndexes",
                                Q_RETURN_ARG(QVariant, selectedIndexesVar));

  if (!invokeSuccess || !selectedIndexesVar.canConvert<QVariantList>()) {
    qWarning() << "调用QML方法getSelectedIndexes失败";
    return;
  }

  QVariantList selectedIndexes = selectedIndexesVar.toList();
  if (selectedIndexes.isEmpty()) {
    auto msg = std::make_unique<ShowMessage>(this);
    msg->showMsg("Knot", tr("Please select at least one item to delete."), 1);
    return;
  }

  // 3. 批量删除确认
  auto m_ShowMsg = std::make_unique<ShowMessage>(this);
  if (!m_ShowMsg->showMsg(
          "Knot",
          tr("Whether to delete the selected %1 items permanently?")
              .arg(selectedIndexes.count()),
          2)) {
    return;
  }

  // 4. 解决QVariant无>运算符问题：转换为QList<int>并倒序排序
  QList<int> selectedIntIndexes;
  // 核心：将遍历对象改为const QVariantList&，明确只读属性
  const QVariantList& constSelectedIndexes = selectedIndexes;
  for (const QVariant& indexVar : constSelectedIndexes) {
    if (indexVar.canConvert<int>()) {
      selectedIntIndexes.append(indexVar.toInt());
    }
  }

  // 对整数索引倒序排序（无编译错误，整数有默认比较运算符）
  std::sort(selectedIntIndexes.begin(), selectedIntIndexes.end(),
            std::greater<int>());

  // 5. 遍历整数索引，复用核心删除逻辑
  for (int qmlIndex : selectedIntIndexes) {
    QTreeWidgetItem* topItem = twrb->topLevelItem(0);
    if (!topItem) continue;

    if (qmlIndex < 0 || qmlIndex >= topItem->childCount()) {
      qWarning() << "无效的twrb子节点索引：" << qmlIndex;
      continue;
    }

    QTreeWidgetItem* curItem = topItem->child(qmlIndex);
    twrb->setCurrentItem(curItem);

    if (curItem->parent() == NULL) continue;

    // ===== 核心删除逻辑 =====
    QString md = iniDir + curItem->text(1);
    QStringList imagesInMD = extractLocalImagesFromMarkdown(md);
    for (int i = 0; i < imagesInMD.count(); i++) {
      QString image_file = imagesInMD.at(i);
      image_file = "KnotData/memo/" + image_file;
      needDelWebDAVFiles.append(image_file);
    }

    delFile(md);

    // 删除笔记后，更新图谱
    if (m_graphController) {
      m_graphController->parser()->deleteNoteCache(md);
    }

    QString json = m_Notes->getCurrentJSON(md);
    delFile(json);

    QStringList tempList = m_Notes->notes_sync_files;
    for (int i = 0; i < tempList.count(); i++) {
      QString file = tempList.at(i);
      QString baseFlag = m_Method->getBaseFlag(md);
      if (file.contains(baseFlag)) m_Notes->notes_sync_files.removeOne(file);
    }

    qDebug() << "删除笔记后的同步文件列表：" << m_Notes->notes_sync_files;

    setDelNoteFlag(curItem->text(1));
    curItem->parent()->removeChild(curItem);
    delete curItem;
    isDelNoteRecycle = true;
  }

  // 6. 收尾流程
  saveNotesList();

  // 7. 清空QML选中状态
  QMetaObject::invokeMethod(qmlRootObj, "resetQMLToNewState",
                            Qt::DirectConnection);
  resetQML_Recycle();

  // 8. 完成提示
  auto msg = std::make_unique<ShowMessage>(this);
  msg->showMsg("Knot",
               tr("Batch delete %1 items completed successfully.")
                   .arg(selectedIntIndexes.count()),
               1);
}

void NotesList::on_btnDel_Recycle_clicked() {
  QTreeWidgetItem* curItem = twrb->currentItem();
  if (curItem->parent() == NULL) {
    return;
  } else {
    auto m_ShowMsg = std::make_unique<ShowMessage>(this);
    if (!m_ShowMsg->showMsg(
            "Knot", tr("Whether to remove") + "  " + curItem->text(0) + " ? ",
            2)) {
      return;
    }

    QString md = iniDir + curItem->text(1);
    needDelWebDAVFiles.append(md + ".zip");
    QStringList imagesInMD = extractLocalImagesFromMarkdown(md);
    for (int i = 0; i < imagesInMD.count(); i++) {
      QString image_file = imagesInMD.at(i);
      image_file = "KnotData/memo/" + image_file;
      needDelWebDAVFiles.append(image_file);
    }

    delFile(md);
    QString json = m_Notes->getCurrentJSON(md);
    delFile(json);

    QStringList tempList = m_Notes->notes_sync_files;
    for (int i = 0; i < tempList.count(); i++) {
      QString file = tempList.at(i);
      QString baseFlag = m_Method->getBaseFlag(file);
      if (file.contains(baseFlag)) m_Notes->notes_sync_files.removeOne(file);
    }

    setDelNoteFlag(curItem->text(1));

    curItem->parent()->removeChild(curItem);

    isDelNoteRecycle = true;
  }

  saveNotesList();

  resetQML_Recycle();
}
void NotesList::on_btnClose_clicked() { this->close(); }

void NotesList::on_btnNewNoteBook_clicked() {
  QTreeWidgetItem* item = new QTreeWidgetItem();
  item->setText(0, notebookName);
  item->setText(2, "#FF0000");
  item->setForeground(0, Qt::red);
  item->setIcon(0, QIcon(":/res/nb.png"));

  if (rootIndex == 0) {
    tw->addTopLevelItem(item);
    tw->setCurrentItem(item);
  } else {
    QTreeWidgetItem* topItem = tw->topLevelItem(rootIndex - 1);
    tw->setCurrentItem(topItem);
    topItem->addChild(item);
    tw->setCurrentItem(item);
  }
}

void NotesList::on_btnNewNote_clicked() {
  if (tw->topLevelItemCount() == 0) return;

  QString noteFile = "memo/" + m_Notes->getDateTimeStr() + "_" +
                     m_Method->generateRandom3() + ".md";
  QTreeWidgetItem* parentitem = tw->currentItem();

  QTreeWidgetItem* item1 = new QTreeWidgetItem(parentitem);
  item1->setText(0, "");
  item1->setText(1, noteFile);
  item1->setIcon(0, QIcon(":/res/n.png"));

  QTextEdit* edit = new QTextEdit();
  edit->append("");
  TextEditToFile(edit, iniDir + noteFile);
  delete edit;

  tw->setCurrentItem(item1);
  noteName = item1->text(0);

  pNoteItems.clear();
  int count = parentitem->childCount();
  for (int i = 0; i < count; i++) {
    pNoteItems.append(parentitem->child(i));
  }
}

void NotesList::moveChildToRecycle(QTreeWidgetItem* parentItem, QString iniDir,
                                   QVector<QString>& delFilesIndex,
                                   QTreeWidget* twrb)

{
  if (!parentItem) return;
  int childCount = parentItem->childCount();

  // 倒序遍历，删除节点不会影响下标
  for (int i = childCount - 1; i >= 0; --i) {
    QTreeWidgetItem* child = parentItem->child(i);
    if (!child) continue;

    QString str0 = child->text(0);
    QString str1 = child->text(1);

    if (str1.isEmpty()) {
      // 子笔记本：先递归清理它的所有子项，再删除自身
      moveChildToRecycle(child, iniDir, delFilesIndex, twrb);
      parentItem->removeChild(child);
      delete child;
    } else {
      // 笔记文件：移入回收站 + 记录待删文件
      QTreeWidgetItem* recycleItem = new QTreeWidgetItem;
      recycleItem->setText(0, str0);
      recycleItem->setText(1, str1);
      addItem(twrb, recycleItem);
      delFilesIndex.append(iniDir + str1);

      parentItem->removeChild(child);
      delete child;
    }
  }
}

void NotesList::on_btnDel_clicked() {
  if (tw->topLevelItemCount() == 0) return;

  QTreeWidgetItem* item = tw->currentItem();
  if (item == NULL) return;

  bool isNoteBook = pNoteBookItems.contains(item);

  // 判断：笔记本 / 笔记
  QString strFlag = (isNoteBook) ? tr("NoteBook") : tr("Note");

  auto m_ShowMsg = std::make_unique<ShowMessage>(this);
  if (!m_ShowMsg->showMsg("Knot",
                          tr("Move to the recycle bin?") + "\n\n" + strFlag +
                              " : " + item->text(0),
                          2)) {
    return;
  }

  QStringList delFilesIndex;
  QString str0, str1;

  // ==========================================
  // 【删除笔记本】：先移所有笔记 → 最后删 TOP
  // ==========================================

  if (isNoteBook) {
    // 递归清理所有子项
    moveChildToRecycle(item, iniDir, delFilesIndex, twrb);

    // 移除并删除当前笔记本自身（兼容顶层 + 嵌套子节点）
    QTreeWidgetItem* parent = item->parent();
    if (parent) {
      // 嵌套子笔记本：从父节点移除
      parent->removeChild(item);
    } else {
      // 顶层笔记本：从树控件移除
      tw->takeTopLevelItem(tw->indexOfTopLevelItem(item));
    }
    delete item;
  }

  // ==========================================
  // 【删除单条笔记】：只删自己，不碰笔记本
  // ==========================================
  else {
    str0 = item->text(0);
    str1 = item->text(1);

    // 只移当前这一条笔记
    QTreeWidgetItem* recycleItem = new QTreeWidgetItem;
    recycleItem->setText(0, str0);
    recycleItem->setText(1, str1);
    addItem(twrb, recycleItem);

    delFilesIndex.append(iniDir + str1);

    // ✅ 只删除笔记，不删笔记本
    delete item;
  }

  // 安全刷新界面
  if (tw->topLevelItemCount() == 0) {
    m_Notes->loadEmptyNote();
    mui->lblNoteBook->setText(tr("Note Book"));
    mui->lblNoteList->setText(tr("Note List"));
  } else {
    if (tw->currentItem() != nullptr) {
      m_Notes->loadEmptyNote();
    }
  }

  startBackgroundTaskDelFilesIndex(delFilesIndex);
  saveNotesList();
  resetQML_List();
}

void NotesList::on_btnDown_clicked() {
  QTreeWidgetItem* oldItem = tw->currentItem();
  bool isNoteBook = pNoteBookItems.contains(oldItem);

  moveBy(1);

  if (isNoteBook) {
    isMouseClick = true;
  } else {
    int index = m_Method->getCurrentIndexFromQW(mui->qwNoteList);
    setNotesListCurrentIndex(index + 1);
    clickNoteList();
  }
}

void NotesList::on_btnUp_clicked() {
  QTreeWidgetItem* oldItem = tw->currentItem();
  bool isNoteBook = pNoteBookItems.contains(oldItem);

  moveBy(-1);

  if (isNoteBook) {
    isMouseClick = true;
  } else {
    int index = m_Method->getCurrentIndexFromQW(mui->qwNoteList);
    setNotesListCurrentIndex(index - 1);
    clickNoteList();
  }
}

void NotesList::on_btnExport_clicked() {
  if (tw->topLevelItemCount() == 0) return;

  QTreeWidgetItem* item = tw->currentItem();
  if (item->parent() == NULL) return;

  QString name = item->text(0);
  name = name + ".md";
  QString fileName;
  QFileDialog fd;
  fileName = fd.getSaveFileName(this, name, name, tr("MD File(*.*)"));

  if (fileName == "") return;

  QString mdfile = iniDir + item->text(1);

  QString str = loadText(mdfile);
  QTextEdit* edit = new QTextEdit();
  edit->setAcceptRichText(false);
  edit->setPlainText(str);

  TextEditToFile(edit, fileName);
}
int NotesList::on_btnImport_clicked() {
  if (tw->topLevelItemCount() == 0) return 0;

#ifdef Q_OS_ANDROID
  // 安卓：点击即显示进度条，锁定界面
  mw_one->showProgress();
#endif

  QStringList fileNames =
      QFileDialog::getOpenFileNames(this, tr("Knot"), "", tr("MD File (*.*)"));
  qDebug() << "Import Files:" << fileNames;

  if (fileNames.isEmpty()) {
#ifdef Q_OS_ANDROID
    mw_one->closeProgress();
#endif
    isImportFilesEnd = true;
    return 0;
  }

  QStringList MDFileList;
  QTreeWidgetItem* item = tw->currentItem();

  for (const QString& fileName : std::as_const(fileNames)) {
    QString strFile = fileName.toLower();
    if (strFile.contains(".md") || strFile.contains(".txt")) {
      MDFileList.append(fileName);
    } else {
      qDebug() << tr("Invalid Markdown file.") << fileName;
    }
  }

#ifndef Q_OS_ANDROID
  // 桌面：选择完成后显示进度条
  mw_one->showProgress();
#endif

  isImportFilesEnd = false;

  if (MDFileList.size() > 1000) {
    MDFileList.resize(10);
    auto msg = std::make_unique<ShowMessage>(this);
    msg->showMsg(appName,
                 tr("A maximum of 10 files can be imported at a time."), 1);
  }

  // 后台线程处理所有文件（全部完成才会进入 finished）
  QFuture<void> future = QtConcurrent::run([this, MDFileList, item]() {
    for (int i = 0; i < MDFileList.size(); ++i) {
      const QString& fileName = MDFileList[i];
      if (QFile::exists(fileName)) {
        QFileInfo fi(fileName);
        QString name = fi.completeBaseName();

        QTreeWidgetItem* item1 = new QTreeWidgetItem(item);
        item1->setText(0, name);

        QString a = "memo/" + m_Notes->getDateTimeStr() + "_" +
                    QString::number(i) + m_Method->generateRandom3() + ".md";
        currentMDFile = iniDir + a;

        QFile::copy(fileName, currentMDFile);
        item1->setText(1, a);

        m_Notes->updateMDFileToSyncLists();
      }
    }
  });

  QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
  connect(watcher, &QFutureWatcher<void>::finished, this,
          [this, watcher, MDFileList]() {
            // ==============================================
            // 重点：这里一定是【所有文件全部导入完成】才执行！
            // ==============================================
            qDebug() << "All files imported, count:" << MDFileList.size();

            m_Notes->startBackgroundTaskUpdateNoteIndexes(MDFileList);
            isImportNotes = true;
            isImportFilesEnd = true;

            // 统一关闭进度条（双端都在这里关闭）
            mw_one->closeProgress();

            watcher->deleteLater();
          });

  watcher->setFuture(future);
  return MDFileList.size();
}

void NotesList::on_btnMoveTo_clicked() {
  QTreeWidgetItem* item = tw->currentItem();
  if (item == NULL) return;

  int count = item->childCount();
  for (int i = 0; i < count; i++) {
    if (item->child(i)->text(1).isEmpty()) return;
  }

  if (moveItem(tw)) {
    saveNotesList();
    updateAllNoteIndexManager();

    m_NotesList->loadAllNoteBook();

    QTimer::singleShot(10, this,
                       [this]() { setCurrentItemFromMDFile(currentMDFile); });
  }
}

void NotesList::on_btnRename_clicked() {
  QTreeWidgetItem* item = tw->currentItem();
  if (item == NULL) return;

  m_RenameNotes = new QDialog(this);
  QVBoxLayout* vbox0 = new QVBoxLayout;
  m_RenameNotes->setLayout(vbox0);
  vbox0->setContentsMargins(5, 5, 5, 5);
  if (!isAndroid) m_RenameNotes->setModal(true);
  m_RenameNotes->setWindowFlag(Qt::FramelessWindowHint);

  QFrame* frame = new QFrame(this);
  vbox0->addWidget(frame);

  QVBoxLayout* vbox = new QVBoxLayout;

  frame->setLayout(vbox);
  vbox->setContentsMargins(25, 6, 25, 10);
  vbox->setSpacing(10);

  QLabel* lblTitle = new QLabel(this);
  lblTitle->adjustSize();
  lblTitle->setWordWrap(true);
  lblTitle->setText(tr("Rename"));
  vbox->addWidget(lblTitle);

  QFrame* hframe = new QFrame(this);
  hframe->setFrameShape(QFrame::HLine);
  hframe->setStyleSheet("QFrame{background:red;min-height:2px}");
  vbox->addWidget(hframe);
  hframe->hide();

  QTextEdit* edit = new QTextEdit(this);
  edit->setAcceptRichText(false);

  initTextToolbarDynamic(m_RenameNotes);
  EditEventFilter* editFilter =
      new EditEventFilter(textToolbarDynamic, m_RenameNotes);
  editFilter->setParent(m_RenameNotes);
  edit->installEventFilter(editFilter);
  edit->viewport()->installEventFilter(editFilter);

  vbox->addWidget(edit);
  edit->setPlainText(item->text(0));
  QScroller::grabGesture(edit, QScroller::LeftMouseButtonGesture);
  edit->horizontalScrollBar()->setHidden(true);
  edit->verticalScrollBar()->setStyleSheet(
      mui->editDetails->verticalScrollBar()->styleSheet());

  if (edit->toPlainText().trimmed() == "无标题笔记" ||
      edit->toPlainText().trimmed() == "Untitled Note") {
    edit->setPlainText(m_Notes->new_title);
  }

  QToolButton* btnCancel = new QToolButton(m_RenameNotes);
  QToolButton* btnPaste = new QToolButton(m_RenameNotes);
  QToolButton* btnCopy = new QToolButton(m_RenameNotes);
  QToolButton* btnOk = new QToolButton(m_RenameNotes);
  btnCancel->setText(tr("Cancel"));
  btnPaste->setText(tr("Paste"));
  btnCopy->setText(tr("Copy"));
  btnOk->setText(tr("OK"));

  btnOk->setFixedHeight(50);
  btnCancel->setFixedHeight(50);
  btnCopy->setFixedHeight(50);
  btnPaste->setFixedHeight(50);

  QHBoxLayout* hbox = new QHBoxLayout;
  hbox->addWidget(btnCancel);
  hbox->addWidget(btnPaste);
  hbox->addWidget(btnCopy);
  hbox->addWidget(btnOk);

  btnPaste->hide();
  btnCopy->hide();

  btnCancel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  btnPaste->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  btnCopy->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  btnOk->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

  QSpacerItem* sparcer_item =
      new QSpacerItem(0, 60, QSizePolicy::Fixed, QSizePolicy::Expanding);
  vbox->addItem(sparcer_item);

  vbox->addLayout(hbox, 0);

  connect(btnCancel, &QToolButton::clicked, m_RenameNotes,
          [=]() mutable { m_RenameNotes->close(); });
  connect(m_RenameNotes, &QDialog::rejected, m_RenameNotes,
          [=]() mutable { m_Method->closeGrayWindows(); });
  connect(m_RenameNotes, &QDialog::accepted, m_RenameNotes,
          [=]() mutable { m_Method->closeGrayWindows(); });
  connect(btnPaste, &QToolButton::clicked, m_RenameNotes,
          [=]() mutable { edit->paste(); });
  connect(btnCopy, &QToolButton::clicked, m_RenameNotes, [=]() mutable {
    edit->selectAll();
    edit->copy();
    m_RenameNotes->close();
  });
  connect(btnOk, &QToolButton::clicked, m_RenameNotes, [=]() mutable {
    renameCurrentItem(edit->toPlainText().trimmed());

    saveNotesList();
    m_RenameNotes->close();
  });

  connect(m_RenameNotes, &QDialog::finished, this, [this](int result) {
    Q_UNUSED(result);
    closeTextToolBar();
    m_Method->closeGrayWindows();
  });

  edit->setFixedHeight(280);

  int x, y, w, h;
  h = 500;
  if (isAndroid) {
    w = mw_one->width() - 2;
    y = mw_one->geometry().y();
  } else {
    w = 320;
    y = mw_one->geometry().y() + (mw_one->height() - h) / 2;
  }

  x = mw_one->geometry().x() + (mw_one->width() - w) / 2;
  m_RenameNotes->setGeometry(x, y, w, h);

  m_Method->set_ToolButtonStyle(m_RenameNotes);

  m_Method->showGrayWindows();
  m_RenameNotes->show();
}

/*void NotesList::clickNoteBook() {
  int count = m_Method->getCountFromQW(mui->qwNoteBook);
  if (count <= 0) return;

  int index = m_Method->getCurrentIndexFromQW(mui->qwNoteBook);
  if (index < 0) return;

  mui->lblNoteBook->setText(QString::number(index + 1) + "/" +
                            QString::number(count));

  pNoteItems.clear();
  isActColorFlagStatus = true;

  QString text1 = m_Method->getText1(mui->qwNoteBook, index);
  QString text2 = m_Method->getText2(mui->qwNoteBook, index);
  if (text2.isEmpty()) {
    int index_top = text1.toInt();
    QTreeWidgetItem* topItem = tw->topLevelItem(index_top);

    readyNotesData(topItem);
  }
}*/

void NotesList::clickNoteBook() {
  int count = m_Method->getCountFromQW(mui->qwNoteBook);
  if (count <= 0) return;

  int index = m_Method->getCurrentIndexFromQW(mui->qwNoteBook);
  if (index < 0 || index >= pNoteBookItems.size()) return;

  // 更新顶部计数
  mui->lblNoteBook->setText(QString::number(index + 1) + "/" +
                            QString::number(count));

  pNoteItems.clear();
  isActColorFlagStatus = true;

  // 核心：直接取缓存的树节点指针
  QTreeWidgetItem* item = pNoteBookItems.at(index);
  if (!item) return;

  // 可选：同步选中 TreeWidget 对应节点（UI联动）
  tw->setCurrentItem(item);
  item->setSelected(true);

  // 加载笔记数据
  readyNotesData(item);
}

void NotesList::clickNoteList() {
  int indexTop = m_Method->getCurrentIndexFromQW(mui->qwNoteBook);
  int index = m_Method->getCurrentIndexFromQW(mui->qwNoteList);

  if (index < 0) {
    currentMDFile = "";
    return;
  }

  if (indexTop < 0) return;

  QString strMD = m_Method->getText3(mui->qwNoteList, index);
  currentMDFile = iniDir + strMD;

  setNoteLabel();

  if (!QFile::exists(currentMDFile)) {
    return;
  }

  QString noteName = m_Method->getText0(mui->qwNoteList, index);
  noteTitle = noteName;

  QTreeWidgetItem* item = pNoteItems.at(index);
  tw->setCurrentItem(item);

  int indexNoteBook = m_Method->getCurrentIndexFromQW(mui->qwNoteBook);
  if (indexNoteBook < 0) return;

  QString s_tr = QString::number(indexNoteBook) + "=" + QString::number(index);
  int count2 = mIndexList.count();
  int i = 0;
  for (i = 0; i < count2; i++) {
    QString str = mIndexList.at(i);
    if (str.split("=").at(0).toInt() == indexNoteBook) {
      mIndexList.removeOne(str);
      break;
    }
  }
  mIndexList.append(s_tr);

  updateNoteIndexManager(currentMDFile, indexNoteBook, index);

  // qDebug() << "单击条目设置当前md文件：" << currentMDFile << indexNoteBook
  //          << index;
}
