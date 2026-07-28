#include "NotesList.h"
#include "src/AI/EmbeddingEngine.h"

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

    // ✅ 预编译高亮正则，避免循环内重复创建
    QRegularExpression highlightRe(
        QString("(%1)").arg(QRegularExpression::escape(keyword)),
        QRegularExpression::CaseInsensitiveOption);

    for (const QString& file : files) {
      ExactMatchResult emr;
      emr.filePath = file;
      emr.title = m_Notes->m_NoteIndexManager->getNoteTitle(file);
      emr.lineNumber = -1;
      emr.matchCount = 0;

      // ✅ 1. 先检查标题是否匹配
      bool titleMatched = regex.match(emr.title).hasMatch();
      if (titleMatched) {
        emr.matchCount++;
        // 标题匹配时，用标题作为预览源，lineNumber 保持 -1 表示非正文匹配
        emr.preview = emr.title;
      }

      // ✅ 2. 再搜索文件内容
      QFile f(file);
      if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&f);
        int lineNum = 0;
        while (!in.atEnd()) {
          lineNum++;
          QString line = in.readLine();
          if (regex.match(line).hasMatch()) {
            emr.allLineNumbers.append(lineNum);
            emr.matchCount++;
            // 仅取首个正文匹配行作为预览源（标题匹配不覆盖此逻辑）
            // 如果标题已匹配且尚无正文匹配预览，仍优先保留标题预览
            // 若希望正文预览优先，去掉 titleMatched 判断即可
            if (emr.lineNumber == -1 && !titleMatched) {
              emr.lineNumber = lineNum;
              emr.preview = line.trimmed();
            } else if (emr.lineNumber == -1 && titleMatched) {
              // 标题已设为预览，记录首个正文匹配行号但不替换预览
              emr.lineNumber = lineNum;
            }
          }
        }
        f.close();
      }

      // ✅ 3. 有匹配才加入结果
      if (emr.matchCount > 0) {
        // 预览清洗
        const int maxLen = 200;
        if (emr.preview.length() > maxLen)
          emr.preview = emr.preview.left(maxLen).trimmed() + "...";
        emr.preview.replace(QRegularExpression("[\\r\\n]+"), " ");
        emr.preview = emr.preview.simplified();

        // 关键词高亮
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

void NotesList::startBackgroundTaskUpdateFilesIndex() {
  QFuture<void> future = QtConcurrent::run([=]() {

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

  });

  QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
  connect(watcher, &QFutureWatcher<void>::finished, this, [=]() {
    qDebug() << "Database del files completed.";

    // ✅ 内存操作回到主线程，安全且无性能损失
    for (const QString& file : files) {
      m_Notes->m_NoteIndexManager->removeNote(file);

      // 清理向量引擎中的向量(暂时不清理，因为这只是让笔记进入回收站）
      // m_Notes->removeNoteVector(file);

      // 🆕 精确失效该文件的图谱缓存
      m_NotesList->m_graphController->parser()->invalidateNoteCache(
          file, NoteRelationParser::CACHE_DELETE);
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

void NotesList::startVectorSerach(const QString& text) {
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
      isVectorSearchDone = false;

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
              // ✅ 适配层：基于 charStart/charEnd 从原文精准截取预览

              adaptedResults.clear();
              adaptedResults.reserve(results.size());

              for (const auto& item : results) {
                SearchResult sr;
                sr.filePath = item.filePath;

                sr.title =
                    m_Notes->m_NoteIndexManager->getNoteTitle(sr.filePath);

                // 2. 精准回源截取预览
                QString rawPreview;
                bool sourceExtracted = false;

                // 仅当 charStart/charEnd 有效时才尝试读取原文
                if (item.charStart >= 0 && item.charEnd > item.charStart) {
                  QFile f(item.filePath);
                  if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    // ⚠️ 关键：必须用 QTextStream 按 UTF-8
                    // 读取，确保字节偏移与字符偏移一致
                    QTextStream stream(&f);
                    stream.setEncoding(QStringConverter::Utf8);

                    // 直接跳转到目标位置并截取指定长度
                    if (stream.seek(item.charStart)) {
                      const qsizetype readLen = item.charEnd - item.charStart;
                      rawPreview = stream.read(readLen);
                      sourceExtracted = !rawPreview.isEmpty();
                    }
                    f.close();
                  }
                }

                // Fallback：回源失败时降级使用 DB 中的 snippet
                if (!sourceExtracted) {
                  rawPreview = item.snippet;
                }

                // 3. 安全截断与格式化（防止极端情况下原文过长）
                const qsizetype maxPreviewLen = 200;
                if (rawPreview.length() > maxPreviewLen) {
                  sr.preview = rawPreview.left(maxPreviewLen).trimmed() + "...";
                } else {
                  sr.preview = rawPreview.trimmed();
                }

                // 4. ✅ 深度清洗：过滤乱码、控制符与Markdown残留
                // 4.1 移除 Unicode
                // 替换字符及私有区/未分配区字符（修复UTF-8截断乱码）
                sr.preview.remove(QRegularExpression(
                    "[\\x{FFFD}\\x{E000}-\\x{F8FF}\\x{FFF0}-\\x{FFFF}]"));

                // 4.2 移除不可见控制字符（保留常规空白
                // \s，防止零宽空格等干扰显示）
                sr.preview.remove(
                    QRegularExpression("[\\p{Cc}\\p{Cf}&&[^\\s]]"));

                // 4.3 清理被截断的 Markdown 链接/图片语法残留 (如 "[text" 或
                // "![" )
                sr.preview.replace(QRegularExpression(R"(!?  $ [^ $  ]* $ )"),
                                   "");
                sr.preview.replace(QRegularExpression(R"(^  $ [^ $  ]*)"), "");

                // 4.4 清理换行噪音，合并多余空白
                sr.preview.replace(QRegularExpression("[\\r\\n]+"), " ");
                sr.preview = sr.preview.simplified();

                adaptedResults.append(sr);
              }

              if (isEditBoxSearchTextChanged) {
                isEditBoxSearchTextChanged = false;
                m_searchModel.setResults(adaptedResults);
                mui->lblNoteSearchResult->setText(
                    tr("AI Search Results: %1").arg(adaptedResults.size()));
              }

              isVectorSearchDone = true;

              // 桌面端AI产生链接
              if (m_Notes->isBtnAILinkClicked) {
                m_Notes->popupNoteLinkList("");
              }

              // 安卓端AI产生链接
              if (m_Notes->isAndroidAILinkGen) {
                m_Notes->isAndroidAILinkGen = false;
                popupAndroidNoteLinkList(adaptedResults);
              }
            },
            Qt::QueuedConnection);
      });
    }
  });
}

