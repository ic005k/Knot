#include "MainWindow.h"
#include "SearchWorker.h"
#include "defines.h"

extern SearchWorker* m_searchWorker;

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
  CurrentYear = QString::number(QDate::currentDate().year());
  tabData = mui->tabWidget;

  m_MainHelper = new MainHelper(this);

  m_Method = new Method(this);

  m_AboutThis = new AboutThis(this);
  m_Preferences = new Preferences(this);
  m_EditRecord = new EditRecord();

  m_Todo = new Todo(this);
  m_Report = new Report(this);

  // m_Notes = new Notes(this);

  // m_StepsOptions = new StepsOptions(this);

  // m_Steps = new Steps(this);

  return;

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
  setToolButtonAnimation(mui->btnMenu, true);
  setToolButtonAnimation(mui->btnHome, true);
  setToolButtonAnimation(mui->btnReader, true);
  setToolButtonAnimation(mui->btnTodo, true);
  setToolButtonAnimation(mui->btnSteps, true);
  setToolButtonAnimation(mui->btnNotes, true);
  setToolButtonAnimation(mui->btnAdd, true);
  setToolButtonAnimation(mui->btnDel, true);

  setToolButtonAnimation(mui->btnSync, true);
  setToolButtonAnimation(mui->btnFind, true);
  setToolButtonAnimation(mui->btnSelTab, true);

  if (isDark) {
  } else {
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

void MainWindow::initMainQW() {}

void MainWindow::initNotesQW() {}

void MainWindow::init_Theme() {
  // Get the background color to fit the dark mode
  QPalette pal = mw_one->palette();
  QBrush brush = pal.window();
  red = brush.color().red();

  qDebug() << "red=" << red;

  if (!isDark) {
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
}

void MainWindow::init_UIWidget() {
  // QFontMetrics fontMetrics(font());
  // int nFontHeight = fontMetrics.height();
  // int nHeight = nFontHeight * 1.5;
  //  mui->tabWidget->tabBar()->setFixedHeight(nHeight);

  // mui->tabWidget->setFixedHeight(mui->tabWidget->tabBar()->height() + 0);

  if (isAndroid) {
    mui->tabWidget->hide();
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

  mui->lblVectorStatus->setText("");
  mui->lblVectorStatus->hide();

  mui->lblStats->hide();

  mui->btnSelTab->hide();

  mui->menubar->hide();
  mui->statusbar->hide();

  mui->frameSteps->hide();

  mui->frameCategory->hide();

  mui->frameEditRecord->hide();

  m_Reader->hideBookListWin();

  mui->frameMain->layout()->setContentsMargins(1, 1, 1, 1);
  mui->frameMain->setContentsMargins(1, 1, 1, 1);
  mui->frameMain->layout()->setSpacing(5);

  mui->frameOne->hide();
  mui->btnDel->hide();

  mui->lblMonthSum->hide();

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

  mui->tabWidget->tabBar()->installEventFilter(mw_one);
  mui->tabWidget->installEventFilter(mw_one);
  mui->tabWidget->setMouseTracking(true);
  mui->lblStats->installEventFilter(mw_one);

  mui->lblTitleEditRecord->installEventFilter(mw_one);

  mui->lblStats->adjustSize();
  mui->lblStats->setWordWrap(true);

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
