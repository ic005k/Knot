#include "NotesList.h"

QString strNoteNameIndexFile = "";

NotesList::NotesList(QWidget* parent) : QDialog(parent), ui(new Ui::NotesList) {
  ui->setupUi(this);
  this->installEventFilter(this);

  strNoteNameIndexFile = privateDir + "MyNoteNameIndex";

  tw = new QTreeWidget(nullptr);
  twrb = new QTreeWidget(nullptr);

  twrb->setSelectionMode(QAbstractItemView::MultiSelection);

  noteModel = new NoteListModel(this);

  mui->f_FindNotes->setStyleSheet(
      "QFrame{background-color: #455364;color: #FFFFFF;border-radius:10px; "
      "border:0px solid gray;}");
  mui->f_Tools->setStyleSheet(mui->f_FindNotes->styleSheet());
  mui->qwNoteTools->setFixedHeight(60);

  mui->btnBackNoteList->hide();
  mui->btnEditNote->hide();
  mui->btnOpenNote->hide();
  mui->btnShowFindNotes->hide();
  mui->btnTools->hide();

  if (isAndroid) {
    QFont font = this->font();
    font.setPointSize(13);
    mui->lblNoteBook->setFont(font);
    mui->lblNoteList->setFont(font);
  }

  setModal(true);
  this->layout()->setSpacing(5);
  this->layout()->setContentsMargins(2, 2, 2, 2);
  ui->frame1->hide();

  tw->headerItem()->setText(0, tr("Notebook"));
  tw->setColumnHidden(1, false);
  tw->setColumnWidth(0, 300);
  tw->setColumnWidth(1, 300);
  twrb->header()->hide();
  twrb->setColumnHidden(1, true);
  twrb->setColumnWidth(0, 180);

  set_memo_dir();

  ui->btnImport->hide();
  ui->btnExport->hide();

  initNotesList();
  initRecycle();

  // 连接搜索框
  connect(mui->editNotesSearch, &QLineEdit::textChanged, this,
          &NotesList::onSearchTextChanged);
  mui->qwNotesSearchResult->rootContext()->setContextProperty("searchModel",
                                                              &m_searchModel);
  initSerachDatabase();

  loadNotesListIndex();

  // 初始化笔记关系图谱功能
  initNoteGraphView();  // 关键：注册控制器到QML引擎
}

NotesList::~NotesList() {
  delete ui;
  delete tw;
  delete twrb;

  if (watcher) {
    watcher->cancel();
    watcher->waitForFinished();
  }
}

void NotesList::set_memo_dir() {
  QString path = iniDir + "memo/";
  QDir dir(path);
  if (!dir.exists()) dir.mkdir(path);
}

bool NotesList::eventFilter(QObject* watch, QEvent* evn) {
  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
      if (!ui->frame1->isHidden()) {
        on_btnBack_clicked();
        return true;
      }

      if (!ui->frame0->isHidden()) {
        on_btnClose_clicked();
        return true;
      }
    }

    if (keyEvent->key() == Qt::Key_Return) {
      QTreeWidgetItem* item = tw->currentItem();
      on_treeWidget_itemClicked(item, 0);
      return true;
    }
  }

  return QWidget::eventFilter(watch, evn);
}

QString NotesList::getCurrentMDFile() {
  QSettings Reg(iniDir + "curmd.ini", QSettings::IniFormat);

  QString curmd = Reg.value("/MainNotes/currentItem", "memo/xxx.md").toString();
  QString title = Reg.value("/MainNotes/NoteName", tr("Note Name")).toString();

  noteTitle = title;

  return iniDir + curmd;
}

void NotesList::renameCurrentItem(QString title) {
  QTreeWidgetItem* item = tw->currentItem();
  if (item == NULL) return;

  item->setText(0, title.trimmed());
  if (item->parent() != NULL && !item->text(1).isEmpty()) {
    setNoteName(item->text(0));

    m_Notes->m_NoteIndexManager->setNoteTitle(iniDir + item->text(1),
                                              item->text(0));
    m_Notes->m_NoteIndexManager->saveIndex(strNoteNameIndexFile);

    for (int i = 0; i < listRecentOpen.count(); i++) {
      QString str = listRecentOpen.at(i);
      if (str.split("===").at(1) == item->text(1)) {
        QString newStr = item->text(0) + "===" + item->text(1);
        listRecentOpen.removeAt(i);
        listRecentOpen.insert(i, newStr);
        saveRecentOpen();
        break;
      }
    }
  }
  resetQML_List();
}

