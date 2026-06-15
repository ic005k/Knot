#include "MainWindow.h"

void MainWindow::init_TotalData() {
  int count = mui->tabWidget->tabBar()->count();
  for (int i = 0; i < count; i++) {
    mui->tabWidget->removeTab(0);
  }

  QString ini_file;
  ini_file = iniDir + "tab.ini";
  QSettings RegTab(ini_file, QSettings::IniFormat);
  int TabCount = RegTab.value("TabCount", 0).toInt();

  clearAll();

  for (int i = 0; i < TabCount; i++) {
    QString name;
    name = RegTab.value("twName" + QString::number(i)).toString();
    if (name.trimmed().length() == 0) name = "tab" + QString::number(i + 1);
    QTreeWidget* tw = init_TreeWidget(name);

    QString tabText = RegTab
                          .value("TabName" + QString::number(i),
                                 tr("Tab") + QString::number(i + 1))
                          .toString();
    mui->tabWidget->addTab(tw, tabText);

    addItem(tabText, "", "", "", 0);
  }

  if (TabCount == 0) {
    QString tw_name = m_Notes->getDateTimeStr() + "_" + QString::number(1);
    QTreeWidget* tw = init_TreeWidget(tw_name);

    QString tabText = tr("Tab") + " " + QString::number(1);
    mui->tabWidget->addTab(tw, tabText);
    addItem(tabText, "", "", "", 0);

    saveTab();
  }

  m_EditRecord->init_MyCategory();

  if (TabCount > 0)
    currentTabIndex = RegTab.value("CurrentIndex").toInt();
  else
    currentTabIndex = 0;

  mui->tabWidget->setCurrentIndex(currentTabIndex);
  setCurrentIndex(currentTabIndex);
  QTreeWidget* twCur = (QTreeWidget*)tabData->currentWidget();
  readData(twCur);
  mui->actionImport_Data->setEnabled(false);
  mui->actionExport_Data->setEnabled(false);
  mui->actionDel_Tab->setEnabled(false);
  mui->actionAdd_Tab->setEnabled(false);
  mui->actionView_App_Data->setEnabled(false);

  if (!initMain) {
    mui->progBar->setHidden(false);
    mui->progBar->setMaximum(0);
  }

  m_ReadTWThread->start();
}

void MainWindow::init_Instance() {
  mw_one = this;
  m_MainHelper = new MainHelper(this);
  CurrentYear = QString::number(QDate::currentDate().year());

  tabData = mui->tabWidget;

  m_Method = new Method(this);

  m_AboutThis = new AboutThis(this);
  m_Preferences = new Preferences(this);
  m_EditRecord = new EditRecord();
  m_Todo = new Todo(this);
  m_Report = new Report(this);
  m_Notes = new Notes(this);
  m_StepsOptions = new StepsOptions(this);
  m_Steps = new Steps(this);
  m_Reader = new Reader(this);
  m_TodoAlarm = new TodoAlarm(this);
  m_DateSelector = new DateSelector(this);
  m_CloudBackup = new CloudBackup;
  m_ReaderSet = new ReaderSet(this);

  m_NotesList = new NotesList(this);

  m_ReceiveShare = new ReceiveShare(this);

  if (m_Preferences->getDefaultFont() == "None")
    m_Preferences->setDefaultFont(this->font().family());

  m_Method->setOSFlag();

  connect(this, &MainWindow::androidBackSignal, this,
          &MainWindow::onAndroidBackHandle, Qt::QueuedConnection);
}

void MainWindow::init_Thread_Timer() {
  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));

  timerSyncData = new QTimer(this);
  connect(timerSyncData, SIGNAL(timeout()), this, SLOT(on_timerSyncData()));
  timerMousePress = new QTimer(this);
  connect(timerMousePress, SIGNAL(timeout()), this, SLOT(on_timerMousePress()));
  timerMousePress->setSingleShot(true);
  tmeFlash = new QTimer(this);
  connect(tmeFlash, SIGNAL(timeout()), this, SLOT(on_tmeFlash()));
  tmeStartRecordAudio = new QTimer(this);
  connect(tmeStartRecordAudio, SIGNAL(timeout()), this,
          SLOT(on_StartRecordAudio()));

  myReadEBookThread = new ReadEBookThread();
  connect(myReadEBookThread, &ReadEBookThread::isDone, this,
          &MainWindow::readEBookDone);

  m_ReadTWThread = new ReadTWThread();
  connect(m_ReadTWThread, &ReadTWThread::isDone, this, &MainWindow::readTWDone);

  myReadChartThread = new ReadChartThread();
  connect(myReadChartThread, &ReadChartThread::isDone, this,
          &MainWindow::readChartDone);

  mySaveThread = new SaveThread();
  connect(mySaveThread, &SaveThread::isDone, this, &MainWindow::saveDone);

  myBakDataThread = new BakDataThread();
  connect(myBakDataThread, &BakDataThread::isDone, this,
          &MainWindow::bakDataDone);

  myImportDataThread = new ImportDataThread();
  connect(myImportDataThread, &ImportDataThread::isDone, this,
          &MainWindow::importDataDone);

  mySearchThread = new SearchThread();
  // connect(mySearchThread, &SearchThread::isDone, this,
  // &MainWindow::searchDone);
  connect(mySearchThread, &QThread::finished, this, &MainWindow::searchDone,
          Qt::QueuedConnection);  // 强制切主线程

  m_workerThread = new QThread(this);
  m_searchWorker = new SearchWorker();
  m_searchWorker->moveToThread(m_workerThread);
  connect(m_workerThread, &QThread::finished, m_searchWorker,
          &QObject::deleteLater);
  connect(m_workerThread, &QThread::finished, m_workerThread,
          &QObject::deleteLater);
  // 搜索结束 → 主线程更新UI
  connect(m_searchWorker, &SearchWorker::searchFinished, this,
          [this](const QList<QString>& results) {
            // 主线程安全更新
            resultsList = results;
            m_Method->initSearchResults();
            mw_one->closeProgress();
          });
  m_workerThread->start();

  myUpdateGpsMapThread = new UpdateGpsMapThread();
  connect(myUpdateGpsMapThread, &UpdateGpsMapThread::isDone, this,
          &MainWindow::updateGpsMapDone);

  myGetGpsDataThread = new GetGpsDataThread();
  connect(myGetGpsDataThread, &GetGpsDataThread::isDone, this,
          &MainWindow::GetGpsDataThreadDone);
}

