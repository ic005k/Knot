#include "NotesList.h"

static QObject* getQmlRoot(QQuickWidget* quickWidget) {
  if (!quickWidget) return nullptr;
  return quickWidget->rootObject();
}

QStringList NotesList::getRecycleNoteFiles() {
  QStringList cycleFiles;
  QTreeWidgetItem* cycleTopItem = twrb->topLevelItem(0);
  int count = cycleTopItem->childCount();
  for (int i = 0; i < count; i++) {
    QString filePath = iniDir + cycleTopItem->child(i)->text(1);
    cycleFiles.append(filePath);
  }
  return cycleFiles;
}

bool NotesList::delFile(QString file) {
  bool isOk = false;
  QFile _file(file);
  if (_file.exists()) {
    _file.remove();
    isOk = true;
  } else {
    isOk = false;
  }

  _file.close();
  return isOk;
}

void NotesList::addItemToQW(QQuickWidget* qw, QString text0, QString text1,
                            QString text2, QString text3, QString text4,
                            int itemH) {
  QQuickItem* root = qw->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "addItem", Q_ARG(QVariant, text0),
                            Q_ARG(QVariant, text1), Q_ARG(QVariant, text2),
                            Q_ARG(QVariant, text3), Q_ARG(QVariant, text4),
                            Q_ARG(QVariant, itemH));
}

void NotesList::addItemToQW_Level(QObject* qmlRoot, const QString& t0,
                                  const QString& t1, const QString& t2,
                                  const QString& t3, const QString& t4,
                                  int fontSize, int level, int parentIndex,
                                  bool isExpand) {
  if (!qmlRoot) return;

  // QML addItem 完整参数顺序：t0,t1,t2,t3,t4,f_size,level,parentIndex,isExpand
  QMetaObject::invokeMethod(
      qmlRoot, "addItem", Q_ARG(QVariant, t0), Q_ARG(QVariant, t1),
      Q_ARG(QVariant, t2), Q_ARG(QVariant, t3), Q_ARG(QVariant, t4),
      Q_ARG(QVariant, fontSize), Q_ARG(QVariant, level),
      Q_ARG(QVariant, parentIndex), Q_ARG(QVariant, isExpand));
}

void NotesList::traverseTreeItem(QTreeWidgetItem* item, int parentQmlIndex,
                                 int level) {
  if (!item) return;

  // 普通笔记直接跳过，只处理笔记本
  if (!item->text(1).isEmpty()) {
    return;
  }

  QObject* qmlRoot = mui->qwNoteBook->rootObject();
  if (!qmlRoot) return;

  QString strName = item->text(0);
  QString strTopColor = item->text(2);

  // 统计当前笔记本【直属一级笔记】数量
  int noteCount = 0;
  int totalChild = item->childCount();
  for (int n = 0; n < totalChild; ++n) {
    QTreeWidgetItem* child = item->child(n);
    if (!child->text(1).isEmpty()) {
      noteCount++;
    }
  }
  QString strSum = QString::number(noteCount);

  int curFontSize = (level == 0) ? fontSize : (fontSize - 1);

  // 区分顶层笔记本 / 子笔记本
  QString col1Text;
  if (item->parent() == nullptr) {
    col1Text = QString::number(pNoteBookItems.size());
  } else {
    col1Text = "isNoteBook";
  }

  int currentQmlIndex = pNoteBookItems.size();
  bool expand = item->isExpanded();

  // 添加到 QML 列表
  addItemToQW_Level(qmlRoot, strName, col1Text, "", strSum, strTopColor,
                    curFontSize, level, parentQmlIndex, expand);
  pNoteBookItems.append(item);

  // 递归遍历直属子笔记本，构建层级列表
  for (int j = 0; j < totalChild; ++j) {
    QTreeWidgetItem* childItem = item->child(j);
    if (childItem->text(1).isEmpty()) {
      traverseTreeItem(childItem, currentQmlIndex, level + 1);
    }
  }
}

void NotesList::setColorFlag(QString strColor) {
  QQuickItem* root = mui->qwNoteBook->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "setColorFlag",
                            Q_ARG(QVariant, strColor));
}

void NotesList::getAllFiles(const QString& foldPath, QStringList& folds,
                            const QStringList& formats) {
  QDirIterator it(foldPath, QDir::Files | QDir::NoDotAndDotDot,
                  QDirIterator::Subdirectories);
  while (it.hasNext()) {
    it.next();
    QFileInfo fileInfo = it.fileInfo();
    if (formats.contains(fileInfo.suffix())) {  // 检测格式，按需保存
      folds << fileInfo.absoluteFilePath();
    }
  }
}