void NotesList::closeEvent(QCloseEvent* event) { Q_UNUSED(event); }

void NotesList::saveNotesList() {
  // 【高频防抖】如果正在保存，直接跳过，不允许重复触发
  if (m_isSaving) return;

  m_isSaving = true;

  QFuture<void> future = QtConcurrent::run([=]() {
    // 【全局互斥锁】同一时间只能有一个线程执行保存
    QMutexLocker locker(&m_saveMutex);

    // 保存函数
    saveNotesListToFile();
  });

  QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
  connect(watcher, &QFutureWatcher<void>::finished, this, [=]() {
    mw_one->strLatestModify = tr("Modi Notes List");
    m_Notes->isSaveNotesConfig = true;
    m_isSaving = false;  // 保存完成，解锁
    watcher->deleteLater();
  });
  watcher->setFuture(future);
}

void NotesList::saveNotesListToFile() {
  QString tempFile0 = iniDir + "temp.json";
  QString endFile0 = iniDir + "mainnotes.json";

  QJsonObject rootObj;

  // 当前 MD 文件
  QString md = currentMDFile;
  md = md.replace(iniDir, "");
  rootObj["CurrentMD"] = md;

  // 主笔记树
  QJsonArray mainNotesArray;
  int count = tw->topLevelItemCount();
  for (int i = 0; i < count; i++) {
    QTreeWidgetItem* topItem = tw->topLevelItem(i);
    QJsonObject topObj;
    topObj["name"] = topItem->text(0);
    topObj["colorFlag"] = topItem->text(2);

    QJsonArray childrenArray;
    int childCount = topItem->childCount();
    int n_less = 0;
    for (int j = 0; j < childCount; j++) {
      QTreeWidgetItem* childItem = topItem->child(j);
      QString strChild1 = childItem->text(1);
      QString md_file = iniDir + strChild1;

      if (QFile::exists(md_file) && !strChild1.isEmpty()) {
        QJsonObject childObj;
        childObj["name"] = childItem->text(0);
        childObj["file"] = strChild1;
        childrenArray.append(childObj);
      } else {
        n_less++;
      }
    }
    topObj["children"] = childrenArray;
    mainNotesArray.append(topObj);
  }
  rootObj["mainNotes"] = mainNotesArray;

  // 回收站
  QJsonArray recycleBinArray;
  if (twrb->topLevelItemCount() > 0) {
    QTreeWidgetItem* rbTopItem = twrb->topLevelItem(0);
    int rbChildCount = rbTopItem->childCount();
    for (int j = 0; j < rbChildCount; j++) {
      QTreeWidgetItem* childItem = rbTopItem->child(j);
      QJsonObject rbObj;
      rbObj["name"] = childItem->text(0);
      rbObj["file"] = childItem->text(1);
      recycleBinArray.append(rbObj);
    }
  }
  rootObj["recycleBin"] = recycleBinArray;

  // 待删除文件
  needDelFiles.removeDuplicates();
  QJsonArray needDelArray;
  for (const QString& file : std::as_const(needDelFiles)) {
    needDelArray.append(file);
  }
  rootObj["needDelNotes"] = needDelArray;

  // 保存 JSON 到临时文件
  QJsonDocument doc(rootObj);
  QFile f(tempFile0);
  if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
    f.write(doc.toJson(QJsonDocument::Indented));
    f.close();
  }

  // 覆盖到最终文件
  m_Method->upIniFile(tempFile0, endFile0);

  // Save Note Name
  saveCurrentNoteInfo();
}