void MainWindow::init_Stats(QTreeWidget* tw) {
  int count = tw->topLevelItemCount();
  int tatol = 0;
  double amount = 0;
  for (int i = 0; i < count; i++) {
    if (isBreak) break;
    QString str1 = tw->topLevelItem(i)->text(1);
    QString str2 = tw->topLevelItem(i)->text(2);
    tatol = tatol + str1.toInt();
    amount = amount + str2.toDouble();
  }

  QString strAmount = QString("%1").arg(amount, 0, 'f', 2);
  strStats = tr("Total") + " : " + QString::number(tatol) + "    $" + strAmount;
}

void MainWindow::init_ButtonStyle() {
  m_Method->set_ToolButtonStyle(mw_one);

  // 主按钮
  setToolButtonAnimation(mui->btnMenu, true);
  setToolButtonAnimation(mui->btnHome, true);
  setToolButtonAnimation(mui->btnReader, true);
  setToolButtonAnimation(mui->btnTodo, true);
  setToolButtonAnimation(mui->btnSteps, true);
  setToolButtonAnimation(mui->btnNotes, true);
  setToolButtonAnimation(mui->btnAdd, true);
  setToolButtonAnimation(mui->btnDel, true);
  setToolButtonAnimation(mui->btnPasteTodo, true);
  setToolButtonAnimation(mui->btnSync, true);
  setToolButtonAnimation(mui->btnFind, true);
  setToolButtonAnimation(mui->btnSelTab, true);

  // Todo
  setToolButtonAnimation(mui->btnBackTodo, true);
  setToolButtonAnimation(mui->btnHigh, true);
  setToolButtonAnimation(mui->btnLow, true);
  setToolButtonAnimation(mui->btnModify, true);
  setToolButtonAnimation(mui->btnSetTime, true);
  setToolButtonAnimation(mui->btnRecycle, true);

  // Reader
  setToolButtonAnimation(mui->btnBackReader, true);
  setToolButtonAnimation(mui->btnCatalogue, true);
  setToolButtonAnimation(mui->btnShowBookmark, true);
  setToolButtonAnimation(mui->btnAutoRun, true);
  setToolButtonAnimation(mui->btnAutoStop, true);
  setToolButtonAnimation(mui->btnPages, true);
  setToolButtonAnimation(mui->btnOpen, true);
  setToolButtonAnimation(mui->btnReadList, true);
  setToolButtonAnimation(mui->btnRotation, true);
  setToolButtonAnimation(mui->btnSpeak, true);
  setToolButtonAnimation(mui->btnStopSpeak, true);

  // Notes
  setToolButtonAnimation(mui->btnNoteBookMenu, true);
  setToolButtonAnimation(mui->btnNewNote, true);
  setToolButtonAnimation(mui->btnFindNotes2, true);
  setToolButtonAnimation(mui->btnNoteMenu, true);
  setToolButtonAnimation(mui->btnRecentOpen, true);

  if (isDark) {
    // Reader
    mui->btnBackReader->setIcon(QIcon(":/res/reader/exit_l.svg"));
    mui->btnRotation->setIcon(QIcon(":/res/reader/rotation_l.svg"));
    mui->btnCatalogue->setIcon(QIcon(":/res/reader/cata_l.svg"));
    mui->btnShowBookmark->setIcon(QIcon(":/res/reader/bookmark_l.svg"));
    mui->btnAutoRun->setIcon(QIcon(":/res/reader/run_l.svg"));
    mui->btnAutoStop->setIcon(QIcon(":/res/reader/stop_l.svg"));
    mui->btnOpen->setIcon(QIcon(":/res/reader/open_l.svg"));
    mui->btnReadList->setIcon(QIcon(":/res/reader/booklist_l.svg"));
    mui->btnPages->setIcon(QIcon(":/res/set_l.svg"));
    mui->btnSpeak->setIcon(QIcon(":/res/reader/speak_l.svg"));
    mui->btnStopSpeak->setIcon(QIcon(":/res/reader/stopspeak_l.svg"));

    // Todo
    mui->btnBackTodo->setIcon(QIcon(":/res/back_l.svg"));
    mui->btnHigh->setIcon(QIcon(":/res/high_l.svg"));
    mui->btnLow->setIcon(QIcon(":/res/low_l.svg"));
    mui->btnModify->setIcon(QIcon(":/res/edit_l.svg"));
    mui->btnSetTime->setIcon(QIcon(":/res/alarm_l.svg"));
    mui->btnRecycle->setIcon(QIcon(":/res/recycle_l.svg"));

    // Notes
    mui->btnNoteBookMenu->setIcon(QIcon(":/res/nb_l.svg"));
    mui->btnNewNote->setIcon(QIcon(":/res/newnote_l.svg"));
    mui->btnFindNotes2->setIcon(QIcon(":/res/find2_l.svg"));
    mui->btnNoteMenu->setIcon(QIcon(":/res/notes_l.svg"));
    mui->btnRecentOpen->setIcon(QIcon(":/res/recent_l.svg"));

  } else {
    // Reader
    mui->btnBackReader->setIcon(QIcon(":/res/reader/exit.svg"));
    mui->btnRotation->setIcon(QIcon(":/res/reader/rotation.svg"));
    mui->btnCatalogue->setIcon(QIcon(":/res/reader/cata.svg"));
    mui->btnShowBookmark->setIcon(QIcon(":/res/reader/bookmark.svg"));
    mui->btnAutoRun->setIcon(QIcon(":/res/reader/run.svg"));
    mui->btnAutoStop->setIcon(QIcon(":/res/reader/stop.svg"));
    mui->btnOpen->setIcon(QIcon(":/res/reader/open.svg"));
    mui->btnReadList->setIcon(QIcon(":/res/reader/booklist.svg"));
    mui->btnPages->setIcon(QIcon(":/res/set.svg"));
    mui->btnSpeak->setIcon(QIcon(":/res/reader/speak.svg"));
    mui->btnStopSpeak->setIcon(QIcon(":/res/reader/stopspeak.svg"));

    // Todo
    mui->btnBackTodo->setIcon(QIcon(":/res/back.svg"));
    mui->btnHigh->setIcon(QIcon(":/res/high.svg"));
    mui->btnLow->setIcon(QIcon(":/res/low.svg"));
    mui->btnModify->setIcon(QIcon(":/res/edit.svg"));
    mui->btnSetTime->setIcon(QIcon(":/res/alarm.svg"));
    mui->btnRecycle->setIcon(QIcon(":/res/recycle.svg"));

    // Notes
    mui->btnNoteBookMenu->setIcon(QIcon(":/res/nb.svg"));
    mui->btnNewNote->setIcon(QIcon(":/res/newnote.svg"));
    mui->btnFindNotes2->setIcon(QIcon(":/res/find2.svg"));
    mui->btnNoteMenu->setIcon(QIcon(":/res/notes.svg"));
    mui->btnRecentOpen->setIcon(QIcon(":/res/recent.svg"));
  }

  /*mui->btnPages->setStyleSheet(
    "color: rgb(255, 255, 255);background-color: #FF9933;border: "
    "0px solid "
    "rgb(255,0,0);border-radius: 4px;"
    "font-weight: bold;");*/

  QString style =
      "QToolButton {background-color: rgb(255, 0, 0); color: "
      "rgb(255,255,255); "
      "border-radius:10px; "
      "border:0px solid gray; } QToolButton:pressed { background-color: "
      "rgb(220,220,230); color: black}";
  mw_one->m_Preferences->ui->btnReStart->setStyleSheet(style);
}