int NotesList::getNoteBookCount() {
  int count = m_Method->getCountFromQW(mui->qwNoteBook);
  return count;
}

int NotesList::getNotesListCount() {
  int count = m_Method->getCountFromQW(mui->qwNoteList);
  return count;
}

int NotesList::getNoteBookCurrentIndex() {
  int index = m_Method->getCurrentIndexFromQW(mui->qwNoteBook);
  return index;
}

int NotesList::getNotesListCurrentIndex() {
  int index = m_Method->getCurrentIndexFromQW(mui->qwNoteList);
  return index;
}

void NotesList::setNoteBookCurrentIndex(int index) {
  m_Method->setCurrentIndexFromQW(mui->qwNoteBook, index);
}

void NotesList::setNotesListCurrentIndex(int index) {
  m_Method->setCurrentIndexFromQW(mui->qwNoteList, index);
}

int NotesList::getNoteBookIndex_twToqml() {
  QTreeWidgetItem* item = tw->currentItem();
  int index = 0;
  if (item->parent() == NULL) {
    index = tw->indexOfTopLevelItem(item);
  } else {
    int index0 = tw->indexOfTopLevelItem(item->parent());
    int index1 = 0;
    int count = item->parent()->childCount();
    for (int i = 0; i < count; i++) {
      QTreeWidgetItem* item0 = item->parent()->child(i);
      if (item0 == item) break;

      if (item0->text(1).isEmpty()) index1++;
    }

    index = index0 + index1 + 1;
  }

  return index;
}

QString NotesList::getNoteBookText0(int index) {
  return m_Method->getText0(mui->qwNoteBook, index);
}

QString NotesList::getNotesListText0(int index) {
  return m_Method->getText0(mui->qwNoteList, index);
}

void NotesList::modifyNoteBookText0(QString text0, int index) {
  m_Method->modifyItemText0(mui->qwNoteBook, index, text0);
}

void NotesList::modifyNotesListText0(QString text0, int index) {
  m_Method->modifyItemText0(mui->qwNoteList, index, text0);
}

void NotesList::setNoteLabel() {
  QString notesSum = QString::number(getNotesListCount());
  int index_note = getNotesListCurrentIndex();
  QString notesSn = "";
  if (index_note >= 0) notesSn = QString::number(index_note + 1) + "/";
  mui->lblNoteList->setText(notesSn + notesSum);
  int index = getNoteBookCurrentIndex();
  m_Method->modifyItemText3(mui->qwNoteBook, index, notesSum);
}

void NotesList::loadAllRecycle() {
  m_Method->clearAllBakList(mui->qwNoteRecycle);
  int childCount = twrb->topLevelItem(0)->childCount();
  for (int i = 0; i < childCount; i++) {
    QTreeWidgetItem* childItem = twrb->topLevelItem(0)->child(i);
    QString text0 = childItem->text(0);
    QString text3 = iniDir + childItem->text(1);

    m_Method->addItemToQW(mui->qwNoteRecycle, text0, "", "", text3, 0);
  }
}

QVariant NotesList::addQmlTreeTopItem(QString strItem) {
  QQuickItem* root = mui->qwNotesTree->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject*)root, "addTopItem",
                            Q_RETURN_ARG(QVariant, item),
                            Q_ARG(QVariant, strItem));
  return item;
}

QVariant NotesList::addQmlTreeChildItem(QVariant parentItem,
                                        QString strChildItem,
                                        QString iconFile) {
  QQuickItem* root = mui->qwNotesTree->rootObject();
  QVariant item;
  QMetaObject::invokeMethod(
      (QObject*)root, "addChildItem", Q_RETURN_ARG(QVariant, item),
      Q_ARG(QVariant, parentItem), Q_ARG(QVariant, strChildItem),
      Q_ARG(QVariant, iconFile));
  return item;
}

void NotesList::clearQmlTree() {
  QQuickItem* root = mui->qwNotesTree->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "clearAll");
}