void NotesList::initNotesList() {
  tw->clear();
  noteFiles.clear();

  QString jsonFile = iniDir + "mainnotes.json";

  if (QFile::exists(jsonFile)) {
    QFile f(jsonFile);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
      return;  // 文件不存在或无法打开
    }

    QByteArray data = f.readAll();
    f.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
      return;  // JSON 解析失败
    }

    QJsonObject rootObj = doc.object();

    // 获取 mainNotes 数组
    QJsonArray mainNotesArray = rootObj["mainNotes"].toArray();

    int nNoteBook = mainNotesArray.size();
    int notesTotal = 0;

    for (int i = 0; i < mainNotesArray.size(); ++i) {
      QJsonObject topObj = mainNotesArray[i].toObject();

      QString strTop = topObj["name"].toString();
      QString strTopColorFlag = topObj["colorFlag"].toString("#FF0000");

      QTreeWidgetItem* topItem = new QTreeWidgetItem;
      topItem->setText(0, strTop);
      topItem->setText(2, strTopColorFlag);

      // 子节点
      QJsonArray childrenArray = topObj["children"].toArray();
      notesTotal += childrenArray.size();

      for (int j = 0; j < childrenArray.size(); ++j) {
        QJsonObject childObj = childrenArray[j].toObject();
        QString str0 = childObj["name"].toString();
        QString str1 = childObj["file"].toString();

        if (!str1.isEmpty()) {
          QTreeWidgetItem* childItem = new QTreeWidgetItem(topItem);
          childItem->setText(0, str0);
          childItem->setText(1, str1);

          QString md = iniDir + str1;
          m_Notes->m_NoteIndexManager->setNoteTitle(md, str0);
          updateNoteIndexManager(md, i, j);

          noteFiles.append(md);
        }
      }

      tw->addTopLevelItem(topItem);
    }

    tw->headerItem()->setText(
        0, tr("Notebook") + " : " + QString::number(nNoteBook) + "  " +
               tr("Notes") + " : " + QString::number(notesTotal));
    tw->expandAll();

  } else {
    QSettings* iniNotes =
        new QSettings(iniDir + "mainnotes.ini", QSettings::IniFormat, NULL);

    int topCount = iniNotes->value("/MainNotes/topItemCount").toInt();
    int nNoteBook = topCount;

    int notesTotal = 0;
    QString str0, str1;
    for (int i = 0; i < topCount; i++) {
      QString strTop =
          iniNotes->value("/MainNotes/strTopItem" + QString::number(i))
              .toString();
      QString strTopColorFlag =
          iniNotes
              ->value("/MainNotes/strTopColorFlag" + QString::number(i),
                      "#FF0000")
              .toString();
      int childCount =
          iniNotes->value("/MainNotes/childCount" + QString::number(i)).toInt();
      notesTotal = notesTotal + childCount;

      QTreeWidgetItem* topItem = new QTreeWidgetItem;
      topItem->setText(0, strTop);
      topItem->setText(2, strTopColorFlag);
      topItem->setForeground(0, Qt::red);
      QFont font = this->font();
      font.setBold(true);
      topItem->setFont(0, font);
      topItem->setIcon(0, QIcon(":/res/nb.png"));

      for (int j = 0; j < childCount; j++) {
        str0 = iniNotes
                   ->value("/MainNotes/childItem0_" + QString::number(i) + "_" +
                           QString::number(j))
                   .toString();
        str1 = iniNotes
                   ->value("/MainNotes/childItem1_" + QString::number(i) + "_" +
                           QString::number(j))
                   .toString();

        if (!str1.isEmpty()) {
          QTreeWidgetItem* childItem = new QTreeWidgetItem(topItem);
          childItem->setText(0, str0);
          childItem->setText(1, str1);
          childItem->setIcon(0, QIcon(":/res/n.png"));

          QString md = iniDir + str1;
          m_Notes->m_NoteIndexManager->setNoteTitle(md, str0);
          updateNoteIndexManager(md, i, j);

          noteFiles.append(md);
        }
      }
      tw->addTopLevelItem(topItem);
    }

    tw->headerItem()->setText(
        0, tr("Notebook") + " : " + QString::number(nNoteBook) + "  " +
               tr("Notes") + " : " + QString::number(notesTotal));
    tw->expandAll();
  }

  m_Notes->m_NoteIndexManager->saveIndex(strNoteNameIndexFile);

  initRecentOpen();
}