void MainWindow::initMainQW() {
  qmlRegisterType<DocumentHandler>("MyModel2", 1, 0, "DocumentHandler");

  if (mui->qwBakList->source().isEmpty()) {
    mui->qwBakList->rootContext()->setContextProperty("isDark", isDark);
    mui->qwBakList->rootContext()->setContextProperty("m_Method", m_Method);
    mui->qwBakList->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/baklist.qml")));
  }

  if (mui->qwSearch->source().isEmpty()) {
    mui->qwSearch->rootContext()->setContextProperty("isDark", isDark);
    mui->qwSearch->rootContext()->setContextProperty("m_Method", m_Method);
    mui->qwSearch->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/search.qml")));
  }

  if (mui->qwTodo->source().isEmpty()) {
    int f_size = 19;
    if (fontSize <= f_size) f_size = fontSize;

    mui->qwTodo->rootContext()->setContextProperty("isDark", isDark);
    mui->qwTodo->rootContext()->setContextProperty("maxFontSize", f_size);
    mui->qwTodo->rootContext()->setContextProperty("isBtnVisible",
                                                   QVariant(false));
    mui->qwTodo->rootContext()->setContextProperty("m_Todo", mw_one->m_Todo);
    mui->qwTodo->rootContext()->setContextProperty("FontSize", fontSize);
    mui->qwTodo->rootContext()->setContextProperty("mainW",
                                                   mw_one->geometry().width());
    mui->qwTodo->rootContext()->setContextProperty("mainH",
                                                   mw_one->geometry().height());
    mui->qwTodo->setSource(QUrl(QStringLiteral("qrc:/src/qmlsrc/todo.qml")));

    mui->qwRecycle->rootContext()->setContextProperty("isDark", isDark);
    mui->qwRecycle->rootContext()->setContextProperty("FontSize", fontSize);
    mui->qwRecycle->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/todorecycle.qml")));
  }

  if (mui->qwSteps->source().isEmpty()) {
    int f_size = 19;
    if (fontSize <= f_size) f_size = fontSize;

    mui->qwSteps->rootContext()->setContextProperty("isDark", isDark);
    mui->qwSteps->rootContext()->setContextProperty("maxFontSize", f_size);
    mui->qwSteps->rootContext()->setContextProperty("myW", mw_one->width());
    mui->qwSteps->rootContext()->setContextProperty("text0", "");
    mui->qwSteps->rootContext()->setContextProperty("text1", "");
    mui->qwSteps->rootContext()->setContextProperty("text2", "");
    mui->qwSteps->rootContext()->setContextProperty("text3", "");
    mui->qwSteps->setSource(QUrl(QStringLiteral("qrc:/src/qmlsrc/steps.qml")));

    mui->qwGpsList->rootContext()->setContextProperty("isDark", isDark);
    mui->qwGpsList->rootContext()->setContextProperty("isShowRoute",
                                                      m_Steps->isShowRoute);
    mui->qwGpsList->rootContext()->setContextProperty("myW", mw_one->width());
    mui->qwGpsList->rootContext()->setContextProperty("m_Steps", m_Steps);
    mui->qwGpsList->rootContext()->setContextProperty("FontSize", fontSize);
    mui->qwGpsList->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/gps_list.qml")));

    mui->qwMap->setResizeMode(QQuickWidget::SizeRootObjectToView);
    mui->qwMap->setFocusPolicy(Qt::StrongFocus);  // 关键设置
    mui->qwMap->setClearColor(Qt::transparent);   // 避免渲染冲突
    mui->qwMap->setAttribute(Qt::WA_AcceptTouchEvents, true);
    mui->qwMap->setAttribute(Qt::WA_TouchPadAcceptSingleTouchEvents, true);
    mui->qwMap->setSource(QUrl(QStringLiteral("qrc:/src/qmlsrc/map.qml")));
  }

  // mui->qwMainTab->setFixedHeight(50);
  mui->qwMainTab->rootContext()->setContextProperty("FontSize", fontSize);
  mui->qwMainTab->rootContext()->setContextProperty("isDark", isDark);
  mui->qwMainTab->rootContext()->setContextProperty("maintabHeight",
                                                    mui->qwMainTab->height());
  mui->qwMainTab->rootContext()->setContextProperty("mw_one", mw_one);
  mui->qwMainTab->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/maintab.qml")));

  mui->qwMainDate->rootContext()->setContextProperty("FontSize", fontSize);
  mui->qwMainDate->rootContext()->setContextProperty("isDark", isDark);
  mui->qwMainDate->rootContext()->setContextProperty("isAniEffects", true);
  mui->qwMainDate->rootContext()->setContextProperty("maindateWidth",
                                                     mui->qwMainDate->width());
  mui->qwMainDate->rootContext()->setContextProperty("m_Method", m_Method);
  mui->qwMainDate->rootContext()->setContextProperty("mw_one", mw_one);
  mui->qwMainDate->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/maindate.qml")));

  mui->qwMainEvent->rootContext()->setContextProperty("isDark", isDark);
  mui->qwMainEvent->rootContext()->setContextProperty("fontSize", fontSize);
  mui->qwMainEvent->rootContext()->setContextProperty("isAniEffects", true);
  mui->qwMainEvent->rootContext()->setContextProperty(
      "maineventWidth", mui->qwMainEvent->width());
  mui->qwMainEvent->rootContext()->setContextProperty("m_Method", m_Method);
  mui->qwMainEvent->rootContext()->setContextProperty("mw_one", mw_one);
  mui->qwMainEvent->rootContext()->setContextProperty("main_date", "");
  mui->qwMainEvent->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/mainevent.qml")));

  // MainChart
  mui->qwMainChart->hide();
  mui->qwMainChart->rootContext()->setContextProperty("isDark", isDark);
  mui->qwMainChart->rootContext()->setContextProperty("mw_one", mw_one);
  mui->qwMainChart->rootContext()->setContextProperty("chartCategories",
                                                      mw_one->chartCategories);
  mui->qwMainChart->rootContext()->setContextProperty("chartFreqValues",
                                                      mw_one->qmlFreqValues);
  mui->qwMainChart->rootContext()->setContextProperty("chartAmountValues",
                                                      mw_one->qmlAmountValues);
  mui->qwMainChart->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/mainchart.qml")));

  mui->qwNotesTree->rootContext()->setContextProperty("fontSize", fontSize);
  mui->qwNotesTree->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/tree_main.qml")));

  mui->qw_Img->rootContext()->setContextProperty("myW", mw_one->width());
  mui->qw_Img->rootContext()->setContextProperty("myH", mw_one->height());

  // Reader
  mui->qwReader->rootContext()->setContextProperty("myW", mw_one->width());
  mui->qwReader->rootContext()->setContextProperty("myH", mw_one->height());
  mui->qwReader->rootContext()->setContextProperty("m_Reader", m_Reader);
  mui->qwReader->rootContext()->setContextProperty("mw_one", mw_one);
  mui->qwReader->rootContext()->setContextProperty("currentPage", currentPage);
  mui->qwReader->rootContext()->setContextProperty("totalPages", totalPages);
  mui->qwReader->rootContext()->setContextProperty("myBackgroundColor",
                                                   "#FFFFFF");
  mui->qwReader->rootContext()->setContextProperty("isAutoRun",
                                                   QVariant(false));

  mui->qwReader->rootContext()->setContextProperty("FontSize", fontSize);
  mui->qwReader->rootContext()->setContextProperty("uiFontSize", fontSize);
  mui->qwReader->rootContext()->setContextProperty("backImgFile", "/res/b.png");
  mui->qwReader->rootContext()->setContextProperty("myTextColor", "#664E30");

  m_ReaderSet->setScrollValue();

  // Book List
  if (mui->qwBookList->source().isEmpty()) {
    mui->qwBookList->rootContext()->setContextProperty("isDark", isDark);
    mui->qwBookList->rootContext()->setContextProperty("fontSize", fontSize);
    mui->qwBookList->rootContext()->setContextProperty("m_Reader", m_Reader);
    mui->qwBookList->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/booklist.qml")));
  }

  initNotesQW();
}

