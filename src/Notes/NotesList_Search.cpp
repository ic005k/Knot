#include "NotesList.h"

// 文件搜索实现
QStringList findMarkdownFiles(const QString& dirPath) {
  Q_UNUSED(dirPath);

  QList<QString> paths;
  QDir dir(iniDir + "memo/");
  const QStringList files = dir.entryList(QStringList() << "*.md", QDir::Files);
  for (const QString& file : files) {
    QFileInfo info(dir.absoluteFilePath(file));
    QString canonicalPath = info.canonicalFilePath();  // 规范化路径
    if (!canonicalPath.isEmpty() && info.exists()) {
      paths.append(canonicalPath);
    }
  }
  // 使用 QSet 去重
  return QSet<QString>(paths.begin(), paths.end()).values();
}

MySearchResult searchInFile(const QString& filePath,
                            const QRegularExpression& regex) {
  MySearchResult result;
  QFile file(filePath);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream in(&file);
    int lineNum = 0;
    while (!in.atEnd()) {
      ++lineNum;
      if (regex.match(in.readLine()).hasMatch()) {
        result.filePath = filePath;
        result.lineNumbers.append(lineNum);
      }
    }
  }
  return result;
}

// 结果归并
void reduceResults(ResultsMap& result, const MySearchResult& partial) {
  if (!partial.filePath.isEmpty()) {
    result[partial.filePath] = partial;
  }
}

QFuture<ResultsMap> NotesList::performSearchAsync(const QString& dirPath,
                                                  const QString& keyword) {
  return QtConcurrent::run([this, dirPath, keyword]() {
    QStringList files = findMarkdownFiles(dirPath);
    QStringList cycleFiles = m_NotesList->getRecycleNoteFiles();

    files.removeIf([&cycleFiles](const QString& file) {
      return cycleFiles.contains(file);
    });

    QRegularExpression regex(
        keyword, QRegularExpression::CaseInsensitiveOption |
                     QRegularExpression::UseUnicodePropertiesOption);

    // 注意这里用 blockingMappedReduced
    return QtConcurrent::blockingMappedReduced(
        files, SearchMapper(regex), reduceResults,
        QtConcurrent::ReduceOption::UnorderedReduce);
  });
}

void NotesList::displayResults(const ResultsMap& results) {
  for (auto it = results.begin(); it != results.end(); ++it) {
    if (!isAndroid) {
      qDebug() << "文件：" << it.key();
      qDebug() << "匹配行号：" << it.value().lineNumbers;
    }

    QString file = it.key();
    QList<int> lineNumbersList = it.value().lineNumbers;
    QString strLine;
    for (int i = 0; i < lineNumbersList.count(); i++) {
      strLine = strLine + " " + QString::number(lineNumbersList.at(i));
    }
    m_NotesList->searchResultList.append(file + "-==-" + strLine.trimmed());
  }
  qDebug() << m_NotesList->searchResultList;

  m_NotesList->showNotesSearchResult();
}

void NotesList::showNotesSearchResult() {
  if (searchResultList.count() == 0) {
    mw_one->closeProgress();
    return;
  }

  mui->frameNoteList->hide();
  mui->frameNotesSearchResult->show();

  m_Method->clearAllBakList(mui->qwNotesSearchResult);
  for (int i = 0; i < searchResultList.count(); i++) {
    QStringList list = searchResultList.at(i).split("-==-");
    QString note_name = getCurrentNoteNameFromMDFile(list.at(0));
    if (note_name != "")
      m_Method->addItemToQW(mui->qwNotesSearchResult, note_name, list.at(0),
                            list.at(1), "", 0);
  }

  mw_one->closeProgress();
}

void NotesList::startFind(QString strFind) {
  mw_one->showProgress();

  QString directory = iniDir + "memo/";
  QString keyword = strFind;
  searchResultList.clear();
  findCount = -1;

  // ========== 如果已有任务，必须先安全停止 ==========
  if (watcher) {
    // 取消旧任务
    watcher->cancel();
    // 等待旧任务真正结束（防止异步对象被提前销毁）
    if (watcher->isRunning()) {
      watcher->waitForFinished();
    }
    // 断开旧信号槽，避免重复连接
    disconnect(watcher, nullptr, this, nullptr);
  }

  // ========== 重新创建 watcher ==========
  watcher = new QFutureWatcher<ResultsMap>(this);
  connect(watcher, &QFutureWatcher<ResultsMap>::finished, this,
          &NotesList::onSearchFinished);

  // 启动新任务
  auto future = performSearchAsync(directory, keyword);
  watcher->setFuture(future);
}

