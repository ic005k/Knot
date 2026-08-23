#include "MainWindow.h"

void MainWindow::on_btn7_clicked() { m_EditRecord->on_btn7_clicked(); }

void MainWindow::on_btn8_clicked() { m_EditRecord->on_btn8_clicked(); }

void MainWindow::on_btn9_clicked() { m_EditRecord->on_btn9_clicked(); }

void MainWindow::on_btn4_clicked() { m_EditRecord->on_btn4_clicked(); }

void MainWindow::on_btn5_clicked() { m_EditRecord->on_btn5_clicked(); }

void MainWindow::on_btn6_clicked() { m_EditRecord->on_btn6_clicked(); }

void MainWindow::on_btn1_clicked() { m_EditRecord->on_btn1_clicked(); }

void MainWindow::on_btn2_clicked() { m_EditRecord->on_btn2_clicked(); }

void MainWindow::on_btn3_clicked() { m_EditRecord->on_btn3_clicked(); }

void MainWindow::on_btn0_clicked() { m_EditRecord->on_btn0_clicked(); }

void MainWindow::on_btnDot_clicked() { m_EditRecord->on_btnDot_clicked(); }

void MainWindow::on_btnDel_Number_clicked() {
  m_EditRecord->on_btnDel_clicked();
}

void MainWindow::on_btnBackNoteDiff_clicked() { m_NotesList->closeNoteDiff(); }

void MainWindow::on_btnBackBookList_clicked() {
  if (!isGpsRun) m_Reader->cancelKeepScreenOn();

  m_Reader->hideBookListWin();
  mui->frameMain->show();
}

void MainWindow::on_btnOpenBookFile_clicked() { on_btnOpen_clicked(); }

void MainWindow::on_btnOkBookList_clicked() { m_Reader->openBookListItem(); }

void MainWindow::on_btnClearAllRecords_clicked() {
  m_Reader->clearAllReaderRecords();
}

void MainWindow::on_btnAnd_clicked() { mui->editSearchText->insert("&"); }

void MainWindow::on_btnClear_clicked() { mui->editTodo->clear(); }

void MainWindow::on_btnModify_clicked() { m_Todo->reeditText(); }

void MainWindow::on_btnTabMoveUp_clicked() {
  if (tabData->count() == 0) return;
  int curIndex = tabData->currentIndex();
  if (curIndex > 0) {
    tabData->tabBar()->moveTab(curIndex, curIndex - 1);
    updateMainTab();
    saveTab();
    getMainTabs();
  }
}

void MainWindow::on_btnTabMoveDown_clicked() {
  if (tabData->count() == 0) return;
  int curIndex = tabData->currentIndex();
  if (curIndex <= tabData->count() - 2) {
    tabData->tabBar()->moveTab(curIndex, curIndex + 1);
    updateMainTab();
    saveTab();
    getMainTabs();
  }
}

void MainWindow::updateMainTab() {
  clearAll();
  for (int i = 0; i < tabData->count(); i++) {
    QString tabText = tabData->tabText(i);
    QTreeWidget* tw = get_tw(i);
    int isFlagToday = m_Method->getFlagToday(tw);
    addItem(tabText, "", "", "", isFlagToday);
  }
  setCurrentIndex(tabData->currentIndex());
}

void MainWindow::on_btnChart() { m_MainHelper->clickBtnChart(); }

void MainWindow::on_btnManagement_clicked() {
  mui->frameNotesTree->show();
  mui->frameNoteList->hide();
}

void MainWindow::on_btnUpMove_clicked() {
  if (m_Method->getCountFromQW(mui->qwNoteBook) == 0) return;

  m_NotesList->on_btnUp_clicked();
}

void MainWindow::on_btnDownMove_clicked() {
  if (m_Method->getCountFromQW(mui->qwNoteBook) == 0) return;

  m_NotesList->on_btnDown_clicked();
}

void MainWindow::on_btnDelNote_NoteBook_clicked() {
  m_NotesList->on_btnDel_clicked();
}

void MainWindow::on_btnMoveTo_clicked() {
  m_NotesList->setTWCurrentItem();
  m_NotesList->on_btnMoveTo_clicked();
}

void MainWindow::on_btnBack_Tree_clicked() {
  mui->frameNotesTree->hide();
  mui->frameNoteList->show();
}

void MainWindow::on_btnRename_clicked() { m_Notes->renameTitle(false); }

void MainWindow::on_btnHideFind_clicked() {
  closeTextToolBar();
  mui->f_FindNotes->hide();
}

void MainWindow::on_btnStepsOptions_clicked() { m_StepsOptions->init(); }

void MainWindow::on_btnRecentOpen_clicked() {
  // m_NotesList->genRecentOpenMenu();

  mui->frameNoteList->hide();
  mui->frameFavorites->show();

  m_Notes->refreshRecentOpenByCounter();
  mui->editTitleKey->clear();
  // 加载最近打开列表推送到QML
  QVariantList data = buildRecentList();
  setDisplayResult(data);

  mui->editTitleKey->setFocus();
}

void MainWindow::on_btnMenuReport_clicked() { m_Report->genReportMenu(); }

void MainWindow::on_btnBookCata_clicked() {
  if (mui->lblBookName->text() == "Book Name") return;

  mui->btnAutoStop->click();
  if (mui->f_ReaderSet->isVisible()) {
    on_btnBackReaderSet_clicked();
  }

  m_Reader->showCatalogue();
}

void MainWindow::on_btnShowBookmark_clicked() {
  m_Reader->showOrHideBookmark();
}

void MainWindow::on_btnRemoveBookList_clicked() { m_Reader->removeBookList(); }

void MainWindow::on_btnShareImage_clicked() {
  m_ReceiveShare->shareImage(tr("Share to"), bookimgFileName, "image/png");
}

void MainWindow::on_btnDelImage_clicked() {}

void MainWindow::on_btnBackReaderSet_clicked() {
  closeTextToolBar();
  mui->f_ReaderSet->hide();
  qreal pos = m_Reader->getVPos();
  m_Reader->setVPos(pos + 0.01);

  QSettings Reg(privateDir + "reader.ini", QSettings::IniFormat);
  Reg.setValue("/Reader/editAutoStopTTS", mui->editAutoStopTTS->text());
  Reg.setValue("/Reader/chkAutoStopTTS", mui->chkAutoStopTTS->isChecked());
}

void MainWindow::on_btnSetBookmark_clicked() {
  mw_one->on_btnBackReaderSet_clicked();
  QTimer::singleShot(200, this, SLOT(slotSetBookmark()));
}

void MainWindow::slotSetBookmark() { m_ReaderSet->on_btnSetBookmark_clicked(); }

void MainWindow::on_btnFontLess_clicked() {
  m_ReaderSet->on_btnFontLess_clicked();
}