void MainWindow::initNotesQW() {
  if (mui->qwNoteBook->source().isEmpty()) {
    mui->qwNoteBook->rootContext()->setContextProperty("isDark", isDark);
    mui->qwNoteBook->rootContext()->setContextProperty("m_NotesList",
                                                       m_NotesList);
    mui->qwNoteBook->rootContext()->setContextProperty("mw_one", mw_one);
    mui->qwNoteBook->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/notebook.qml")));

    if (isAndroid)
      mui->qwNoteList->rootContext()->setContextProperty("noteTimeFontSize",
                                                         12);
    else
      mui->qwNoteList->rootContext()->setContextProperty("noteTimeFontSize", 8);
    mui->qwNoteList->rootContext()->setContextProperty("isDark", isDark);
    mui->qwNoteList->rootContext()->setContextProperty("m_NotesList",
                                                       m_NotesList);
    mui->qwNoteList->rootContext()->setContextProperty("mw_one", mw_one);
    mui->qwNoteList->rootContext()->setContextProperty("noteModel",
                                                       m_NotesList->noteModel);
    mui->qwNoteList->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/notelist.qml")));

    mui->qwNoteTools->rootContext()->setContextProperty("isDark", isDark);
    mui->qwNoteTools->rootContext()->setContextProperty("m_NotesList",
                                                        m_NotesList);
    mui->qwNoteTools->rootContext()->setContextProperty("m_Notes", m_Notes);
    mui->qwNoteTools->rootContext()->setContextProperty("mw_one", mw_one);
    mui->qwNoteTools->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/note_toolsbar.qml")));

    mui->qwNotesSearchResult->rootContext()->setContextProperty("isDark",
                                                                isDark);
    mui->qwNotesSearchResult->rootContext()->setContextProperty("fontSize",
                                                                fontSize);
    mui->qwNotesSearchResult->rootContext()->setContextProperty("m_NotesList",
                                                                m_NotesList);
    mui->qwNotesSearchResult->rootContext()->setContextProperty("mw_one",
                                                                mw_one);
    mui->qwNotesSearchResult->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/SearchResults.qml")));

    mui->qwNoteRecycle->rootContext()->setContextProperty("isDark", isDark);
    mui->qwNoteRecycle->rootContext()->setContextProperty("m_Method", m_Method);
    mui->qwNoteRecycle->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/noterecycle.qml")));
  }
}

