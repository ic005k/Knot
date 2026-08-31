#include "NotesList.h"

#include "src/AI/EmbeddingEngine.h"
#include "src/AI/GlobalAI.h"
#include "src/MainWindow.h"
#include "src/Notes/VectorSearchService.h"

NotesList::NotesList(QWidget* parent) : QDialog(parent), ui(new Ui::NotesList) {
  ui->setupUi(this);
  this->installEventFilter(this);

  tw = new QTreeWidget(nullptr);
  twrb = new QTreeWidget(nullptr);

  twrb->setSelectionMode(QAbstractItemView::MultiSelection);

  noteModel = new NoteListModel(this);

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

  m_treeProxyModel = new QTreeWidgetProxyModel(tw, this);
  // mui->qwNotesTree->rootContext()->setContextProperty("treeModel",
  //                                                     m_treeProxyModel);

  initNotesList();
  initRecycle();

  // 连接搜索框
  connect(mui->editNotesSearch, &QLineEdit::textChanged, this,
          &NotesList::onSearchTextChanged);
  mui->qwNotesSearchResult->rootContext()->setContextProperty("searchModel",
                                                              &m_searchModel);
  connect(m_Notes->m_NoteManager, &NoteManager::noteMetaChanged, this,
          [this](const QString& filePath, const NoteMetadata& meta) {
            if (m_vectorSearchService) {
              m_vectorSearchService->registerNoteMeta(filePath, filePath,
                                                      meta.title);
            }
          });

  connect(m_Notes->m_NoteManager, &NoteManager::noteRemoved, this,
          [this](const QString& filePath) {
            if (m_vectorSearchService) {
              m_vectorSearchService->unregisterNoteMeta(filePath);
            }
          });

  connect(m_Notes->m_NoteManager, &NoteManager::indexReloaded, this,
          &NotesList::initVectorSearchService);

  // 初始化笔记关系图谱功能，注册控制器到QML引擎
  initNoteGraphView();

  // 初始化笔记向量搜索
  initVectorSearchService();
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

void NotesList::initVectorSearchService() {
  if (isLocalAIModel && g_embEngine && g_embEngine->isValid()) {
    if (m_vectorSearchService) {
      m_vectorSearchService->deleteLater();
      m_vectorSearchService = nullptr;
    }

    m_vectorSearchService = new VectorSearchService(g_embEngine.get(), this);

    // 仅在此处做全量注册
    auto allMeta = m_Notes->m_NoteManager->getAllMetadata();
    for (auto it = allMeta.constBegin(); it != allMeta.constEnd(); ++it) {
      m_vectorSearchService->registerNoteMeta(it.key(), it.key(),
                                              it.value().title);
    }

    qDebug() << "[VectorSearch] 初始化完成, 注册笔记数:" << allMeta.size();
  } else {
    if (m_vectorSearchService) {
      m_vectorSearchService->deleteLater();
      m_vectorSearchService = nullptr;
    }
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
      return true;
    }
  }

  return QWidget::eventFilter(watch, evn);
}

QString NotesList::getCurrentMDFile() {
  QSettings Reg(iniDir + "curmd.ini", QSettings::IniFormat);
  QString curmd = Reg.value("/MainNotes/currentItem", "memo/xxx.md").toString();
  return iniDir + curmd;
}

void NotesList::renameCurrentItem(QString title) {
  QTreeWidgetItem* item = tw->currentItem();
  if (item == NULL) return;

  item->setText(0, title.trimmed());

  m_Notes->m_NoteManager->setNoteTitle(iniDir + item->text(1), item->text(0));

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

  resetQML_List();
}

void NotesList::closeEvent(QCloseEvent* event) { Q_UNUSED(event); }