void MainWindow::on_btnFontPlus_clicked() {
  m_ReaderSet->on_btnFontPlus_clicked();
}

void MainWindow::on_btnFont_clicked() { m_ReaderSet->on_btnFont_clicked(); }

void MainWindow::on_btnBackgroundColor_clicked() {
  m_ReaderSet->on_btnBackgroundColor_clicked();
}

void MainWindow::on_btnForegroundColor_clicked() {
  m_ReaderSet->on_btnForegroundColor_clicked();
}

void MainWindow::on_editBackgroundColor_textChanged(const QString& arg1) {
  m_ReaderSet->on_editBackgroundColor_textChanged(arg1);
}

void MainWindow::on_editForegroundColor_textChanged(const QString& arg1) {
  m_ReaderSet->on_editForegroundColor_textChanged(arg1);
}

void MainWindow::on_btnStyle1_clicked() { m_ReaderSet->on_btnStyle1_clicked(); }

void MainWindow::on_btnStyle2_clicked() { m_ReaderSet->on_btnStyle2_clicked(); }

void MainWindow::on_btnStyle3_clicked() { m_ReaderSet->on_btnStyle3_clicked(); }

void MainWindow::on_btnGoPage_clicked() { m_ReaderSet->on_btnGoPage_clicked(); }

void MainWindow::on_btnShareBook_clicked() { m_Reader->shareBook(); }

void MainWindow::on_btnAutoRun_clicked() {
  mui->qwViewBookNote->hide();
  mui->qwBookCata->hide();
  mui->qwBookmark->hide();
  mui->qwReader->show();

  if (!m_Reader->isAutoRun) {
    mui->qwReader->rootContext()->setContextProperty("isAutoRun", true);
    mui->btnAutoRun->hide();
    mui->btnAutoStop->show();
    m_Reader->isAutoRun = true;
  }
}

void MainWindow::on_btnAutoStop_clicked() {
  if (m_Reader->isAutoRun) {
    mui->qwReader->rootContext()->setContextProperty("isAutoRun",
                                                     QVariant(false));
    mui->btnAutoStop->hide();
    mui->btnAutoRun->show();
    m_Reader->isAutoRun = false;
  }
}

void MainWindow::on_btnLessen_clicked() { m_ReaderSet->on_btnLessen_clicked(); }

void MainWindow::on_btnDefault_clicked() {
  m_ReaderSet->on_btnDefault_clicked();
}

void MainWindow::on_btnPlus_clicked() { m_ReaderSet->on_btnPlus_clicked(); }

void MainWindow::on_btnAddTodo_clicked() { m_Todo->AddTodoText(); }

void MainWindow::on_btnAddTodo_pressed() {
  m_Todo->isRecordVoice = false;
  tmeStartRecordAudio->start(750);
}

void MainWindow::on_btnAddTodo_released() {
  tmeStartRecordAudio->stop();
  m_Todo->stopRecordVoice();
}

void MainWindow::on_btnClearReaderFont_clicked() {
  m_ReaderSet->on_btnClear_clicked();
}

void MainWindow::on_btnMove() {
  isMoveEntry = true;
  if (del_Data((QTreeWidget*)mui->tabWidget->currentWidget())) {
    mui->btnTabMoveDown->hide();
    mui->btnTabMoveUp->hide();
    on_btnSelTab_clicked();

    while (mui->frameEditRecord->isHidden()) {
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
      QThread::msleep(1);
    }

    mui->editCategory->setText(strCategory);
    mui->editDetails->setText(strDetails);
    mui->editAmount->setText(strAmount);

    on_btnOkEditRecord_clicked();
  }
}

void MainWindow::on_btnGPS_clicked() {
  if (mui->btnGPS->text() == tr("Start")) {
    m_Steps->startRecordMotion();

  } else if (mui->btnGPS->text() == tr("Stop")) {
    m_Steps->stopRecordMotion();
    mui->btnGPS->setText(tr("Start"));
  }
}

void MainWindow::on_btnSelGpsDate_clicked() { m_Steps->selGpsListYearMonth(); }

void MainWindow::on_btnGetGpsListData_clicked() {
  m_Steps->getGpsListDataFromYearMonth();
}

void MainWindow::on_btnBackBakList_clicked() {
  mui->frameMain->show();
  mui->frameBakList->hide();
}

void MainWindow::on_btnImportBakList_clicked() {
  m_MainHelper->importBakFileList();
}

void MainWindow::on_btnOkViewCate_clicked() { m_Report->on_CateOk(); }

void MainWindow::on_btnBackTabRecycle_clicked() {
  mui->frameMain->show();
  mui->frameTabRecycle->hide();
}

void MainWindow::on_btnDelTabRecycle_clicked() {
  m_MainHelper->delTabRecycleFile();
}

void MainWindow::on_btnRestoreTab_clicked() {
  m_MainHelper->clickBtnRestoreTab();
}

void MainWindow::on_btnDelBakFile_clicked() { m_MainHelper->delBakFile(); }

void MainWindow::on_btnBackNoteList_clicked() {
  // 延迟一小段时间再触发，避免模块快速切换时反复启停
  QTimer::singleShot(500, m_NotesList, &NotesList::rebuilderNotesVector);

  clearWidgetFocus();

  m_Notes->updateMainnotesIniToSyncLists();

  saveNeedSyncNotes();

  m_Notes->saveNotesCounter();

  // ========== 完全异步延迟界面切换，释放渲染资源 ==========
  QTimer::singleShot(0, this, []() {
    // 找到页面内所有QQuickWidget
    auto quickWidgets = mui->frameNoteList->findChildren<QQuickWidget*>();
    for (auto it = quickWidgets.cbegin(); it != quickWidgets.cend(); ++it) {
      QQuickWidget* w = *it;
      w->blockSignals(true);
      w->quickWindow()->releaseResources();
    }

    // 再执行显示/隐藏，此时无活跃离屏渲染任务
    mui->frameMain->show();
    mui->frameNoteList->hide();

    // 恢复信号
    for (auto it = quickWidgets.cbegin(); it != quickWidgets.cend(); ++it) {
      QQuickWidget* w = *it;
      w->blockSignals(false);
    }
  });
  // ========================================================

  if (m_Notes->checkAndUpdateCleanDate())
    qDebug() << "已到自动清理服务器文件时间，过去3个月内的文件将被清理";
  else
    qDebug() << "不到日期，不用自动清理服务器上的旧文件";

  // 先清空旧连接，避免重复触发
  disconnect(m_Notes, &Notes::syncFinished, this, nullptr);

  // 绑定：等 sync 全部结束 → 再删除
  connect(
      m_Notes, &Notes::syncFinished, this,
      []() { m_NotesList->delRemoteWebDAVFiles(); },
      Qt::ConnectionType(Qt::QueuedConnection | Qt::SingleShotConnection));

  // 现在才开始同步
  m_Notes->syncToWebDAV();
}