void MainWindow::init_Theme() {
  // Get the background color to fit the dark mode
  QPalette pal = mw_one->palette();
  QBrush brush = pal.window();
  red = brush.color().red();

  qDebug() << "red=" << red;

  mui->qwMainTab->rootContext()->setContextProperty("isDark", isDark);
  mui->qwMainDate->rootContext()->setContextProperty("isDark", isDark);
  mui->qwMainEvent->rootContext()->setContextProperty("isDark", isDark);
  mui->qwMainChart->rootContext()->setContextProperty("isDark", isDark);

  mui->qwTodo->rootContext()->setContextProperty("isDark", isDark);
  mui->qwRecycle->rootContext()->setContextProperty("isDark", isDark);
  mui->qwNoteBook->rootContext()->setContextProperty("isDark", isDark);
  mui->qwNoteList->rootContext()->setContextProperty("isDark", isDark);
  mui->qwNoteVersion->rootContext()->setContextProperty("isDark", isDark);
  mui->qwNoteDiff->rootContext()->setContextProperty("isDark", isDark);
  mui->qwNoteGraphView->rootContext()->setContextProperty("isDark", isDark);

  mui->qwNotesSearchResult->rootContext()->setContextProperty("isDark", isDark);
  mui->qwSearch->rootContext()->setContextProperty("isDark", isDark);
  mui->qwBakList->rootContext()->setContextProperty("isDark", isDark);
  mui->qwViewCate->rootContext()->setContextProperty("isDark", isDark);
  mui->qwTabRecycle->rootContext()->setContextProperty("isDark", isDark);
  mui->qwNoteRecycle->rootContext()->setContextProperty("isDark", isDark);
  mui->qwCategory->rootContext()->setContextProperty("isDark", isDark);
  mui->qwSelTab->rootContext()->setContextProperty("isDark", isDark);
  mui->qwBookList->rootContext()->setContextProperty("isDark", isDark);
  mui->qwReportSub->rootContext()->setContextProperty("isDark", isDark);
  mui->qwSteps->rootContext()->setContextProperty("isDark", isDark);
  mui->qwGpsList->rootContext()->setContextProperty("isDark", isDark);
  mui->qwReport->rootContext()->setContextProperty("isDark", isDark);

  mui->qwCata->rootContext()->setContextProperty("isDark", isDark);
  mui->qwBookmark->rootContext()->setContextProperty("isDark", isDark);
  mui->qwReader->rootContext()->setContextProperty("isDark", isDark);
  mui->qwViewBookNote->rootContext()->setContextProperty("isDark", isDark);

  if (!isDark) {
    mui->btnAddTodo->setIcon(QIcon(":/res/plus_l.svg"));
    mui->btnClear->setIcon(QIcon(":/res/clear.png"));

    mui->btnReader->setIcon(QIcon(":/res/reader.svg"));
    mui->btnTodo->setIcon(QIcon(":/res/todo.svg"));
    mui->btnSteps->setIcon(QIcon(":/res/steps.svg"));
    mui->btnNotes->setIcon(QIcon(":/res/note.svg"));

    mui->btnFind->setIcon(QIcon(":/res/find.svg"));

    mui->btnSelTab->setIcon(QIcon(":/res/tab.svg"));

    mui->btnMenu->setIcon(QIcon(":/res/mainmenu.svg"));
    mui->btnHome->setIcon(QIcon(":/res/home.svg"));
    mui->btnAdd->setIcon(QIcon(":/res/additem.svg"));
    mui->btnDel->setIcon(QIcon(":/res/delitem.svg"));
    mui->btnSync->setIcon(QIcon(":/res/upload.svg"));

    m_Steps->m_speedometer->setBackgroundColor(QColor(0xF0, 0xF0, 0xF0));
    m_Steps->m_speedometer->updateThemeColors();

  } else {
    mui->btnAddTodo->setIcon(QIcon(":/res/plus_l.svg"));
    mui->btnClear->setIcon(QIcon(":/res/clear.png"));

    mui->btnFind->setIcon(QIcon(":/res/find_l.png"));

    mui->btnReader->setIcon(QIcon(":/res/reader_l.svg"));
    mui->btnTodo->setIcon(QIcon(":/res/todo_l.png"));
    mui->btnSteps->setIcon(QIcon(":/res/steps_l.svg"));
    mui->btnNotes->setIcon(QIcon(":/res/note_l.svg"));

    mui->btnSelTab->setIcon(QIcon(":/res/tab_l.svg"));

    mui->btnMenu->setIcon(QIcon(":/res/mainmenu_l.svg"));
    mui->btnHome->setIcon(QIcon(":/res/home_l.svg"));
    mui->btnAdd->setIcon(QIcon(":/res/additem_l.svg"));
    mui->btnDel->setIcon(QIcon(":/res/delitem_l.svg"));
    mui->btnSync->setIcon(QIcon(":/res/upload_l.svg"));

    m_Steps->m_speedometer->setBackgroundColor(QColor(0x32, 0x32, 0x32));
    m_Steps->m_speedometer->updateThemeColors();
  }

  mui->editTodo->verticalScrollBar()->setStyleSheet(m_Method->vsbarStyleBig);
  mui->editDetails->verticalScrollBar()->setStyleSheet(m_Method->vsbarStyleBig);

  // Edit Record UI
  int nH = mui->editCategory->height();
  if (isDark) {
    m_Method->setQLabelImage(mui->lblCategory, nH, nH, ":/res/fl_l.svg");
    m_Method->setQLabelImage(mui->lblDetailsType, nH, nH, ":/res/xq_l.svg");
    m_Method->setQLabelImage(mui->lblAmount, nH, nH, ":/res/je_l.svg");
  } else {
    m_Method->setQLabelImage(mui->lblCategory, nH, nH, ":/res/fl.svg");
    m_Method->setQLabelImage(mui->lblDetailsType, nH, nH, ":/res/xq.svg");
    m_Method->setQLabelImage(mui->lblAmount, nH, nH, ":/res/je.svg");
  }

  mw_one->m_EditRecord->on_editAmount_textChanged(mui->editAmount->text());
  mw_one->m_EditRecord->on_editCategory_textChanged(mui->editCategory->text());
  mw_one->m_EditRecord->on_editDetails_textChanged();

  // Todo
  mw_one->m_Todo->changeTodoIcon(mw_one->m_Todo->isToday);

  // Android
  m_Method->setDark(isDark);

  // Notes Editor
  m_Notes->init_md();

  // Notes ToolsBar
  mui->qwNoteTools->rootContext()->setContextProperty("isDark", isDark);

  mui->lblNoteGraphView->setWordWrap(true);
  mui->lblNoteGraphView->adjustSize();
  mw_one->init_ButtonStyle();
}