void NotesList::saveNotesList() {
  // 【高频防抖】如果正在保存，直接跳过，不允许重复触发
  if (m_isSaving) return;

  m_isSaving = true;

  QPointer<NotesList> self(this);
  QFuture<void> future = QtConcurrent::run([self]() {
    // 对象已销毁则直接返回
    if (!self) return;
    QMutexLocker locker(&self->m_saveMutex);
    self->saveNotesListToFile();
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

// 递归序列化节点：完全沿用原规则，text(1)空=子笔记本，非空=笔记
QJsonObject NotesList::serializeNotebookItem(QTreeWidgetItem* item) {
  QJsonObject obj;
  if (!item) return obj;

  // 笔记本基础字段，和原代码一致
  obj["name"] = item->text(0);
  obj["colorFlag"] = item->text(2);
  obj["notebookID"] = item->text(3);

  QJsonArray childrenArray;
  int childCount = item->childCount();

  for (int j = 0; j < childCount; j++) {
    QTreeWidgetItem* childItem = item->child(j);
    QString strChild1 = childItem->text(1);
    QString md_file = QDir(iniDir).filePath(strChild1);

    if (!strChild1.isEmpty()) {
      // 是笔记：沿用原版文件存在性判断
      if (QFile::exists(md_file)) {
        QJsonObject childObj;
        childObj["name"] = childItem->text(0);
        childObj["file"] = strChild1;
        childrenArray.append(childObj);
      }
    } else {
      // 是空 = 子笔记本，递归序列化
      childrenArray.append(serializeNotebookItem(childItem));
    }
  }

  // 固定使用 children 字段，和旧文件格式兼容
  obj["children"] = childrenArray;
  return obj;
}

void NotesList::saveNotesListToFile() {
  // 空指针防护
  if (!tw || !twrb || !m_Method) return;

  const QString tempFile = QDir(iniDir).filePath("temp.json");
  const QString endFile = QDir(iniDir).filePath("mainnotes.json");

  QJsonObject rootObj;

  // CurrentMD 路径处理（保留优化方案，比replace更安全）
  QDir baseDir(iniDir);
  rootObj["CurrentMD"] = baseDir.relativeFilePath(currentMDFile);

  // 遍历所有顶级根笔记本，逐个序列化
  QJsonArray mainNotesArray;
  int count = tw->topLevelItemCount();
  for (int i = 0; i < count; i++) {
    QTreeWidgetItem* topItem = tw->topLevelItem(i);
    mainNotesArray.append(serializeNotebookItem(topItem));
  }
  rootObj["mainNotes"] = mainNotesArray;

  // ========== 回收站 完全保留原逻辑 ==========
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

  // ========== 待删除文件 完全保留原逻辑 ==========
  needDelFiles.removeDuplicates();
  QJsonArray needDelArray;
  for (const QString& file : std::as_const(needDelFiles)) {
    needDelArray.append(file);
  }
  rootObj["needDelNotes"] = needDelArray;

  // 写入临时文件
  QFile tempF(tempFile);
  if (tempF.exists()) tempF.remove();

  QJsonDocument doc(rootObj);
  if (!tempF.open(QIODevice::WriteOnly | QIODevice::Text)) return;

  tempF.write(doc.toJson(QJsonDocument::Indented));
  tempF.close();

  // 替换正式文件 + 清理临时文件
  m_Method->upIniFile(tempFile, endFile);
  if (tempF.exists()) tempF.remove();
}

/**
 * @brief 递归加载子笔记本及其下属笔记、子笔记本
 * @param bookObj 笔记本JSON对象
 * @param parentItem 父树节点
 * @param parentRow 当前节点在父级中的行号
 * @param totalNotes 全局笔记总数（引用，累加所有层级笔记）
 */
void NotesList::loadSubNotebook(const QJsonObject& bookObj,
                                QTreeWidgetItem* parentItem, int parentRow,
                                int& totalNotes) {
  Q_UNUSED(parentRow);

  if (bookObj.isEmpty() || !parentItem) return;

  // 创建子笔记本节点，保留原有UI样式
  QString bookName = bookObj["name"].toString();
  QString colorFlag = bookObj["colorFlag"].toString("#FF0000");
  QString notebookID = bookObj["notebookID"].toString();

  QTreeWidgetItem* subBookItem = new QTreeWidgetItem(parentItem);
  subBookItem->setText(0, bookName);
  subBookItem->setText(2, colorFlag);
  subBookItem->setText(3, notebookID);

  // 统一读取 children 数组（保存端唯一字段）
  QJsonArray childrenArray = bookObj["children"].toArray();

  // 遍历子项：区分 笔记 / 子笔记本
  for (int idx = 0; idx < childrenArray.size(); ++idx) {
    QJsonObject childObj = childrenArray[idx].toObject();
    if (childObj.isEmpty()) continue;

    // 规则：存在 file 字段 = 普通笔记；否则 = 子笔记本
    if (childObj.contains("file")) {
      // 加载笔记
      QString noteName = childObj["name"].toString();
      QString noteFile = childObj["file"].toString();
      if (noteFile.isEmpty()) continue;

      totalNotes++;  // 笔记计数累加

      QTreeWidgetItem* noteItem = new QTreeWidgetItem(subBookItem);
      noteItem->setText(0, noteName);
      noteItem->setText(1, noteFile);

      QString md = QDir(iniDir).filePath(noteFile);
      m_Notes->m_NoteManager->setNoteTitle(md, noteName);

      noteFiles.append(md);
    } else {
      // 递归加载子笔记本
      loadSubNotebook(childObj, subBookItem, idx, totalNotes);
    }
  }
}

void NotesList::initNotesList() {
  if (!tw) return;

  tw->clear();
  noteFiles.clear();

  const QString jsonFile = QDir(iniDir).filePath("mainnotes.json");
  if (!QFile::exists(jsonFile)) return;

  QFile f(jsonFile);
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;

  QByteArray data = f.readAll();
  f.close();

  QJsonDocument doc = QJsonDocument::fromJson(data);
  if (doc.isNull() || !doc.isObject()) return;

  QJsonObject rootObj = doc.object();
  QJsonArray mainNotesArray = rootObj["mainNotes"].toArray();

  int nNoteBook = mainNotesArray.size();
  int notesTotal = 0;  // 全局笔记总数，所有层级统一累加

  // 遍历所有顶层笔记本
  for (int i = 0; i < mainNotesArray.size(); ++i) {
    QJsonObject topObj = mainNotesArray[i].toObject();
    QString strTop = topObj["name"].toString();
    QString strTopColorFlag = topObj["colorFlag"].toString("#FF0000");
    QString strTopNoteBookID = topObj["notebookID"].toString();

    // 创建顶层笔记本节点
    QTreeWidgetItem* topItem = new QTreeWidgetItem;
    topItem->setText(0, strTop);
    topItem->setText(2, strTopColorFlag);
    topItem->setText(3, strTopNoteBookID);

    // 读取当前顶层笔记本的 children 数组
    QJsonArray childrenArray = topObj["children"].toArray();

    // 遍历子项：区分笔记 / 子笔记本
    for (int j = 0; j < childrenArray.size(); ++j) {
      QJsonObject childObj = childrenArray[j].toObject();
      if (childObj.isEmpty()) continue;

      if (childObj.contains("file")) {
        // 普通笔记
        QString str0 = childObj["name"].toString();
        QString str1 = childObj["file"].toString();
        if (str1.isEmpty()) continue;

        notesTotal++;
        QTreeWidgetItem* childItem = new QTreeWidgetItem(topItem);
        childItem->setText(0, str0);
        childItem->setText(1, str1);

        QString md = QDir(iniDir).filePath(str1);
        m_Notes->m_NoteManager->setNoteTitle(md, str0);

        noteFiles.append(md);
      } else {
        // 子笔记本，递归加载
        loadSubNotebook(childObj, topItem, j, notesTotal);
      }
    }

    tw->addTopLevelItem(topItem);
  }

  // 更新表头统计信息
  tw->headerItem()->setText(
      0, tr("Notebook") + " : " + QString::number(nNoteBook) + "  " +
             tr("Notes") + " : " + QString::number(notesTotal));

  tw->expandAll();

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

  QTreeWidgetItem* topItem = new QTreeWidgetItem;
  topItem->setText(0, tr("Unclassified"));
  topItem->setText(2, "#FF0000");

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

    m_Notes->m_NoteManager->setNoteTitle(mdFile, str0);
  }

  tw->addTopLevelItem(topItem);
  tw->expandAll();

  qDebug() << "未分类笔记：" << result;
}

void NotesList::setWinPos() {
  int w = mw_one->width();
  int x = mw_one->geometry().x();
  this->setGeometry(x, mw_one->geometry().y(), w, mw_one->height());
  mui->btnBackNotesGraph->hide();
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
}

/*void NotesList::loadAllNoteBook() {

  pNoteBookItems.clear();
  m_Method->clearAllBakList(mui->qwNoteBook);

  if (!tw) return;

  // 遍历所有顶层节点（根节点）
  int topCount = tw->topLevelItemCount();
  for (int i = 0; i < topCount; ++i) {
    QTreeWidgetItem* topItem = tw->topLevelItem(i);
    // 根节点：level = 0，父索引 = -1
    traverseTreeItem(topItem, -1, 0);
  }

  // 原有代理模型代码保留
  if (m_treeProxyModel) {
    // m_treeProxyModel->resetAll();
  }
}*/

QStringList NotesList::loadAllNoteBook() {
  // 1. 清空
  pNoteBookItems.clear();
  // m_Method->clearAllBakList(mui->qwNoteBook);

  QStringList result;  // ← 新增：收集结果

  if (!tw) return result;

  // 2. 遍历顶层节点
  int topCount = tw->topLevelItemCount();
  for (int i = 0; i < topCount; ++i) {
    QTreeWidgetItem* topItem = tw->topLevelItem(i);
    // 把 result 的引用传进去
    traverseTreeItem(topItem, -1, 0, result);
  }

  // 3. 代理模型（保留）
  if (m_treeProxyModel) {
    // m_treeProxyModel->resetAll();
  }

  return result;
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

void NotesList::readyNotesData(QTreeWidgetItem* item) {
  if (!item) return;

  // 同步树控件选中
  tw->setCurrentItem(item);

  QVector<QPair<QString, QString>> uiDataList;
  QVector<QTreeWidgetItem*> childItems;

  int child_count = item->childCount();
  // ========== 关键：无子笔记直接返回，避免空容器引发崩溃 ==========
  if (child_count <= 0) {
    noteModel->replaceAll({});  // 清空笔记列表
    pNoteItems.clear();
    setNoteLabel();
    isExecRecentOpen = false;
    return;
  }

  for (int i = 0; i < child_count; ++i) {
    QTreeWidgetItem* child = item->child(i);
    if (!child) continue;  // 子节点空指针防护

    // text(1) 为空 = 子笔记本，直接跳过
    if (child->text(1).isEmpty()) {
      continue;
    }

    uiDataList.append({child->text(0), child->text(1)});
    childItems.append(child);
  }

  QString iniDirCopy = iniDir;

  struct RawResult {
    QString text0;
    QString text1;
    QString text2;
    QString text3;
  };

  // 现在用【过滤后的有效笔记数量】判断，不再用原始child_count
  int validNoteCount = uiDataList.size();

  QFuture<QVector<RawResult>> future = QtConcurrent::run([=]() {
    listNoteEntry.clear();

    QVector<RawResult> rawResults;
    rawResults.reserve(validNoteCount);

    for (int i = 0; i < validNoteCount; ++i) {
      const auto& uiData = uiDataList[i];
      QString text0 = uiData.first;
      QString text3 = uiData.second;

      if (text3.isEmpty()) continue;

      QString file = iniDirCopy + text3;
      // 基础文件接口防护
      QFile f(file);
      qint64 fileSize = 0;
      if (f.exists()) {
        fileSize = f.size();
      }

      QString strLastModi = m_Method->getLastModified(file);
      QString strSize = m_Method->getFileSize(fileSize, 2);
      QString strFileNoExists = f.exists() ? "" : tr("File does not exist");

      rawResults.push_back(
          {text0, strLastModi + " " + strSize, strFileNoExists, text3});
      listNoteEntry.append(text0 + "\n\n" + strLastModi + " " + strSize + "\n" +
                           strFileNoExists);
    }
    return rawResults;
  });

  QFutureWatcher<QVector<RawResult>>* watcher =
      new QFutureWatcher<QVector<RawResult>>(this);
  connect(watcher, &QFutureWatcher<QVector<RawResult>>::finished, this, [=]() {
    // 防止多次触发、对象已销毁
    if (!watcher) return;

    QVector<RawResult> rawResults = watcher->result();

    QList<NoteItem> batchItems;
    batchItems.reserve(rawResults.size());
    foreach (const auto& result, rawResults) {
      NoteItem item;
      item.text0 = result.text0;
      item.text1 = result.text1;
      item.text2 = result.text2;
      item.text3 = result.text3;
      item.myh = 0;
      batchItems.append(item);
    }

    noteModel->replaceAll(batchItems);
    pNoteItems.clear();
    pNoteItems.append(childItems.begin(), childItems.end());

    int noteCount = getNotesListCount();

    if (noteCount == 0) return;

    if (isSetCurrentMDFilePosition) {
      isSetCurrentMDFilePosition = false;
      setNotesListCurrentIndex(currentNoteslistIndex);
    } else {
      if (isImportNotes) {
        setNotesListCurrentIndex(noteCount - 1);
        isImportNotes = false;
      } else {
        QString notebookId = item->text(3).trimmed();
        if (!notebookId.isEmpty()) {
          QString targetMd = loadNotePosition(notebookId);
          if (!targetMd.isEmpty()) {
            // 在当前笔记本笔记列表内查找md
            QString mdFileFlag = targetMd.replace(iniDir, "");
            int targetIndex = 0;

            for (int i = 0; i < pNoteItems.count(); i++) {
              if (mdFileFlag == pNoteItems[i]->text(1)) {
                targetIndex = i;
                break;
              }
            }

            if (targetIndex >= 0) {
              setNotesListCurrentIndex(targetIndex);

            } else {
              // 无记录 / md文件不存在，默认选中第一条
              setNotesListCurrentIndex(0);
            }
          }
        } else {
          setNotesListCurrentIndex(0);
        }
      }
    }

    clickNoteList();

    if (isMouseClick) {
      setNotesListCurrentIndex(-1);
      int idxNoteBook = getNoteBookCurrentIndex();
      tw->setCurrentItem(pNoteBookItems.at(idxNoteBook));
      isMouseClick = false;
    }

    m_Notes->setNoteEntryList();

    watcher->deleteLater();

    /*if (isExecRecentOpen) {
        isExecRecentOpen = false;

        QTimer::singleShot(50, this, [=]() {
#ifdef Q_OS_ANDROID
            mw_one->on_btnOpenNote_clicked();
#else
            mw_one->on_btnEditNote_clicked();
#endif
        });
    }*/
  });
  watcher->setFuture(future);
}

bool NotesList::setCurrentItemFromMDFile(QString mdFile) {
  if (!QFile::exists(mdFile)) return false;

  QString mdFileFlag = QDir(iniDir).relativeFilePath(mdFile);

  // ✅ 不再用递归搜索计算 flatNotebookIndex
  // 直接在 pNoteBookItems（UI扁平列表）中查找匹配的笔记本
  int foundFlatIndex = -1;
  QTreeWidgetItem* foundNotebookItem = nullptr;
  QTreeWidgetItem* foundNoteItem = nullptr;

  for (int i = 0; i < pNoteBookItems.size(); ++i) {
    QTreeWidgetItem* bookItem = pNoteBookItems.at(i);
    // 在该笔记本的直接子项中查找目标笔记
    for (int j = 0; j < bookItem->childCount(); ++j) {
      QTreeWidgetItem* child = bookItem->child(j);
      if (!child->text(1).isEmpty() && child->text(1) == mdFileFlag) {
        foundFlatIndex = i;
        foundNotebookItem = bookItem;
        foundNoteItem = child;
        break;
      }
    }
    if (foundFlatIndex != -1) break;
  }

  if (foundFlatIndex == -1 || !foundNoteItem) {
    qWarning() << "未在扁平列表中定位到:" << mdFileFlag;
    return false;
  }

  int noteListIndex = calcNoteIndexInsideBook(foundNotebookItem, foundNoteItem);
  if (noteListIndex == -1) return false;

  qDebug() << "扁平笔记本索引:" << foundFlatIndex
           << "笔记索引:" << noteListIndex;

  tw->setCurrentItem(foundNotebookItem);
  isSetCurrentMDFilePosition = true;

  currentNoteslistIndex = noteListIndex;
  activateNoteBook(foundNotebookItem);

  return true;
}

QString NotesList::getCurrentNoteNameFromMDFile(QString mdFile) {
  return m_Notes->m_NoteManager->getNoteTitle(mdFile);
}

void NotesList::moveToFirst() {}

void NotesList::qmlOpenEdit() {}

/**
 * @brief 在指定笔记本节点内部，计算目标笔记在【右侧笔记列表】中的下标
 * 过滤所有子笔记本节点，只统计笔记节点，和readyNotesData筛选逻辑完全对齐
 */
int NotesList::calcNoteIndexInsideBook(QTreeWidgetItem* bookItem,
                                       QTreeWidgetItem* targetNote) {
  int idx = 0;
  int childCnt = bookItem->childCount();
  for (int i = 0; i < childCnt; i++) {
    QTreeWidgetItem* child = bookItem->child(i);
    if (child->text(1).isEmpty())
      continue;  // 子笔记本，跳过（和readyNotesData保持一致）

    if (child == targetNote) return idx;
    idx++;
  }
  return -1;
}