void MainWindow::on_btnBackNoteRecycle_clicked() {
  mui->frameNoteRecycle->hide();
  mui->frameNoteList->show();

  if (m_NotesList->isDelNoteRecycle) {
    m_Notes->startBackgroundTaskDelAndClear();
  }
}

void MainWindow::on_btnNoteRecycle_clicked() {
  mui->frameNoteList->hide();
  mui->frameNoteRecycle->show();

  m_NotesList->loadAllRecycle();
}

void MainWindow::on_btnDelNoteRecycle_clicked() {
  int count = m_Method->getCountFromQW(mui->qwNoteRecycle);
  if (count == 0) return;

  int index = m_Method->getCurrentIndexFromQW(mui->qwNoteRecycle);
  if (index < 0) return;

  m_NotesList->setTWRBCurrentItem();
  m_NotesList->on_btnBatchDel_Recycle_clicked();
}

void MainWindow::on_btnRestoreNoteRecycle_clicked() {
  isStopMoveNote = false;
  m_NotesList->restoreNoteFromRecycle();
}

void MainWindow::on_btnFindNotes_clicked() {
  QString str = mui->editFindNote->text().trimmed();
  if (str.length() == 0) return;
  mySearchText = str;
  m_NotesList->startFind(str);
}

void MainWindow::on_btnClearNoteFindText_clicked() {
  mui->editFindNote->setText("");
}

void MainWindow::on_btnShowFindNotes_clicked() { m_NotesList->showFindNotes(); }

void MainWindow::on_btnNoteBookMenu_clicked() {
  m_NotesList->showNoteBookMenu(mui->qwNoteBook->x(), mui->qwNoteBook->y());
}

void MainWindow::on_btnNoteMenu_clicked() {
  m_NotesList->showNotsListMenu(mui->qwNoteList->x(), mui->qwNoteList->y());
}

void MainWindow::on_btnCancelType_clicked() {
  m_CategoryList->on_btnCancel_clicked();
}

void MainWindow::on_btnOkType_clicked() { m_CategoryList->on_btnOk_clicked(); }

void MainWindow::on_btnDelType_clicked() {
  m_CategoryList->on_btnDel_clicked();
}

void MainWindow::on_btnRenameType_clicked() {
  m_CategoryList->ui->editRename->setText(
      mui->editRenameType->text().trimmed());
  m_CategoryList->on_Rename();
}

void MainWindow::on_btnBackSetTab_clicked() {
  mui->frameMain->show();
  mui->frameSetTab->hide();

  if (mui->btnTabMoveDown->isHidden()) {
    mui->btnTabMoveDown->show();
    mui->btnTabMoveUp->show();
    on_btnAdd_clicked();
    m_EditRecord->setCurrentValue();
  }
}

void MainWindow::on_btnBackSearch_clicked() {
  clearWidgetFocus();

  mui->frameMain->show();
  mui->frameSearch->hide();
}

void MainWindow::on_btnClearSearchText_clicked() {
  mui->editSearchText->setText("");
  mui->editSearchText->setFocus();
}

void MainWindow::on_btnStartSearch_clicked() {
  mui->editSearchText->clearFocus();

  searchStr = mui->editSearchText->text().trimmed();
  if (searchStr.length() == 0) return;

  showProgress();
  m_Method->data_for_search = m_Method->exportAllDataForSearch();

  qDebug() << "ExportAllDataForSearch:" << m_Method->data_for_search.size();

  // 安卓安全的跨线程调用
  QMetaObject::invokeMethod(
      m_searchWorker,
      [=]() {
        m_searchWorker->startSearch(m_Method->data_for_search, searchStr);
      },
      Qt::QueuedConnection);
}

void MainWindow::on_btnStartDate_clicked() {
  m_DateSelector->initStartEndDate("start");
}

void MainWindow::on_btnEndDate_clicked() {
  m_DateSelector->initStartEndDate("end");
}

void MainWindow::on_btnBackEditRecord_clicked() {
  clearWidgetFocus();

  mui->frameMain->show();
  mui->frameEditRecord->hide();
}

void MainWindow::on_btnType_clicked() { m_EditRecord->on_btnType_clicked(); }

void MainWindow::on_btnOkEditRecord_clicked() {
  m_EditRecord->on_btnOk_clicked();
}

void MainWindow::on_btnClearType_clicked() { mui->editCategory->setText(""); }

void MainWindow::on_btnClearDetails_clicked() { mui->editDetails->setText(""); }

void MainWindow::on_btnClearAmount_clicked() { mui->editAmount->setText(""); }

void MainWindow::on_btnBackSteps_clicked() { m_Steps->closeSteps(); }

void MainWindow::on_btnReset_clicked() { m_Steps->on_btnReset_clicked(); }

void MainWindow::on_btnBack_Report_clicked() { m_Report->on_btnBack_clicked(); }

void MainWindow::on_btnYear_clicked() { m_Report->on_btnYear_clicked(); }

void MainWindow::on_btnMonth_clicked() { m_Report->on_btnMonth_clicked(); }

void MainWindow::on_btnViewCategory_clicked() {
  m_Report->on_btnCategory_clicked();
}

void MainWindow::on_btnAIExerciseSuggestions_clicked() {
  // 获取当前程序生效的语言标识
  QLocale loc = QLocale::system();
  QString langCode = loc.name();

  QString Weather = m_Steps->strCurrentTemp;

  // 模板A：优先使用已有天气信息；天气为空时，依据经纬度坐标解析当地天气，生成途中运动建议
  QString promptInSport = R"(
Generate real-time on-the-way sports reminders for cycling/hiking/running based on the combined info below. All text uses language matching %1.
Sport & coordinate data: %2
Available local weather information: %3

Rules:
1. Simple Markdown (headings, bullet points) allowed, complex formats are banned.
2. Focus entirely on in-motion real-time reminders (mid-trip risks, action adjustment, on-the-go physical care, terrain hazards). Do NOT repeat pre-sport preparation content.
3. Prioritize exclusive tips for the current sport given in the data, attach brief auxiliary hints for the other two sports. Randomize layout to avoid fixed template.
4. If weather information is not empty, analyze based on the provided weather data. If weather information is blank, extract and analyze local weather from coordinates. Place prominent warning at top if extreme weather exists.
5. Rotate practical themes each request: terrain slip risk, mid-sport hydration, long-distance fatigue relief, on-road emergency handling.
6. Keep text concise without empty generic phrases, no opening/closing redundant text.
7. Full safety & injury prevention reminders for all languages.
)";

  // 模板B：无GPS定位，仅输出全新随机运动常识，不查询天气
  QString promptCommon = R"(
Output unique short outdoor tips for cycling, hiking and running, every reply different content. All text uses language %1.