void MainWindow::init_UIWidget() {
  QFontMetrics fontMetrics(font());
  int nFontHeight = fontMetrics.height();
  int nHeight = nFontHeight * 1.5;
  mui->tabWidget->tabBar()->setFixedHeight(nHeight);

  mui->tabWidget->setFixedHeight(mui->tabWidget->tabBar()->height() + 0);

  // if (nHeight <= 36) nHeight = 36;
  //  mui->qwMainTab->setFixedHeight(nHeight);
  mui->qwMainTab->installEventFilter(mw_one);

  mui->tabWidget->hide();

  mw_one->loginTime = m_Method->setCurrentDateTimeValue();
  strDate = m_Method->setCurrentDateValue();
  isReadEnd = true;

  mw_one->installEventFilter(mw_one);

  // init textedit toolbar
  textToolbar = new TextEditToolbar(mw_one);
  EditEventFilter* editFilter = new EditEventFilter(textToolbar, mw_one);
  editFilter->setParent(mw_one);
  m_Method->setLineEditToolBar(mw_one, editFilter);
  m_Method->setTextEditToolBar(mw_one, editFilter);

  if (isAndroid) {
  } else {
    mui->btnShareBakFile->hide();
  }

  mui->qwMainDate->hide();
  mui->qwMainEvent->hide();
  mui->lblStats->hide();
  mui->lblTabTitle->hide();
  mui->btnSelTab->hide();

  mui->menubar->hide();
  mui->statusbar->hide();
  mui->frameReader->hide();
  mui->frameTodo->hide();
  mui->frameTodoRecycle->hide();
  mui->frameSteps->hide();
  mui->frameReport->hide();
  mui->frameSearch->hide();
  mui->frameBakList->hide();

  mui->frameViewCate->hide();
  mui->frameTabRecycle->hide();
  mui->frameNoteList->hide();
  mui->frameNotesSearchResult->hide();
  mui->frameNoteRecycle->hide();
  mui->f_FindNotes->hide();
  mui->btnFindNextNote->setEnabled(false);
  mui->btnFindPreviousNote->setEnabled(false);
  mui->frameNotesTree->hide();
  mui->qwCata->hide();
  mui->qwBookmark->hide();
  mui->frameDiff->hide();

  mui->frameCategory->hide();
  mui->frameSetTab->hide();
  mui->frameEditRecord->hide();
  mui->frameBookList->hide();
  mui->f_ReaderSet->hide();
  mui->btnStopSpeak->hide();

  mui->frameReader->layout()->setContentsMargins(0, 0, 0, 1);
  mui->frameReader->setContentsMargins(0, 0, 0, 1);
  mui->frameReader->layout()->setSpacing(1);
  mui->frameImgView->hide();

  mui->frameMain->layout()->setContentsMargins(1, 0, 1, 0);
  mui->frameMain->setContentsMargins(1, 0, 1, 0);
  mui->frameMain->layout()->setSpacing(1);

  mui->frameOne->hide();
  mui->btnDel->hide();

  mui->lblMonthSum->hide();

  mui->frameNotesGraph->hide();
  mui->frameNotesGraph->layout()->setContentsMargins(1, 1, 1, 1);

  mui->chkWebDAV->setStyleSheet(mw_one->m_Preferences->chkStyle);
  mui->chkAutoSync->setStyleSheet(mw_one->m_Preferences->chkStyle);
  mui->chkPlayRunVoice->setStyleSheet(mw_one->m_Preferences->chkStyle);
  mui->twCloudBackup->setCurrentIndex(1);
  mui->twCloudBackup->setTabVisible(0, false);
  mui->chkWebDAV->hide();
  mui->lblWebDAV->hide();

  mui->editWebDAVPassword->setEchoMode(QLineEdit::EchoMode::Password);
  mui->lblWebDAV->setStyleSheet(mw_one->labelNormalStyleSheet);
  mui->lblTitleEditRecord->setStyleSheet(
      m_MainHelper->clickableLabelButtonStyle);

  mui->qwReader->installEventFilter(mw_one);

  mui->tabWidget->tabBar()->installEventFilter(mw_one);
  mui->tabWidget->installEventFilter(mw_one);
  mui->tabWidget->setMouseTracking(true);
  mui->lblStats->installEventFilter(mw_one);
  mui->editSearchText->installEventFilter(mw_one);
  mui->editFindNote->installEventFilter(mw_one);

  mui->lblTitleEditRecord->installEventFilter(mw_one);

  mui->lblStats->adjustSize();
  mui->lblStats->setWordWrap(true);

  mui->lblNoteTitle->adjustSize();
  mui->lblNoteTitle->setWordWrap(true);
  mui->lblNoteTitle->hide();

  mui->progBar->setMaximumHeight(4);
  mui->progBar->hide();
  mui->progBar->setStyleSheet(
      "QProgressBar{border:0px solid #FFFFFF;"
      "height:30;"
      "background:rgba(25,255,25,0);"
      "text-align:right;"
      "color:rgb(255,255,255);"
      "border-radius:0px;}"

      "QProgressBar:chunk{"
      "border-radius:0px;"
      "background-color:rgba(18,150,219,255);"
      "}");
  mui->progReader->setStyleSheet(mui->progBar->styleSheet());
  mui->progReader->setFixedHeight(4);
  mui->progPage->setStyleSheet(
      "QProgressBar{border:0px solid #FFFFFF;"
      "height:30;"
      "background:rgba(25,255,25,0);"
      "text-align:right;"
      "color:rgb(255,255,255);"
      "border-radius:0px;}"

      "QProgressBar:chunk{"
      "border-radius:0px;"
      "background-color:rgba(230, 150, 50, 255);"
      "}");
  mui->progPage->setFixedHeight(4);

  int nIConFontSize;
#ifdef Q_OS_ANDROID
  nIConFontSize = 12;
#else
  nIConFontSize = 9;
#endif
  QFont f = mw_one->font();
  f.setPointSize(nIConFontSize);
  mui->btnTodo->setFont(f);
  mui->btnSteps->setFont(f);

  mui->btnReader->setFont(f);
  mui->btnNotes->setFont(f);
  mui->btnSelTab->setFont(f);

  f.setPointSize(nIConFontSize + 0);
  mui->btnMenu->setFont(f);
  mui->btnHome->setFont(f);
  mui->btnAdd->setFont(f);
  mui->btnDel->setFont(f);
  mui->btnSync->setFont(f);
  mui->btnFind->setFont(f);

  mui->btnFind->setFont(f);

  f.setBold(true);
  mui->lblShowLineSn->setFont(f);
  mui->lblShowLineSn->setWordWrap(true);
  mui->lblShowLineSn->adjustSize();

  QString lblStyle = mw_one->labelNormalStyleSheet;
  mui->lblTotal->setStyleSheet(lblStyle);
  mui->lblDetails->setStyleSheet(lblStyle);
  mui->lblTitle->setStyleSheet(lblStyle);
  mui->lblTitle_Report->setStyleSheet(lblStyle);

  mui->f_steps_btn->setFixedHeight(mui->tabMotion->tabBar()->height());
  mui->f_steps_btn->setContentsMargins(0, 0, 0, 0);
  mui->f_steps_btn->layout()->setContentsMargins(0, 0, 0, 0);
  mui->tabMotion->setCornerWidget(mui->f_steps_btn, Qt::TopRightCorner);

  mui->tabMotion->setCurrentIndex(1);
  QString rbStyle = mui->rbCycling->styleSheet();
  mui->rbHiking->setStyleSheet(rbStyle);
  mui->rbRunning->setStyleSheet(rbStyle);
  QSettings Reg(iniDir + "gpslist.ini", QSettings::IniFormat);

  mui->rbCycling->setChecked(Reg.value("/GPS/isCycling", 0).toBool());
  mui->rbHiking->setChecked(Reg.value("/GPS/isHiking", 0).toBool());
  mui->rbRunning->setChecked(Reg.value("/GPS/isRunning", 0).toBool());
  mui->chkPlayRunVoice->setChecked(
      Reg.value("/GPS/isPlayRunVoice", 0).toBool());
  m_Steps->isChkPlayRunVoice = mui->chkPlayRunVoice->isChecked();

  mui->btnGPS->setStyleSheet(m_Steps->btnRoundStyle);
  mui->btnGPS->hide();

  mui->frame_btnGps->setFixedHeight(80);
  QWidget* centralWidget = new QWidget(mw_one);
  QVBoxLayout* layout = new QVBoxLayout(centralWidget);

  m_MainHelper->sliderButton = new SliderButton(centralWidget);
  m_MainHelper->sliderButton->setTipText(tr("Slide Right to Start or Stop"));
  layout->addWidget(m_MainHelper->sliderButton);

  QObject::connect(m_MainHelper->sliderButton, &SliderButton::sliderMovedToEnd,
                   mw_one, [&]() { mui->btnGPS->click(); });

  mui->frame_btnGps->layout()->addWidget(centralWidget);

  int fh = 80 - mui->frame_btnGps->contentsMargins().top() * 2 -
           layout->contentsMargins().top() * 2 -
           m_MainHelper->sliderButton->contentsMargins().top() * 2 -
           centralWidget->contentsMargins().top() * 2 - 10;
  mui->btnPause->setFixedHeight(fh);
  mui->btnPause->setFixedWidth(fh);
  mui->btnPause->setIcon(QIcon(":/res/epaused.svg"));
  mui->btnPause->setIconSize(QSize(fh - 10, fh - 10));
  mui->frame_btnGps->layout()->removeWidget(mui->btnPause);
  mui->frame_btnGps->layout()->addWidget(mui->btnPause);
  mui->btnPause->setEnabled(false);
  mui->btnPause->hide();
}

