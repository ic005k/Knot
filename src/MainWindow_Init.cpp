#include "MainWindow.h"
#include "SearchWorker.h"
#include "defines.h"

extern SearchWorker* m_searchWorker;

void MainWindow::init_TotalData() {
  int count = mw_one->ui->tabWidget->tabBar()->count();
  for (int i = 0; i < count; i++) {
    mw_one->ui->tabWidget->removeTab(0);
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
    mw_one->ui->tabWidget->addTab(tw, tabText);

    addItem(tabText, "", "", "", 0);
  }

  if (TabCount == 0) {
    QString tw_name = m_Notes->getDateTimeStr() + "_" + QString::number(1);
    QTreeWidget* tw = init_TreeWidget(tw_name);

    QString tabText = tr("Tab") + " " + QString::number(1);
    mw_one->ui->tabWidget->addTab(tw, tabText);
    addItem(tabText, "", "", "", 0);

    saveTab();
  }

  m_EditRecord->init_MyCategory();

  if (TabCount > 0)
    currentTabIndex = RegTab.value("CurrentIndex").toInt();
  else
    currentTabIndex = 0;

  mw_one->ui->tabWidget->setCurrentIndex(currentTabIndex);
  setCurrentIndex(currentTabIndex);
  QTreeWidget* twCur = (QTreeWidget*)tabData->currentWidget();
  readData(twCur);
  mw_one->ui->actionImport_Data->setEnabled(false);
  mw_one->ui->actionExport_Data->setEnabled(false);
  mw_one->ui->actionDel_Tab->setEnabled(false);
  mw_one->ui->actionAdd_Tab->setEnabled(false);
  mw_one->ui->actionView_App_Data->setEnabled(false);

  if (!initMain) {
    mw_one->ui->progBar->setHidden(false);
    mw_one->ui->progBar->setMaximum(0);
  }

  m_ReadTWThread->start();
}