Rules:
1. Simple Markdown allowed, no complex formats.
2. Random layout, rotate themes (gear maintenance, recovery, road safety, off-road skills).
3. Keep sentences concise, no repetitive generic phrases.
4. No extra opening/closing text, equal safety tips for all languages.
)";

  quint64 randNum = QRandomGenerator::global()->bounded(100000, 999999);
  QString randomTag = QString::number(randNum);
  QString fullPrompt;

  QString text = m_Steps->ai_latlon_text;
  QString trimText = text.trimmed();
  if (trimText.isEmpty()) {
    // 无GPS定位：只用通用常识模板
    fullPrompt = promptCommon.arg(langCode) + "\nRandom tag: " + randomTag;
  } else {
    fullPrompt = promptInSport.arg(langCode, trimText, Weather) +
                 "\nRandom tag: " + randomTag;
  }

  aiChatQuery(fullPrompt);
}

void MainWindow::on_btnAIExplanation_clicked() {
  QString text = mui->editSetText->text();
  QString trimText = text.trimmed();
  if (trimText.isEmpty()) {
    auto msg = std::make_unique<ShowMessage>(this);
    msg->showMsg(tr("Tip"), tr("No consumption record data available"), 0);
    return;
  }

  // qDebug() << trimText;

  // 获取当前程序生效的语言标识
  QLocale loc = QLocale::system();
  // 如果切换过软件语言，改用之前存储的语言代码变量：
  // QString langCode = m_appLanguageCode;
  QString langCode = loc.name();  // 格式 zh_CN / en_US / ja_JP

  // 标准化英文指令，明确指定输出语言，精准可控
  QString promptTemplate = R"(
Interpret the text excerpt selected from the e-book below, and output accurate, easy-to-understand localized explanations based on the specified language.
Mandatory rules that must be fully followed without omission:
1. All explanations, annotations and supplementary content must use the language corresponding to language code: %1
2. Input text to be interpreted (selected e-book content):
%2
3. Unified output structure requirements (output strictly in this order, no extra opening remarks or closing greetings):
   Part 1: Core Text Simplified Interpretation
      Rewrite the original text in plain vernacular, eliminate obscure literary expressions, ensure the original semantic logic is completely unchanged.
   Part 2: Key Difficult Point Breakdown
      Extract rare words, polysemous words, archaic words, professional jargon, allusions and complex sentence structures from the text for separate explanation;
      Special mandatory rule only for Chinese language (%1 == zh series):
         - Every rare character, rarely-used variant character must attach complete pinyin + tone mark;
         - Polyphonic characters mark the pinyin matching the context meaning;
         - Ancient Chinese vocabulary supplement modern daily usage scenarios;
      For non-Chinese languages: only explain word meaning, grammatical structure and cultural background without pinyin.
   Part 3: Two optional interpretation depth schemes for reading reference
      Scheme A (Mild, suitable for quick reading): Concise interpretation, only retain core meaning and necessary word annotations, low reading burden, fast to understand.
      Scheme B (In-depth, suitable for intensive reading): Add contextual background, rhetorical analysis, logical implication and extended relevant knowledge on the basis of complete interpretation.
4. Content limit rules:
   4.1 No redundant empty descriptions, no irrelevant digressions unrelated to the e-book text;
   4.2 Strictly retain the original text’s emotional tendency, narrative logic and core viewpoint, cannot distort or arbitrarily expand the original meaning;
   4.3 Balance readability and accuracy: avoid overly obscure academic terminology in mainstream interpretation, maintain accessibility for ordinary readers.
5. Special abnormal processing rules:
   - If the input text is blank: only output "No selected text to interpret";
   - If the text contains garbled/illegible characters: mark the garbled position and explain the recognizable surrounding content only;
   - If the text is poetry/classical prose: add brief rhythm/rhetoric analysis under the in-depth scheme.
)";

  m_Reader->isAIReaderExplanation = true;

  QString fullPrompt = promptTemplate.arg(langCode, trimText);
  aiChatQuery(fullPrompt);
}

void MainWindow::on_btnAISteps_clicked() {
  QString text = m_Steps->ai_stepstext;
  QString trimText = text.trimmed();
  if (trimText.isEmpty()) {
    auto msg = std::make_unique<ShowMessage>(this);
    msg->showMsg(tr("Tip"), tr("No consumption record data available"), 0);
    return;
  }

  // qDebug() << trimText;

  // 获取当前程序生效的语言标识
  QLocale loc = QLocale::system();
  // 如果切换过软件语言，改用之前存储的语言代码变量：
  // QString langCode = m_appLanguageCode;
  QString langCode = loc.name();  // 格式 zh_CN / en_US / ja_JP

  // 标准化英文指令，明确指定输出语言，精准可控
  QString promptTemplate = R"(
Analyze the personal daily walking steps data below and provide practical health walking suggestions.
Strict rules you must follow:
1. All analysis and suggestions must be written in language code: %1
2. First summarize your analysis: highest daily steps, lowest daily steps, abnormal step data (too few/excessive/zero steps), and unreasonable walking habits.
3. Give targeted, easy-to-operate health walking suggestions matching the daily step data characteristics.
4. Do not output redundant descriptions, only data analysis and practical health suggestions.
5. Balance walking exercise effect and physical comfort. Do not give overly harsh, extreme exercise suggestions. Provide two options for each adjustment category: one mild adjustment (less physical pressure, easy to stick to) and one aggressive improvement plan (better exercise effect) for reference.
6. Focus on judging walking health status: long-term sedentary zero steps, insufficient daily steps, excessive single-day steps that may cause physical fatigue, and irregular walking rhythm.

Daily walking steps records:

%2
)";

  QString fullPrompt = promptTemplate.arg(langCode, trimText);
  aiChatQuery(fullPrompt);
}

void MainWindow::on_btnAIReportAnalysis_clicked() {
  QString text = m_Report->catetext;
  QString trimText = text.trimmed();
  if (trimText.isEmpty()) {
    auto msg = std::make_unique<ShowMessage>(this);
    msg->showMsg(tr("Tip"), tr("No consumption record data available"), 0);
    return;
  }

  // 获取当前程序生效的语言标识
  QLocale loc = QLocale::system();
  // 如果切换过软件语言，改用之前存储的语言代码变量：
  // QString langCode = m_appLanguageCode;
  QString langCode = loc.name();  // 格式 zh_CN / en_US / ja_JP

  // 标准化英文指令，明确指定输出语言，精准可控
  QString promptTemplate = R"(
Analyze the personal consumption data below and provide practical money-saving suggestions.
Strict rules you must follow:

1. All analysis and suggestions must be written in language code: %1

2. First summarize your analysis: highest-spending categories and unreasonable consumption behavior.

3. Give targeted, easy-to-operate saving advice matching the consumption structure.

4. Do not output redundant descriptions, only analysis and suggestions.

5. Balance cost savings and quality of life. Do not give overly harsh, extreme austerity suggestions. Provide two options for each category: one mild adjustment (less impact on pleasure) and one aggressive saving plan for reference.

Consumption records:

%2
)";

  QString fullPrompt = promptTemplate.arg(langCode, trimText);
  aiChatQuery(fullPrompt);
}