void NotesList::initRecycle() {
  twrb->clear();
  recycleFiles.clear();

  QString jsonFile = iniDir + "mainnotes.json";

  if (QFile::exists(jsonFile)) {
    QFile f(jsonFile);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
      return;  // 文件不存在或无法打开
    }

    QByteArray data = f.readAll();
    f.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
      return;  // JSON 解析失败
    }

    QJsonObject rootObj = doc.object();

    // 获取回收站数组
    QJsonArray recycleBinArray = rootObj["recycleBin"].toArray();

    // 创建顶层项
    QTreeWidgetItem* topItem = new QTreeWidgetItem;
    topItem->setText(0, tr("Notes Recycle Bin"));

    // 遍历回收站数组
    for (QJsonArray::const_iterator it = recycleBinArray.cbegin();
         it != recycleBinArray.cend(); ++it) {
      const QJsonValue& val = *it;  // 明确使用const引用
      QJsonObject obj = val.toObject();
      QString str0 = obj["name"].toString();
      QString str1 = obj["file"].toString();

      QTreeWidgetItem* childItem = new QTreeWidgetItem(topItem);
      childItem->setText(0, str0);
      childItem->setText(1, str1);

      recycleFiles.append(iniDir + str1);
    }

    twrb->addTopLevelItem(topItem);
    twrb->expandAll();

  } else {
    QSettings iniNotes(iniDir + "mainnotes.ini", QSettings::IniFormat);

    int i = 0;
    QTreeWidgetItem* topItem = new QTreeWidgetItem;
    topItem->setText(0, tr("Notes Recycle Bin"));

    int childCount =
        iniNotes.value("/MainNotes/rbchildCount" + QString::number(i)).toInt();
    for (int j = 0; j < childCount; j++) {
      QString str0, str1;
      str0 = iniNotes
                 .value("/MainNotes/rbchildItem0" + QString::number(i) +
                        QString::number(j))
                 .toString();
      str1 = iniNotes
                 .value("/MainNotes/rbchildItem1" + QString::number(i) +
                        QString::number(j))
                 .toString();

      QTreeWidgetItem* childItem = new QTreeWidgetItem(topItem);
      childItem->setText(0, str0);
      childItem->setText(1, str1);
      childItem->setIcon(0, QIcon(":/res/n.png"));

      recycleFiles.append(iniDir + str1);
    }
    twrb->addTopLevelItem(topItem);

    twrb->expandAll();
  }
}

void NotesList::initUnclassified() {
  QStringList dirFiles = m_Method->getMdFilesInDir(iniDir + "memo/", true);

  QStringList excludeFiles = noteFiles + recycleFiles;
  excludeFiles.removeDuplicates();

  QStringList result;
  foreach (const QString& file, dirFiles) {
    if (!excludeFiles.contains(file)) {
      result.append(file);
    }
  }

  int count = result.count();
  if (count == 0) return;

  int topCount = tw->topLevelItemCount();

  QTreeWidgetItem* topItem = new QTreeWidgetItem;
  topItem->setText(0, tr("Unclassified"));
  topItem->setText(2, "#FF0000");
  topItem->setForeground(0, Qt::red);

  TitleGenerator generator;

  for (int i = 0; i < count; i++) {
    QString mdFile = result.at(i);
    QString str0, str1;
    str0 = generator.genNewTitle(loadText(mdFile));
    str1 = mdFile;
    str1 = str1.replace(iniDir, "");
    QTreeWidgetItem* childItem = new QTreeWidgetItem(topItem);
    childItem->setText(0, str0);
    childItem->setText(1, str1);

    m_Notes->m_NoteIndexManager->setNoteTitle(mdFile, str0);
    updateNoteIndexManager(mdFile, topCount, i);
  }

  tw->addTopLevelItem(topItem);
  tw->expandAll();

  qDebug() << result;
}

void NotesList::setWinPos() {
  int w = mw_one->width();
  int x = mw_one->geometry().x();
  this->setGeometry(x, mw_one->geometry().y(), w, mw_one->height());
  mui->btnBackNotesGraph->hide();
}