void MainWindow::init_Instance() {
  CurrentYear = QString::number(QDate::currentDate().year());
  tabData = ui->tabWidget;

  m_Method = new Method(this);

  m_MainHelper = new MainHelper(this);

  m_Preferences = new Preferences(this);

  m_AboutThis = new AboutThis(this);

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

  // AI
  m_ainetMgr = new QNetworkAccessManager(nullptr);
  m_ainetMgr->setTransferTimeout(90000);

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
  connect(mySearchThread, &QThread::finished, this, &MainWindow::searchDone,
          Qt::QueuedConnection);  // 强制切主线程

  m_workerThread = new QThread(this);
  m_workerThread->setObjectName("SearchWorkerThread");
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
            safeCloseProgress();
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
  setToolButtonAnimation(mw_one->ui->btnMenu, true);
  setToolButtonAnimation(mw_one->ui->btnHome, true);
  setToolButtonAnimation(mw_one->ui->btnReader, true);
  setToolButtonAnimation(mw_one->ui->btnTodo, true);
  setToolButtonAnimation(mw_one->ui->btnSteps, true);
  setToolButtonAnimation(mw_one->ui->btnNotes, true);
  setToolButtonAnimation(mw_one->ui->btnAdd, true);
  setToolButtonAnimation(mw_one->ui->btnDel, true);

  setToolButtonAnimation(mw_one->ui->btnSync, true);
  setToolButtonAnimation(mw_one->ui->btnFind, true);
  setToolButtonAnimation(mw_one->ui->btnSelTab, true);

  if (isDark) {
  } else {
  }

  /*mw_one->ui->btnPages->setStyleSheet(
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

void MainWindow::initMainQW() {}

void MainWindow::initNotesQW() {}

void MainWindow::init_Theme() {
  // Get the background color to fit the dark mode
  QPalette pal = mw_one->palette();
  QBrush brush = pal.window();
  red = brush.color().red();

  qDebug() << "red=" << red;

  if (!isDark) {
    mw_one->ui->btnReader->setIcon(QIcon(":/res/reader.svg"));
    mw_one->ui->btnTodo->setIcon(QIcon(":/res/todo.svg"));
    mw_one->ui->btnSteps->setIcon(QIcon(":/res/steps.svg"));
    mw_one->ui->btnNotes->setIcon(QIcon(":/res/note.svg"));

    mw_one->ui->btnFind->setIcon(QIcon(":/res/find.svg"));

    mw_one->ui->btnSelTab->setIcon(QIcon(":/res/tab.svg"));

    mw_one->ui->btnMenu->setIcon(QIcon(":/res/mainmenu.svg"));
    mw_one->ui->btnHome->setIcon(QIcon(":/res/home.svg"));
    mw_one->ui->btnAdd->setIcon(QIcon(":/res/additem.svg"));
    mw_one->ui->btnDel->setIcon(QIcon(":/res/delitem.svg"));
    mw_one->ui->btnSync->setIcon(QIcon(":/res/upload.svg"));

    m_Steps->m_speedometer->setBackgroundColor(QColor(0xF0, 0xF0, 0xF0));
    m_Steps->m_speedometer->updateThemeColors();

  } else {
    mw_one->ui->btnFind->setIcon(QIcon(":/res/find_l.png"));

    mw_one->ui->btnReader->setIcon(QIcon(":/res/reader_l.svg"));
    mw_one->ui->btnTodo->setIcon(QIcon(":/res/todo_l.png"));
    mw_one->ui->btnSteps->setIcon(QIcon(":/res/steps_l.svg"));
    mw_one->ui->btnNotes->setIcon(QIcon(":/res/note_l.svg"));

    mw_one->ui->btnSelTab->setIcon(QIcon(":/res/tab_l.svg"));

    mw_one->ui->btnMenu->setIcon(QIcon(":/res/mainmenu_l.svg"));
    mw_one->ui->btnHome->setIcon(QIcon(":/res/home_l.svg"));
    mw_one->ui->btnAdd->setIcon(QIcon(":/res/additem_l.svg"));
    mw_one->ui->btnDel->setIcon(QIcon(":/res/delitem_l.svg"));
    mw_one->ui->btnSync->setIcon(QIcon(":/res/upload_l.svg"));

    m_Steps->m_speedometer->setBackgroundColor(QColor(0x32, 0x32, 0x32));
    m_Steps->m_speedometer->updateThemeColors();
  }

  mw_one->ui->editDetails->verticalScrollBar()->setStyleSheet(
      m_Method->vsbarStyleBig);

  // Edit Record UI
  int nH = mw_one->ui->editCategory->height();
  if (isDark) {
    m_Method->setQLabelImage(mw_one->ui->lblCategory, nH, nH, ":/res/fl_l.svg");
    m_Method->setQLabelImage(mw_one->ui->lblDetailsType, nH, nH,
                             ":/res/xq_l.svg");
    m_Method->setQLabelImage(mw_one->ui->lblAmount, nH, nH, ":/res/je_l.svg");
  } else {
    m_Method->setQLabelImage(mw_one->ui->lblCategory, nH, nH, ":/res/fl.svg");
    m_Method->setQLabelImage(mw_one->ui->lblDetailsType, nH, nH,
                             ":/res/xq.svg");
    m_Method->setQLabelImage(mw_one->ui->lblAmount, nH, nH, ":/res/je.svg");
  }

  mw_one->m_EditRecord->on_editAmount_textChanged(
      mw_one->ui->editAmount->text());
  mw_one->m_EditRecord->on_editCategory_textChanged(
      mw_one->ui->editCategory->text());
  mw_one->m_EditRecord->on_editDetails_textChanged();

  // Todo
  mw_one->m_Todo->changeTodoIcon(mw_one->m_Todo->isToday);

  // Android
  m_Method->setDark(isDark);

  // Notes Editor
  m_Notes->init_md();

  mw_one->init_ButtonStyle();

  if (isDark) {
    m_Notes->ui->listSearchResults->setStyleSheet(
        m_Method->listWidgetDarkStyle);
    m_Notes->ui->listNoteLink->setStyleSheet(m_Method->listWidgetDarkStyle);
  } else {
    m_Notes->ui->listSearchResults->setStyleSheet(
        m_Method->listWidgetLightStyle);
    m_Notes->ui->listNoteLink->setStyleSheet(m_Method->listWidgetLightStyle);
  }

  qInfo() << "Theme初始化完成。";
}

void MainWindow::init_UIWidget() {
  if (isAndroid) {
    mw_one->ui->tabWidget->hide();
  }

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

  mw_one->ui->lblVectorStatus->setText("");
  mw_one->ui->lblVectorStatus->hide();

  mw_one->ui->lblStats->hide();

  mw_one->ui->btnSelTab->hide();

  mw_one->ui->menubar->hide();
  mw_one->ui->statusbar->hide();

  mw_one->ui->frameCategory->hide();

  mw_one->ui->frameEditRecord->hide();

  mw_one->ui->frameMain->layout()->setContentsMargins(1, 1, 1, 1);
  mw_one->ui->frameMain->setContentsMargins(1, 1, 1, 1);
  mw_one->ui->frameMain->layout()->setSpacing(5);

  mw_one->ui->frameOne->hide();
  mw_one->ui->btnDel->hide();

  mw_one->ui->lblMonthSum->hide();

  mw_one->ui->chkWebDAV->setStyleSheet(mw_one->m_Preferences->chkStyle);
  mw_one->ui->chkAutoSync->setStyleSheet(mw_one->m_Preferences->chkStyle);
  // m_Steps->ui->chkPlayRunVoice->setStyleSheet(mw_one->m_Preferences->chkStyle);
  mw_one->ui->twCloudBackup->setCurrentIndex(1);
  mw_one->ui->twCloudBackup->setTabVisible(0, false);
  mw_one->ui->chkWebDAV->hide();
  mw_one->ui->lblWebDAV->hide();

  mw_one->ui->editWebDAVPassword->setEchoMode(QLineEdit::EchoMode::Password);
  mw_one->ui->lblWebDAV->setStyleSheet(mw_one->labelNormalStyleSheet);
  mw_one->ui->lblTitleEditRecord->setStyleSheet(
      m_MainHelper->clickableLabelButtonStyle);

  mw_one->ui->tabWidget->tabBar()->installEventFilter(mw_one);
  mw_one->ui->tabWidget->installEventFilter(mw_one);
  mw_one->ui->tabWidget->setMouseTracking(true);
  mw_one->ui->lblStats->installEventFilter(mw_one);

  mw_one->ui->lblTitleEditRecord->installEventFilter(mw_one);

  mw_one->ui->lblStats->adjustSize();
  mw_one->ui->lblStats->setWordWrap(true);

  mw_one->ui->progBar->setMaximumHeight(4);
  mw_one->ui->progBar->hide();
  mw_one->ui->progBar->setStyleSheet(
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

  int nIConFontSize;
#ifdef Q_OS_ANDROID
  nIConFontSize = 12;
#else
  nIConFontSize = 9;
#endif
  QFont f = mw_one->font();
  f.setPointSize(nIConFontSize);
  mw_one->ui->btnTodo->setFont(f);
  mw_one->ui->btnSteps->setFont(f);

  mw_one->ui->btnReader->setFont(f);
  mw_one->ui->btnNotes->setFont(f);
  mw_one->ui->btnSelTab->setFont(f);

  f.setPointSize(nIConFontSize + 0);
  mw_one->ui->btnMenu->setFont(f);
  mw_one->ui->btnHome->setFont(f);
  mw_one->ui->btnAdd->setFont(f);
  mw_one->ui->btnDel->setFont(f);
  mw_one->ui->btnSync->setFont(f);
  mw_one->ui->btnFind->setFont(f);

  mw_one->ui->btnFind->setFont(f);
}

QTreeWidget* MainWindow::init_TreeWidget(QString name) {
  QTreeWidget* tw = new QTreeWidget(mw_one);
  // tw->setFixedHeight(0);
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

  btnMonthText = Reg2.value("/YMD/btnMonthText", "01").toString();

  s_y1 = Reg2.value("/YMD/Y1", 2022).toInt();
  s_y2 = Reg2.value("/YMD/Y2", 2022).toInt();
  s_m1 = Reg2.value("/YMD/M1", 1).toInt();
  s_m2 = Reg2.value("/YMD/M2", 12).toInt();
  s_d1 = Reg2.value("/YMD/D1", 1).toInt();
  s_d2 = Reg2.value("/YMD/D2", 1).toInt();

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
}