void MainWindow::on_btnSync_clicked() {
  if (!mui->btnReader->isEnabled() || !mui->btnWebDAVBackup->isEnabled() ||
      !mui->btnWebDAVRestore->isEnabled())
    return;

  mui->btnWebDAVBackup->click();
}

void MainWindow::on_btnPasteTodo_clicked() { mui->editTodo->paste(); }

void MainWindow::on_btnBackDir_clicked() { m_Reader->backDir(); }

void MainWindow::on_btnWebDAVBackup_clicked() {
  if (!mui->btnReader->isEnabled()) return;
  m_CloudBackup->startBakData();
}

void MainWindow::on_btnWebDAVRestore_clicked() {
  m_CloudBackup->webDAVRestoreData();
}

void MainWindow::on_chkWebDAV_clicked() {}

void MainWindow::on_btnBack_NotesSearchResult_clicked() {
  clearWidgetFocus();
  mui->frameNotesSearchResult->hide();
  mui->frameNoteList->show();
  isOpenSearchResult = false;

  if (mui->f_FindNotes->isVisible()) {
    mui->editFindNote->setFocus();
  }
}

void MainWindow::on_btnClearSearchResults_clicked() {
  mui->editNotesSearch->clear();
  mui->editNotesSearch->setFocus();
}

void MainWindow::on_btnOpenSearchEdit_clicked() {
  QString mdFile = m_NotesList->getSearchResultQmlFile();
  if (!QFile::exists(mdFile)) return;
  isOpenSearchResult = true;
  currentMDFile = mdFile;

  on_btnEditNote_clicked();
  m_NotesList->setCurrentItemFromMDFile(mdFile);
}

void MainWindow::on_btnOpenSearchView_clicked() {
  QString mdFile = m_NotesList->getSearchResultQmlFile();
  if (!QFile::exists(mdFile)) return;

  currentMDFile = mdFile;

  m_Notes->previewNote();
  m_NotesList->setCurrentItemFromMDFile(mdFile);
}

void MainWindow::on_btnFindNotes2_clicked() {
  if (mui->f_FindNotes->isHidden()) {
    mui->f_FindNotes->show();
    mui->editFindNote->setFocus();
  } else
    mui->f_FindNotes->hide();
}

void MainWindow::on_btnTools_clicked() {
  if (mui->f_Tools->isHidden())
    mui->f_Tools->show();
  else
    mui->f_Tools->hide();
}

void MainWindow::on_btnCopyNoteLink_clicked() {
  QString mdFile = m_NotesList->getSearchResultQmlFile();
  if (!QFile::exists(mdFile)) return;
  QString file = mdFile;
  file = file.replace(iniDir, "");
  QString name = m_Notes->m_NoteManager->getNoteTitle(mdFile);
  QString strlink = "[" + name + "](" + file + ")";
  QClipboard* clipboard = QApplication::clipboard();
  clipboard->setText(strlink);

  QString res = m_Method->escapeAllHtml(strlink);

  auto msg = std::make_unique<ShowMessage>(mw_one);
  msg->showMsg(appName, res, 1);
}

void MainWindow::on_btnRotation_clicked() {
  if (mui->qwReader->isHidden()) return;

  QQuickItem* rootItem = mui->qwReader->rootObject();
  QQuickItem* orientationButton =
      rootItem->findChild<QQuickItem*>("orientationButton");
  if (orientationButton) {
    QMetaObject::invokeMethod(orientationButton, "clicked");
    m_Reader->isLandscape = !m_Reader->isLandscape;
    m_Reader->readReadNote(m_Reader->cPage);
  }
}

void MainWindow::on_btnAddBookNote_clicked() { m_Reader->addBookNote(""); }

void MainWindow::on_btnViewBookNote_clicked() { m_Reader->viewBookNote(); }

void MainWindow::on_btnMap_clicked() { m_Steps->openMapWindow(); }

void MainWindow::on_btnSportsChart_clicked() { m_Steps->showSportsChart(); }

void MainWindow::on_btnSpeak_clicked() {
  mui->btnSpeak->hide();
  mui->btnStopSpeak->show();

  isPlayBook = true;
  m_Reader->setAutoStopPlayTime();
  m_Reader->startSpeak();
}

void MainWindow::on_btnStopSpeak_clicked() {
  mui->btnStopSpeak->hide();
  mui->btnSpeak->show();
  m_Reader->stopSpeak();
}

void MainWindow::on_btnSteps_clicked() { m_Steps->openStepsUI(); }

void MainWindow::on_btnNotes_clicked() { m_Notes->openNotes(); }

void MainWindow::on_btnAdd_clicked() {
  // m_EditRecord->monthSum();

  m_EditRecord->on_AddRecord();
}

void MainWindow::on_btnDel_clicked() {
  isMoveEntry = false;
  del_Data((QTreeWidget*)mui->tabWidget->currentWidget());
}

void MainWindow::resizeEvent(QResizeEvent* event) {
  const QSize sz = event->size();
  if (sz.width() <= 0 || sz.height() <= 0) {
    event->ignore();
    return;
  }

  QMainWindow::resizeEvent(event);

  mui->qwReader->rootContext()->setContextProperty("myW", mw_one->width());
  mui->qwReader->rootContext()->setContextProperty("myH", mw_one->height());
  mui->qwTodo->rootContext()->setContextProperty("isBtnVisible",
                                                 QVariant(false));
  mui->qwSteps->rootContext()->setContextProperty("myW", this->width());

#ifdef Q_OS_ANDROID

#else
  if (!mui->frameTodo->isHidden()) {
    mui->qwTodo->rootContext()->setContextProperty("m_width", mw_one->width());
    m_Todo->init_Todo();
  }

  if (!mui->frameNoteList->isHidden()) {
    m_NotesList->clickNoteBook();
  }
#endif
}

void MainWindow::on_btnBackTodo_clicked() { m_Todo->closeTodo(); }

void MainWindow::on_btnHigh() { m_Todo->on_btnHigh(); }

void MainWindow::on_btnLow() { m_Todo->on_btnLow(); }

void MainWindow::on_btnSetTime() { m_Todo->on_btnSetTime(); }

void MainWindow::on_btnRecycle() { m_Todo->on_btnRecycle(); }

void MainWindow::on_btnReturnRecycle_clicked() {
  m_Todo->on_btnReturn_clicked();
}

void MainWindow::on_btnClearRecycle_clicked() { m_Todo->on_btnClear_clicked(); }

void MainWindow::on_btnDelRecycle_clicked() { m_Todo->on_btnDel_clicked(); }