void NotesList::localItem() {
  QTreeWidgetItem* item = tw->currentItem();
  // NoteBook

  if (item->childCount() > 0) {
    if (item->parent() == NULL) {
      int topIndex = tw->indexOfTopLevelItem(item);
      setNoteBookCurrentIndex(topIndex);
    } else {
      int index = tw->indexOfTopLevelItem(item->parent());

      int childCount = item->parent()->childCount();
      int newRow = 0;
      for (int i = 0; i < childCount; i++) {
        QTreeWidgetItem* item1 = item->parent()->child(i);
        if (item1 == item) break;

        if (item1->text(1).isEmpty()) newRow++;
      }

      setNoteBookCurrentIndex(index + newRow + 1);
    }

    clickNoteBook();
    setNotesListCurrentIndex(-1);

    // Notes
  } else {
    QTreeWidgetItem* top_item = item->parent()->parent();
    if (top_item == NULL) {
      int topIndex = tw->indexOfTopLevelItem(item->parent());
      int childIndex = tw->currentIndex().row();
      setNoteBookCurrentIndex(topIndex);
      clickNoteBook();
      setNotesListCurrentIndex(childIndex);
    } else {
      int index = tw->indexOfTopLevelItem(top_item);

      int childCount = top_item->childCount();
      int newRow = 0;
      for (int i = 0; i < childCount; i++) {
        QTreeWidgetItem* item1 = top_item->child(i);
        if (item1 == item->parent()) break;

        if (item1->text(1).isEmpty()) newRow++;
      }

      setNoteBookCurrentIndex(index + newRow + 1);

      clickNoteBook();
      setNotesListCurrentIndex(item->parent()->indexOfChild(item));
    }
  }
}

void NotesList::moveBy(int ud) {
  QTreeWidgetItem* item = tw->currentItem();

  if (item == NULL) return;

  if (item->parent() != NULL) {
    QTreeWidgetItem* parentItem = item->parent();
    int index = parentItem->indexOfChild(item);
    if (ud == -1) {
      if (index - 1 >= 0) {
        int n = 0;
        // NoteBook
        if (item->text(1).isEmpty()) {
          for (int i = index - 1; i >= 0; i--) {
            n++;
            if (parentItem->child(i)->text(1).isEmpty()) {
              break;
            }
          }
        }
        // Note
        if (!item->text(1).isEmpty()) {
          for (int i = index - 1; i >= 0; i--) {
            n++;
            if (!parentItem->child(i)->text(1).isEmpty()) {
              break;
            }
          }
        }

        parentItem->removeChild(item);
        parentItem->insertChild(index - n, item);
        tw->setCurrentItem(item);
        tw->scrollToItem(item);
      }
    }
    if (ud == 1) {
      int count = parentItem->childCount() - 1;
      int n = 0;
      // NoteBook
      if (item->text(1).isEmpty()) {
        for (int i = index + 1; i <= count; i++) {
          n++;
          if (parentItem->child(i)->text(1).isEmpty()) {
            break;
          }
        }
      }
      // Note
      if (!item->text(1).isEmpty()) {
        for (int i = index + 1; i <= count; i++) {
          n++;
          if (!parentItem->child(i)->text(1).isEmpty()) {
            break;
          }
        }
      }

      if (index + n <= count) {
        parentItem->removeChild(item);
        parentItem->insertChild(index + n, item);
        tw->setCurrentItem(item);
        tw->scrollToItem(item);
      }
    }
  } else {
    // top item
    int index = tw->indexOfTopLevelItem(item);
    if (ud == -1) {
      if (index - 1 >= 0) {
        tw->takeTopLevelItem(index);
        tw->insertTopLevelItem(index - 1, item);
        tw->setCurrentItem(item);
        tw->scrollToItem(item);
        tw->expandAll();
      }
    }
    if (ud == 1) {
      if (index + 1 <= tw->topLevelItemCount() - 1) {
        tw->takeTopLevelItem(index);
        tw->insertTopLevelItem(index + 1, item);
        tw->setCurrentItem(item);
        tw->scrollToItem(item);
        tw->expandAll();
      }
    }
  }

  resetQML_List();
  saveNotesList();

  updateAllNoteIndexManager();
}

void NotesList::loadAllNoteBook() {
  pNoteBookItems.clear();

  m_Method->clearAllBakList(mui->qwNoteBook);

  int count = tw->topLevelItemCount();
  for (int i = 0; i < count; i++) {
    QTreeWidgetItem* topItem = tw->topLevelItem(i);
    QString str = topItem->text(0);
    QString strtopcolorflag = topItem->text(2);

    int sum = topItem->childCount();
    int child_count = sum;
    for (int n = 0; n < child_count; n++) {
      if (topItem->child(n)->text(1).isEmpty()) sum--;
    }

    QString strSum = QString::number(sum);
    addItemToQW(mui->qwNoteBook, str, QString::number(i), "", strSum,
                strtopcolorflag, fontSize);
    pNoteBookItems.append(topItem);

    int childCount = topItem->childCount();
    for (int j = 0; j < childCount; j++) {
      QTreeWidgetItem* childItem = topItem->child(j);
      if (childItem->text(1).isEmpty()) {
        addItemToQW(mui->qwNoteBook, childItem->text(0),
                    QString::number(i) + "===" + QString::number(j),
                    "isNoteBook", QString::number(childItem->childCount()), "",
                    fontSize - 1);

        pNoteBookItems.append(childItem);
      }
    }
  }
}