// 通用导航函数：step 为 -1 表示上一个，1 表示下一个
void NotesList::navigateFindResult(int step) {
  // 检查搜索结果列表是否为空
  if (searchResultList.isEmpty()) {
    mui->lblFindNoteCount->setText("0 -> 0");
    return;
  }

  // 更新当前索引（根据步长调整）
  findCount += step;

  // 处理循环边界（索引超出范围时循环到首尾）
  int listCount = searchResultList.count();
  if (findCount < 0) {
    findCount = listCount - 1;
  } else if (findCount >= listCount) {
    findCount = 0;
  }

  // 解析当前结果项（带格式校验）
  QStringList list = searchResultList.at(findCount).split("-==-");
  if (list.size() >= 3) {  // 确保分割后有足够的元素
    QString md_file = list.at(0);
    mui->lblShowLineSn->setText(list.at(2));
    setCurrentItemFromMDFile(md_file);
    clickNoteList();
  }

  // 更新计数标签
  mui->lblFindNoteCount->setText(QString::number(findCount + 1) + " -> " +
                                 QString::number(listCount));

  // 隐藏安卓键盘（如果可见）
  if (pAndroidKeyboard->isVisible()) {
    pAndroidKeyboard->hide();
  }
}

// 上一个结果（复用通用导航函数，步长为 -1）
void NotesList::goPrevious() { navigateFindResult(-1); }

// 下一个结果（复用通用导航函数，步长为 1）
void NotesList::goNext() { navigateFindResult(1); }

void NotesList::goFindResult(int index) {
  if (findResult.count() == 0) return;

  findCount = index;
  QString str = findResult.at(index);
  QStringList list = str.split("===");
  if (list.at(1) == "NoteBook") {
    int indexNoteBook = list.at(0).toInt();
    setNoteBookCurrentIndex(indexNoteBook);
    clickNoteBook();
    setNotesListCurrentIndex(-1);
  } else {
    int index0, index1;
    index0 = list.at(0).toInt();
    index1 = list.at(1).toInt();
    setNoteBookCurrentIndex(index0);
    clickNoteBook();
    setNotesListCurrentIndex(index1);
  }

  mui->lblFindNoteCount->setText(QString::number(findCount + 1) + " -> " +
                                 QString::number(findResult.count()));
}

void NotesList::initSerachDatabase() {
  QString databaseFile = privateDir + "md_database_v3.db";
  m_dbManager.initDatabase(databaseFile);
  QFile m_dfile(databaseFile);
  if (m_dfile.size() < 100000) {
    startBackgroundTaskUpdateFilesIndex();
  }
}

void NotesList::startBackgroundTaskUpdateFilesIndex() {
  QString fullPath = iniDir + "memo";  // 先构造完整路径

  QFuture<void> future = QtConcurrent::run([=]() {
    m_dbManager.updateFilesIndex(fullPath);  // 值捕获保证线程安全
  });

  // 可选：使用 QFutureWatcher 监控进度
  QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
  connect(watcher, &QFutureWatcher<void>::finished, this, [=]() {
    qDebug() << "Database update completed.";
    mw_one->closeProgress();

    QStringList cycleFiles = getRecycleNoteFiles();
    if (cycleFiles.count() > 0) startBackgroundTaskDelFilesIndex(cycleFiles);

    watcher->deleteLater();
  });
  watcher->setFuture(future);
}

void NotesList::startBackgroundTaskDelFilesIndex(const QStringList& files) {
  QFuture<void> future = QtConcurrent::run(
      [this, files]() { m_dbManager.batchDeleteFileIndexes(files); });

  QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
  connect(watcher, &QFutureWatcher<void>::finished, this, [=]() {
    qDebug() << "Database del files completed.";
    watcher->deleteLater();
  });
  watcher->setFuture(future);
}

void NotesList::showFindNotes() {
  recycleNotesList.clear();
  int count = twrb->topLevelItem(0)->childCount();
  for (int i = 0; i < count; i++) {
    QString file = iniDir + twrb->topLevelItem(0)->child(i)->text(1);
    recycleNotesList.append(file);
  }
  qDebug() << "recycle notes = " << recycleNotesList;

  mui->frameNoteList->hide();
  mui->frameNotesSearchResult->show();
  mui->editNotesSearch->setFocus();

  openSearch();
}

void NotesList::openSearch() { return; }

QString NotesList::getSearchResultQmlFile() {
  QQuickItem* root = mui->qwNotesSearchResult->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject*)root, "getQmlCurrentMDFile",
                            Q_RETURN_ARG(QVariant, item));
  return item.toString();
}
