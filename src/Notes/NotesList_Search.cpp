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

QFuture<QVector<ExactMatchResult>> NotesList::performSearchAsync(
    const QString& dirPath, const QString& keyword) {
  return QtConcurrent::run([dirPath, keyword]() {
    QVector<ExactMatchResult> results;
    QStringList files = findMarkdownFiles(dirPath);
    QStringList cycleFiles = m_NotesList->getRecycleNoteFiles();

    files.removeIf(
        [&cycleFiles](const QString& f) { return cycleFiles.contains(f); });

    QRegularExpression regex(
        keyword, QRegularExpression::CaseInsensitiveOption |
                     QRegularExpression::UseUnicodePropertiesOption);

    for (const QString& file : files) {
      QFile f(file);
      if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) continue;

      QTextStream in(&f);
      // in.setCodec("UTF-8");

      ExactMatchResult emr;
      emr.filePath = file;

      emr.title = m_Notes->m_NoteIndexManager->getNoteTitle(file);

      emr.lineNumber = -1;
      emr.matchCount = 0;

      int lineNum = 0;
      while (!in.atEnd()) {
        lineNum++;
        QString line = in.readLine();
        if (regex.match(line).hasMatch()) {
          emr.allLineNumbers.append(lineNum);
          emr.matchCount++;
          // 仅取首个匹配行作为预览源
          if (emr.lineNumber == -1) {
            emr.lineNumber = lineNum;
            emr.preview = line.trimmed();
          }
        }
      }
      f.close();

      if (emr.matchCount > 0) {
        // ✅ 预览清洗（与AI搜索适配层保持一致的处理逻辑）
        const int maxLen = 200;
        if (emr.preview.length() > maxLen)
          emr.preview = emr.preview.left(maxLen).trimmed() + "...";
        emr.preview.replace(QRegularExpression("[\\r\\n]+"), " ");
        emr.preview = emr.preview.simplified();

        // ✅ 注入关键词高亮（复用 DatabaseManager 的方案）
        QRegularExpression highlightRe(
            QString("(%1)").arg(QRegularExpression::escape(keyword)),
            QRegularExpression::CaseInsensitiveOption);

        emr.preview.replace(
            highlightRe,
            "<span style='background-color:#fff9c4; color:#c62828; "
            "padding:2px; border-radius:2px;'>\\1</span>");

        results.append(emr);
      }
    }
    return results;
  });
}

void NotesList::startFind(QString strFind) {
  mw_one->showProgress();

  QString directory = iniDir + "memo/";
  QString keyword = strFind;
  searchResultList.clear();
  findCount = -1;

  // ========== 如果已有任务，必须先安全停止 ==========
  if (watcher != nullptr) {
    watcher->cancel();
    if (watcher->isRunning()) {
      watcher->waitForFinished();
    }
    disconnect(watcher, nullptr, this, nullptr);
  }

  // ✅ 重新创建 watcher（类型已更新）
  watcher = new QFutureWatcher<QVector<ExactMatchResult>>(this);

  // ✅ 连接信号（类型自动推导，无需显式指定模板参数）
  connect(watcher, &QFutureWatcher<QVector<ExactMatchResult>>::finished, this,
          &NotesList::onSearchFinished);

  // 启动新任务
  auto future = performSearchAsync(directory, keyword);
  watcher->setFuture(future);
}

void NotesList::onSearchFinished() {
  if (!watcher) return;
  mw_one->safeCloseProgress();

  const QVector<ExactMatchResult> exactResults = watcher->result();

  if (exactResults.isEmpty()) {
    m_searchModel.setResults({});
    mui->lblNoteSearchResult->setText(tr("Note Search Results: 0"));

    auto msg = std::make_unique<ShowMessage>(mw_one);
    msg->showMsg("Knot", tr("No match was found."), 1);
  } else {
    // ========== 适配层：ExactMatchResult → SearchResult ==========
    QVector<SearchResult> adaptedResults;
    adaptedResults.reserve(exactResults.size());

    for (const auto& emr : exactResults) {
      SearchResult sr;
      sr.filePath = emr.filePath;
      sr.title = emr.title;
      sr.preview = emr.preview;  // 已在后台清洗完毕，直接赋值
      adaptedResults.append(sr);
    }

    // ✅ 设置UI模型（复用同一个 SearchModel）
    m_searchModel.setResults(adaptedResults);

    // ✅ 缓存精准搜索专属导航数据（不参与UI显示）
    m_exactMatchCache = exactResults;
    m_currentExactMatchIndex = 0;

    // 更新UI状态
    mui->lblNoteSearchResult->setText(tr("Note Search Results:") +
                                      QString::number(adaptedResults.size()));

    mui->editNotesSearch->hide();
    mui->btnClearSearchResults->hide();
    mui->frameNoteList->hide();
    mui->frameNotesSearchResult->show();
  }

  watcher->deleteLater();
  watcher = nullptr;
}

void NotesList::initSerachDatabase() {
  QString databaseFile = privateDir + "md_database_v3.db";
  // m_dbManager.initDatabase(databaseFile);
  QFile m_dfile(databaseFile);
  if (m_dfile.size() < 100000) {
    startBackgroundTaskUpdateFilesIndex();
  }
}

void NotesList::startBackgroundTaskUpdateFilesIndex() {
  QString fullPath = iniDir + "memo";  // 先构造完整路径

  QFuture<void> future = QtConcurrent::run([=]() {
    // m_dbManager.updateFilesIndex(fullPath);  // 值捕获保证线程安全
  });

  // 可选：使用 QFutureWatcher 监控进度
  QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
  connect(watcher, &QFutureWatcher<void>::finished, this, [=]() {
    qDebug() << "Database update completed.";
    mw_one->safeCloseProgress();

    QStringList cycleFiles = getRecycleNoteFiles();
    if (cycleFiles.count() > 0) startBackgroundTaskDelFilesIndex(cycleFiles);

    watcher->deleteLater();
  });
  watcher->setFuture(future);
}