int NotesList::countMdFilesImages(const QString& dirPath) {
  // 1. 构造QDir对象，指定目标目录路径
  QDir targetDir(dirPath);

  // 2. 仅需调用QDir::exists()，即可校验目录是否存在且为有效目录
  // （QDir::exists() 对目录路径返回true，对文件/无效路径返回false）
  if (!targetDir.exists()) {
    return 0;  // 无效目录（或文件、不存在路径）返回0个文件
  }

  // 3. 设置文件筛选规则：仅获取文件（排除子目录、符号链接等）
  QFileInfoList fileInfoList =
      targetDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

  // 4. 返回文件列表的长度，即该目录下直接文件的数量
  return fileInfoList.count();
}

int NotesList::getSelectedVersionIndex() {
  QQuickItem* rootItem = mui->qwNoteVersion->rootObject();
  if (!rootItem) {
    qWarning() << "获取 QML 根对象失败，无法读取选中索引";
    return -1;
  }

  // 读取根对象的 currentSelectedIndex 属性
  // 注意：属性名必须和 QML 中定义的完全一致（currentSelectedIndex）
  QVariant selectedIndexVar = rootItem->property("currentSelectedIndex");
  if (!selectedIndexVar.isValid()) {
    qWarning() << "QML 中未找到 currentSelectedIndex 属性，可能 QML 未正确修改";
    return -1;
  }

  int selectedIndex = selectedIndexVar.toInt();
  qDebug() << "当前选中的列表索引：" << selectedIndex;

  if (selectedIndex == -1) {
    qDebug() << "当前无选中项";
  }

  return selectedIndex;
}

bool NotesList::moveItem(QTreeWidget* twMain) {
  // 1. 获取当前选中项
  QTreeWidgetItem* srcItem = twMain->currentItem();
  if (!srcItem) return false;

  // 空笔记本禁止移动（无子项的顶级笔记本）
  if (srcItem->text(1).isEmpty() && srcItem->childCount() == 0) {
    return false;
  }

  // 2. 弹出目标选择对话框
  m_MoveTo = new MoveTo(this);
  m_MoveTo->showDialog();
  if (!m_MoveTo->isOk) return false;

  // 目标笔记本
  QTreeWidgetItem* targetFolder = m_MoveTo->currentItem;
  if (!targetFolder) return false;

  // ==============================================
  // 共同规则：不能移动到自己
  // ==============================================
  if (srcItem == targetFolder) {
    return false;
  }

  // ==============================================
  // 【功能 1】移动整个笔记本（顶级笔记本）
  // ==============================================
  if (srcItem->text(1).isEmpty()) {
    // 唯一禁止：自己不能移到自己
    if (srcItem == targetFolder) return false;

    // 安全取出所有子项（保证顺序、不乱索引）
    QList<QTreeWidgetItem*> children;
    while (srcItem->childCount() > 0) {
      children.append(srcItem->takeChild(0));
    }

    // 移动到目标顶级笔记本
    for (QTreeWidgetItem* child : children) {
      targetFolder->addChild(child);
    }

    // 删除原来的旧笔记本
    delete srcItem;
    srcItem = nullptr;

    twMain->setCurrentItem(targetFolder);
    return true;
  }

  // ==============================================
  // 【功能 2】移动单条笔记（原来正常的逻辑完整保留）
  // ==============================================
  else {
    // 笔记不能移动到自己所在的笔记本
    QTreeWidgetItem* parentNotebook = srcItem->parent();
    if (parentNotebook == targetFolder) {
      return false;
    }

    // 从原笔记本移除
    if (parentNotebook) {
      parentNotebook->removeChild(srcItem);
    }

    // 添加到目标笔记本
    targetFolder->addChild(srcItem);
    twMain->setCurrentItem(srcItem);

    return true;
  }

  return false;
}