void NotesList::initQmlTree() {
  clearQmlTree();

  QString strItem, strChildItem;
  int topcount = tw->topLevelItemCount();
  for (int i = 0; i < topcount; i++) {
    QTreeWidgetItem* topItem = tw->topLevelItem(i);
    strItem = topItem->text(0);
    auto parentItem = addQmlTreeTopItem(strItem);
    int childcount = topItem->childCount();
    for (int j = 0; j < childcount; j++) {
      QTreeWidgetItem* childItem = topItem->child(j);
      strChildItem = childItem->text(0);
      if (childItem->text(1).isEmpty()) {
        auto parentItem2 =
            addQmlTreeChildItem(parentItem, strChildItem, "/res/nb.png");
        int count2 = childItem->childCount();
        for (int n = 0; n < count2; n++) {
          QString str = childItem->child(n)->text(0);
          addQmlTreeChildItem(parentItem2, str, "/res/n.png");
        }
      } else {
        addQmlTreeChildItem(parentItem, strChildItem, "/res/n.png");
      }
    }
  }
}

QStringList NotesList::extractLocalImagesFromMarkdown(const QString& filePath) {
  QStringList images;
  QFile file(filePath);

  // 打开文件
  if (!file.open(QIODevice::ReadOnly)) {
    return images;  // 返回空列表如果打开失败
  }

  QTextStream in(&file);
  QString content = in.readAll();
  file.close();

  // 正则表达式匹配所有图片路径
  QRegularExpression re("!\\[.*?\\]\\((.*?)\\)");
  QRegularExpressionMatchIterator matchIterator = re.globalMatch(content);

  while (matchIterator.hasNext()) {
    QRegularExpressionMatch match = matchIterator.next();
    QString captured = match.captured(1).trimmed();  // 去除首尾空格

    // 提取第一个空格前的部分作为路径（处理可能存在的标题）
    QString path = captured.section(' ', 0, 0);

    // 过滤网络路径并检查非空
    if (!path.startsWith("http://") && !path.startsWith("https://") &&
        !path.isEmpty()) {
      images.append(path);
    }
  }

  // 可选：去重
  images.removeDuplicates();

  qDebug() << "images=" << images;

  return images;
}

QStringList NotesList::getValidMDFiles() { return validMDFiles; }
template class QFutureWatcher<ResultsMap>;

void NotesList::restoreNoteFromRecycle() {
  int count = m_Method->getCountFromQW(mui->qwNoteRecycle);
  if (count == 0) return;

  int index = m_Method->getCurrentIndexFromQW(mui->qwNoteRecycle);
  if (index < 0) return;

  if (getNoteBookCount() == 0) return;

  setTWRBCurrentItem();
  on_btnBatchRestore_clicked();
}

// 安全的文件写入函数
bool NotesList::safeWriteFile(const QString& filePath, const QString& content) {
  QTemporaryFile tempFile;
  if (tempFile.open()) {
    QTextStream stream(&tempFile);
    stream << content;
    tempFile.close();

    // 原子操作：替换原文件
    return QFile::rename(tempFile.fileName(), filePath);
  }
  return false;
}

void NotesList::genCursorText() {
  // 直接从文件读取内容（避免创建临时UI组件）
  QString strBuffer = loadText(currentMDFile);
  if (strBuffer.isEmpty()) {
    qWarning() << "Loaded text is empty for file:" << currentMDFile;
    return;
  }

  int curPos = mw_one->m_ReceiveShare->getCursorPos();
  curPos = qBound(0, curPos, strBuffer.length());

  // 使用QString方法替代QTextCursor操作
  QString str0 = strBuffer.mid(qMax(0, curPos - 5), 5);
  QString str1 = strBuffer.mid(curPos, 5);

  QString curText =
      QString::number(curPos) + "  (\"" + str0 + "|" + str1 + "\"" + ")";
  QString filePath = privateDir + "cursor_text.txt";

  // 使用Qt的原子写入函数
  if (!safeWriteFile(filePath, curText)) {
    qCritical() << "Failed to write cursor_text file:" << filePath;
  } else {
    qDebug() << "cursor_text saved:" << curText;
  }
}

void NotesList::setNoteName(QString name) { noteTitle = name; }

void NotesList::clearFiles() {
  QFile::remove(iniDir + "memo.zip");

  QString tempDir = iniDir;
  knot_all_files.clear();
  QStringList fmt = QString("zip;md;html;jpg;bmp;png;ini").split(';');
  getAllFiles(tempDir, knot_all_files, fmt);

  int count = knot_all_files.count();
  for (int i = 0; i < count; i++) {
    QString filePath = knot_all_files.at(i);

    QFile file(filePath);
    if (filePath.contains(".sync-conflict-")) {
      file.remove();
    }
  }

  clearMD_Pic();
}

