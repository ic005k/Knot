#include "MainWindow.h"
#include "SearchWorker.h"
#include "defines.h"

SearchWorker *m_searchWorker;

void MainWindow::btnRecentOpen_clicked() {
  // m_NotesList->genRecentOpenMenu();

  m_Notes->refreshRecentOpenByCounter();

  if (isAndroid)
    m_Method->openActivity("openRecentNotesActivity",
                           m_NotesList->listRecentOpen);
  else
    m_NotesList->m_RecentOpen->showRecentOpen(m_NotesList->listRecentOpen);
}

void MainWindow::backNoteList() {
  // 延迟一小段时间再触发，避免模块快速切换时反复启停
  QTimer::singleShot(500, m_NotesList, &NotesList::rebuilderNotesVector);

  clearWidgetFocus();

  m_Notes->updateMainnotesIniToSyncLists();

  saveNeedSyncNotes();

  m_Notes->saveNotesCounter();

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

void MainWindow::btnStartSearch_clicked(const QString &searchStr) {
  // 主事件搜索

  if (searchStr.length() == 0)
    return;

  if (!isAndroid)
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

void MainWindow::btnStartDate_clicked() {
  m_DateSelector->initStartEndDate("start");
}

void MainWindow::btnEndDate_clicked() {
  m_DateSelector->initStartEndDate("end");
}

void MainWindow::on_btnAIExerciseSuggestions_clicked() {
  if (!isEnabledAiApi)
    return;

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

void MainWindow::btnAIBookExplanation_clicked(const QString &selectedText) {
  QString trimText = selectedText.trimmed();
  if (trimText.isEmpty()) {
    return;
  }

  if (!isEnabledAiApi)
    return;

  // qDebug() << trimText;

  // 获取当前程序生效的语言标识
  QLocale loc = QLocale::system();
  // 如果切换过软件语言，改用之前存储的语言代码变量：
  // QString langCode = m_appLanguageCode;
  QString langCode = loc.name(); // 格式 zh_CN / en_US / ja_JP

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

void MainWindow::btnAISteps_clicked() {
  QString text = m_Steps->ai_stepstext;
  QString trimText = text.trimmed();
  if (trimText.isEmpty()) {
    return;
  }

  if (!isEnabledAiApi)
    return;

  m_Steps->isAiSteps = true;

  // qDebug() << trimText;

  // 获取当前程序生效的语言标识
  QLocale loc = QLocale::system();
  // 如果切换过软件语言，改用之前存储的语言代码变量：
  // QString langCode = m_appLanguageCode;
  QString langCode = loc.name(); // 格式 zh_CN / en_US / ja_JP

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

void MainWindow::btnAIReportAnalysis_clicked() {
  QString text = m_Report->catetext;
  QString trimText = text.trimmed();
  if (trimText.isEmpty()) {
    return;
  }

  if (!isEnabledAiApi)
    return;

  mw_one->m_Report->isAiCategory = true;

  // 获取当前程序生效的语言标识
  QLocale loc = QLocale::system();
  // 如果切换过软件语言，改用之前存储的语言代码变量：
  // QString langCode = m_appLanguageCode;
  QString langCode = loc.name(); // 格式 zh_CN / en_US / ja_JP

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

void MainWindow::on_btnWebDAVBackup_clicked() { m_CloudBackup->startBakData(); }

void MainWindow::on_btnWebDAVRestore_clicked() {
  m_CloudBackup->webDAVRestoreData();
}

void MainWindow::on_btnSteps_clicked() { m_Steps->openStepsUI(); }

void MainWindow::on_btnNotes_clicked() { m_Notes->openNotes(); }

void MainWindow::btnDel_clicked() {
  isMoveEntry = false;
  del_Data((QTreeWidget *)mw_one->ui->tabWidget->currentWidget());
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  const QSize sz = event->size();
  if (sz.width() <= 0 || sz.height() <= 0) {
    event->ignore();
    return;
  }

  QMainWindow::resizeEvent(event);

#ifdef Q_OS_ANDROID

#else

#endif
}

void MainWindow::on_btnDownload_clicked() {
  m_CloudBackup->on_pushButton_downloadFile_clicked();
}

void MainWindow::on_btnBack_One_clicked() {
  m_CloudBackup->backExit();
  if (isAndroid)
    m_Method->openMainEntranceWindow();
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

void MainWindow::on_btnTestWebDav_clicked() {
  ui->btnTestWebDav->setEnabled(false);
  m_CloudBackup->checkWebDAVConnectionAsync();
}

void MainWindow::btnHome_clicked() {
  if (isAndroid) {
    m_Method->openMainEntranceWindow();
  }

  mw_one->ui->lblStats->hide();
}

void MainWindow::on_btnMenu_clicked() {
  mainMenu = new QMenu(this);
  init_Menu(mainMenu);

  int x = 0;
  int y = 0;

#ifdef Q_OS_ANDROID
  int statusBarHeight = 36;

  x = mw_one->geometry().x() + 2;
  y = geometry().y() + mw_one->ui->f_Menu->height() + 2 + statusBarHeight;
#else
  x = mw_one->geometry().x() + mw_one->ui->btnMenu->x() + 2;
  y = geometry().y() + mw_one->ui->f_Menu->height() + 2;
#endif

  QPoint pos(x, y);
  mainMenu->exec(pos);

  mainMenu->deleteLater();
  mainMenu = nullptr;
}

void MainWindow::on_btnShowCboxList_clicked() {
  mw_one->ui->cboxWebDAV->showPopup();
}

void MainWindow::on_btnReader_clicked() { m_Reader->openReader(); }

void MainWindow::on_btnTodo_clicked() { m_Todo->openTodo(); }

void MainWindow::onAndroidBackHandle() {
  if (mw_one->dlgProg != nullptr || m_Method->infoWindow != nullptr) {
    return;
  }

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

  if (m_Method->m_EnColorPicker != nullptr) {
    if (m_Method->m_EnColorPicker->isVisible()) {
      m_Method->m_EnColorPicker->close();
      return;
    }
  }

  if (m_PrintPDF != nullptr) {
    if (m_PrintPDF->isVisible()) {
      m_PrintPDF->close();

      m_Notes->openNoteWindow();

      return;
    }
  }

  if (mw_one->m_AboutThis->m_AutoUpdate != nullptr) {
    if (mw_one->m_AboutThis->m_AutoUpdate->isVisible()) {
      return;
    }
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

  if (!mw_one->ui->frameOne->isHidden()) {
    on_btnBack_One_clicked();
    return;
  }

  if (m_Steps->isVisible()) {
    m_Steps->close();
    m_Method->openMainEntranceWindow();
    return;
  }

  if (mw_one->m_BakFileList->isVisible()) {
    mw_one->m_BakFileList->close();
    m_Method->openMainEntranceWindow();
    return;
  }

  if (mw_one->ui->frameMain->isVisible()) {
    mw_one->ui->frameMain->hide();
    m_Method->openMainEntranceWindow();

    return;
  }

  if (ui->frameMain->isHidden() && !mw_one->dlgProg) {
    m_Method->openMainEntranceWindow();
    return;
  }
}

void MainWindow::setToolButtonAnimation(QToolButton *btn, bool setMyStyle) {
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