void MainWindow::on_btnRestoreRecycle_clicked() {
  m_Todo->on_btnRestore_clicked();
}

void MainWindow::on_btnSelText() {
  if (mui->f_ReaderSet->isVisible()) {
    on_btnBackReaderSet_clicked();
  }
  m_Reader->selectText();
}

void MainWindow::on_btnDownload_clicked() {
  m_CloudBackup->on_pushButton_downloadFile_clicked();
}

void MainWindow::on_btnBack_One_clicked() { m_CloudBackup->backExit(); }

void MainWindow::on_btnBackNotesGraph_clicked() {
  mui->frameNotesGraph->hide();
  mui->frameNoteList->show();
  m_NotesList->clickNoteList();
}

void MainWindow::on_btnCopy_clicked() {
  QClipboard* clipboard = QApplication::clipboard();
  clipboard->setText(mui->editSetText->text().trimmed());
}

void MainWindow::on_btnCancelSel_clicked() {
  m_Reader->resetTextSelection();

  mui->f_ReaderNote->hide();

  mui->qwReader->show();
  mui->f_ReaderFun->show();
  m_Reader->isSelText = false;
}

void MainWindow::on_btnBackImg_clicked() {
  mui->frameImgView->hide();
  if (isReaderVisible) mui->frameReader->show();
  if (isMemoVisible) mui->frameNotesGraph->show();
}

void MainWindow::on_btnZoomIn_clicked() {
  QQuickItem* root = mui->qw_Img->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "zoomin");
}

void MainWindow::on_btnZoomOut_clicked() {
  QQuickItem* root = mui->qw_Img->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "zoomout");
}

void MainWindow::on_btnReport() {
  on_actionReport_triggered();
  mui->btnYear->setFixedHeight(mui->btnMonth->height());
}

void MainWindow::on_btnShowPassword_pressed() {
  m_Preferences->on_btnShowPassword_pressed();
}

void MainWindow::on_btnShowPassword_released() {
  m_Preferences->on_btnShowPassword_released();
}

void MainWindow::on_btnShowValidate_pressed() {
  m_Preferences->on_btnShowValidate_pressed();
}

void MainWindow::on_btnShowValidate_released() {
  m_Preferences->on_btnShowValidate_released();
}

void MainWindow::on_btnSendEmail() {
  if (m_Method->getCountFromQW(mui->qwBakList) == 0) return;

  int cur_index = m_Method->getCurrentIndexFromQW(mui->qwBakList);
  QString filePath = m_Method->getText3(mui->qwBakList, cur_index);

  if (m_Method->sendMailWithAttachment("", filePath)) {
    qDebug() << "The default email client of the system has been invoked.";
  } else {
    qDebug() << "The call to the email client failed!";
  }
}

void MainWindow::on_btnShareBakFile_clicked() {
  if (m_Method->getCountFromQW(mui->qwBakList) == 0) return;

  int cur_index = m_Method->getCurrentIndexFromQW(mui->qwBakList);
  QString filePath = m_Method->getText3(mui->qwBakList, cur_index);
  if (QFile::exists(filePath)) {
    mw_one->m_ReceiveShare->shareImage(tr("Share to"), filePath, "*/*");
  }
}

void MainWindow::on_btnNewNote_clicked() {
  m_NotesList->on_actionAdd_Note_triggered();
}

void MainWindow::on_btnShareBookText_clicked() {
  QString txt = mui->editSetText->text().trimmed();
  if (txt.length() > 0) {
    mw_one->m_ReceiveShare->shareString(tr("Share to"), txt);
  }
}

void MainWindow::on_btnBackReader_clicked() { m_Reader->closeReader(); }

void MainWindow::on_btnOpen_clicked() {
  if (mui->qwViewBookNote->isVisible()) return;

  mui->btnAutoStop->click();

  m_Reader->saveReader("", false);
  m_Reader->savePageVPos();

  if (mui->f_ReaderSet->isVisible()) {
    on_btnBackReaderSet_clicked();
  }
  if (mui->qwBookmark->isVisible()) {
    on_btnShowBookmark_clicked();
  }
  m_ReaderSet->close();
  m_Reader->closeSelText();
  m_Reader->on_btnOpen_clicked();
}

void MainWindow::on_btnPageUp_clicked() { m_Reader->goUpPage(); }

void MainWindow::on_btnPageNext_clicked() { m_Reader->goNextPage(); }

void MainWindow::on_btnPages_clicked() {
  if (mui->f_ReaderSet->isHidden()) {
    mui->lblTotalReading->setText(tr("Total Reading: ") +
                                  m_Reader->getReadTotalTime() + " h");
    mui->btnAutoStop->click();
    mui->qwViewBookNote->hide();
    mui->qwBookCata->hide();
    mui->qwBookmark->hide();
    mui->qwReader->show();
    mui->f_ReaderSet->show();

    m_Reader->closeSelText();

    QStringList list = mui->btnPages->text().split("\n");
    if (list.count() == 2) {
      QString cur = list.at(0);
      QString total = list.at(1);
      mui->lblProg->setText(tr("Reading Progress") + " : " + cur + " -> " +
                            total);

      mui->hSlider->setMaximum(total.toInt());
      mui->hSlider->setMinimum(1);
      mui->hSlider->setValue(cur.toInt());
    }
  } else
    on_btnBackReaderSet_clicked();
}

void MainWindow::on_btnOpenNote_clicked() { m_Notes->previewNote(); }

void MainWindow::on_btnEditNote_clicked() { m_Notes->openEditUI(); }

void MainWindow::on_btnToPDF_clicked() {
  if (!QFile::exists(currentMDFile)) return;

  m_Notes->on_btnPDF_clicked();
}

void MainWindow::on_btnPause_clicked() {
  if (!isRunPaused) {
    isRunPaused = true;
    mui->btnPause->setIcon(QIcon(":/res/erun.svg"));
    mui->lblGpsInfo->setStyleSheet(m_Steps->lblPausedStyle);
  } else {
    isRunPaused = false;
    mui->btnPause->setIcon(QIcon(":/res/epaused.svg"));
    mui->lblGpsInfo->setStyleSheet(m_Steps->lblStartStyle);
  }
}

void MainWindow::on_btnTestWebDav_clicked() {
  auto msg = std::make_unique<ShowMessage>(mw_one);
  if (!m_CloudBackup->checkWebDAVConnection()) {
    msg->showMsg(appName,
                 tr("WebDAV connection failed. Please check the network, "
                    "website address or login information."),
                 1);
  } else {
    msg->showMsg(appName, tr("WebDav connection successful."), 1);
  }
}

void MainWindow::on_btnReader_clicked() { m_Reader->openReader(); }

void MainWindow::on_btnFind_clicked() {
  mui->frameMain->hide();
  mui->frameSearch->show();
  mui->editSearchText->setFocus();
  mui->btnClearSearchText->setFixedHeight(mui->btnStartSearch->height());
}