void NotesList::clearMD_Pic() {
  // 获取所有MD文件和图片文件
  QStringList allmdFiles = m_Method->getMdFilesInDir(iniDir + "memo/", true);
  QStringList allimgFiles;
  QStringList fmt = QString("jpg;bmp;png").split(';');
  getAllFiles(iniDir + "memo/images/", allimgFiles, fmt);

  // 存储所有被引用的图片文件名
  QStringList usedImageNames;

  // 使用静态QRegularExpression对象，避免重复创建
  static const QRegularExpression regex(
      R"(!\[.*?\]\(([\w\-./\\]+\.(jpg|jpeg|bmp|png|gif))\))",
      QRegularExpression::CaseInsensitiveOption);

  // 1. 解析所有Markdown文件，提取引用的图片
  foreach (const QString& mdFilePath, allmdFiles) {
    QFile mdFile(mdFilePath);
    if (!mdFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qWarning() << "无法打开Markdown文件:" << mdFilePath;
      continue;
    }

    // 逐行读取，降低内存占用
    QTextStream in(&mdFile);
    QString line;
    while (in.readLineInto(&line)) {
      QRegularExpressionMatchIterator it = regex.globalMatch(line);
      while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString imagePath = match.captured(1);
        // 处理相对路径：基于当前MD文件所在目录解析图片路径
        QFileInfo mdFileInfo(mdFilePath);
        QFileInfo imageInfo(mdFileInfo.dir(), imagePath);
        usedImageNames << imageInfo.fileName();
      }
    }
    mdFile.close();
  }

  // 去重处理
  usedImageNames.removeDuplicates();

  // 2. 找出并删除未被引用的图片
  int deletedCount = 0;
  int failedCount = 0;

  foreach (const QString& imgFilePath, allimgFiles) {
    QFileInfo imgInfo(imgFilePath);
    QString imgFileName = imgInfo.fileName();

    // 检查图片是否未被引用
    if (!usedImageNames.contains(imgFileName)) {
      QFile imgFile(imgFilePath);
      if (imgFile.remove()) {
        qDebug() << "已删除未使用的图片:" << imgFilePath;
        deletedCount++;

        QString image_file;
        image_file = "KnotData/memo/images/" + imgFileName;
        needDelWebDAVFiles.append(image_file);
      } else {
        qWarning() << "删除失败:" << imgFilePath
                   << "原因:" << imgFile.errorString();
        failedCount++;
      }
    }
  }

  if (mui->chkAutoSync->isChecked() && mui->chkWebDAV->isChecked()) {
    int count = needDelWebDAVFiles.count();
    if (count > 0) m_Notes->delRemoteFile(needDelWebDAVFiles);
    m_Method->setAccessCount(needDelWebDAVFiles.count());
  }

  qDebug() << "图片清理完成 - 已删除:" << deletedCount
           << "个, 删除失败:" << failedCount << "个";
}

void NotesList::addItem(QTreeWidget* tw, QTreeWidgetItem* item) {
  item->setIcon(0, QIcon(":/res/n.png"));
  tw->setFocus();
  if (tw == twrb) {
    tw->setCurrentItem(tw->topLevelItem(0));
    QTreeWidgetItem* curItem = tw->currentItem();
    curItem->addChild(item);
  } else {
    QTreeWidgetItem* curItem = tw->currentItem();
    curItem->addChild(item);
  }

  tw->expandAll();
}

void NotesList::resetQML_List() {
  if (tw->topLevelItemCount() == 0) {
    m_Method->clearAllBakList(mui->qwNoteBook);
    m_Method->clearAllBakList(mui->qwNoteList);
    return;
  }

  int indexNoteBook = getNoteBookCurrentIndex();
  int indexNote = getNotesListCurrentIndex();

  QTreeWidgetItem* pOldNoteBook = nullptr;
  QTreeWidgetItem* pOldNote = nullptr;

  // 合法索引才取值，避免 at(-1) 崩溃
  if (indexNoteBook >= 0 && indexNoteBook < pNoteBookItems.size()) {
    pOldNoteBook = pNoteBookItems.at(indexNoteBook);
  }
  if (indexNote >= 0 && indexNote < pNoteItems.size()) {
    pOldNote = pNoteItems.at(indexNote);
  }

  loadAllNoteBook();

  // 指针匹配，查找新列表中的真实下标
  int newBookIdx = -1;
  for (int i = 0; i < pNoteBookItems.size(); ++i) {
    if (pNoteBookItems[i] == pOldNoteBook) {
      newBookIdx = i;
      break;
    }
  }

  if (newBookIdx != -1) {
    setNoteBookCurrentIndex(newBookIdx);
    clickNoteBook();
  }

  // 匹配笔记下标并恢复
  int newNoteIdx = -1;
  if (pOldNote) {
    for (int i = 0; i < pNoteItems.size(); ++i) {
      if (pNoteItems[i] == pOldNote) {
        newNoteIdx = i;
        break;
      }
    }
  }
  if (newNoteIdx != -1) {
    setNotesListCurrentIndex(newNoteIdx);
  }
}