void NotesList::onSearchTextChanged(const QString& text) {
  isEditBoxSearchTextChanged = true;
  startVectorSerach(text);
}

void NotesList::popupAndroidNoteLinkList(QVector<SearchResult> adaptedResults) {
  Q_UNUSED(adaptedResults);
#ifdef Q_OS_ANDROID

  QJniEnvironment env;
  // 1. 找到NoteEditor主类
  jclass clsEditor = env->FindClass("com/x/NoteEditor");
  if (!clsEditor) {
    qDebug() << "popupAndroidNoteLinkList: 未找到 NoteEditor 类";
    return;
  }

  // 2. 获取静态回调方法 receiveAiLinkResult(List<AiLinkItem>)
  jmethodID midCallback = env->GetStaticMethodID(
      clsEditor, "receiveAiLinkResult", "(Ljava/util/List;)V");
  if (!midCallback) {
    qDebug() << "popupAndroidNoteLinkList: 未找到 receiveAiLinkResult 静态方法";
    env->DeleteLocalRef(clsEditor);
    return;
  }

  // 3. 创建 ArrayList 对象
  jclass clsArrayList = env->FindClass("java/util/ArrayList");
  jmethodID ctorArrayList = env->GetMethodID(clsArrayList, "<init>", "()V");
  jobject objList = env->NewObject(clsArrayList, ctorArrayList);
  jmethodID midAdd =
      env->GetMethodID(clsArrayList, "add", "(Ljava/lang/Object;)Z");

  // 4. 获取内部静态类 AiLinkItem 的构造方法
  jclass clsAiItem = env->FindClass("com/x/NoteEditor$AiLinkItem");
  jmethodID ctorAiItem = env->GetMethodID(
      clsAiItem, "<init>",
      "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");

  // 5. 循环遍历 SearchResult，转换为 AiLinkItem 存入 List
  for (const SearchResult& res : adaptedResults) {
    jstring jPath = env->NewStringUTF(res.filePath.toUtf8().constData());
    jstring jTitle = env->NewStringUTF(res.title.toUtf8().constData());
    jstring jPreview = env->NewStringUTF(res.preview.toUtf8().constData());

    jobject itemObj =
        env->NewObject(clsAiItem, ctorAiItem, jPath, jTitle, jPreview);
    env->CallBooleanMethod(objList, midAdd, itemObj);

    // 释放临时字符串与条目局部引用，防止内存泄漏
    env->DeleteLocalRef(jPath);
    env->DeleteLocalRef(jTitle);
    env->DeleteLocalRef(jPreview);
    env->DeleteLocalRef(itemObj);
  }

  // 6. 调用 Java 静态回调函数
  env->CallStaticVoidMethod(clsEditor, midCallback, objList);

  // 统一释放所有顶层局部引用
  env->DeleteLocalRef(objList);
  env->DeleteLocalRef(clsArrayList);
  env->DeleteLocalRef(clsAiItem);
  env->DeleteLocalRef(clsEditor);

#endif
}