void NotesList::readyNotesData(QTreeWidgetItem* topItem) {
  // 主线程预收集UI数据（按值捕获到后台线程，安全）
  QVector<QPair<QString, QString>> uiDataList;
  QVector<QTreeWidgetItem*> childItems;
  int child_count = topItem->childCount();
  for (int i = 0; i < child_count; ++i) {
    QTreeWidgetItem* child = topItem->child(i);
    uiDataList.append({child->text(0), child->text(1)});
    childItems.append(child);
  }
  QString iniDirCopy = iniDir;  // 拷贝一份，避免捕获this的成员变量（更安全）

  // 定义后台线程返回的结果类型
  struct RawResult {
    QString text0;
    QString text1;
    QString text2;
    QString text3;
  };

  // 关键：后台线程通过return返回结果，不引用捕获局部变量
  QFuture<QVector<RawResult>> future = QtConcurrent::run([=]() {
    qDebug() << "后台处理开始，共" << child_count << "项";
    QVector<RawResult> rawResults;  // 后台线程内部创建，生命周期由线程管理
    rawResults.reserve(child_count);

    for (int i = 0; i < child_count; ++i) {
      const auto& uiData = uiDataList[i];
      QString text0 = uiData.first;
      QString text3 = uiData.second;

      if (!text3.isEmpty()) {
        QString file =
            iniDirCopy + text3;  // 使用拷贝的iniDir，避免访问this成员
        QString item1 = m_Method->getLastModified(file);
        QString strSize = m_Method->getFileSize(QFile(file).size(), 2);
        QString text2 = "";
        if (!QFile::exists(file)) text2 = tr("File does not exist");

        rawResults.push_back({text0, item1 + " " + strSize, text2, text3});
      }
    }
    qDebug() << "后台处理完成，有效数据" << rawResults.size() << "项";
    return rawResults;  // 返回结果，由QFuture管理
  });

  // 主线程接收后台返回的结果
  QFutureWatcher<QVector<RawResult>>* watcher =
      new QFutureWatcher<QVector<RawResult>>(this);
  connect(watcher, &QFutureWatcher<QVector<RawResult>>::finished, this, [=]() {
    qDebug() << "主线程开始处理结果";

    // 获取后台线程返回的结果（安全，由QFuture保证生命周期）
    QVector<RawResult> rawResults = watcher->result();

    // 构造NoteItem并更新模型
    QList<NoteItem> batchItems;
    batchItems.reserve(rawResults.size());

    // 迭代器循环
    for (auto it = rawResults.constBegin(); it != rawResults.constEnd(); ++it) {
      const RawResult& result = *it;
      NoteItem item;
      item.text0 = result.text0;
      item.text1 = result.text1;
      item.text2 = result.text2;
      item.text3 = result.text3;
      item.myh = 0;
      batchItems.append(item);
    }

    // noteModel->addBatchItems(batchItems);
    noteModel->replaceAll(batchItems);  // 直接替换！
    pNoteItems.append(childItems.begin(), childItems.end());

    // 其他UI操作

    int index = m_Method->getCurrentIndexFromQW(mui->qwNoteBook);
    int noteslistIndex = getSavedNotesListIndex(index);
    setNotesListCurrentIndex(noteslistIndex);

    if (isImportNotes) {
      int noteCount = getNotesListCount();
      setNotesListCurrentIndex(noteCount - 1);
      isImportNotes = false;
    }

    setNoteLabel();
    clickNoteList();
    if (isMouseClick) {
      setNotesListCurrentIndex(-1);
      isMouseClick = false;
    }

    // ==============================
    // 【优化】自动跳转到保存的笔记索引
    // ==============================
    if (m_autoJumpNoteIndex >= 0) {
      int targetIndex = m_autoJumpNoteIndex;
      m_autoJumpNoteIndex = -1;  // 立即清空，防止重复执行

      int countNotes = m_Method->getCountFromQW(mui->qwNoteList);
      if (targetIndex >= 0 && targetIndex < countNotes) {
        setNotesListCurrentIndex(targetIndex);
        clickNoteList();
      }
    }

    watcher->deleteLater();

    qDebug() << "主线程处理结果完成！";
  });
  watcher->setFuture(future);
}

void NotesList::mouseClickNoteBook() {
  isMouseClick = true;
  clickNoteBook();
}

void NotesList::saveNotesListIndex() {
  QJsonObject root;
  QJsonArray array;

  for (const QString& item : std::as_const(mIndexList)) {
    array.append(item);
  }

  root["list"] = array;

  QFile file(privateDir + QDir::separator() + "noteslistindex.json");
  if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QJsonDocument doc(root);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
  } else {
    qWarning() << "无法打开文件进行写入:" << file.errorString();
  }
}