void NotesList::resetQML_Recycle() {
  int childCount = twrb->topLevelItem(0)->childCount();
  if (childCount == 0) return;

  loadAllRecycle();

  int index = 0;
  QTreeWidgetItem* item = twrb->currentItem();
  if (item->parent() == NULL) {
    index = -1;
  } else {
    index = twrb->currentIndex().row();
  }

  m_Method->setCurrentIndexFromQW(mui->qwNoteRecycle, index);
}

void NotesList::saveCurrentNoteInfo() {
  const QString iniPath = QDir(iniDir).filePath("curmd.ini");
  const QString tempIniPath = iniPath + ".tmp";  // 临时文件，原子替换

  // 1. 路径合法性校验，防止目录不存在
  QFileInfo dirInfo(iniDir);
  if (!dirInfo.dir().exists()) {
    dirInfo.dir().mkpath(".");  // 不存在则递归创建目录
  }

  // 2. 先写入临时文件，不覆盖原文件
  {
    QSettings reg(tempIniPath, QSettings::IniFormat);

    QString str = currentMDFile;
    QString iniName = str.replace(iniDir, "");

    reg.setValue("/MainNotes/currentItem", iniName);
    reg.setValue("/MainNotes/NoteName", noteTitle);
    reg.sync();  // 确保全部落盘到临时文件
  }

  // 3. 原子替换原文件（IO安全核心：先删旧文件再改名，避免截断）
  QFile iniFile(iniPath);
  QFile tempFile(tempIniPath);

  // 存在旧文件则删除
  if (iniFile.exists()) {
    iniFile.remove();
  }
  // 临时文件更名覆盖正式ini
  if (!tempFile.rename(iniPath)) {
    // 更名失败兜底：直接复制内容
    bool tempOk = tempFile.open(QIODevice::ReadOnly);
    if (!tempOk) {
      // 失败处理逻辑
    }

    bool iniOk = iniFile.open(QIODevice::WriteOnly);
    if (!iniOk) {
      // 打开失败处理
    }

    iniFile.write(tempFile.readAll());
    tempFile.close();
    iniFile.close();
    tempFile.remove();  // 删除临时垃圾文件
  }
}

void NotesList::setTWRBCurrentItem() {
  int index = m_Method->getCurrentIndexFromQW(mui->qwNoteRecycle);
  QTreeWidgetItem* topItem = twrb->topLevelItem(0);
  twrb->setCurrentItem(topItem->child(index));
}

void NotesList::setTWCurrentItem() {
  int index0, index1;
  index0 = getNoteBookCurrentIndex();
  index1 = getNotesListCurrentIndex();
  if (index0 >= 0) tw->setCurrentItem(pNoteBookItems.at(index0));
  if (index1 >= 0) tw->setCurrentItem(pNoteItems.at(index1));
}

void NotesList::setNoteBookCurrentItem() {
  int index = getNoteBookCurrentIndex();
  if (index < 0) return;

  QString text1 = m_Method->getText1(mui->qwNoteBook, index);
  QString text2 = m_Method->getText2(mui->qwNoteBook, index);
  if (text2.isEmpty()) {
    tw->setCurrentItem(tw->topLevelItem(text1.toInt()));

  } else {
    QStringList list = text1.split("===");
    int indexMain, indexChild;
    if (list.count() == 2) {
      indexMain = list.at(0).toInt();
      indexChild = list.at(1).toInt();

      QTreeWidgetItem* topItem = tw->topLevelItem(indexMain);
      QTreeWidgetItem* childItem = topItem->child(indexChild);
      tw->setCurrentItem(childItem);
    }
  }
}