void MainWindow::on_btnTodo_clicked() { m_Todo->openTodo(); }

void MainWindow::on_btnHome_clicked() {
  mui->qwMainTab->show();
  mui->qwMainDate->hide();
  mui->qwMainEvent->hide();
  mui->lblStats->hide();
  mui->lblTabTitle->hide();
}

void MainWindow::on_btnReadList_clicked() {
  if (mui->qwViewBookNote->isVisible()) return;

  mui->btnAutoStop->click();

  m_Reader->saveReader("", false);
  m_Reader->savePageVPos();

  if (isAndroid) m_Reader->closeMyPDF();

  if (mui->f_ReaderSet->isVisible()) {
    on_btnBackReaderSet_clicked();
  }

  if (mui->qwBookmark->isVisible()) {
    mw_one->on_btnShowBookmark_clicked();
  }

  m_ReaderSet->close();
  m_Reader->closeSelText();

  if (mui->frameMain->isVisible()) mui->frameMain->hide();
  mui->frameReader->hide();
  m_Reader->showBookListWin();

  m_Reader->getReadList();
}

void MainWindow::on_btnMenu_clicked() {
  mainMenu = new QMenu(this);
  init_Menu(mainMenu);

  int x = 0;
  int y = 0;

#ifdef Q_OS_ANDROID
  int statusBarHeight = 36;

  x = mw_one->geometry().x() + 2;
  y = geometry().y() + mui->f_Menu->height() + 2 + statusBarHeight;
#else
  x = mw_one->geometry().x() + mui->btnMenu->x() + 2;
  y = geometry().y() + mui->f_Menu->height() + 2;
#endif

  QPoint pos(x, y);
  mainMenu->exec(pos);

  mainMenu->deleteLater();
  mainMenu = nullptr;
}

void MainWindow::on_btnModifyRecord() { m_Method->reeditMainEventData(); }

void MainWindow::on_btnSelTab_clicked() {
  mui->frameMain->hide();
  mui->frameSetTab->show();
  getMainTabs();
}

void MainWindow::on_btnSearch_clicked() {
  QString str = mui->editSetText->text().trimmed();
  if (str == "") return;

  QString strurl;
  strurl = "https://bing.com/search?q=" + str;

  QUrl url(strurl);
  QDesktopServices::openUrl(url);
}

void MainWindow::on_btnShowCboxList_clicked() { mui->cboxWebDAV->showPopup(); }