QTreeWidget* MainWindow::init_TreeWidget(QString name) {
  QTreeWidget* tw = new QTreeWidget(mw_one);
  tw->setFixedHeight(0);
  tw->setObjectName(name);

  QFont font;
  font.setPointSize(fontSize);
  tw->setFont(font);
  font.setBold(true);
  tw->header()->setFont(font);

  font.setPointSize(fontSize + 1);

  tw->setColumnCount(4);
  tw->headerItem()->setText(0, "  " + tr("Date") + "  ");
  tw->headerItem()->setText(1, "  " + tr("Freq") + "  ");
  tw->headerItem()->setText(2, tr("Amount"));
  tw->headerItem()->setText(3, tr("Year"));
  tw->setColumnHidden(3, true);

  tw->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  tw->header()->setDefaultAlignment(Qt::AlignCenter);
  tw->headerItem()->setTextAlignment(2, Qt::AlignRight);
  tw->setAlternatingRowColors(true);
  tw->setFrameShape(QTreeWidget::NoFrame);
  tw->installEventFilter(mw_one);
  tw->viewport()->installEventFilter(mw_one);
  tw->setUniformRowHeights(true);  // 加快展开速度
  connect(tw, &QTreeWidget::itemClicked, mw_one, &MainWindow::on_twItemClicked);
  connect(tw, &QTreeWidget::itemDoubleClicked, mw_one,
          &MainWindow::on_twItemDoubleClicked);
  connect(tw, &QTreeWidget::itemPressed, [=]() {});

  connect(tw->verticalScrollBar(), &QScrollBar::valueChanged, [=]() {});

  QScrollBar* SB = tw->verticalScrollBar();
  SB->setStyleSheet(m_Method->vsbarStyleSmall);
  tw->setStyleSheet(mw_one->treeStyle);
  tw->setVerticalScrollMode(QTreeWidget::ScrollPerPixel);

  return tw;
}