void NotesList::loadNotesListIndex() {
  QFile file(privateDir + QDir::separator() + "noteslistindex.json");
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning() << "无法打开文件进行读取:" << file.errorString();
    return;
  }

  QByteArray data = file.readAll();
  file.close();

  QJsonDocument doc = QJsonDocument::fromJson(data);
  if (doc.isNull()) {
    qWarning() << "JSON 解析失败";
    return;
  }

  QJsonObject root = doc.object();
  QJsonArray array = root["list"].toArray();

  mIndexList.clear();
  for (const QJsonValue& value : std::as_const(array)) {
    if (value.isString()) {
      mIndexList.append(value.toString());
    }
  }
}

int NotesList::getSavedNotesListIndex(int notebookIndex) {
  int index = 0;
  int in0, in1;
  for (int i = 0; i < mIndexList.count(); i++) {
    QString str = mIndexList.at(i);
    in0 = str.split("=").at(0).toInt();
    in1 = str.split("=").at(1).toInt();

    if (notebookIndex == in0) {
      index = in1;
      break;
    }
  }

  int count = m_Method->getCountFromQW(mui->qwNoteList);
  if (count > 0) {
    if (index < 0) index = 0;
    if (index > count - 1) index = count - 1;
  } else {
    index = -1;
  }

  return index;
}

void NotesList::updateNoteIndexManager(QString mdFile, int notebookIndex,
                                       int noteIndex) {
  m_Notes->m_NoteIndexManager->setNotebookIndex(mdFile, notebookIndex);
  m_Notes->m_NoteIndexManager->setNoteIndex(mdFile, noteIndex);
}

void NotesList::updateAllNoteIndexManager() {
  int topCount = tw->topLevelItemCount();
  for (int i = 0; i < topCount; i++) {
    QTreeWidgetItem* topItem = tw->topLevelItem(i);
    int childCount = topItem->childCount();
    for (int j = 0; j < childCount; j++) {
      QTreeWidgetItem* childItem = topItem->child(j);
      QString mdFile = iniDir + childItem->text(1);
      QString title = childItem->text(0);
      m_Notes->m_NoteIndexManager->setNoteTitle(mdFile, title);
      updateNoteIndexManager(mdFile, i, j);
    }
  }
}

bool NotesList::setCurrentItemFromMDFile(QString mdFile) {
  if (!QFile::exists(mdFile)) return false;

  int indexNoteBook, indexNote, countNoteBook;
  indexNoteBook = m_Notes->m_NoteIndexManager->getNotebookIndex(mdFile);
  indexNote = m_Notes->m_NoteIndexManager->getNoteIndex(mdFile);
  countNoteBook = m_Method->getCountFromQW(mui->qwNoteBook);

  if (indexNoteBook < 0) return false;
  if (indexNoteBook >= countNoteBook) return false;

  // ==============================
  // 【关键】记录要自动定位的笔记索引
  // ==============================
  m_autoJumpNoteIndex = indexNote;

  setNoteBookCurrentIndex(indexNoteBook);

  clickNoteBook();

  qDebug() << "已切换笔记本，等待加载完成后自动定位笔记：" << mdFile;

  return true;
}

QString NotesList::getCurrentNoteNameFromMDFile(QString mdFile) {
  return m_Notes->m_NoteIndexManager->getNoteTitle(mdFile);
}

void NotesList::moveToFirst() {
  return;

  ////////////////////////////////////////////////////////

  int indexNote = m_Method->getCurrentIndexFromQW(mui->qwNoteList);
  if (indexNote <= 0) return;
  int countNote = m_Method->getCountFromQW(mui->qwNoteList);
  if (countNote == 1) return;

  QTreeWidgetItem* item = tw->currentItem();
  if (item == NULL) return;

  if (item->parent() != NULL) {
    QTreeWidgetItem* parentItem = item->parent();
    parentItem->removeChild(item);
    parentItem->insertChild(0, item);
    tw->setCurrentItem(item);

    resetQML_List();
    saveNotesList();

    updateAllNoteIndexManager();

    setNotesListCurrentIndex(0);
  }
}

void NotesList::qmlOpenEdit() { mui->btnEditNote->click(); }