void MainWindow::onAndroidBackHandle() {
  // ====== 焦点重置，覆盖所有 return 路径 ======
  // ====== 自动获取 mw_one 下所有 QQuickWidget ======
  struct FocusGuard {
    QList<QQuickWidget*> widgets;
    ~FocusGuard() {
      for (auto* w : std::as_const(widgets)) {
        if (w && w->isVisible()) {
          w->clearFocus();
          w->setFocus(Qt::OtherFocusReason);
        }
      }
    }
  } focusGuard{mw_one->findChildren<QQuickWidget*>()};

  ///////////////////////////////////////////////

  if (textToolbarDynamic != nullptr && textToolbarDynamic->isVisible()) {
    closeTextToolBar();
    return;
  }

  if (textToolbar != nullptr && textToolbar->isVisible()) {
    closeTextToolBar();
    return;
  }

  if (colorDlg != nullptr) {
    if (colorDlg->isVisible()) {
      colorDlg->close();
      return;
    }
  }

  if (m_MsgBox != nullptr) {
    if (m_MsgBox->isVisible()) {
      m_MsgBox->close();
      return;
    }
  }

  if (m_Method->m_EnColorPicker != nullptr) {
    if (m_Method->m_EnColorPicker->isVisible()) {
      m_Method->m_EnColorPicker->close();
      return;
    }
  }

  if (m_PrintPDF != nullptr) {
    if (m_PrintPDF->isVisible()) {
      m_PrintPDF->close();
      return;
    }
  }

  if (mw_one->mainMenu != nullptr) {
    if (mw_one->mainMenu->isVisible()) {
      mw_one->mainMenu->close();
      return;
    }
  }

  if (mw_one->m_Report->m_Menu != nullptr) {
    if (mw_one->m_Report->m_Menu->isVisible()) {
      mw_one->m_Report->m_Menu->close();
      return;
    }
  }

  if (m_NotesList->menuNoteBook != nullptr) {
    if (m_NotesList->menuNoteBook->isVisible()) {
      m_NotesList->menuNoteBook->close();
      return;
    }
  }

  if (m_NotesList->menuNoteList != nullptr) {
    if (m_NotesList->menuNoteList->isVisible()) {
      m_NotesList->menuNoteList->close();
      return;
    }
  }

  if (m_NotesList->menuRecentOpen != nullptr) {
    if (m_NotesList->menuRecentOpen->isVisible()) {
      m_NotesList->menuRecentOpen->close();
      return;
    }
  }

  if (m_NotesList->m_MoveTo != nullptr) {
    if (m_NotesList->m_MoveTo->isVisible()) {
      m_NotesList->m_MoveTo->ui->btnCancel->click();
      return;
    }
  }

  if (mw_one->m_RenameDlg != nullptr) {
    if (mw_one->m_RenameDlg->isVisible()) {
      mw_one->m_RenameDlg->close();
      return;
    }
  }

  if (mw_one->m_Todo->m_ReeditTodo != nullptr) {
    if (mw_one->m_Todo->m_ReeditTodo->isVisible()) {
      mw_one->m_Todo->m_ReeditTodo->close();
      return;
    }
  }

  if (m_NotesList->m_RenameNotes != nullptr) {
    if (m_NotesList->m_RenameNotes->isVisible()) {
      if (textToolbarDynamic != nullptr && textToolbarDynamic->isVisible()) {
        closeTextToolBar();
        return;
      }

      m_NotesList->m_RenameNotes->close();
      m_Method->closeGrayWindows();
      return;
    }
  }

  if (mui->frameAIAPIList->isVisible()) {
    on_btnBackAIAPIList_clicked();
    return;
  }

  if (mw_one->m_Preferences->isVisible()) {
    mw_one->m_Preferences->ui->btnBack->click();
    return;
  }

  if (mw_one->m_AboutThis->isVisible()) {
    mw_one->m_AboutThis->ui->btnBack_About->click();
    return;
  }

  if (m_StepsOptions->isVisible()) {
    m_StepsOptions->ui->btnBack->click();
    return;
  }

  // Reader
  if (m_Reader->dlgAddBookNote != nullptr) {
    if (m_Reader->dlgAddBookNote->isVisible()) {
      m_Reader->dlgAddBookNote->close();
      return;
    }
  }

  if (m_Reader->dlgEditBookNote != nullptr) {
    if (m_Reader->dlgEditBookNote->isVisible()) {
      m_Reader->dlgEditBookNote->close();
      return;
    }
  }

  if (mui->qwViewBookNote->isVisible()) {
    QTimer::singleShot(100, mw_one, []() { m_Reader->closeViewBookNote(); });

    return;
  }

  if (mui->f_ReaderNote->isVisible()) {
    mw_one->on_btnCancelSel_clicked();
    return;
  }

  if (mui->f_ReaderSet->isVisible()) {
    mw_one->on_btnBackReaderSet_clicked();
    return;
  }

  if (mui->qwBookCata->isVisible()) {
    mw_one->on_btnBookCata_clicked();
    return;
  }

  if (mui->qwBookmark->isVisible()) {
    mw_one->on_btnShowBookmark_clicked();
    return;
  }

  if (!mui->frameReader->isHidden()) {
    on_btnBackReader_clicked();
    return;
  }

  if (!mui->frameImgView->isHidden()) {
    on_btnBackImg_clicked();
    return;
  }

  if (!mui->qwMainChart->isHidden()) {
    QTimer::singleShot(100, mw_one, []() { mw_one->on_btnChart(); });
    return;
  }

  if (!mui->frameOne->isHidden()) {
    on_btnBack_One_clicked();
    return;
  }

  if (!mui->frameNoteRecycle->isHidden()) {
    on_btnBackNoteRecycle_clicked();
    return;
  }

  if (!mui->frameNotesSearchResult->isHidden()) {
    on_btnBack_NotesSearchResult_clicked();
    return;
  }

  if (mui->f_FindNotes->isVisible()) {
    mui->f_FindNotes->hide();
    return;
  }

  if (mui->frameFavorites->isVisible()) {
    on_btnBackFavorites_clicked();
    return;
  }

  if (!mui->frameNoteList->isHidden()) {
    QTimer::singleShot(200, mw_one,
                       []() { mw_one->on_btnBackNoteList_clicked(); });
    return;
  }

  if (!mui->frameDiff->isHidden()) {
    on_btnBackNoteDiff_clicked();
    return;
  }

  if (!mui->frameNotesGraph->isHidden()) {
    on_btnBackNotesGraph_clicked();
    return;
  }

  if (mw_one->m_TodoAlarm->isVisible()) {
    mw_one->m_TodoAlarm->ui->btnBack->click();
    return;
  }

  if (mw_one->m_Todo->isTodoAlarmShow) {
    QTimer::singleShot(100, mw_one, []() { mw_one->m_Todo->closeTodoAlarm(); });
    return;
  }

  if (!mui->frameTodoRecycle->isHidden()) {
    on_btnReturnRecycle_clicked();
    return;
  }

  if (!mui->frameTodo->isHidden()) {
    on_btnBackTodo_clicked();
    return;
  }

  if (!mui->frameTabRecycle->isHidden()) {
    on_btnBackTabRecycle_clicked();
    return;
  }

  if (m_Steps->isRouteShow()) {
    m_Steps->closeRouteDialog();
    return;
  }

  if (m_Steps->m_remarksDialog != nullptr) {
    if (m_Steps->m_remarksDialog->isVisible()) {
      m_Steps->m_remarksDialog->close();
      return;
    }
  }

  if (m_Steps->statsDialog != nullptr) {
    m_Steps->statsDialog->close();

    return;
  }

  if (!mui->frameSteps->isHidden()) {
    on_btnBackSteps_clicked();
    return;
  }

  if (!mui->frameViewCate->isHidden()) {
    QTimer::singleShot(100, mw_one, []() {
      mui->frameViewCate->hide();
      mui->frameReport->show();
    });

    return;
  }

  if (!mui->frameReport->isHidden()) {
    on_btnBack_Report_clicked();
    return;
  }

  if (!mui->frameSearch->isHidden()) {
    on_btnBackSearch_clicked();
    return;
  }

  if (!mui->frameBakList->isHidden()) {
    on_btnBackBakList_clicked();
    return;
  }

  if (!mui->frameCategory->isHidden()) {
    on_btnCancelType_clicked();
    return;
  }

  if (!mui->frameSetTab->isHidden()) {
    on_btnBackSetTab_clicked();
    return;
  }

  if (!mui->frameEditRecord->isHidden()) {
    on_btnBackEditRecord_clicked();

    return;
  }

  if (m_Reader->isBookListWinVisible()) {
    on_btnBackBookList_clicked();
    return;
  }

  if (!mui->frameNotesTree->isHidden()) {
    mui->btnBack_Tree->click();

    return;
  }

  if (!mui->qwMainDate->isHidden()) {
    mui->btnHome->clicked();

    return;
  }

  if (!mui->frameMain->isHidden()) {
    mw_one->setMini();

    return;
  }
}

void MainWindow::setToolButtonAnimation(QToolButton* btn, bool setMyStyle) {
  if (setMyStyle) {
    // 根据明暗主题区分tooltip样式
    QString tipStyle;
    if (!isAndroid) {
      if (isDark) {
        tipStyle = R"(
        QToolTip {
            background-color: #2b2b2b;
            color: #f0f0f0;
            border: 1px solid #555555;
            padding: 4px;
        }
        )";
      } else {
        tipStyle = R"(
        QToolTip {
            background-color: white;
            color: black;
            border: 1px solid #aaa;
            padding: 4px;
        }
        )";
      }
    }

    // 拼接按钮基础样式 + 动态tooltip样式，存入变量sheet
    QString sheet = QString(R"(
        QToolButton {
            border: none;
            background: transparent;
        }
        %1
    )")
                        .arg(tipStyle);

    // 把拼接好的完整样式赋值给按钮
    btn->setStyleSheet(sheet);
  }
}

void MainWindow::on_btnBackFavorites_clicked() {
  mui->frameNoteList->show();
  mui->frameFavorites->hide();
}

void MainWindow::on_btnOpenFavoritesNote_clicked() {
  if (!QFile::exists(currentMDFile)) return;

  // mui->btnBackFavorites->click();
  m_Notes->openEditUI();
}

void MainWindow::on_btnOpenFavoritesView_clicked() {
  if (!QFile::exists(currentMDFile)) return;

  // mui->btnBackFavorites->click();
  m_Notes->previewNote();
}

void MainWindow::on_btnClearTitleKey_clicked() {
  mui->editTitleKey->clear();
  mui->editTitleKey->setFocus();
}

void MainWindow::on_btnBackAIAPIList_clicked() {
  mui->frameAIAPIList->hide();
  mui->frameMain->show();
  mw_one->m_Preferences->openPreferences();
}

void MainWindow::on_btnAIAPIListOk_clicked(int index) {
  if (index < 0) return;

  mui->frameAIAPIList->hide();
  mui->frameMain->show();
  mw_one->m_Preferences->openPreferences();
  mw_one->m_Preferences->ui->cboxEndpoint->setCurrentIndex(index);
}