void MainWindow::init_Options() {
  QSettings Reg2(iniDir + "ymd.ini", QSettings::IniFormat);

  btnYText = Reg2.value("/YMD/btnYText", 2022).toString();

  btnMText = Reg2.value("/YMD/btnMText", tr("Month")).toString();

  btnDText = Reg2.value("/YMD/btnDText", 1).toString();

  btnYearText = Reg2.value("/YMD/btnYearText", "2022").toString();
  mui->btnYear->setText(btnYearText);
  btnMonthText = Reg2.value("/YMD/btnMonthText", "01").toString();
  mui->btnMonth->setText(btnMonthText);

  s_y1 = Reg2.value("/YMD/Y1", 2022).toInt();
  s_y2 = Reg2.value("/YMD/Y2", 2022).toInt();
  s_m1 = Reg2.value("/YMD/M1", 1).toInt();
  s_m2 = Reg2.value("/YMD/M2", 12).toInt();
  s_d1 = Reg2.value("/YMD/D1", 1).toInt();
  s_d2 = Reg2.value("/YMD/D2", 1).toInt();

  mui->btnStartDate->setText(QString::number(s_y1) + "  " +
                             QString("%1").arg(s_m1, 2, 10, QLatin1Char('0')) +
                             "  " +
                             QString("%1").arg(s_d1, 2, 10, QLatin1Char('0')));
  mui->btnEndDate->setText(QString::number(s_y2) + "  " +
                           QString("%1").arg(s_m2, 2, 10, QLatin1Char('0')) +
                           "  " +
                           QString("%1").arg(s_d2, 2, 10, QLatin1Char('0')));

  isWholeMonth = Reg2.value("/YMD/isWholeMonth", 1).toBool();
  isDateSection = Reg2.value("/YMD/isDateSection", 0).toBool();

  // time machine
  QSettings RegTime(privateDir + "timemachine.ini", QSettings::IniFormat);

  int countTime = RegTime.value("/TimeLines/Count", 0).toInt();
  for (int i = 0; i < countTime; i++)
    timeLines.append(
        RegTime.value("/TimeLines/Files" + QString::number(i)).toString());

  m_Preferences->initOptions();
  m_Preferences->ui->btnReStart->hide();

  mainMenu = new QMenu(this);
  init_Menu(mainMenu);
}