void NotesList::startBackgroundTaskDelFilesIndex(const QStringList& files) {
  QFuture<void> future = QtConcurrent::run([this, files]() {
    // m_dbManager.batchDeleteFileIndexes(files);
  });

  QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
  connect(watcher, &QFutureWatcher<void>::finished, this, [=]() {
    qDebug() << "Database del files completed.";

    // ✅ 内存操作回到主线程，安全且无性能损失
    for (const QString& file : files) {
      m_Notes->m_NoteIndexManager->removeNote(
          file);                        // emit noteRemoved → 清理向量搜索哈希表
      m_Notes->removeNoteVector(file);  // 清理向量引擎中的向量
    }

    qDebug() << "Vector del files completed.";
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
  mui->editNotesSearch->show();
  mui->btnClearSearchResults->show();
}

QString NotesList::getSearchResultQmlFile() {
  QQuickItem* root = mui->qwNotesSearchResult->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject*)root, "getQmlCurrentMDFile",
                            Q_RETURN_ARG(QVariant, item));
  return item.toString();
}

void NotesList::onSearchTextChanged(const QString& text) {
  QTimer::singleShot(300, this, [this, text]() {
    // 空文本时清空结果
    if (text.trimmed().isEmpty()) {
      mui->lblNoteSearchResult->setText(tr("Note Search Results: 0"));
      return;
    }

    mw_one->mySearchText = text;

    if (isLocalAIModel && m_vectorSearchService && g_embEngine &&
        g_embEngine->isValid()) {
      // ✅ 向量搜索：异步执行，避免阻塞UI
      mui->lblNoteSearchResult->setText(tr("Searching (AI)..."));

      ////////////////////////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////////////

      // 🔍 诊断测试代码
      /*{
        // 1. 检查索引是否为空
        int indexSize = m_vectorSearchService->debugIndexSize();
        qDebug() << "[DIAG] 当前向量索引大小:" << indexSize;

        // 2. 验证嵌入模型
        QString testText = text.isEmpty() ? QStringLiteral("红楼梦") : text;
        auto testVec = g_embEngine->encode(testText);

        bool isZeroVec = testVec.isEmpty() ||
                         std::all_of(testVec.constBegin(), testVec.constEnd(),
                                     [](float v) { return v == 0.0f; });

        qDebug() << "[DIAG] 测试文本:" << testText
                 << "| 向量维度:" << testVec.size() << "| 全零:" << isZeroVec;

        // 3. 端到端验证
        if (!isZeroVec && testVec.size() > 0) {
          QString testId = QStringLiteral("__diag_test__");
          bool selfFound = m_vectorSearchService->debugAddAndSearch(
              testId, testVec, testText);
          qDebug() << "[DIAG] 自检索是否命中自身:" << selfFound;
          m_vectorSearchService->debugRemove(testId);
        }

        // 4. 提前终止无效搜索
        if (indexSize == 0 || isZeroVec) {
          mui->lblNoteSearchResult->setText(
              tr("⚠️ AI诊断: 索引=%1, 向量%2")
                  .arg(indexSize)
                  .arg(isZeroVec ? "无效" : "正常"));
          return;
        }
      }*/

      //////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////

      QtConcurrent::run([this, text]() {
        // 在后台线程执行向量检索,最多返回25条，且相似度≥0.5
        auto results = m_vectorSearchService->search(text, 20, 0.5f);

        // ⚠必须回到主线程更新UI模型
        QMetaObject::invokeMethod(
            this,
            [this, results]() {
              // ✅ 适配层：将向量搜索结果转换为 SearchModel 期望的格式

              QVector<SearchResult> adaptedResults;
              adaptedResults.reserve(results.size());

              for (const auto& item : results) {
                SearchResult sr;

                // 1. 基础字段直接映射
                sr.filePath = item.filePath;
                sr.title = item.noteName.isEmpty()
                               ? QFileInfo(item.filePath).baseName()
                               : item.noteName;

                // 2. snippet → preview 的格式化（关键！）
                // 向量搜索返回的是完整 chunk，直接显示会撑爆 UI，必须截断
                const int maxPreviewLen = 200;
                if (item.snippet.length() > maxPreviewLen) {
                  sr.preview =
                      item.snippet.left(maxPreviewLen).trimmed() + "...";
                } else {
                  sr.preview = item.snippet;
                }

                // 3. 清理 chunk 中可能残留的 Markdown/换行噪音
                sr.preview.replace(QRegularExpression("[\\r\\n]+"), " ");
                sr.preview = sr.preview.simplified();  // 合并多余空格
                // ✅ 仅在 preview 显示前做视觉弱化，不触碰原始数据
                sr.preview = sr.preview.replace(
                    QRegularExpression(R"(\bfor\b)"), "·");  // 或 " "（空格）

                adaptedResults.append(sr);
              }

              m_searchModel.setResults(adaptedResults);
              mui->lblNoteSearchResult->setText(
                  tr("AI Search Results:") +
                  QString::number(adaptedResults.size()));
            },
            Qt::QueuedConnection);
      });

    } else {
      // 原有分词搜索路径（同步，因为通常很快）
      // auto results =
      //    m_dbManager.searchDocuments(text, m_Notes->m_NoteIndexManager);
      // m_searchModel.setResults(results);
      // mui->lblNoteSearchResult->setText(tr("Note Search Results:") +
      //                                  QString::number(results.count()));
    }
  });
}
