#include "MainWindow.h"

#include "ui_MainWindow.h"

QString ver = "2.0.20";
QString appName = "Knot";

QList<QPointF> PointList;
QList<double> doubleList;

QGridLayout *gl1;
QTreeWidgetItem *parentItem;
bool isrbFreq = true;
bool isEBook, isReport, isUpData, isZipOK, isMenuImport, isDownData, isEncrypt,
    isRemovedTopItem;
bool isAdd = false;

QString iniFile, iniDir, privateDir, bakfileDir, strDate, readDate, noteText,
    strStats, SaveType, strY, strM, btnYText, btnMText, btnDText, errorInfo,
    CurrentYearMonth, zipfile, txt, searchStr, currentMDFile, copyText,
    imgFileName, defaultFontFamily, customFontFamily, encPassword;
QStringList listM;

int today, fontSize, red, currentTabIndex;
int chartMax = 5;

double yMaxMonth, yMaxDay;
MainWindow *mw_one;
Method *m_Method;
QTabWidget *tabData, *tabChart;
bool loading, isReadEnd, isReadTWEnd;
bool isReadEBookEnd = true;
bool isSaveEnd = true;
bool isBreak = false;
bool isDark = false;
bool isDelData = false;

QRegularExpression regxNumber("^-?[0-9.]*$");

QSettings *iniPreferences;
CloudBackup *m_CloudBackup;

extern bool isAndroid, isIOS, zh_cn, isEpub, isEpubError, isText, isPDF,
    isWholeMonth, isDateSection, isPasswordError, isInitThemeEnd,
    isNeedExecDeskShortcut;
extern QString btnYearText, btnMonthText, strPage, ebookFile, strTitle,
    fileName, strOpfPath, catalogueFile, strShowMsg;
extern int iPage, sPos, totallines, baseLines, htmlIndex, s_y1, s_m1, s_d1,
    s_y2, s_m2, s_d2, totalPages, currentPage;
extern QStringList readTextList, htmlFiles, listCategory;

extern CategoryList *m_CategoryList;
extern ReaderSet *m_ReaderSet;

extern void setTableNoItemFlags(QTableWidget *t, int row);
extern int deleteDirfile(QString dirName);
extern QString loadText(QString textFile);
extern QString getTextEditLineText(QTextEdit *txtEdit, int i);
extern void TextEditToFile(QTextEdit *txtEdit, QString fileName);
extern void StringToFile(QString buffers, QString fileName);
extern bool unzipToDir(const QString &zipPath, const QString &destDir);

extern WebDavHelper *listWebDavFiles(const QString &url,
                                     const QString &username,
                                     const QString &password);

extern QSplashScreen *splash;
extern ShowMessage *m_ShowMessage;
extern ColorDialog *colorDlg;
extern PrintPDF *m_PrintPDF;

void RegJni(const char *myClassName);

#ifdef Q_OS_ANDROID
static void JavaNotify_0();
static void JavaNotify_1();
static void JavaNotify_2();
static void JavaNotify_3();
static void JavaNotify_4();
static void JavaNotify_5();
static void JavaNotify_6();
static void JavaNotify_7();
static void JavaNotify_8();
static void JavaNotify_9();
static void JavaNotify_10();
static void JavaNotify_11();
static void JavaNotify_12();
static void JavaNotify_13();
static void JavaNotify_14();
static void JavaNotify_15();
#endif

BakDataThread::BakDataThread(QObject *parent) : QThread{parent} {}
void BakDataThread::run() {
  mw_one->bakData();

  emit isDone();
}

void MainWindow::bakDataDone() {
  closeProgress();

  if (errorInfo != "") {
    ShowMessage *msg = new ShowMessage(this);
    msg->showMsg("Knot", errorInfo, 1);
    return;
  }

  if (isUpData) {
    m_CloudBackup->uploadData();
  } else {
    if (QFile(zipfile).exists()) {
      m_Preferences->appendBakFile(
          QDateTime::currentDateTime().toString("yyyy-M-d HH:mm:ss") + "\n" +
              strLatestModify + "\n" +
              m_Method->getFileSize(QFile(zipfile).size(), 2),
          zipfile);

      ShowMessage *m_ShowMsg = new ShowMessage(this);
      m_ShowMsg->showMsg("Knot",
                         tr("The data was exported successfully.") + "\n\n" +
                             zipfile + "\n\n" +
                             m_Method->getFileSize(QFile(zipfile).size(), 2),
                         1);
    }
  }

  isUpData = false;
}

ImportDataThread::ImportDataThread(QObject *parent) : QThread{parent} {}
void ImportDataThread::run() {
  if (isMenuImport || isDownData) mw_one->importBakData(zipfile);

  emit isDone();
}

void MainWindow::importDataDone() {
  m_Method->setOSFlag();

  if (isPasswordError) {
    closeProgress();
    ShowMessage *msg = new ShowMessage(this);
    msg->showMsg("Knot", tr("The password of the encrypted file is wrong!"), 1);
    return;
  }

  if (!zipfile.isNull() && isZipOK) {
    m_Notes->init_all_notes();
    m_Todo->init_Todo();

    loading = true;
    init_TotalData();
    loading = false;

    while (!isReadTWEnd)
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    while (!isSaveEnd)
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    on_tabWidget_currentChanged(tabData->currentIndex());

    m_Steps->clearAllGpsList();
    m_Steps->loadGpsList(QDate::currentDate().year(),
                         QDate::currentDate().month());
    m_Steps->allGpsTotal();

    m_NotesList->m_dbManager.updateFilesIndex(iniDir + "memo");
  }

  closeProgress();

  if (isMenuImport) {
    if (!isZipOK) {
      m_Method->m_widget = new QWidget(mw_one);
      ShowMessage *m_ShowMsg = new ShowMessage(this);
      m_ShowMsg->showMsg("Knot",
                         tr("Invalid data file.") + "\n\n" +
                             tr("Or the operation is canceled by the user."),
                         1);
    }
  }
}

SearchThread::SearchThread(QObject *parent) : QThread{parent} {}
void SearchThread::run() {
  m_Method->startSearch();

  emit isDone();
}

void MainWindow::searchDone() {
  m_Method->initSearchResults();
  closeProgress();
}

UpdateGpsMapThread::UpdateGpsMapThread(QObject *parent) : QThread{parent} {}
void UpdateGpsMapThread::run() {
  mw_one->m_Steps->updateGpsTrack();

  emit isDone();
}

void MainWindow::updateGpsMapDone() {
  mw_one->m_Steps->updateGpsMapUi();
  closeProgress();
}

ReadEBookThread::ReadEBookThread(QObject *parent) : QThread{parent} {}
void ReadEBookThread::run() {
  isReadEBookEnd = false;

  if (isEBook) {
    mw_one->m_Reader->openFile(ebookFile);
  }

  if (isReport) {
    mw_one->m_Report->getMonthData();
  }

  m_Method->Sleep(100);

  emit isDone();
}

void MainWindow::readEBookDone() {
  if (isEBook) {
    m_Reader->readBookDone();
    isEBook = false;
  }

  if (isReport) {
    m_Report->updateTable();
    ui->lblTitle->setText(tabData->tabText(tabData->currentIndex()));

    ui->btnCategory->hide();
    if (listCategory.count() > 0) ui->btnCategory->setHidden(false);

    isReport = false;
    closeProgress();
  }

  isReadEBookEnd = true;
}

ReadTWThread::ReadTWThread(QObject *parent) : QThread{parent} {}
void ReadTWThread::run() {
  isReadTWEnd = false;
  MainWindow::readDataInThread(currentTabIndex);
  emit isDone();
}

void MainWindow::readTWDone() {
  for (int i = 0; i < tabData->tabBar()->count(); i++) {
    QTreeWidget *tw = (QTreeWidget *)tabData->widget(i);
    tw->setCurrentItem(tw->topLevelItem(tw->topLevelItemCount() - 1));
  }

  ui->actionImport_Data->setEnabled(true);
  ui->actionExport_Data->setEnabled(true);
  ui->actionDel_Tab->setEnabled(true);
  ui->actionAdd_Tab->setEnabled(true);
  ui->actionView_App_Data->setEnabled(true);
  isReadTWEnd = true;

  ui->progBar->setMaximum(100);
}

ReadThread::ReadThread(QObject *parent) : QThread{parent} {}

void ReadThread::run() {
  if (isBreak) {
    emit isDone();
    return;
  }
  isReadEnd = false;
  MainWindow::ReadChartData();
  emit isDone();
}

void MainWindow::ReadChartData() {
  int index = tabData->currentIndex();
  QTreeWidget *tw = (QTreeWidget *)tabData->widget(index);

  strY = get_Year(readDate);
  strM = get_Month(readDate);

  if (tabChart->currentIndex() == 0) {
    drawMonthChart();
  }
  if (tabChart->currentIndex() == 1) {
    drawDayChart();
  }
  get_Today(tw);
  init_Stats(tw);
}

void MainWindow::readChartDone() {
  if (tabChart->currentIndex() == 0) {
    initChartMonth();
  }
  if (tabChart->currentIndex() == 1) {
    initChartDay();
  }

  if (isShowDetails)
    ui->lblStats->setText(strShowDetails);
  else
    ui->lblStats->setText(strStats);
  isReadEnd = true;
}

SaveThread::SaveThread(QObject *parent) : QThread{parent} {}
void SaveThread::run() {
  isSaveEnd = false;
  MainWindow::SaveFile(SaveType);
  emit isDone();
}

void MainWindow::saveDone() {
  if (isBreak) {
    isSaveEnd = true;
    return;
  }

  isSaveEnd = true;

  ui->progBar->setMaximum(100);

  if (SaveType == "tab" || SaveType == "alltab") startRead(strDate);
}

void MainWindow::SaveFile(QString SaveType) {
  if (SaveType == "tab") {
    if (isDelData) {
      EditRecord::saveDeleted();
    } else {
      if (isAdd)
        EditRecord::saveAdded();
      else
        EditRecord::saveModified();
    }

    saveTab();

    EditRecord::saveMyClassification();

    isAdd = false;
    isDelData = false;
  }

  if (SaveType == "alltab") {
    for (int i = 0; i < tabData->tabBar()->count(); i++) {
      if (isBreak) break;

      QTreeWidget *tw = (QTreeWidget *)tabData->widget(i);
      QString name = tw->objectName();
      QString iniName =
          QString::number(QDate::currentDate().year()) + "-" + name;
      QString ini_file = iniDir + iniName + ".ini";
      if (QFile(ini_file).exists()) QFile(ini_file).remove();

      saveData(tw, i);
    }

    saveTab();
    EditRecord::saveMyClassification();
  }

  if (SaveType == "todo") {
  }

  if (SaveType == "notes") {
  }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  initMain = true;

  if (!isAndroid) {
#ifdef Q_OS_MAC
    this->setGeometry(800, 0, 390, this->height() - 60);
#endif

#ifdef Q_OS_WIN
    this->setGeometry(730, 25, 450, 620);
#endif
  }

  qRegisterMetaType<QVector<int>>("QVector<int>");
  loading = true;

  init_Instance();

  init_Options();
  init_UIWidget();
  init_ChartWidget();

  init_Theme();
  initQW();

  init_TotalData();

  m_Reader->initReader();

  loading = false;

  QTreeWidget *tw = (QTreeWidget *)tabData->currentWidget();
  startRead(strDate);
  get_Today(tw);
  init_Stats(tw);

  resetWinPos();

  m_Todo->refreshTableListsFromIni();
  m_Todo->refreshAlarm();

  QTimer::singleShot(10, this, [this]() {
    reloadMain();
    clickData();
  });

  currentMDFile = m_NotesList->getCurrentMDFile();
  if (isAndroid) {
    QTimer::singleShot(2000, this, SLOT(on_ReceiveShare()));

    if (m_Method->getExecDone() == "false") {
      m_Method->setExecDone("true");
      QTimer::singleShot(2000, this, SLOT(on_ExecShortcut()));
    }
  }

  m_Method->setAndroidFontSize(fontSize);

  init_CloudBacup();
  setEncSyncStatusTip();

  if (QFile::exists(currentMDFile)) {
    m_Notes->MD2Html(currentMDFile);
    m_Notes->loadNoteToQML();
  }

  splash->close();
  delete splash;

  initMain = false;
}

void MainWindow::init_Options() {
  QSettings Reg2(iniDir + "ymd.ini", QSettings::IniFormat);

  btnYText = Reg2.value("/YMD/btnYText", 2022).toString();

  btnMText = Reg2.value("/YMD/btnMText", tr("Month")).toString();

  btnDText = Reg2.value("/YMD/btnDText", 1).toString();

  btnYearText = Reg2.value("/YMD/btnYearText", "2022").toString();
  ui->btnYear->setText(btnYearText);
  btnMonthText = Reg2.value("/YMD/btnMonthText", "01").toString();
  ui->btnMonth->setText(btnMonthText);

  s_y1 = Reg2.value("/YMD/Y1", 2022).toInt();
  s_y2 = Reg2.value("/YMD/Y2", 2022).toInt();
  s_m1 = Reg2.value("/YMD/M1", 1).toInt();
  s_m2 = Reg2.value("/YMD/M2", 12).toInt();
  s_d1 = Reg2.value("/YMD/D1", 1).toInt();
  s_d2 = Reg2.value("/YMD/D2", 1).toInt();

  ui->btnStartDate->setText(QString::number(s_y1) + "  " +
                            QString("%1").arg(s_m1, 2, 10, QLatin1Char('0')) +
                            "  " +
                            QString("%1").arg(s_d1, 2, 10, QLatin1Char('0')));
  ui->btnEndDate->setText(QString::number(s_y2) + "  " +
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
}

void MainWindow::init_ChartWidget() {
  ui->centralwidget->layout()->setContentsMargins(1, 0, 1, 2);
  ui->centralwidget->layout()->setSpacing(1);
  ui->f_charts->setContentsMargins(0, 0, 0, 0);

  ui->f_charts->layout()->setContentsMargins(0, 0, 0, 0);
  ui->f_charts->layout()->setSpacing(0);
  frameChartHeight = 105;
  ui->f_charts->setFixedHeight(frameChartHeight);
  tabChart->setCurrentIndex(0);

  ui->glMonth->layout()->setContentsMargins(0, 0, 0, 0);
  ui->glMonth->layout()->setSpacing(0);
  ui->glDay->layout()->setContentsMargins(0, 0, 0, 0);
  ui->glDay->layout()->setSpacing(0);

  ui->f_charts->hide();
  ui->btnChartDay->hide();
  ui->btnChartMonth->hide();
  ui->rbAmount->hide();
  ui->rbFreq->hide();
  ui->rbSteps->hide();

  int a0 = 0;
  int a1 = -2;
  // Month
  chartMonth = new QChart();
  chartview = new QChartView(chartMonth);
  chartview->installEventFilter(this);
  ui->glMonth->addWidget(chartview);
  chartview->setRenderHint(QPainter::Antialiasing);
  chartMonth->legend()->hide();
  chartMonth->setMargins(QMargins(a0, a0, a0, a0));
  chartMonth->setContentsMargins(a1, a1, a1, a1);
  chartMonth->setAnimationOptions(QChart::SeriesAnimations);

  barSeries = new QBarSeries();
  series = new QSplineSeries();
  series->setPen(QPen(Qt::blue, 3, Qt::SolidLine));
  m_scatterSeries = new QScatterSeries();
  m_scatterSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);
  m_scatterSeries->setMarkerSize(10);
  chartMonth->addSeries(barSeries);
  chartMonth->addSeries(series);
  chartMonth->addSeries(m_scatterSeries);

  // Day
  chartDay = new QChart();
  chartview1 = new QChartView(chartDay);
  chartview1->installEventFilter(this);
  ui->glDay->addWidget(chartview1);
  chartview1->setRenderHint(QPainter::Antialiasing);
  chartDay->legend()->hide();
  chartDay->setMargins(QMargins(a0, a0, a0, a0));
  chartDay->setContentsMargins(a1, a1, a1, a1);
  chartDay->setAnimationOptions(QChart::SeriesAnimations);

  series2 = new QSplineSeries(chartDay);
  series2->setPen(QPen(Qt::blue, 2, Qt::SolidLine));
  m_scatterSeries2 = new QScatterSeries();
  m_scatterSeries2_1 = new QScatterSeries();

  // 散点图(用于边框)
  m_scatterSeries2->setMarkerShape(
      QScatterSeries::MarkerShapeCircle);                 // 圆形的点
  m_scatterSeries2->setBorderColor(QColor(255, 0, 0));    // 边框颜色
  m_scatterSeries2->setBrush(QBrush(QColor(255, 0, 0)));  // 背景颜色
  m_scatterSeries2->setMarkerSize(5);                     // 点大小

  // 散点图(用于中心)
  m_scatterSeries2_1->setMarkerShape(
      QScatterSeries::MarkerShapeCircle);         // 圆形的点
  m_scatterSeries2_1->setBorderColor(Qt::red);    // 边框颜色
  m_scatterSeries2_1->setBrush(QBrush(Qt::red));  // 背景颜色
  m_scatterSeries2_1->setMarkerSize(4);           // 点大小
  connect(m_scatterSeries2_1, &QScatterSeries::hovered, this,
          &MainWindow::slotPointHoverd);  // 用于鼠标移动到点上显示数值
  m_valueLabel = new QLabel(this);
  m_valueLabel->adjustSize();
  m_valueLabel->setHidden(true);

  chartDay->addSeries(series2);
  chartDay->addSeries(m_scatterSeries2);
  chartDay->addSeries(m_scatterSeries2_1);

  // chartMonth->createDefaultAxes();
  axisX = new QBarCategoryAxis();
  chartMonth->addAxis(axisX, Qt::AlignBottom);
  barSeries->attachAxis(axisX);

  axisY = new QValueAxis();
  chartMonth->addAxis(axisY, Qt::AlignLeft);
  barSeries->attachAxis(axisY);

  // chartDay->createDefaultAxes();
  axisX2 = new QValueAxis();
  chartDay->addAxis(axisX2, Qt::AlignBottom);
  series2->attachAxis(axisX2);

  axisY2 = new QValueAxis();
  chartDay->addAxis(axisY2, Qt::AlignLeft);
  series2->attachAxis(axisY2);

  m_scatterSeries2->attachAxis(axisX2);
  m_scatterSeries2->attachAxis(axisY2);
  m_scatterSeries2_1->attachAxis(axisX2);
  m_scatterSeries2_1->attachAxis(axisY2);
}

void MainWindow::slotPointHoverd(const QPointF &point, bool state) {
  if (state) {
    m_valueLabel->setText(QString::asprintf("%1.0f", point.y()));

    QPoint curPos = mapFromGlobal(QCursor::pos());
    m_valueLabel->move(curPos.x() - m_valueLabel->width() / 2,
                       curPos.y() - m_valueLabel->height() * 1.5);  // 移动数值

    m_valueLabel->show();
  } else
    m_valueLabel->hide();
}

void MainWindow::init_TotalData() {
  int count = ui->tabWidget->tabBar()->count();
  for (int i = 0; i < count; i++) {
    ui->tabWidget->removeTab(0);
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
    QTreeWidget *tw = init_TreeWidget(name);

    QString tabText = RegTab
                          .value("TabName" + QString::number(i),
                                 tr("Tab") + QString::number(i + 1))
                          .toString();
    ui->tabWidget->addTab(tw, tabText);

    addItem(tabText, "", "", "", 0);

    RegTab.setValue("twName" + QString::number(i), name);
  }

  if (TabCount == 0) {
    QTreeWidget *tw = init_TreeWidget("20220303_101010_1");

    QString tabText = tr("Tab") + " " + QString::number(1);
    ui->tabWidget->addTab(tw, tabText);
    addItem(tabText, "", "", "", 0);

    ui->tabWidget->setTabToolTip(0, "");
  }

  m_EditRecord->init_MyCategory();

  currentTabIndex = RegTab.value("CurrentIndex").toInt();
  ui->tabWidget->setCurrentIndex(currentTabIndex);
  setCurrentIndex(currentTabIndex);
  QTreeWidget *twCur = (QTreeWidget *)tabData->currentWidget();
  readData(twCur);
  ui->actionImport_Data->setEnabled(false);
  ui->actionExport_Data->setEnabled(false);
  ui->actionDel_Tab->setEnabled(false);
  ui->actionAdd_Tab->setEnabled(false);
  ui->actionView_App_Data->setEnabled(false);

  if (!initMain) {
    ui->progBar->setHidden(false);
    ui->progBar->setMaximum(0);
  }

  m_ReadTWThread->start();
}

void MainWindow::readDataInThread(int ExceptIndex) {
  int count = tabData->tabBar()->count();
  for (int i = 0; i < count; i++) {
    if (i != ExceptIndex) {
      QTreeWidget *tw = (QTreeWidget *)tabData->widget(i);
      readData(tw);
    }
  }
}

void MainWindow::timerUpdate() {
  if (QTime::currentTime().toString("hh-mm-ss") == "00-30-00") {
    m_Preferences->isFontChange = true;
    this->close();
  }
}

void MainWindow::execDeskShortcut() {
  m_ReceiveShare->closeAllChildWindows();
  on_ExecShortcut();
}

void MainWindow::on_ExecShortcut() {
  keyType = m_Method->getKeyType();
  if (keyType == "todo") m_Todo->NewTodo();
  if (keyType == "note") ui->btnNotes->click();
  ;
  if (keyType == "reader") m_Reader->ContinueReading();
  if (keyType == "add") ui->btnAdd->click();
  if (keyType == "exercise") {
    QTimer::singleShot(100, this, [this]() { ui->btnSteps->click(); });
  }
  if (keyType == "defaultopen") {
#ifdef Q_OS_ANDROID

    JavaNotify_9();

#endif
  }
}

void MainWindow::on_ReceiveShare() {
  QString shareDone = m_ReceiveShare->getShareDone();
  if (shareDone == "false") {
    m_ReceiveShare->setShareDone("true");
    m_ReceiveShare->goReceiveShare();
  }
}

void MainWindow::on_timerSyncData() {
  qDebug() << ".......Sync.......";
  timerSyncData->stop();
  on_btnSync_clicked();
}

void MainWindow::startSyncData() {
  if (initMain) return;
  timerSyncData->start(10000);
}

MainWindow::~MainWindow() {
  delete ui;
  mySaveThread->quit();
  mySaveThread->wait();

  myReadThread->quit();
  myReadThread->wait();

  m_ReadTWThread->quit();
  m_ReadTWThread->wait();

  myReadEBookThread->quit();
  myReadEBookThread->wait();

  delete iniPreferences;
}

void MainWindow::startSave(QString str_type) {
  if (!isSaveEnd) {
    isBreak = true;
    mySaveThread->quit();
    mySaveThread->wait();

    while (!isSaveEnd)
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
  }
  if (isSaveEnd) {
    isBreak = false;
    SaveType = str_type;

    ui->progBar->setHidden(false);
    ui->progBar->setMaximum(0);

    mySaveThread->start();
  }
}

void MainWindow::startRead(QString Date) {
  if (!isSaveEnd || loading) return;

  readDate = Date;
  if (!isReadEnd) {
    isBreak = true;
    myReadThread->quit();
    myReadThread->wait();

    while (!isReadEnd)
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
  }

  if (isReadEnd) {
    isBreak = false;
    myReadThread->start();
    if (ui->rbSteps->isChecked()) ui->rbFreq->click();
  }
}

void MainWindow::add_Data(QTreeWidget *tw, QString strTime, QString strAmount,
                          QString strDesc) {
  bool isYes = false;

  strDate = m_Method->setCurrentDateValue();

  int topc = tw->topLevelItemCount();
  for (int i = 0; i < topc; i++) {
    QString str = tw->topLevelItem(topc - 1 - i)->text(0) + " " +
                  tw->topLevelItem(topc - 1 - i)->text(3);
    if (getYMD(str) == getYMD(strDate)) {
      isYes = true;

      QTreeWidgetItem *topItem = tw->topLevelItem(topc - 1 - i);
      QTreeWidgetItem *item11 = new QTreeWidgetItem(topItem);
      item11->setText(0, strTime);
      if (strAmount == "")
        item11->setText(1, "");
      else
        item11->setText(1, QString("%1").arg(strAmount.toDouble(), 0, 'f', 2));

      item11->setText(2, strDesc);
      item11->setText(3, ui->editDetails->toPlainText().trimmed());

      int childCount = topItem->childCount();

      topItem->setTextAlignment(1, Qt::AlignHCenter | Qt::AlignVCenter);
      topItem->setTextAlignment(2, Qt::AlignRight | Qt::AlignVCenter);
      item11->setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);

      // Amount
      double amount = 0;
      for (int m = 0; m < childCount; m++) {
        QString str = topItem->child(m)->text(1);
        amount = amount + str.toDouble();
      }
      QString strAmount = QString("%1").arg(amount, 0, 'f', 2);
      topItem->setText(1, QString::number(childCount));
      if (strAmount == "0.00")
        topItem->setText(2, "");
      else
        topItem->setText(2, strAmount);

      break;
    } else
      break;
  }

  if (!isYes) {
    QTreeWidgetItem *topItem = new QTreeWidgetItem;

    QStringList lista = strDate.split(" ");
    if (lista.count() == 4) {
      QString a = lista.at(0) + " " + lista.at(1) + " " + lista.at(2);
      topItem->setText(0, a);
      topItem->setText(3, lista.at(3));
    }

    tw->addTopLevelItem(topItem);
    QTreeWidgetItem *item11 = new QTreeWidgetItem(topItem);
    item11->setText(0, strTime);
    if (strAmount == "")
      item11->setText(1, "");
    else
      item11->setText(1, QString("%1").arg(strAmount.toDouble(), 0, 'f', 2));
    item11->setText(2, strDesc);
    item11->setText(3, ui->editDetails->toPlainText().trimmed());

    topItem->setTextAlignment(1, Qt::AlignHCenter | Qt::AlignVCenter);
    topItem->setTextAlignment(2, Qt::AlignRight | Qt::AlignVCenter);
    item11->setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);

    //  Amount
    int child = topItem->childCount();
    double amount = 0;
    for (int m = 0; m < child; m++) {
      QString str = topItem->child(m)->text(1);
      amount = amount + str.toDouble();
    }

    QString strAmount = QString("%1").arg(amount, 0, 'f', 2);
    topItem->setText(1, QString::number(child));
    if (strAmount == "0.00")
      topItem->setText(2, "");
    else
      topItem->setText(2, strAmount);
  }

  int topCount = tw->topLevelItemCount();
  QTreeWidgetItem *topItem = tw->topLevelItem(topCount - 1);
  tw->setCurrentItem(topItem);
  sort_childItem(topItem->child(0));
  tw->setCurrentItem(topItem->child(topItem->childCount() - 1));

  isEditItem = true;
  reloadMain();
}

int MainWindow::calcStringPixelWidth(QString s_str, QFont font,
                                     int n_font_size) {
  font.setPointSize(n_font_size);
  QFontMetrics fm(font);
  return fm.horizontalAdvance(s_str);
}

int MainWindow::calcStringPixelHeight(QFont font, int n_font_size) {
  font.setPointSize(n_font_size);
  QFontMetrics fm(font);
  return fm.height();
}

bool MainWindow::del_Data(QTreeWidget *tw) {
  if (tw->topLevelItemCount() == 0) return false;

  bool isTodayData = false;
  isRemovedTopItem = false;

  strDate = m_Method->setCurrentDateValue();
  for (int i = 0; i < tw->topLevelItemCount(); i++) {
    QString str =
        tw->topLevelItem(i)->text(0) + " " + tw->topLevelItem(i)->text(3);
    if (getYMD(str) == getYMD(strDate)) {
      isTodayData = true;
      QTreeWidgetItem *topItem = tw->topLevelItem(i);
      int childCount = topItem->childCount();
      if (childCount > 0) {
        QString str = ui->tabWidget->tabText(ui->tabWidget->currentIndex());
        strTime = topItem->child(childCount - 1)->text(0);
        strAmount = topItem->child(childCount - 1)->text(1);
        strCategory = topItem->child(childCount - 1)->text(2);
        strDetails = topItem->child(childCount - 1)->text(3);
        QString str1 = tr("Time") + " : " + strTime + "\n" + tr("Amount") +
                       " : " + strAmount + "\n" + tr("Category") + " : " +
                       strCategory + "\n" + tr("Details") + " : " + strDetails +
                       "\n";

        QString strTip;
        if (isMoveEntry)
          strTip = tr("The last record will be moved.");
        else
          strTip = tr("The last record will be deleted.");
        ShowMessage *m_ShowMsg = new ShowMessage(this);
        if (!m_ShowMsg->showMsg(str, strTip + "\n\n" + str1, 2)) return false;

        strLatestModify = tr("Del Item") + " ( " + getTabText() + " ) ";

        topItem->removeChild(topItem->child(childCount - 1));
        topItem->setTextAlignment(1, Qt::AlignHCenter | Qt::AlignVCenter);

        // Amount
        double amount = 0;
        for (int m = 0; m < childCount - 1; m++) {
          QString str = topItem->child(m)->text(1);
          amount = amount + str.toDouble();
        }
        QString str_amount = QString("%1").arg(amount, 0, 'f', 2);
        if (str_amount == "0.00") str_amount = "";
        topItem->setText(1, QString::number(childCount - 1));
        topItem->setText(2, str_amount);

        if (topItem->childCount() == 0) {
          tw->takeTopLevelItem(tw->topLevelItemCount() - 1);
          isRemovedTopItem = true;
        }

        isDelItem = true;
        reloadMain();

        break;
      }
    }
  }

  if (!isTodayData) {
    QString str = ui->tabWidget->tabText(ui->tabWidget->currentIndex());

    QString strTip;
    if (isMoveEntry)
      strTip = tr("Only the current day's records can be moved.");
    else
      strTip = tr("Only the current day's records can be deleted.");
    ShowMessage *m_ShowMsg = new ShowMessage(this);
    m_ShowMsg->showMsg(str, strTip, 1);

    return false;
  }

  int topCount = tw->topLevelItemCount();
  if (topCount > 0) {
    QTreeWidgetItem *topItem = tw->topLevelItem(topCount - 1);
    tw->setCurrentItem(topItem);
  }

  isDelData = true;
  startSave("tab");
  return true;
}

void MainWindow::on_AddRecord() {
  isAdd = true;

  ui->lblTitleEditRecord->setText(tr("Add") + "  : " +
                                  tabData->tabText(tabData->currentIndex()));

  ui->hsH->setValue(QTime::currentTime().hour());
  ui->hsM->setValue(QTime::currentTime().minute());
  m_EditRecord->getTime(ui->hsH->value(), ui->hsM->value());

  ui->editDetails->clear();
  ui->editCategory->setText("");
  ui->editAmount->setText("");

  ui->frameMain->hide();
  ui->frameEditRecord->show();

  // tmeFlash->start(300);
}

void MainWindow::on_tmeFlash() {
  nFlashCount = nFlashCount + 1;
  if (nFlashCount % 2 == 0)
    ui->lblTitleEditRecord->setStyleSheet(m_Method->lblStyle0);
  else
    ui->lblTitleEditRecord->setStyleSheet(m_Method->lblStyle);
  if (nFlashCount == 3) {
    tmeFlash->stop();
    nFlashCount = 0;
  }
}

void MainWindow::saveTab() {
  // Tab
  QSettings Reg(iniDir + "tab.ini", QSettings::IniFormat);

  int TabCount = tabData->tabBar()->count();
  Reg.setValue("TabCount", TabCount);
  int CurrentIndex = tabData->currentIndex();
  Reg.setValue("CurrentIndex", CurrentIndex);
  for (int i = 0; i < TabCount; i++) {
    if (isBreak) break;
    Reg.setValue("TabName" + QString::number(i), tabData->tabText(i));

    QTreeWidget *tw = (QTreeWidget *)tabData->widget(i);
    Reg.setValue("twName" + QString::number(i), tw->objectName());
  }
}

void MainWindow::saveData(QTreeWidget *tw, int tabIndex) {
  Q_UNUSED(tabIndex);

  QString name = tw->objectName();
  QString strYear = QString::number(QDate::currentDate().year());
  QString iniName = strYear + "-" + name;
  QString ini_file = iniDir + iniName + ".ini";
  QSettings Reg(ini_file, QSettings::IniFormat);

  if (!QFile::exists(ini_file)) {
    Reg.setValue("/" + name + "/" + "CreatedTime",
                 QDateTime::currentDateTime().toString());
  }

  int count = tw->topLevelItemCount();
  int m_sn = 0;

  QString flag;
  QString group = Reg.childGroups().at(0);
  if (group.trimmed().length() == 0)
    flag = "/" + name + "/";
  else
    flag = "/" + group + "/";

  for (int i = 0; i < count; i++) {
    if (isBreak) break;
    int childCount = tw->topLevelItem(i)->childCount();
    QString topText3 = tw->topLevelItem(i)->text(3);
    if (childCount > 0 && topText3 == strYear) {
      Reg.setValue(flag + QString::number(m_sn + 1) + "-topDate",
                   tw->topLevelItem(i)->text(0));
      Reg.setValue(flag + QString::number(m_sn + 1) + "-topYear", topText3);
      Reg.setValue(flag + QString::number(m_sn + 1) + "-topFreq",
                   tw->topLevelItem(i)->text(1));
      Reg.setValue(flag + QString::number(m_sn + 1) + "-topAmount",
                   tw->topLevelItem(i)->text(2));

      Reg.setValue(flag + QString::number(m_sn + 1) + "-childCount",
                   childCount);
      for (int j = 0; j < childCount; j++) {
        if (isBreak) return;
        Reg.setValue(flag + QString::number(m_sn + 1) + "-childTime" +
                         QString::number(j),
                     tw->topLevelItem(i)->child(j)->text(0));
        Reg.setValue(flag + QString::number(m_sn + 1) + "-childAmount" +
                         QString::number(j),
                     tw->topLevelItem(i)->child(j)->text(1));
        Reg.setValue(flag + QString::number(m_sn + 1) + "-childDesc" +
                         QString::number(j),
                     tw->topLevelItem(i)->child(j)->text(2));
        Reg.setValue(flag + QString::number(m_sn + 1) + "-childDetails" +
                         QString::number(j),
                     tw->topLevelItem(i)->child(j)->text(3));
      }

      m_sn++;
    }

    Reg.setValue(flag + "TopCount", m_sn);
  }
}

void MainWindow::drawMonthChart() {
  listM.clear();
  listM = get_MonthList(strY, strM);
  CurrentYearMonth = strY + strM;
}

void MainWindow::drawDayChart() {
  QTreeWidget *tw = (QTreeWidget *)tabData->currentWidget();
  if (loading) return;
  PointList.clear();

  int topCount = tw->topLevelItemCount();
  if (topCount == 0) {
    return;
  }

  if (topCount > 0) {
    if (!tw->currentIndex().isValid()) {
      QTreeWidgetItem *topItem = tw->topLevelItem(topCount - 1);
      tw->setCurrentItem(topItem);
    }
  }

  QTreeWidgetItem *item = tw->currentItem();
  bool child;
  int childCount;
  if (item->parent() == NULL)
    child = false;
  else
    child = true;

  QString month;

  if (child) {
    childCount = item->parent()->childCount();
    parentItem = item->parent();
    month = get_Month(item->parent()->text(0));
  } else {
    childCount = item->childCount();
    parentItem = item;
    month = get_Month(item->text(0));
  }

  QList<double> dList;
  double x, y;
  QString str, str1;
  int step = 1;
  if (childCount > 500) step = 100;

  for (int i = 0; i < childCount; i = i + step) {
    if (isBreak) break;

    if (child) {
      str = item->parent()->child(i)->text(0);
      str1 = item->parent()->child(i)->text(1);
    } else {
      str = item->child(i)->text(0);
      str1 = item->child(i)->text(1);
    }
    QStringList l0 = str.split(".");
    if (l0.count() == 2) str = l0.at(1);
    QStringList list = str.split(":");
    int t = 0;
    if (list.count() == 3) {
      QString a, b, c;
      a = list.at(0);
      b = list.at(1);
      c = list.at(2);
      int a1, b1;
      a1 = a.toInt();
      b1 = b.toInt();
      t = a1 * 3600 + b1 * 60 + c.toInt();
    }
    x = (double)t / 3600;
    if (isrbFreq)
      y = i + 1;
    else {
      y = str1.toDouble();
      dList.append(y);
    }

    PointList.append(QPointF(x, y));
  }

  if (isrbFreq) {
    int a = chartMax;
    if (childCount > a)
      yMaxDay = childCount;
    else
      yMaxDay = a;
  } else {
    yMaxDay = *std::max_element(dList.begin(), dList.end());
  }
}

void MainWindow::readData(QTreeWidget *tw) {
  tw->clear();

  QStringList myTopStrList;

  int iniFileCount = QDate::currentDate().year() - 2025 + 1 + 1;
  QString name = tw->objectName();
  QString ini_file;
  for (int p = 0; p < iniFileCount; p++) {
    if (p == 0) {
      ini_file = iniDir + name + ".ini";
    } else {
      QString strYear = QString::number(2025 + p - 1);
      ini_file = iniDir + strYear + "-" + name + ".ini";
    }

    qDebug() << "read ini_file=" << ini_file;

    if (QFile::exists(ini_file)) {
      QSettings Reg(ini_file, QSettings::IniFormat);

      QString group = Reg.childGroups().at(0);

      int rowCount = Reg.value("/" + group + "/TopCount").toInt();
      for (int i = 0; i < rowCount; i++) {
        int childCount = Reg.value("/" + group + "/" + QString::number(i + 1) +
                                   "-childCount")
                             .toInt();

        // 不显示子项为0的数据
        if (childCount > 0) {
          QTreeWidgetItem *topItem = new QTreeWidgetItem;
          QString strD0 =
              Reg.value("/" + group + "/" + QString::number(i + 1) + "-topDate")
                  .toString();

          QStringList lista = strD0.split(" ");
          if (lista.count() == 4) {
            QString a0 = lista.at(0) + " " + lista.at(1) + " " + lista.at(2);
            topItem->setText(0, a0);
            topItem->setText(3, lista.at(3));
          } else {
            topItem->setText(0, strD0);
            QString year = Reg.value("/" + group + "/" +
                                     QString::number(i + 1) + "-topYear")
                               .toString();
            topItem->setText(3, year);
          }

          topItem->setTextAlignment(1, Qt::AlignHCenter | Qt::AlignVCenter);
          topItem->setTextAlignment(2, Qt::AlignRight | Qt::AlignVCenter);

          topItem->setText(1, Reg.value("/" + group + "/" +
                                        QString::number(i + 1) + "-topFreq")
                                  .toString());
          topItem->setText(2, Reg.value("/" + group + "/" +
                                        QString::number(i + 1) + "-topAmount")
                                  .toString());

          // 移除异项（时间相同，但频次处于累加的异常情况）
          int lastTopIndex = tw->topLevelItemCount() - 1;
          if (lastTopIndex >= 0) {
            QTreeWidgetItem *lastTopItem = tw->topLevelItem(lastTopIndex);
            // 时间相同但频次或金额不同
            if ((lastTopItem->text(0) == topItem->text(0)) &&
                (lastTopItem->text(2) != topItem->text(2) ||
                 lastTopItem->text(1) != topItem->text(1))) {
              tw->takeTopLevelItem(lastTopIndex);
            }
          }

          QString topStr = QString("%1|%2|%3|%4")
                               .arg(topItem->text(0), topItem->text(1),
                                    topItem->text(2), topItem->text(3));

          if (myTopStrList.contains(topStr)) {
            for (int x = 0; x < tw->topLevelItemCount(); x++) {
              QString top, top0, top1, top2, top3;
              top0 = tw->topLevelItem(x)->text(0);
              top1 = tw->topLevelItem(x)->text(1);
              top2 = tw->topLevelItem(x)->text(2);
              top3 = tw->topLevelItem(x)->text(3);
              top = QString("%1|%2|%3|%4").arg(top0, top1, top2, top3);
              if (top == topStr) {
                tw->takeTopLevelItem(x);
                tw->insertTopLevelItem(x, topItem);
                for (int j = 0; j < childCount; j++) {
                  QTreeWidgetItem *item11 = new QTreeWidgetItem(topItem);
                  item11->setText(
                      0, Reg.value("/" + group + "/" + QString::number(i + 1) +
                                   "-childTime" + QString::number(j))
                             .toString());
                  item11->setText(
                      1, Reg.value("/" + group + "/" + QString::number(i + 1) +
                                   "-childAmount" + QString::number(j))
                             .toString());
                  item11->setText(
                      2, Reg.value("/" + group + "/" + QString::number(i + 1) +
                                   "-childDesc" + QString::number(j))
                             .toString());
                  item11->setText(
                      3, Reg.value("/" + group + "/" + QString::number(i + 1) +
                                   "-childDetails" + QString::number(j))
                             .toString());

                  item11->setTextAlignment(1,
                                           Qt::AlignRight | Qt::AlignVCenter);
                }
                break;
              }
            }
          }

          if (!myTopStrList.contains(topStr)) {
            tw->addTopLevelItem(topItem);
            myTopStrList.append(topStr);

            for (int j = 0; j < childCount; j++) {
              QTreeWidgetItem *item11 = new QTreeWidgetItem(topItem);
              item11->setText(
                  0, Reg.value("/" + group + "/" + QString::number(i + 1) +
                               "-childTime" + QString::number(j))
                         .toString());
              item11->setText(
                  1, Reg.value("/" + group + "/" + QString::number(i + 1) +
                               "-childAmount" + QString::number(j))
                         .toString());
              item11->setText(
                  2, Reg.value("/" + group + "/" + QString::number(i + 1) +
                               "-childDesc" + QString::number(j))
                         .toString());
              item11->setText(
                  3, Reg.value("/" + group + "/" + QString::number(i + 1) +
                               "-childDetails" + QString::number(j))
                         .toString());

              item11->setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);
            }
          }
        }
      }
    }
  }
}

void MainWindow::get_Today(QTreeWidget *tw) {
  int count = tw->topLevelItemCount();
  if (count <= 0) {
    today = 0;
    return;
  }
  QString str0 = tw->topLevelItem(count - 1)->text(0);
  QString str1 = tw->topLevelItem(count - 1)->text(1);
  if (strDate == str0) {
    today = str1.toInt();

  } else
    today = 0;
}

void MainWindow::closeEvent(QCloseEvent *event) {
  if (ui->qwCata->isVisible()) {
    on_btnCatalogue_clicked();
    event->ignore();
    return;
  }

  if (ui->qwBookList->isVisible()) {
    on_btnBackBookList_clicked();
    event->ignore();
    return;
  }

  if (ui->frameReader->isVisible()) {
    on_btnBackReader_clicked();
    event->ignore();
    return;
  }

  if (!ui->frameImgView->isHidden()) {
    on_btnBackImg_clicked();
    event->ignore();
    return;
  }

  if (!ui->frameNoteRecycle->isHidden()) {
    on_btnBackNoteRecycle_clicked();
    event->ignore();
    return;
  }

  if (!ui->frameNotesSearchResult->isHidden()) {
    on_btnBack_NotesSearchResult_clicked();
    event->ignore();
    return;
  }

  if (!ui->frameNoteList->isHidden()) {
    on_btnBackNoteList_clicked();
    event->ignore();
    return;
  }

  if (!ui->frameNotes->isHidden()) {
    on_btnBackNotes_clicked();
    event->ignore();
    return;
  }

  if (!ui->frameTodoRecycle->isHidden()) {
    on_btnReturnRecycle_clicked();
    event->ignore();
    return;
  }

  if (!ui->frameTodo->isHidden()) {
    on_btnBackTodo_clicked();
    event->ignore();
    return;
  }

  if (!ui->frameBakList->isHidden()) {
    on_btnBackBakList_clicked();
    event->ignore();
    return;
  }

  if (!ui->frameOne->isHidden()) {
    on_btnBack_One_clicked();
    event->ignore();
    return;
  }

#ifdef Q_OS_ANDROID
  if (m_Preferences->isFontChange) {
    stopJavaTimer();
    event->accept();
    return;
  }
#else
  QSettings Reg(privateDir + "winpos.ini", QSettings::IniFormat);

  Reg.setValue("x", this->geometry().x());
  Reg.setValue("y", this->geometry().y());
  Reg.setValue("w", this->geometry().width());
  Reg.setValue("h", this->geometry().height());

  event->accept();
#endif
}

void MainWindow::setMini() {
#ifdef Q_OS_ANDROID

  QJniObject jo = QNativeInterface::QAndroidApplication::context();
  jo.callStaticMethod<void>("com.x/MyActivity", "setMini", "()V");

#endif
}

void MainWindow::resetWinPos() {
  QSettings Reg(privateDir + "winpos.ini", QSettings::IniFormat);

  int x, y, w, h;
  x = Reg.value("x").toInt();
  y = Reg.value("y").toInt();
  w = Reg.value("w").toInt();
  h = Reg.value("h").toInt();

  if (x < 0) x = 0;
  if (y < 0) y = 0;

  if (x >= 0 && y >= 0 && w > 0 && h > 0) {
    this->setGeometry(x, y, w, h);
  }
}

void MainWindow::init_Stats(QTreeWidget *tw) {
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

void MainWindow::initChartMonth() {
  if (loading) return;

  int count = PointList.count();
  if (count == 0) {
    return;
  }

  barSeries->clear();
  series->clear();
  m_scatterSeries->clear();
  bool isOne = true;

  QBarSet *setY = new QBarSet("Y");
  QStringList categories;

  for (int i = 0; i < count; i++) {
    if (PointList.at(i).y() != 1) isOne = false;
  }

  if (isOne) {
    series->clear();
    m_scatterSeries->clear();
    QList<QPointF> tempPointList;
    for (int i = 0; i < count; i++) {
      double y0 = 0.0;
      QString str = listM.at(i);
      QStringList list1 = str.split(".");
      if (list1.count() == 2) {
        QStringList list = list1.at(1).split(":");
        int t = 0;
        if (list.count() == 3) {
          QString a, b, c;
          a = list.at(0);
          b = list.at(1);
          c = list.at(2);
          int a1, b1;
          a1 = a.toInt();
          b1 = b.toInt();
          t = a1 * 3600 + b1 * 60 + c.toInt();
        }
        y0 = (double)t / 3600;
      }

      tempPointList.append(QPointF(PointList.at(i).x(), y0));
    }
    PointList.clear();
    PointList = tempPointList;
  }

  double maxValue = *std::max_element(doubleList.begin(), doubleList.end());
  double max;
  if (isrbFreq) {
    max = chartMax;
    if (maxValue >= max) {
      max = maxValue;
    }

  } else {
    max = 50.00;
    if (maxValue >= max) max = maxValue;
  }

  yMaxMonth = max;

  QList<double> dList, tempDList;
  for (int i = 0; i < PointList.count(); i++) {
    tempDList.append(PointList.at(i).y());
    categories.append(QString::number(PointList.at(i).x()));
  }
  for (int i = 0; i < max_day; i++) {
    dList.append(0);
  }
  for (int i = 0; i < categories.count(); i++) {
    for (int n = 0; n < max_day; n++) {
      if (categories.at(i) == QString::number(n + 1)) {
        dList.removeAt(n);
        dList.insert(n, PointList.at(i).y());
      }
    }
  }

  for (int i = 0; i < max_day; i++) setY->append(dList.at(i));
  categories.clear();
  for (int i = 0; i < max_day; i++) categories.append(QString::number(i + 1));
  barSeries->append(setY);
  axisX->setRange("", QString::number(max_day));
  axisX->append(categories);
  axisY->setRange(0, yMaxMonth);

  if (isOne) {
    axisY->setRange(0, 24);
    chartMonth->setTitle(CurrentYear + "  Y:" + tr("Time") +
                         "    X:" + tr("Days"));
  } else {
    axisY->setRange(0, yMaxMonth);
    if (ui->rbFreq->isChecked())
      chartMonth->setTitle(CurrentYear + "  Y:" + tr("Freq") +
                           "    X:" + tr("Days"));

    if (ui->rbAmount->isChecked())
      chartMonth->setTitle(CurrentYear + "  Y:" + tr("Amount") +
                           "    X:" + tr("Days"));
  }
}

void MainWindow::initChartDay() {
  if (loading) return;
  series2->clear();
  m_scatterSeries2->clear();
  m_scatterSeries2_1->clear();

  int count = PointList.count();
  if (count == 0) return;
  for (int i = 0; i < count; i++) {
    series2->append(PointList.at(i));
    m_scatterSeries2->append(PointList.at(i));
    m_scatterSeries2_1->append(PointList.at(i));
  }

  axisX2->setRange(0, 24);
  axisX2->setTickType(QValueAxis::TicksFixed);
  axisX2->setTickCount(7);

  axisY2->setRange(0, yMaxDay + 1);

  if (ui->rbFreq->isChecked())
    chartDay->setTitle(CurrentYear + "  Y:" + tr("Freq") +
                       "    X:" + tr("Time"));

  if (ui->rbAmount->isChecked())
    chartDay->setTitle(CurrentYear + "  Y:" + tr("Amount") +
                       "    X:" + tr("Time"));
}

void MainWindow::on_actionRename_triggered() {
  int index = ui->tabWidget->currentIndex();
  bool ok = false;

  QString text;

  if (m_RenameDlg != nullptr) delete m_RenameDlg;

  m_RenameDlg =
      m_Method->inputDialog(tr("Rename tab name : "), tr("Tab name : "),
                            ui->tabWidget->tabText(index));

  if (QDialog::Accepted == m_RenameDlg->exec()) {
    ok = true;
    text = m_RenameDlg->textValue();
    m_RenameDlg->close();
  } else {
    m_RenameDlg->close();
    return;
  }

  if (ok && !text.isEmpty()) {
    ui->tabWidget->setTabText(index, text);

    m_Method->modifyItemText0(mw_one->ui->qwMainTab, index, text);

    updateMainTab();

    saveTab();
  }

  strLatestModify = tr("Rename Tab");
}

void MainWindow::on_actionAdd_Tab_triggered() {
  int count = ui->tabWidget->tabBar()->count();
  QString twName = m_Notes->getDateTimeStr() + "_" + QString::number(count + 1);
  QString ini_file = iniDir + twName + ".ini";
  if (QFile(ini_file).exists()) QFile(ini_file).remove();

  QTreeWidget *tw = init_TreeWidget(twName);

  QString tabText = tr("Tab") + " " + QString::number(count + 1);
  ui->tabWidget->addTab(tw, tabText);
  ui->tabWidget->setCurrentIndex(count);

  addItem(tabText, "", "", "", 0);
  setCurrentIndex(count);

  ui->tabCharts->setTabText(0, tr("Month"));
  ui->tabCharts->setTabText(1, tr("Day"));

  ui->btnChartMonth->setText(tabChart->tabText(0));
  ui->btnChartDay->setText(tabChart->tabText(1));

  on_actionRename_triggered();
  reloadMain();

  saveTab();

  strLatestModify = tr("Add Tab") + " ( " + getTabText() + " ) ";
}

void MainWindow::on_actionDel_Tab_triggered() {
  int index = ui->tabWidget->currentIndex();
  if (index < 0) return;

  QString tab_name = ui->tabWidget->tabText(index);

  m_Method->m_widget = new QWidget(mw_one);
  ShowMessage *m_ShowMsg = new ShowMessage(this);
  if (!m_ShowMsg->showMsg("Knot",
                          tr("Whether to remove") + "  " + tab_name + " ? ", 2))
    return;

  strLatestModify = tr("Del Tab") + " ( " + tab_name + " ) ";

  QString date_time = m_Notes->getDateTimeStr();
  m_Method->saveRecycleTabName(date_time, tab_name);

  QTreeWidget *tw = (QTreeWidget *)tabData->currentWidget();
  QString twName = tw->objectName();

  int c_year = QDate::currentDate().year();
  int iniFileCount = c_year - 2025 + 1 + 1;
  for (int i = 0; i < iniFileCount; i++) {
    QString tab_file;
    if (i == 0)
      tab_file = iniDir + twName + ".ini";
    else {
      tab_file = iniDir + QString::number(2025 + i - 1) + "-" + twName + ".ini";
    }

    if (QFile::exists(tab_file)) {
      QFile::copy(tab_file, iniDir + "recycle_name" + "_" + date_time + "-" +
                                QString::number(i) + ".ini");
      QFile file(tab_file);
      file.remove();
    }
  }

  int TabCount = ui->tabWidget->tabBar()->count();
  if (TabCount > 1) {
    ui->tabWidget->removeTab(index);
    delItem(index);
  }

  if (TabCount == 1) {
    QTreeWidget *tw = (QTreeWidget *)ui->tabWidget->currentWidget();
    tw->clear();
    tabData->setTabText(0, tr("Tab") + " 1");

    clearAll();
    addItem(tabData->tabText(0), "", "", "", 0);

    ui->tabWidget->setTabToolTip(0, "");

    reloadMain();
  }

  saveTab();
}

QTreeWidget *MainWindow::init_TreeWidget(QString name) {
  QTreeWidget *tw = new QTreeWidget(this);
  tw->setFixedHeight(0);
  tw->setObjectName(name);

  QString ini_file = iniDir + name + ".ini";
  if (!QFile::exists(ini_file)) {
    QSettings RegTab(ini_file, QSettings::IniFormat);

    RegTab.setValue("/" + name + "/" + "CreatedTime",
                    QDateTime::currentDateTime().toString());
  }

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
  tw->installEventFilter(this);
  tw->viewport()->installEventFilter(this);
  tw->setUniformRowHeights(true);  // 加快展开速度
  connect(tw, &QTreeWidget::itemClicked, this, &MainWindow::on_twItemClicked);
  connect(tw, &QTreeWidget::itemDoubleClicked, this,
          &MainWindow::on_twItemDoubleClicked);
  connect(tw, &QTreeWidget::itemPressed, [=]() {});

  connect(tw->verticalScrollBar(), &QScrollBar::valueChanged, [=]() {});

  // tw->setUniformRowHeights(false); //对速度可能有影响，数据量大时
  QScrollBar *SB = tw->verticalScrollBar();
  SB->setStyleSheet(m_Method->vsbarStyleSmall);
  tw->setStyleSheet(treeStyle);
  tw->setVerticalScrollMode(QTreeWidget::ScrollPerPixel);
  QScroller::grabGesture(tw, QScroller::LeftMouseButtonGesture);
  m_Method->setSCrollPro(tw);
  return tw;
}

void MainWindow::on_twItemClicked() {
  QTreeWidget *tw = (QTreeWidget *)ui->tabWidget->currentWidget();
  if (!tw->currentIndex().isValid()) return;

  QTreeWidgetItem *item = tw->currentItem();
  if (item->parent() == NULL && item->childCount() == 0) return;

  QTreeWidgetItem *pItem = NULL;

  QString stra;
  if (item->parent() == NULL) {
    CurrentYear = item->text(3);
    stra = item->text(0);
  } else {
    CurrentYear = item->parent()->text(3);
    stra = item->parent()->text(0);
  }
  tw->headerItem()->setText(0, "" + tr("Date") + "  " + CurrentYear);
  ui->tabCharts->setTabText(0, stra.split(" ").at(1));
  ui->tabCharts->setTabText(1, stra.split(" ").at(2));

  ui->btnChartMonth->setText(tabChart->tabText(0));
  ui->btnChartDay->setText(tabChart->tabText(1));

  // top item
  if (item->childCount() > 0) {
    pItem = item;
    QString sy = pItem->text(3);
    QString sm = pItem->text(0).split(" ").at(1);
    max_day = getMaxDay(sy, sm);

    isShowDetails = false;

    ui->lblStats->setText(strStats);
  }

  // child items
  if (item->childCount() == 0 && item->parent()->childCount() > 0) {
    pItem = item->parent();
    QString sy = pItem->text(3);
    QString sm = pItem->text(0).split(" ").at(1);
    max_day = getMaxDay(sy, sm);
  }

  if (tabChart->currentIndex() == 0) {
    QString str = stra + " " + CurrentYear;
    QString strYearMonth = get_Year(str) + get_Month(str);
    if (!isTabChanged) {
      if (strYearMonth == CurrentYearMonth) return;
    } else
      isTabChanged = false;
    startRead(str);
  }

  if (tabChart->currentIndex() == 1) {
    if (!isTabChanged) {
      if (parentItem != pItem) {
        startRead(strDate);
      }
    } else {
      startRead(strDate);
      isTabChanged = false;
    }
  }
}

void MainWindow::modify_Data() {
  QTreeWidget *tw = (QTreeWidget *)ui->tabWidget->currentWidget();
  QTreeWidgetItem *item = tw->currentItem();
  QTreeWidgetItem *topItem = item->parent();
  QString newtime = ui->lblTime->text().trimmed();
  if (item->childCount() == 0 && item->parent()->childCount() > 0) {
    item->setText(0, newtime);
    QString sa = ui->editAmount->text().trimmed();
    if (sa == "")
      item->setText(1, "");
    else
      item->setText(1, QString("%1").arg(sa.toDouble(), 0, 'f', 2));
    item->setText(2, ui->editCategory->text().trimmed());
    item->setText(3, ui->editDetails->toPlainText().trimmed());
    // Amount
    int child = item->parent()->childCount();
    double amount = 0;
    for (int m = 0; m < child; m++) {
      QString str = item->parent()->child(m)->text(1);
      amount = amount + str.toDouble();
    }
    QString strAmount = QString("%1").arg(amount, 0, 'f', 2);
    item->parent()->setTextAlignment(1, Qt::AlignHCenter | Qt::AlignVCenter);
    item->parent()->setText(1, QString::number(child));
    if (strAmount == "0.00")
      item->parent()->setText(2, "");
    else
      item->parent()->setText(2, strAmount);

    int childRow0 = tw->currentIndex().row();
    sort_childItem(item);

    int childRow1 = 0;
    for (int i = 0; i < topItem->childCount(); i++) {
      QTreeWidgetItem *childItem = topItem->child(i);

      QString time = childItem->text(0).split(".").at(1);
      time = time.trimmed();

      if (time == newtime) {
        childRow1 = i;
        break;
      }
    }

    int newrow;
    int row = m_Method->getCurrentIndexFromQW(ui->qwMainEvent);
    if (childRow0 - childRow1 == 0) newrow = row;
    if (childRow0 - childRow1 < 0) newrow = row + childRow1 - childRow0;
    if (childRow0 - childRow1 > 0) newrow = row - (childRow0 - childRow1);

    int maindateIndex = m_Method->getCurrentIndexFromQW(ui->qwMainDate);

    isEditItem = true;
    reloadMain();

    m_Method->setCurrentIndexFromQW(ui->qwMainDate, maindateIndex);
    isEditItem = true;
    m_Method->clickMainDate();
    m_Method->setCurrentIndexFromQW(ui->qwMainEvent, newrow);
  }
}

void MainWindow::sort_childItem(QTreeWidgetItem *item) {
  QStringList keys, list, keyTime, keysNew;
  int childCount = item->parent()->childCount();

  for (int i = 0; i < childCount; i++) {
    QString txt0 = item->parent()->child(i)->text(0);
    QStringList list0 = txt0.split(".");
    if (list0.count() == 2) {
      txt0 = list0.at(1);
      txt0 = txt0.trimmed();
    }
    QString txt1 = item->parent()->child(i)->text(1);
    QString txt2 = item->parent()->child(i)->text(2);
    QString txt3 = item->parent()->child(i)->text(3);
    keys.append(txt0 + "|===|" + txt1 + "|===|" + txt2 + "|===|" + txt3);
    keyTime.append(txt0);
  }

  std::sort(keyTime.begin(), keyTime.end(),
            [](const QString &s1, const QString &s2) { return s1 < s2; });

  for (int i = 0; i < keyTime.count(); i++) {
    QString time = keyTime.at(i);
    for (int n = 0; n < keys.count(); n++) {
      QString str1 = keys.at(n);
      QStringList l0 = str1.split("|===|");
      if (time == l0.at(0)) {
        keysNew.append(str1);
        break;
      }
    }
  }

  for (int i = 0; i < childCount; i++) {
    QTreeWidgetItem *childItem = item->parent()->child(i);
    QString str = keysNew.at(i);
    list.clear();
    list = str.split("|===|");
    if (list.count() == 4) {
      int number = i + 1;
      QString strChildCount = QString::number(childCount);
      QString strNum;
      strNum = QString("%1").arg(number, strChildCount.length(), 10,
                                 QLatin1Char('0'));
      childItem->setText(0, strNum + ". " + list.at(0).trimmed());
      childItem->setText(1, list.at(1).trimmed());
      childItem->setText(2, list.at(2).trimmed());
      childItem->setText(3, list.at(3).trimmed());
    }
  }
}

void MainWindow::on_twItemDoubleClicked() {
  m_EditRecord->monthSum();

  QTreeWidget *tw = (QTreeWidget *)ui->tabWidget->currentWidget();
  QTreeWidgetItem *item = tw->currentItem();
  if (item->childCount() == 0 && item->parent()->childCount() > 0) {
    if (item->parent()->text(3).toInt() != QDate::currentDate().year()) {
      ShowMessage *msg = new ShowMessage(this);
      msg->showMsg("Knot",
                   tr("Only the data of the current year can be modified."), 1);
      return;
    }

    QString t = item->text(0);
    QStringList l0 = t.split(".");
    if (l0.count() == 2) t = l0.at(1);
    QStringList list = t.split(":");
    QString sh, sm, ss;
    if (list.count() == 3) {
      sh = list.at(0);
      sm = list.at(1);
      ss = list.at(2);
    }
    ui->lblTitleEditRecord->setText(tr("Modify") + "  : " +
                                    tabData->tabText(tabData->currentIndex()));

    ui->hsH->setValue(sh.toInt());
    ui->hsM->setValue(sm.toInt());

    ui->lblTime->setText(t.trimmed());

    QString str = item->text(1);
    if (str == "0.00")
      ui->editAmount->setText("");
    else
      ui->editAmount->setText(str);

    ui->editCategory->setText(item->text(2));
    ui->editDetails->setText(item->text(3));
    ui->f_Number->setFocus();

    isAdd = false;
    ui->frameMain->hide();
    ui->frameEditRecord->show();
  }

  if (item == tw->topLevelItem(tw->topLevelItemCount() - 1)) {
    if (item->childCount() > 0) {
    }
  }
}

void MainWindow::clickMainTab() {
  int index = getCurrentIndex();
  tabData->setCurrentIndex(index);
}

void MainWindow::on_tabWidget_currentChanged(int index) {
  int count = ui->tabWidget->tabBar()->count();

  if (isSlide || loading || count <= 0) {
    return;
  }

  QTreeWidget *tw = (QTreeWidget *)tabData->widget(index);
  tw->setFocus();

  if (!loading) {
    QSettings Reg(iniDir + "tab.ini", QSettings::IniFormat);

    Reg.setValue("CurrentIndex", index);
  }

  reloadMain();

  series->clear();
  m_scatterSeries->clear();
  barSeries->clear();

  series2->clear();
  m_scatterSeries2->clear();
  m_scatterSeries2_1->clear();

  isTabChanged = true;

  m_Method->clickMainDateData();
}

void MainWindow::on_btnModifyRecord_clicked() {
  m_Method->reeditMainEventData();
}

bool MainWindow::eventFilter(QObject *watch, QEvent *evn) {
  if (loading) return QWidget::eventFilter(watch, evn);

  QMouseEvent *event = static_cast<QMouseEvent *>(evn);  // 将之转换为鼠标事件
  QTreeWidget *tw = (QTreeWidget *)ui->tabWidget->currentWidget();

  if (evn->type() == QEvent::ToolTip) {
    QToolTip::hideText();
    evn->ignore();
    return true;
  }

  if (watch == ui->lblStats) {
    if (event->type() == QEvent::MouseButtonDblClick) {
      on_btnSelTab_clicked();
      return true;
    }
  }

  if (watch == ui->lblTitleEditRecord) {
    if (event->type() == QEvent::MouseButtonPress) {
      QString title = ui->lblTitleEditRecord->text();
      title = title.mid(0, 4);
      if (!title.contains(tr("Add"))) return true;

      ui->btnTabMoveDown->hide();
      ui->btnTabMoveUp->hide();

      m_EditRecord->saveCurrentValue();
      on_btnBackEditRecord_clicked();
      on_btnSelTab_clicked();
      return true;
    }
  }

  if (watch == ui->lblNoteName) {
    if (event->type() == QEvent::MouseButtonPress) {
      on_btnNotesList_clicked();
      return true;
    }
  }

  if (isAndroid)
    m_Reader->eventFilterReaderAndroid(watch, evn);
  else
    m_Reader->eventFilterReader(watch, evn);

  m_Notes->eventFilterQwNote(watch, evn);

  if (watch == ui->textBrowser->viewport()) {
    if (event->type() == QEvent::MouseButtonPress) {
      isMousePress = true;
    }

    if (event->type() == QEvent::MouseButtonRelease) {
      isMousePress = false;

      QString str = ui->textBrowser->textCursor().selectedText().trimmed();
      if (str == "") {
        mydlgSetText->close();
      } else {
        int y1;
        int a = 30;
        if (event->globalPosition().y() - a - mydlgSetText->height() >= 0)
          y1 = event->globalPosition().y() - a - mydlgSetText->height();
        else
          y1 = event->globalPosition().y() + a;

        mydlgSetText->setFixedWidth(mw_one->width() - 4);
        mydlgSetText->init(
            geometry().x() + (mw_one->width() - mydlgSetText->width()) / 2, y1,
            mydlgSetText->width(), mydlgSetText->height());
      }
    }

    if (event->type() == QEvent::MouseMove) {
      if (isMousePress) {
        QString str = ui->textBrowser->textCursor().selectedText().trimmed();
        if (str != "") {
          int y1;
          int a = 30;
          if (event->globalPosition().y() - a - mydlgSetText->height() >= 0)
            y1 = event->globalPosition().y() - a - mydlgSetText->height();
          else
            y1 = event->globalPosition().y() + a;

          mydlgSetText->setFixedWidth(mw_one->width() - 4);
          mydlgSetText->init(
              geometry().x() + (mw_one->width() - mydlgSetText->width()) / 2,
              y1, mydlgSetText->width(), mydlgSetText->height());
        }
      }
    }

    if (event->type() == QEvent::MouseButtonDblClick) {
      QTextCursor cursor = ui->textBrowser->textCursor();
      cursor.setPosition(cursor.anchor());
      ui->textBrowser->setTextCursor(cursor);

      return true;
    }
  }

  if (watch == chartview || watch == chartview1) {
    if (event->type() == QEvent::MouseButtonDblClick) {
      on_btnChart_clicked();
    }
  }

  if (watch == ui->tabWidget->tabBar()) {
    if (!isReadTWEnd) return QWidget::eventFilter(watch, evn);
    if (event->type() == QEvent::MouseButtonPress) {
    }
    if (event->type() == QEvent::MouseButtonRelease) {
      bool move = false;
      for (int i = 0; i < ui->tabWidget->tabBar()->count(); i++) {
        QTreeWidget *tw = (QTreeWidget *)ui->tabWidget->widget(i);
        QString name = tw->objectName();
        if (name != "tab" + QString::number(i + 1)) {
          move = true;
        }
      }
      if (move) {
        qDebug() << "ok save";
        saveTab();
      }
    }
  }

  if (watch == ui->lblStats) {
    static int press_x;
    static int press_y;
    static int relea_x;
    static int relea_y;
    int index = ui->tabWidget->currentIndex();
    int count = ui->tabWidget->tabBar()->count();

    if (event->type() == QEvent::MouseButtonPress) {
      press_x = event->globalPosition().x();
      press_y = event->globalPosition().y();
      x = 0;
      y = 0;
      w = tw->width();
      h = tw->height();
    }

    if (event->type() == QEvent::MouseButtonRelease) {
      relea_x = event->globalPosition().x();
      relea_y = event->globalPosition().y();
    }

    // Right
    if ((relea_x - press_x) > 75 &&
        event->type() == QEvent::MouseButtonRelease &&
        qAbs(relea_y - press_y) < 35) {
      int current_page = ui->tabWidget->currentIndex();
      if (current_page < count - 1) {
        isSlide = true;

        QPropertyAnimation *animation1 =

            new QPropertyAnimation(ui->tabWidget->currentWidget(), "geometry");
        animation1->setDuration(350);
        animation1->setStartValue(QRect(x, y, w, h));
        animation1->setEndValue(QRect(w * 2, y, w, h));

        ui->tabWidget->setCurrentIndex(current_page + 1);

        QPropertyAnimation *animation2 =
            new QPropertyAnimation(ui->tabWidget->currentWidget(), "geometry");
        animation2->setDuration(350);
        animation2->setStartValue(QRect(-w * 2, y, w, h));
        animation2->setEndValue(QRect(x, y, w, h));

        QParallelAnimationGroup *group = new QParallelAnimationGroup;
        group->addAnimation(animation1);
        group->addAnimation(animation2);
        group->start();
        QElapsedTimer t;
        t.start();
        while (t.elapsed() < 600) {
          QCoreApplication::processEvents();
        }

        isSlide = false;

        on_tabWidget_currentChanged(tabData->currentIndex());
      }
    }

    // Left
    if ((press_x - relea_x) > 75 &&
        event->type() == QEvent::MouseButtonRelease &&
        qAbs(relea_y - press_y) < 35 && index > 0) {
      int current_page = ui->tabWidget->currentIndex();
      if (current_page >= 0) {
        isSlide = true;

        QPropertyAnimation *animation1 =

            new QPropertyAnimation(ui->tabWidget->currentWidget(), "geometry");
        animation1->setDuration(350);
        animation1->setStartValue(QRect(x, y, w, h));
        animation1->setEndValue(QRect(-w, y, w, h));

        ui->tabWidget->setCurrentIndex(current_page - 1);

        QPropertyAnimation *animation2 =
            new QPropertyAnimation(ui->tabWidget->currentWidget(), "geometry");
        animation2->setDuration(350);
        animation2->setStartValue(QRect(w * 2, y, w, h));
        animation2->setEndValue(QRect(x, y, w, h));

        QParallelAnimationGroup *group = new QParallelAnimationGroup;
        group->addAnimation(animation1);
        group->addAnimation(animation2);
        group->start();
        QElapsedTimer t;
        t.start();
        while (t.elapsed() < 500) {
          QCoreApplication::processEvents();
        }

        isSlide = false;

        on_tabWidget_currentChanged(tabData->currentIndex());
      }
    }
  }

  if (evn->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(evn);

    if (watch == ui->editSearchText && keyEvent->key() == Qt::Key_Return) {
      on_btnStartSearch_clicked();
      return true;
    }

    if (keyEvent->key() == Qt::Key_Escape) {
      if (ui->frameReader->isVisible()) on_btnBackReader_clicked();
      return true;
    }
  }

  return QWidget::eventFilter(watch, evn);
}

void MainWindow::clearWidgetFocus() {
  // InputMethodReset::instance().fullReset();
  if (QWidget *focused = focusWidget()) {
    focused->clearFocus();
  }
}

void MainWindow::hideEvent(QHideEvent *event) { QWidget::hideEvent(event); }

void MainWindow::on_actionExport_Data_triggered() {
  if (!isSaveEnd) return;

  isUpData = false;
  showProgress();

  myBakDataThread->start();
}

bool MainWindow::bakData() {
  errorInfo = "";
  m_NotesList->clearFiles();

  // set zip filename
  QString pass = encPassword;
  if (pass.length() > 0) pass = "_encrypt";
  QString str = m_Notes->getDateTimeStr();
  zipfile = bakfileDir + str + pass + "_Knot.zip";

  if (isUpData) {
    zipfile = bakfileDir + "memo.zip";
    QFile::remove(zipfile);
  }

  bool isZipResult = false;
  isZipResult = m_Method->compressDirectory(zipfile, iniDir, encPassword);
  if (isZipResult == false) {
    errorInfo = tr("An error occurred while compressing the file.");
    return false;
  }

  // enc data
  QString enc_file = m_Method->useEnc(zipfile);
  if (enc_file != "") zipfile = enc_file;

  return true;
}

void MainWindow::on_actionShareFile() {
  QString path = "/storage/emulated/0/";
  QString file =
      QFileDialog::getOpenFileName(this, tr("KnotBak"), path, tr("File (*.*)"));

  if (QFile::exists(file)) {
#ifdef Q_OS_ANDROID
    file = m_Method->getRealPathFile(file);
#endif
    m_ReceiveShare->shareImage(tr("Share to"), file, "*/*");
  }
}

void MainWindow::on_actionImport_Data_triggered() {
  if (!isSaveEnd) return;

  zipfile = "";
#ifdef Q_OS_ANDROID
  QString path = "/storage/emulated/0/KnotBak/";
  zipfile = QFileDialog::getOpenFileName(this, tr("KnotBak"), path,
                                         tr("Zip File (*.*)"));
#else
  zipfile = QFileDialog::getOpenFileName(this, tr("KnotBak"), "",
                                         tr("Zip File (*.zip);;All(*.*)"));
#endif

  if (!zipfile.isNull()) {
    m_Method->m_widget = new QWidget(mw_one);
    ShowMessage *m_ShowMsg = new ShowMessage(this);
    if (!m_ShowMsg->showMsg("Kont",
                            tr("Import this data?") + "\n" +
                                mw_one->m_Reader->getUriRealPath(zipfile),
                            2)) {
      isZipOK = false;
      return;
    }
  }

  showProgress();

  isMenuImport = true;
  isDownData = false;

  myImportDataThread->start();
}

bool MainWindow::importBakData(QString fileName) {
  if (fileName.isNull()) return false;

  deleteDirfile(privateDir + "gps");
  m_Reader->copyDirectoryFiles(iniDir + "memo/gps", privateDir + "gps", true);

  QString zipPath = bakfileDir + "memo.zip";
  if (fileName != zipPath) {
    QFile::remove(zipPath);
    QFile::copy(fileName, zipPath);
  }

  QString dec_file;
  if (isEncrypt) {
    dec_file = zipPath + ".dec";
    bool result = false;
    result = m_Method->decryptFile(zipPath, dec_file, encPassword);

    while (result == false && isPasswordError == false)
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
  } else
    dec_file = zipPath;

  // mw_one->m_Notes->unzip(zipPath);

  deleteDirfile(bakfileDir + "KnotData");
  bool unzipResult = false;
  unzipResult =
      m_Method->decompressWithPassword(dec_file, bakfileDir, encPassword);

  while (unzipResult == false && isPasswordError == false)
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

  if (isPasswordError) {
    return false;
  }

  isZipOK = true;

  QString bakFileFrom;
  QSettings Reg(bakfileDir + "KnotData/osflag.ini", QSettings::IniFormat);
  bakFileFrom = Reg.value("os", "mobile").toString();
  qDebug() << "bakFileFrom=" << bakFileFrom;

  if (bakFileFrom == "desktop") {
    deleteDirfile(iniDir + "memo");
    QFile::remove(iniDir + "todo.ini");
    QFile::remove(iniDir + "mainnotes.ini");

    m_Reader->copyDirectoryFiles(bakfileDir + "KnotData/memo", iniDir + "memo",
                                 true);
    QFile::copy(bakfileDir + "KnotData/todo.ini", iniDir + "todo.ini");
    QFile::copy(bakfileDir + "KnotData/mainnotes.ini",
                iniDir + "mainnotes.ini");

    deleteDirfile(iniDir + "memo/gps");
    m_Reader->copyDirectoryFiles(bakfileDir + "KnotData/memo/gps",
                                 iniDir + "memo/gps", true);
  }

  if (bakFileFrom == "mobile") {
    deleteDirfile(iniDir);
    m_Reader->copyDirectoryFiles(bakfileDir + "KnotData", iniDir, true);
  }

  return true;
}

bool MainWindow::copyFileToPath(QString sourceDir, QString toDir,
                                bool coverFileIfExist) {
  toDir.replace("\\", "/");
  if (sourceDir == toDir) {
    return true;
  }
  if (!QFile::exists(sourceDir)) {
    return false;
  }
  QDir *createfile = new QDir;
  bool exist = createfile->exists(toDir);
  if (exist) {
    if (coverFileIfExist) {
      createfile->remove(toDir);
    }
  }  // end if

  if (!QFile::copy(sourceDir, toDir)) {
    return false;
  }
  return true;
}

void MainWindow::showProgress() {
  dlgProg = new QDialog(this);
  dlgProg = m_Method->getProgBar();

  if (!initMain) dlgProg->show();
}

void MainWindow::closeProgress() {
  if (!initMain) {
    if (dlgProg == nullptr) return;

    dlgProg->close();
    delete dlgProg;
    dlgProg = nullptr;
  }
}

int MainWindow::get_Day(QString date) {
  QStringList list = date.split(" ");
  if (list.count() == 4) {
    QString strDay = list.at(2);
    return strDay.toInt();
  }
  return 0;
}

QString MainWindow::get_Month(QString date) {
  QStringList list = date.split(" ");
  if (list.count() > 1) {
    QString str = list.at(1);
    return str;
  }
  return "";
}

QString MainWindow::get_Year(QString date) {
  QStringList list = date.split(" ");
  if (list.count() == 4) {
    QString str = list.at(3);
    return str;
  }
  return "";
}

QTreeWidget *MainWindow::get_tw(int tabIndex) {
  QTreeWidget *tw = (QTreeWidget *)tabData->widget(tabIndex);
  return tw;
}

void MainWindow::on_actionAbout() {
  QTextBrowser *textBrowser = new QTextBrowser;
  textBrowser->append("");
  textBrowser->append(appName + "  Ver: " + ver);

  textBrowser->append("");
  textBrowser->append("Launched: " + loginTime);
  textBrowser->append("");
  textBrowser->setHidden(true);

  m_AboutThis->ui->lblAbout->setText(textBrowser->toPlainText());
  m_AboutThis->ui->frameAbout->show();

  int x, y;
  if (!isAndroid) {
    m_AboutThis->setMaximumWidth(320);

    x = this->geometry().x() +
        (this->geometry().width() - m_AboutThis->width()) / 2;
    y = this->geometry().y() +
        (this->geometry().height() - m_AboutThis->height()) / 2;
  } else {
    m_AboutThis->setFixedWidth(this->width());
    m_AboutThis->setFixedHeight(this->height());
    x = this->geometry().x();
    y = this->geometry().y();
  }
  m_AboutThis->setGeometry(x, y, this->width(), this->height());

  m_AboutThis->show();
}

void MainWindow::on_btnFind_clicked() {
  ui->frameMain->hide();
  ui->frameSearch->show();
  ui->editSearchText->setFocus();
  ui->btnClearSearchText->setFixedHeight(ui->btnStartSearch->height());
}

QStringList MainWindow::get_MonthList(QString strY, QString strM) {
  QStringList listMonth;
  if (loading) return listMonth;
  // 格式：记录第一个子项的时间
  PointList.clear();
  doubleList.clear();

  QTreeWidget *tw = (QTreeWidget *)tabData->currentWidget();
  for (int i = 0; i < tw->topLevelItemCount(); i++) {
    if (isBreak) break;
    QTreeWidgetItem *topItem = tw->topLevelItem(i);
    QString str0 = topItem->text(0) + " " + topItem->text(3);
    QString y, m, d;
    y = get_Year(str0);
    m = get_Month(str0);
    d = QString::number(get_Day(str0));
    double x0 = d.toDouble();
    if (y == strY) {
      if (m == strM) {
        if (isrbFreq) {
          if (topItem->childCount() > 0)
            listMonth.append(
                topItem->child(0)->text(0));  // 记录第一个子项的时间

          double y0 = topItem->text(1).toDouble();
          doubleList.append(y0);

          PointList.append(QPointF(x0, y0));
        } else {
          double y0 = topItem->text(2).toDouble();
          doubleList.append(y0);

          PointList.append(QPointF(x0, y0));
        }
      }
    }
  }

  return listMonth;
}

void MainWindow::gotoMainItem(QTreeWidgetItem *item) {
  int count = getCount();
  for (int i = 0; i < count; i++) {
    QString text0 = getText0(i);
    QString text1 = getText1(i);
    QString text2 = getText2(i);
    if (item->text(0) == text0 && item->text(1) == text1 &&
        item->text(2) == text2) {
      gotoIndex(i);
      setCurrentIndex(i);
      break;
    }
  }
}

QString MainWindow::setLineEditQss(QLineEdit *txt, int radius, int borderWidth,
                                   const QString &normalColor,
                                   const QString &focusColor) {
  QStringList list;
  list.append(QString("QLineEdit{border-style:none;padding:3px;border-radius:%"
                      "1px;border:%2px solid %3;}")
                  .arg(radius)
                  .arg(borderWidth)
                  .arg(normalColor));
  list.append(QString("QLineEdit:focus{border:%1px solid %2;}")
                  .arg(borderWidth)
                  .arg(focusColor));

  QString qss = list.join("");
  txt->setStyleSheet(qss);
  return qss;
}

QString MainWindow::setComboBoxQss(QComboBox *txt, int radius, int borderWidth,
                                   const QString &normalColor,
                                   const QString &focusColor) {
  QStringList list;
  list.append(QString("QComboBox{border-style:none;padding:3px;border-radius:%"
                      "1px;border:%2px solid %3;}")
                  .arg(radius)
                  .arg(borderWidth)
                  .arg(normalColor));
  list.append(QString("QComboBox:focus{border:%1px solid %2;}")
                  .arg(borderWidth)
                  .arg(focusColor));
  list.append(
      QString("QComboBox::down-arrow{image:url(:/icon/"
              "add_bottom.png);width:10px;height:10px;right:2px;}"));
  list.append(QString(
      "QComboBox::drop-down{subcontrol-origin:padding;subcontrol-position:"
      "top "
      "right;width:15px;border-left-width:0px;border-left-style:solid;border-"
      "top-right-radius:3px;border-bottom-right-radius:3px;border-left-color:"
      "#"
      "B6B6B6;}"));
  list.append(QString("QComboBox::drop-down:on{top:1px;}"));

  QString qss = list.join("");
  txt->setStyleSheet(qss);
  return qss;
}

void MainWindow::on_actionFind_triggered() { on_btnFind_clicked(); }

void MainWindow::on_btnTodo_clicked() { m_Todo->openTodo(); }

void MainWindow::on_rbFreq_clicked() {
  tabChart->setTabEnabled(1, true);
  isrbFreq = true;
  CurrentYearMonth = "";
  parentItem = NULL;
  m_Method->clickMainDateData();
}

void MainWindow::on_rbAmount_clicked() {
  tabChart->setTabEnabled(1, true);
  isrbFreq = false;
  CurrentYearMonth = "";
  parentItem = NULL;
  m_Method->clickMainDateData();
}

void MainWindow::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);

  // 获取背景色
  QPalette pal = ui->btnFind->palette();
  QBrush brush = pal.window();
  int c_red = brush.color().red();

  if (c_red != red) {
    red = c_red;
    if (red < 55) {
      chartMonth->setTheme(QChart::ChartThemeDark);
      chartDay->setTheme(QChart::ChartThemeDark);
    }

    else {
      chartMonth->setTheme(QChart::ChartThemeLight);
      chartDay->setTheme(QChart::ChartThemeLight);
    }
  }
}

void MainWindow::on_actionReport_triggered() {
  if (isEBook || !isSaveEnd || !isReadEBookEnd) return;

  if (isReadEBookEnd) {
    m_Report->init();
    startInitReport();
  }
}

void MainWindow::startInitReport() {
  showProgress();

  isReport = true;
  myReadEBookThread->start();
}

void MainWindow::on_actionPreferences_triggered() {
  int x, y;
  if (isAndroid) {
    m_Preferences->setFixedWidth(this->width());
    m_Preferences->setFixedHeight(this->height());
    x = geometry().x();
    y = geometry().y();
  } else {
    x = geometry().x() + (width() - m_Preferences->width()) / 2;
    y = geometry().y() + (height() - m_Preferences->height()) / 2;
    m_Preferences->setFixedWidth(350);
  }

  if (y < 0) y = 50;

  m_Preferences->setGeometry(x, y, m_Preferences->width(), height());
  m_Preferences->setModal(true);
  m_Preferences->ui->sliderFontSize->setStyleSheet(ui->hsM->styleSheet());
  m_Preferences->ui->sliderFontSize->setValue(fontSize);
  m_Preferences->show();
  m_Preferences->initCheckStatus();
}

void MainWindow::on_tabCharts_currentChanged(int index) {
  if (ui->rbSteps->isChecked() || loading || index < 0) return;

  m_Method->clickMainDateData();
}

void MainWindow::on_btnSteps_clicked() { m_Steps->openStepsUI(); }

void MainWindow::changeEvent(QEvent *event) {
  if (event->type() == QEvent::WindowStateChange) {
  }
}

void MainWindow::on_rbSteps_clicked() {
  int count = m_Steps->getCount();
  if (count <= 0) return;

  tabChart->setCurrentIndex(0);
  tabChart->setTabEnabled(1, false);

  PointList.clear();
  doubleList.clear();

  QString sm = get_Month(QDate::currentDate().toString("ddd MM dd yyyy"));
  for (int i = 0; i < count; i++) {
    QString strD = m_Steps->getDate(i);
    if (sm == get_Month(strD)) {
      int day = get_Day(strD);
      int steps = m_Steps->getSteps(i);
      PointList.append(QPointF(day, steps));
      doubleList.append(steps);
    }
  }

  initChartMonth();
  chartMonth->setTitle("Y:" + tr("Steps") + "    X:" + tr("Days"));
}

QString MainWindow::secondsToTime(ulong totalTime) {
  // 输入为秒数则ss=1，输入为毫秒数则ss=1000
  qint64 ss = 1;
  qint64 mi = ss * 60;
  qint64 hh = mi * 60;
  qint64 dd = hh * 24;

  qint64 day = totalTime / dd;
  qint64 hour = (totalTime - day * dd) / hh;
  qint64 minute = (totalTime - day * dd - hour * hh) / mi;
  qint64 second = (totalTime - day * dd - hour * hh - minute * mi) / ss;

  QString hou = QString::number(hour, 10);
  QString min = QString::number(minute, 10);
  QString sec = QString::number(second, 10);

  hou = hou.length() == 1 ? QString("0%1").arg(hou) : hou;
  min = min.length() == 1 ? QString("0%1").arg(min) : min;
  sec = sec.length() == 1 ? QString("0%1").arg(sec) : sec;
  return hou + ":" + min + ":" + sec;
}

void MainWindow::on_btnNotes_clicked() { m_Notes->openNotes(); }

void MainWindow::initQW() {
  qmlRegisterType<File>("MyModel1", 1, 0, "File");
  qmlRegisterType<DocumentHandler>("MyModel2", 1, 0, "DocumentHandler");

  int f_size = 19;
  if (fontSize <= f_size) f_size = fontSize;
  ui->qwReport->rootContext()->setContextProperty("maxFontSize", f_size);
  ui->qwReportSub->rootContext()->setContextProperty("maxFontSize", f_size);

  ui->qwNotesTree->rootContext()->setContextProperty("fontSize", fontSize);
  ui->qwNotesTree->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/tree_main.qml")));

  ui->qwReader->rootContext()->setContextProperty("myW", this->width());
  ui->qwReader->rootContext()->setContextProperty("myH", this->height());
  ui->qwReader->rootContext()->setContextProperty("m_Reader", m_Reader);
  ui->qwReader->rootContext()->setContextProperty("myBackgroundColor",
                                                  "#FFFFFF");

  ui->qwCata->rootContext()->setContextProperty("m_Reader", m_Reader);
  ui->qwCata->setSource(QUrl(QStringLiteral("qrc:/src/qmlsrc/epub_cata.qml")));

  ui->qwBookmark->rootContext()->setContextProperty("m_Reader", m_Reader);
  ui->qwBookmark->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/bookmark.qml")));

  ui->qw_Img->rootContext()->setContextProperty("myW", this->width());
  ui->qw_Img->rootContext()->setContextProperty("myH", this->height());

  ui->qwTodo->rootContext()->setContextProperty("maxFontSize", f_size);
  ui->qwTodo->rootContext()->setContextProperty("isBtnVisible",
                                                QVariant(false));
  ui->qwTodo->rootContext()->setContextProperty("m_Todo", m_Todo);
  ui->qwTodo->rootContext()->setContextProperty("FontSize", fontSize);
  ui->qwTodo->setSource(QUrl(QStringLiteral("qrc:/src/qmlsrc/todo.qml")));

  ui->qwRecycle->rootContext()->setContextProperty("FontSize", fontSize);
  ui->qwRecycle->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/todorecycle.qml")));

  ui->qwSteps->setSource(QUrl(QStringLiteral("qrc:/src/qmlsrc/steps.qml")));
  ui->qwSteps->rootContext()->setContextProperty("maxFontSize", f_size);
  ui->qwSteps->rootContext()->setContextProperty("myW", this->width());
  ui->qwSteps->rootContext()->setContextProperty("text0", "");
  ui->qwSteps->rootContext()->setContextProperty("text1", "");
  ui->qwSteps->rootContext()->setContextProperty("text2", "");
  ui->qwSteps->rootContext()->setContextProperty("text3", "");

  ui->qwSpeed->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/Speedometer.qml")));

  ui->qwGpsList->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/gps_list.qml")));
  ui->qwGpsList->rootContext()->setContextProperty("myW", this->width());
  ui->qwGpsList->rootContext()->setContextProperty("m_Steps", m_Steps);

  ui->qwMap->setResizeMode(QQuickWidget::SizeRootObjectToView);
  ui->qwMap->setFocusPolicy(Qt::StrongFocus);  // 关键设置
  ui->qwMap->setClearColor(Qt::transparent);   // 避免渲染冲突
  ui->qwMap->setAttribute(Qt::WA_AcceptTouchEvents, true);
  ui->qwMap->setAttribute(Qt::WA_TouchPadAcceptSingleTouchEvents, true);
  ui->qwMap->setSource(QUrl(QStringLiteral("qrc:/src/qmlsrc/map.qml")));

  ui->qwReport->rootContext()->setContextProperty("m_Report", m_Report);
  ui->qwReport->setSource(QUrl(QStringLiteral("qrc:/src/qmlsrc/report.qml")));
  ui->qwReportSub->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/details.qml")));

  ui->qwSearch->rootContext()->setContextProperty("m_Method", m_Method);
  ui->qwSearch->setSource(QUrl(QStringLiteral("qrc:/src/qmlsrc/search.qml")));

  ui->qwBakList->rootContext()->setContextProperty("m_Method", m_Method);
  ui->qwBakList->setSource(QUrl(QStringLiteral("qrc:/src/qmlsrc/baklist.qml")));

  ui->qwViewCate->rootContext()->setContextProperty("m_Report", m_Report);
  ui->qwViewCate->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/viewcate.qml")));

  ui->qwTabRecycle->rootContext()->setContextProperty("m_Report", m_Report);
  ui->qwTabRecycle->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/tabrecycle.qml")));

  ui->qwNoteBook->rootContext()->setContextProperty("m_NotesList", m_NotesList);
  ui->qwNoteBook->rootContext()->setContextProperty("mw_one", mw_one);
  ui->qwNoteBook->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/notebook.qml")));

  if (isAndroid)
    ui->qwNoteList->rootContext()->setContextProperty("noteTimeFontSize", 12);
  else
    ui->qwNoteList->rootContext()->setContextProperty("noteTimeFontSize", 8);
  ui->qwNoteList->rootContext()->setContextProperty("m_NotesList", m_NotesList);
  ui->qwNoteList->rootContext()->setContextProperty("mw_one", mw_one);
  ui->qwNoteList->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/notelist.qml")));

  ui->qwNotesSearchResult->rootContext()->setContextProperty("fontSize",
                                                             fontSize);
  ui->qwNotesSearchResult->rootContext()->setContextProperty("m_NotesList",
                                                             m_NotesList);
  ui->qwNotesSearchResult->rootContext()->setContextProperty("mw_one", mw_one);
  ui->qwNotesSearchResult->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/SearchResults.qml")));

  ui->qwNoteRecycle->rootContext()->setContextProperty("m_Method", m_Method);
  ui->qwNoteRecycle->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/noterecycle.qml")));

  ui->qwMainTab->setFixedHeight(50);
  ui->qwMainTab->rootContext()->setContextProperty("maintabHeight",
                                                   ui->qwMainTab->height());
  ui->qwMainTab->rootContext()->setContextProperty("mw_one", mw_one);
  ui->qwMainTab->setSource(QUrl(QStringLiteral("qrc:/src/qmlsrc/maintab.qml")));

  ui->qwMainDate->rootContext()->setContextProperty("isAniEffects", true);
  ui->qwMainDate->rootContext()->setContextProperty("maindateWidth",
                                                    ui->qwMainDate->width());
  ui->qwMainDate->rootContext()->setContextProperty("m_Method", m_Method);
  ui->qwMainDate->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/maindate.qml")));

  ui->qwMainEvent->rootContext()->setContextProperty("fontSize", fontSize);
  ui->qwMainEvent->rootContext()->setContextProperty("isAniEffects", true);
  ui->qwMainEvent->rootContext()->setContextProperty("maineventWidth",
                                                     ui->qwMainEvent->width());
  ui->qwMainEvent->rootContext()->setContextProperty("m_Method", m_Method);
  ui->qwMainEvent->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/mainevent.qml")));

  ui->qwCategory->rootContext()->setContextProperty("m_Method", m_Method);
  ui->qwCategory->setSource(QUrl(QStringLiteral("qrc:/src/qmlsrc/type.qml")));

  ui->qwSelTab->rootContext()->setContextProperty("mw_one", mw_one);
  ui->qwSelTab->setSource(QUrl(QStringLiteral("qrc:/src/qmlsrc/seltab.qml")));

  ui->qwBookList->rootContext()->setContextProperty("fontSize", fontSize);
  ui->qwBookList->rootContext()->setContextProperty("m_Reader", m_Reader);
  ui->qwBookList->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/booklist.qml")));
}

void MainWindow::init_Theme() {
  // Get the background color to fit the dark mode
  QPalette pal = this->palette();
  QBrush brush = pal.window();
  red = brush.color().red();

  qDebug() << "red=" << red;

  ui->qwMainTab->rootContext()->setContextProperty("isDark", isDark);
  ui->qwMainDate->rootContext()->setContextProperty("isDark", isDark);
  ui->qwMainEvent->rootContext()->setContextProperty("isDark", isDark);
  ui->qwTodo->rootContext()->setContextProperty("isDark", isDark);
  ui->qwRecycle->rootContext()->setContextProperty("isDark", isDark);
  ui->qwNoteBook->rootContext()->setContextProperty("isDark", isDark);
  ui->qwNoteList->rootContext()->setContextProperty("isDark", isDark);

  ui->qwNotesSearchResult->rootContext()->setContextProperty("isDark", isDark);
  ui->qwSearch->rootContext()->setContextProperty("isDark", isDark);
  ui->qwBakList->rootContext()->setContextProperty("isDark", isDark);
  ui->qwViewCate->rootContext()->setContextProperty("isDark", isDark);
  ui->qwTabRecycle->rootContext()->setContextProperty("isDark", isDark);
  ui->qwNoteRecycle->rootContext()->setContextProperty("isDark", isDark);
  ui->qwCategory->rootContext()->setContextProperty("isDark", isDark);
  ui->qwSelTab->rootContext()->setContextProperty("isDark", isDark);
  ui->qwBookList->rootContext()->setContextProperty("isDark", isDark);
  ui->qwReportSub->rootContext()->setContextProperty("isDark", isDark);
  ui->qwSteps->rootContext()->setContextProperty("isDark", isDark);
  ui->qwGpsList->rootContext()->setContextProperty("isDark", isDark);
  ui->qwReport->rootContext()->setContextProperty("isDark", isDark);

  ui->qwCata->rootContext()->setContextProperty("isDark", isDark);
  ui->qwBookmark->rootContext()->setContextProperty("isDark", isDark);
  ui->qwReader->rootContext()->setContextProperty("isDark", isDark);

  if (!isDark) {
    ui->f_Menu->setStyleSheet("background-color: rgb(243,243,243);");
    ui->f_Btn->setStyleSheet("background-color: rgb(243,243,243);");
    ui->f_cw->setStyleSheet("background-color: rgb(243,243,243);");
    ui->f_charts->setStyleSheet("background-color: rgb(243,243,243);");

    chartMonth->setTheme(QChart::ChartThemeLight);
    chartDay->setTheme(QChart::ChartThemeLight);

    ui->btnAddTodo->setIcon(QIcon(":/res/plus_l.svg"));
    ui->btnClear->setIcon(QIcon(":/res/clear.png"));

    ui->btnModifyRecord->setIcon(QIcon(":/res/edit.svg"));
    ui->btnMove->setIcon(QIcon(":/res/move.svg"));

    ui->btnReader->setIcon(QIcon(":/res/reader.svg"));
    ui->btnTodo->setIcon(QIcon(":/res/todo.svg"));
    ui->btnSteps->setIcon(QIcon(":/res/steps.svg"));
    ui->btnNotes->setIcon(QIcon(":/res/note.svg"));
    ui->btnChart->setIcon(QIcon(":/res/chart.svg"));
    ui->btnFind->setIcon(QIcon(":/res/find.png"));
    ui->btnReport->setIcon(QIcon(":/res/report.svg"));
    ui->btnSelTab->setIcon(QIcon(":/res/tab.svg"));

    ui->btnMenu->setIcon(QIcon(":/res/mainmenu.svg"));
    ui->btnAdd->setIcon(QIcon(":/res/additem.svg"));
    ui->btnDel->setIcon(QIcon(":/res/delitem.svg"));
    ui->btnSync->setIcon(QIcon(":/res/upload.svg"));

    m_Method->setEditLightMode(ui->editTodo);

    ui->editDetails->setStyleSheet(ui->editTodo->styleSheet());

    ui->editTodo->verticalScrollBar()->setStyleSheet(
        m_Method->lightScrollbarStyle);
    ui->editDetails->verticalScrollBar()->setStyleSheet(
        m_Method->lightScrollbarStyle);

    chartMonth->setTheme(QChart::ChartThemeLight);
    chartDay->setTheme(QChart::ChartThemeLight);

  } else {
    ui->f_Menu->setStyleSheet("background-color: #19232D;");
    ui->f_Btn->setStyleSheet("background-color: #19232D;");
    ui->f_cw->setStyleSheet("background-color: #19232D;");
    ui->f_charts->setStyleSheet("background-color: #19232D;");

    chartMonth->setTheme(QChart::ChartThemeDark);
    chartDay->setTheme(QChart::ChartThemeDark);

    ui->btnAddTodo->setIcon(QIcon(":/res/plus_l.svg"));
    ui->btnClear->setIcon(QIcon(":/res/clear.png"));

    ui->btnReport->setIcon(QIcon(":/res/report_l.svg"));
    ui->btnFind->setIcon(QIcon(":/res/find_l.png"));
    ui->btnModifyRecord->setIcon(QIcon(":/res/edit_l.svg"));
    ui->btnMove->setIcon(QIcon(":/res/move_l.svg"));

    ui->btnReader->setIcon(QIcon(":/res/reader_l.svg"));
    ui->btnTodo->setIcon(QIcon(":/res/todo_l.png"));
    ui->btnSteps->setIcon(QIcon(":/res/steps_l.svg"));
    ui->btnNotes->setIcon(QIcon(":/res/note_l.svg"));
    ui->btnChart->setIcon(QIcon(":/res/chart_l.svg"));
    ui->btnSelTab->setIcon(QIcon(":/res/tab_l.svg"));

    ui->btnMenu->setIcon(QIcon(":/res/mainmenu_l.svg"));
    ui->btnAdd->setIcon(QIcon(":/res/additem_l.svg"));
    ui->btnDel->setIcon(QIcon(":/res/delitem_l.svg"));
    ui->btnSync->setIcon(QIcon(":/res/upload_l.svg"));

    m_Method->setEditDarkMode(ui->editTodo);

    ui->editDetails->setStyleSheet(ui->editTodo->styleSheet());

    ui->editTodo->verticalScrollBar()->setStyleSheet(
        m_Method->darkScrollbarStyle);
    ui->editDetails->verticalScrollBar()->setStyleSheet(
        m_Method->darkScrollbarStyle);

    chartMonth->setTheme(QChart::ChartThemeDark);
    chartDay->setTheme(QChart::ChartThemeDark);
  }

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

  m_EditRecord->on_editAmount_textChanged(ui->editAmount->text());
  m_EditRecord->on_editCategory_textChanged(ui->editCategory->text());
  m_EditRecord->on_editDetails_textChanged();

  // Todo
  m_Todo->changeTodoIcon(m_Todo->isToday);

  // Android
  m_Method->setDark(isDark);

  // Notes Editor
  m_Notes->init_md();

  // Chart
  QFont font1;
#ifdef Q_OS_ANDROID
  font1.setPointSize(10);
#else
  font1.setPointSize(10);
#endif
  font1.setBold(true);
  chartMonth->setTitleFont(font1);
  chartDay->setTitleFont(font1);
  axisX->setLabelsFont(font1);
  axisY->setLabelsFont(font1);
  axisY->setTickCount(yScale);
  axisX2->setLabelsFont(font1);
  axisY2->setLabelsFont(font1);
  axisY2->setTickCount(yScale);

  ui->lblNoteName->setStyleSheet("QLabel{background:lightyellow;color:black;}");

  init_ButtonStyle();
}

void MainWindow::init_Instance() {
  mw_one = this;
  CurrentYear = QString::number(QDate::currentDate().year());
  if (defaultFontFamily == "") defaultFontFamily = this->font().family();

  tabData = new QTabWidget;
  tabData = ui->tabWidget;

  tabChart = new QTabWidget;
  tabChart = ui->tabCharts;

  m_Method = new Method(this);
  myfile = new File();
  m_AboutThis = new AboutThis(this);
  m_Preferences = new Preferences(this);
  m_EditRecord = new EditRecord(this);
  m_Todo = new Todo(this);
  m_Report = new Report(this);
  m_Notes = new Notes(this);
  m_StepsOptions = new StepsOptions(this);
  m_Steps = new Steps(this);
  m_Reader = new Reader(this);
  m_TodoAlarm = new TodoAlarm(this);
  m_DateSelector = new DateSelector(this);
  m_CloudBackup = new CloudBackup;
  m_PageIndicator = new PageIndicator(this);
  m_PageIndicator->close();
  m_ReaderSet = new ReaderSet(this);
  mydlgSetText = new dlgSetText(this);
  m_NotesList = new NotesList(this);

  m_ReceiveShare = new ReceiveShare(this);

  if (m_Preferences->getDefaultFont() == "None")
    m_Preferences->setDefaultFont(this->font().family());

  m_Method->setOSFlag();
}

void MainWindow::init_UIWidget() {
  QFontMetrics fontMetrics(font());
  int nFontHeight = fontMetrics.height();
  int nHeight = nFontHeight * 1.5;
  ui->tabWidget->tabBar()->setFixedHeight(nHeight);
  ui->tabWidget->setStyleSheet(ui->tabCharts->styleSheet());
  ui->tabWidget->setFixedHeight(ui->tabWidget->tabBar()->height() + 0);
  if (nHeight <= 36) nHeight = 36;
  ui->qwMainTab->setFixedHeight(nHeight);
  ui->tabWidget->hide();

  loginTime = m_Method->setCurrentDateTimeValue();
  strDate = m_Method->setCurrentDateValue();
  isReadEnd = true;

  this->installEventFilter(this);

  if (isAndroid) {
    textToolbar = new TextEditToolbar(this);
    EditEventFilter *editFilter = new EditEventFilter(textToolbar, this);
    ui->editCategory->installEventFilter(editFilter);
    ui->editDetails->installEventFilter(editFilter);
    ui->editTodo->installEventFilter(editFilter);
    ui->editDetails->viewport()->installEventFilter(editFilter);
    ui->editTodo->viewport()->installEventFilter(editFilter);
    ui->editWebDAV->installEventFilter(editFilter);
    ui->editWebDAVPassword->installEventFilter(editFilter);
    ui->editWebDAVUsername->installEventFilter(editFilter);
    ui->editFindNote->installEventFilter(editFilter);
    ui->editNotesSearch->installEventFilter(editFilter);
    ui->editSearchText->installEventFilter(editFilter);
  }

  ui->menubar->hide();
  ui->statusbar->hide();
  ui->frameReader->hide();
  ui->frameTodo->hide();
  ui->frameTodoRecycle->hide();
  ui->frameSteps->hide();
  ui->frameReport->hide();
  ui->frameSearch->hide();
  ui->frameBakList->hide();

  ui->frameViewCate->hide();
  ui->frameTabRecycle->hide();
  ui->frameNoteList->hide();
  ui->frameNotesSearchResult->hide();
  ui->frameNoteRecycle->hide();
  ui->f_FindNotes->hide();
  ui->btnFindNextNote->setEnabled(false);
  ui->btnFindPreviousNote->setEnabled(false);
  ui->frameNotesTree->hide();
  ui->qwCata->hide();
  ui->qwBookmark->hide();

  ui->frameCategory->hide();
  ui->frameSetTab->hide();
  ui->frameEditRecord->hide();
  ui->frameBookList->hide();
  ui->f_ReaderSet->hide();

  ui->frameReader->layout()->setContentsMargins(0, 0, 0, 1);
  ui->frameReader->setContentsMargins(0, 0, 0, 1);
  ui->frameReader->layout()->setSpacing(1);
  ui->frameImgView->hide();

  ui->frameMain->layout()->setContentsMargins(1, 0, 1, 0);
  ui->frameMain->setContentsMargins(1, 0, 1, 0);
  ui->frameMain->layout()->setSpacing(1);

  ui->frameOne->hide();
  ui->f_FunWeb->hide();
  ui->btnStorageInfo->hide();
  ui->editCode->setLineWrapMode(QTextEdit::NoWrap);
  ui->lblEpubInfo->hide();
  ui->pEpubProg->hide();

  ui->frameNotes->hide();
  ui->frameNotes->layout()->setContentsMargins(1, 1, 1, 1);

  ui->btnSetKey->hide();
  ui->btnNotesList->hide();
  ui->btnWebBack->hide();
  ui->btnRecentOpen0->hide();

  ui->chkOneDrive->setStyleSheet(m_Preferences->chkStyle);
  ui->chkWebDAV->setStyleSheet(m_Preferences->chkStyle);
  ui->chkAutoSync->setStyleSheet(m_Preferences->chkStyle);
  ui->twCloudBackup->setCurrentIndex(1);
  ui->twCloudBackup->setTabVisible(0, false);
  ui->chkWebDAV->hide();
  ui->lblWebDAV->hide();

  ui->editWebDAVPassword->setEchoMode(QLineEdit::EchoMode::Password);
  ui->lblWebDAV->setStyleSheet(labelNormalStyleSheet);
  ui->lblTitleEditRecord->setStyleSheet(labelNormalStyleSheet);

  ui->textBrowser->installEventFilter(this);
  ui->textBrowser->setMouseTracking(true);
  ui->textBrowser->viewport()->installEventFilter(this);
  ui->textBrowser->viewport()->setMouseTracking(true);
  ui->qwReader->installEventFilter(this);

  ui->tabWidget->tabBar()->installEventFilter(this);
  ui->tabWidget->installEventFilter(this);
  ui->tabWidget->setMouseTracking(true);
  ui->lblStats->installEventFilter(this);
  ui->editSearchText->installEventFilter(this);
  ui->editFindNote->installEventFilter(this);

  ui->lblTitleEditRecord->installEventFilter(this);
  ui->lblNoteName->installEventFilter(this);

  ui->lblStats->adjustSize();
  ui->lblStats->setWordWrap(true);

  ui->lblNoteTitle->adjustSize();
  ui->lblNoteTitle->setWordWrap(true);
  ui->lblNoteTitle->hide();
  ui->f_Tools->hide();

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

  myReadThread = new ReadThread();
  connect(myReadThread, &ReadThread::isDone, this, &MainWindow::readChartDone);

  mySaveThread = new SaveThread();
  connect(mySaveThread, &SaveThread::isDone, this, &MainWindow::saveDone);

  myBakDataThread = new BakDataThread();
  connect(myBakDataThread, &BakDataThread::isDone, this,
          &MainWindow::bakDataDone);

  myImportDataThread = new ImportDataThread();
  connect(myImportDataThread, &ImportDataThread::isDone, this,
          &MainWindow::importDataDone);

  mySearchThread = new SearchThread();
  connect(mySearchThread, &SearchThread::isDone, this, &MainWindow::searchDone);

  myUpdateGpsMapThread = new UpdateGpsMapThread();
  connect(myUpdateGpsMapThread, &UpdateGpsMapThread::isDone, this,
          &MainWindow::updateGpsMapDone);

  connect(pAndroidKeyboard, &QInputMethod::visibleChanged, this,
          &MainWindow::on_KVChanged);

  ui->progBar->setMaximumHeight(4);
  ui->progBar->hide();
  ui->progBar->setStyleSheet(
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
  ui->progReader->setStyleSheet(ui->progBar->styleSheet());
  ui->progReader->setFixedHeight(4);

  if (isIOS) {
  }

#ifdef Q_OS_ANDROID
#else

#endif

  // ui->tabCharts->setCornerWidget(ui->frame_cw);

  ui->tabCharts->tabBar()->hide();
  m_Method->setToolButtonQss(ui->btnChartMonth, 5, 3, "#FF0000", "#FFFFFF",
                             "#FF0000", "#FFFFFF", "#FF5555", "#FFFFFF");
  m_Method->setToolButtonQss(ui->btnChartDay, 5, 3, "#455364", "#FFFFFF",
                             "#455364", "#FFFFFF", "#555364", "#FFFFFF");

  int nIConFontSize;
#ifdef Q_OS_ANDROID
  nIConFontSize = 12;
#else
  nIConFontSize = 9;
#endif
  QFont f = this->font();
  f.setPointSize(nIConFontSize);
  ui->btnTodo->setFont(f);
  ui->btnSteps->setFont(f);
  ui->btnChart->setFont(f);
  ui->btnReader->setFont(f);
  ui->btnNotes->setFont(f);
  ui->btnSelTab->setFont(f);

  f.setPointSize(nIConFontSize + 0);
  ui->btnMenu->setFont(f);
  ui->btnAdd->setFont(f);
  ui->btnDel->setFont(f);
  ui->btnSync->setFont(f);

  ui->btnReport->setFont(f);
  ui->btnFind->setFont(f);
  ui->btnModifyRecord->setFont(f);
  ui->btnMove->setFont(f);

  f.setBold(true);
  ui->lblSyncNote->setFont(f);
  ui->lblShowLineSn->setFont(f);
  ui->lblShowLineSn->setWordWrap(true);
  ui->lblShowLineSn->adjustSize();

  QString lblStyle = ui->lblTitleEditRecord->styleSheet();
  ui->lblTotal->setStyleSheet(lblStyle);
  ui->lblDetails->setStyleSheet(lblStyle);
  ui->lblTitle->setStyleSheet(lblStyle);
  ui->lblTitle_Report->setStyleSheet(lblStyle);

  ui->tabMotion->setCornerWidget(ui->btnBackSteps, Qt::TopRightCorner);
  ui->tabMotion->setCurrentIndex(1);
  QString rbStyle = ui->rbCycling->styleSheet();
  ui->rbHiking->setStyleSheet(rbStyle);
  ui->rbRunning->setStyleSheet(rbStyle);
  QSettings Reg(iniDir + "gpslist.ini", QSettings::IniFormat);

  ui->rbCycling->setChecked(Reg.value("/GPS/isCycling", 0).toBool());
  ui->rbHiking->setChecked(Reg.value("/GPS/isHiking", 0).toBool());
  ui->rbRunning->setChecked(Reg.value("/GPS/isRunning", 0).toBool());

  ui->btnGPS->setStyleSheet(m_Steps->btnRoundStyle);
  ui->btnGPS->hide();
  ui->frame_btnGps->setFixedHeight(80);
  QWidget *centralWidget = new QWidget(this);
  QVBoxLayout *layout = new QVBoxLayout(centralWidget);

  SliderButton *sliderButton = new SliderButton(centralWidget);
  sliderButton->setTipText(tr("Slide Right to Start or Stop."));
  layout->addWidget(sliderButton);

  QObject::connect(sliderButton, &SliderButton::sliderMovedToEnd, this,
                   [&]() { ui->btnGPS->click(); });
  ui->frame_btnGps->layout()->addWidget(centralWidget);
}

void MainWindow::init_ButtonStyle() {
  m_Method->set_ToolButtonStyle(this);
  ui->btnMenu->setStyleSheet("border:none");
  ui->btnModifyRecord->setStyleSheet("border:none");
  ui->btnMove->setStyleSheet("border:none");

  ui->btnTodo->setStyleSheet("border:none");
  ui->btnSteps->setStyleSheet("border:none");
  ui->btnChart->setStyleSheet("border:none");
  ui->btnReader->setStyleSheet("border:none");
  ui->btnNotes->setStyleSheet("border:none");
  ui->btnAdd->setStyleSheet("border:none");
  ui->btnDel->setStyleSheet("border:none");
  ui->btnPasteTodo->setStyleSheet("border:none");
  ui->btnSync->setStyleSheet("border:none");
  ui->btnFind->setStyleSheet("border:none");
  ui->btnReport->setStyleSheet("border:none");
  ui->btnSelTab->setStyleSheet("border:none");

  if (isDark) {
    ui->f_ReaderFun->setStyleSheet("QFrame{background-color: #2874AC;}");
    ui->btnOpen->setStyleSheet("border:none; background-color:#2874AC;");
    ui->btnBackReader->setStyleSheet("border:none; background-color:#2874AC;");
    ui->btnCatalogue->setStyleSheet("border:none; background-color:#2874AC;");
    ui->btnBackDir->setStyleSheet("border:none; background-color:#2874AC;");

    ui->btnReadList->setStyleSheet("border:none; background-color:#2874AC;");
    ui->btnShowBookmark->setStyleSheet(
        "border:none; background-color:#2874AC;");
    ui->btnPages->setStyleSheet("border:none; background-color:#2874AC;");
    ui->btnAutoRun->setStyleSheet("border:none; background-color:#2874AC;");
    ui->btnAutoStop->setStyleSheet("border:none; background-color:#2874AC;");

    mw_one->ui->btnPages->setStyleSheet(
        "color: rgb(255, 255, 255);background-color: #2874AC; "
        "border: "
        "0px solid "
        "rgb(255,0,0);border-radius: 0px;"
        "font-weight: bold;");
  } else {
    ui->f_ReaderFun->setStyleSheet("QFrame{background-color: #3498DB;}");
    ui->btnOpen->setStyleSheet("border:none; background-color:#3498DB;");
    ui->btnBackReader->setStyleSheet("border:none; background-color:#3498DB;");
    ui->btnCatalogue->setStyleSheet("border:none; background-color:#3498DB;");
    ui->btnBackDir->setStyleSheet("border:none; background-color:#3498DB;");

    ui->btnReadList->setStyleSheet("border:none; background-color:#3498DB;");
    ui->btnShowBookmark->setStyleSheet(
        "border:none; background-color:#3498DB;");
    ui->btnPages->setStyleSheet("border:none; background-color:#3498DB;");
    ui->btnAutoRun->setStyleSheet("border:none; background-color:#3498DB;");
    ui->btnAutoStop->setStyleSheet("border:none; background-color:#3498DB;");

    mw_one->ui->btnPages->setStyleSheet(
        "color: rgb(255, 255, 255);background-color: #3498DB; "
        "border: "
        "0px solid "
        "rgb(255,0,0);border-radius: 0px;"
        "font-weight: bold;");
  }

  QString style =
      "QToolButton {background-color: rgb(255, 0, 0); color: "
      "rgb(255,255,255); "
      "border-radius:10px; "
      "border:0px solid gray; } QToolButton:pressed { background-color: "
      "rgb(220,220,230); color: black}";
  m_Preferences->ui->btnReStart->setStyleSheet(style);
}

void MainWindow::selTab() {
  int index = m_Method->getCurrentIndexFromQW(ui->qwSelTab);
  tabData->setCurrentIndex(index);
  on_btnBackSetTab_clicked();
  m_Method->clearAllBakList(ui->qwSelTab);

  if (ui->btnTabMoveDown->isHidden()) {
    ui->btnTabMoveDown->show();
    ui->btnTabMoveUp->show();
    on_btnAdd_clicked();
    m_EditRecord->setCurrentValue();
  }
}

void MainWindow::getMainTabs() {
  m_Method->clearAllBakList(ui->qwSelTab);
  int tab_count = tabData->tabBar()->count();
  for (int i = 0; i < tab_count; i++) {
    QString text0 = tabData->tabText(i);
    m_Method->addItemToQW(ui->qwSelTab, text0, "", "", "", 0);
  }

  int index = ui->tabWidget->currentIndex();
  m_Method->setCurrentIndexFromQW(ui->qwSelTab, index);

  ui->lblSelTabInfo->setText(tr("Total") + " : " + QString::number(tab_count) +
                             " ( " + QString::number(index + 1) + " ) ");
}

void MainWindow::on_btnSelTab_clicked() {
  ui->frameMain->hide();
  ui->frameSetTab->show();
  getMainTabs();
}

void MainWindow::init_Menu(QMenu *mainMenu) {
  QAction *actAddTab = new QAction(tr("Add Tab"));
  QAction *actDelTab = new QAction(tr("Del Tab"));
  QAction *actRenameTab = new QAction(tr("Rename Tab"));

  QAction *actOpenKnotBakDir = new QAction(tr("Open KnotBak Dir"));

  QAction *actReport = new QAction(tr("Report"));
  actReport->setVisible(false);

  QAction *actExportData = new QAction(tr("Export Data"));
  QAction *actImportData = new QAction(tr("Import Data"));

  QAction *actPreferences = new QAction(tr("Preferences"));

  QAction *actAbout = new QAction(tr("About") + " (" + ver + ")");
  QAction *actOneDrive = new QAction(tr("Cloud Backup and Restore Data"));

  QAction *actBakFileList = new QAction(tr("Backup File List"));
  QAction *actTabRecycle = new QAction(tr("Tab Recycle"));
  QAction *actShareFile = new QAction(tr("Share File"));

  connect(actAddTab, &QAction::triggered, this,
          &MainWindow::on_actionAdd_Tab_triggered);
  connect(actDelTab, &QAction::triggered, this,
          &MainWindow::on_actionDel_Tab_triggered);
  connect(actRenameTab, &QAction::triggered, this,
          &MainWindow::on_actionRename_triggered);

  connect(actBakFileList, &QAction::triggered, this,
          &MainWindow::on_actionBakFileList);

  connect(actTabRecycle, &QAction::triggered, this,
          &MainWindow::on_actionTabRecycle);

  connect(actOpenKnotBakDir, &QAction::triggered, this,
          &MainWindow::on_openKnotBakDir);
  connect(actReport, &QAction::triggered, this,
          &MainWindow::on_actionReport_triggered);

  connect(actExportData, &QAction::triggered, this,
          &MainWindow::on_actionExport_Data_triggered);

  connect(actImportData, &QAction::triggered, this,
          &MainWindow::on_actionImport_Data_triggered);
  connect(actPreferences, &QAction::triggered, this,
          &MainWindow::on_actionPreferences_triggered);

  connect(actOneDrive, &QAction::triggered, this,
          &MainWindow::on_actionOneDriveBackupData);
  connect(actAbout, &QAction::triggered, this, &MainWindow::on_actionAbout);
  connect(actShareFile, &QAction::triggered, this,
          &MainWindow::on_actionShareFile);

  mainMenu->addAction(actAddTab);
  mainMenu->addAction(actDelTab);
  mainMenu->addAction(actRenameTab);

  mainMenu->addAction(actReport);

  mainMenu->addAction(actExportData);
  mainMenu->addAction(actImportData);

#ifdef Q_OS_ANDROID
  mainMenu->addAction(actOpenKnotBakDir);
  actOpenKnotBakDir->setVisible(false);
  actShareFile->setVisible(false);
#else
  actShareFile->setVisible(false);
  if (!m_Preferences->devMode) {
    actAddTab->setVisible(false);
    actDelTab->setVisible(false);
    actRenameTab->setVisible(false);
    actTabRecycle->setVisible(false);
  }
#endif

  mainMenu->addAction(actPreferences);

  mainMenu->addAction(actOneDrive);
  mainMenu->addAction(actBakFileList);
  mainMenu->addAction(actTabRecycle);
  mainMenu->addAction(actShareFile);
  mainMenu->addAction(actAbout);

  mainMenu->setStyleSheet(m_Method->qssMenu);
}

void MainWindow::on_openKnotBakDir() {
#ifdef Q_OS_ANDROID

  QJniObject m_activity = QNativeInterface::QAndroidApplication::context();
  m_activity.callMethod<void>("openKnotBakDir", "()V");

#endif
}

void MainWindow::init_CloudBacup() {
  ui->editWebDAV->setText(
      iniPreferences->value("/webdav/url", "https://dav.jianguoyun.com/dav/")
          .toString());

  ui->editWebDAVUsername->setText(
      iniPreferences->value("/webdav/username").toString());

  QString aesStr = iniPreferences->value("/webdav/password").toString();
  QString password = m_CloudBackup->aesDecrypt(aesStr, aes_key, aes_iv);
  ui->editWebDAVPassword->setText(password);

  ui->chkOneDrive->setChecked(
      iniPreferences->value("/cloudbak/onedrive", 0).toBool());
  ui->chkWebDAV->setChecked(
      iniPreferences->value("/cloudbak/webdav", 1).toBool());
  ui->chkAutoSync->setChecked(
      iniPreferences->value("/cloudbak/autosync", 0).toBool());
}

void MainWindow::on_actionOneDriveBackupData() {
  ui->frameMain->hide();
  ui->frameReader->hide();
  ui->frameOne->show();
}

void MainWindow::on_actionTabRecycle() {
  ui->frameMain->hide();
  ui->frameTabRecycle->show();

  m_Method->clearAllBakList(ui->qwTabRecycle);

  QString tab_name, tab_time;
  QStringList iniFiles;
  QStringList fmt;
  fmt.append("ini");
  m_NotesList->getAllFiles(iniDir, iniFiles, fmt);

  QString iniTotal;
  QStringList myList, nameList, iniList;
  for (int i = 0; i < iniFiles.count(); i++) {
    QString ini_file = iniFiles.at(i);
    if (ini_file.contains("recycle_name_")) {
      QFileInfo fi(ini_file);
      QString ini_filename = fi.fileName();
      ini_filename = ini_filename.replace(".ini", "");
      tab_name = ini_filename.split("_").at(1);
      QString t1, t2;
      t1 = ini_filename.split("_").at(2);
      t2 = ini_filename.split("_").at(3);
      QStringList list = t2.split("-");
      if (list.count() == 2) {
        t2 = list.at(0);
      }
      tab_time = t1 + "  " + t2;

      tab_name = m_Method->getRecycleTabName(t1 + "_" + t2);

      myList.append(tab_name + "-=-" + tab_time + "-=-" + ini_file);
      nameList.append(tab_name + "-=-" + tab_time);
      iniList.append(ini_file);
    }
  }

  int count = myList.count();
  QStringList lastList;
  for (int i = 0; i < count; i++) {
    QString str1 = myList.at(i);
    iniTotal = "";
    QStringList list1 = str1.split("-=-");
    tab_name = list1.at(0);
    tab_time = list1.at(1);
    for (int j = 0; j < count; j++) {
      QString str2 = nameList.at(j);
      if (str1.contains(str2)) {
        iniTotal += iniList.at(j) + "\n";
      }
    }

    lastList.append(tab_name + "-=-" + tab_time + "-=-" + iniTotal);
  }

  // Qt5 使用 QSet 去重
  // QSet<QString> set = QSet<QString>::fromList(lastList);
  // QStringList uniqueList = QStringList::fromSet(set);

  // Qt6 新写法（直接通过迭代器构造）
  QSet<QString> set(lastList.begin(), lastList.end());  // 直接构造 QSet
  QStringList uniqueList(set.begin(), set.end());       // 直接构造 QStringList

  for (int i = 0; i < uniqueList.count(); i++) {
    QString str = uniqueList.at(i);
    tab_name = str.split("-=-").at(0);
    tab_time = str.split("-=-").at(1);
    iniTotal = str.split("-=-").at(2);
    m_Method->addItemToQW(ui->qwTabRecycle, tab_name, tab_time, "", iniTotal,
                          0);
  }

  int t_count = m_Method->getCountFromQW(ui->qwTabRecycle);
  if (t_count > 0) {
    m_Method->setCurrentIndexFromQW(ui->qwTabRecycle, 0);
  }

  ui->lblTitleTabRecycle->setText(tr("Tab Recycle") + "    " + tr("Total") +
                                  " : " + QString::number(t_count));
}

void MainWindow::on_actionBakFileList() {
  startBackgroundTaskUpdateBakFileList();
}

void MainWindow::startBackgroundTaskUpdateBakFileList() {
  ui->frameMain->hide();
  ui->frameBakList->show();

  QFuture<void> future = QtConcurrent::run([=]() {
    bakFileList = m_Preferences->getBakFilesList();
    int bakCount = bakFileList.count();

    if (bakCount > 15) {
      int count_a = bakCount - 15;
      for (int j = 0; j < count_a; j++) {
        QString str = bakFileList.at(0);
        QString fn = str.split("-===-").at(1);
        QFile file(fn);
        file.remove();
        bakFileList.removeAt(0);
      }
    }
  });

  // 可选：使用 QFutureWatcher 监控进度
  QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
  connect(watcher, &QFutureWatcher<void>::finished, this, [=]() {
    m_Method->clearAllBakList(ui->qwBakList);
    int bakCount = bakFileList.count();
    for (int i = 0; i < bakCount; i++) {
      QString action, bakfile;
      QString str = bakFileList.at(bakCount - 1 - i);
      action = str.split("-===-").at(0);
      bakfile = str.split("-===-").at(1);
      m_Method->addItemToQW(ui->qwBakList, action, "", "", bakfile, 0);
    }

    if (m_Method->getCountFromQW(ui->qwBakList) > 0)
      m_Method->setCurrentIndexFromQW(ui->qwBakList, 0);

    ui->lblBakListTitle->setText(
        tr("Backup File List") + "    " + tr("Total") + " : " +
        QString::number(m_Method->getCountFromQW(ui->qwBakList)));

    qDebug() << "BakFileList update completed";
    watcher->deleteLater();
  });
  watcher->setFuture(future);
}

void MainWindow::on_btnMenu_clicked() {
  mainMenu = new QMenu(this);
  init_Menu(mainMenu);
  int x = 0;
#ifdef Q_OS_ANDROID
  x = mw_one->geometry().x() + 2;
#else
  x = mw_one->geometry().x() + ui->btnMenu->x();
#endif
  int y = geometry().y() + ui->f_Menu->height() + 2;
  QPoint pos(x, y);
  mainMenu->exec(pos);
}

void MainWindow::stopJavaTimer() {
#ifdef Q_OS_ANDROID

  QJniObject jo = QNativeInterface::QAndroidApplication::context();
  jo.callStaticMethod<int>("com.x/MyService", "stopTimer", "()I");

#endif
}

#ifdef Q_OS_ANDROID
static void JavaNotify_0() {
  // onResume

  if (mw_one->initMain) return;

  if (mw_one->m_Steps->isNeedRestoreUI) {
    mw_one->ui->btnSteps->click();
  }

  qDebug() << "C++ JavaNotify_0";
}

static void JavaNotify_1() {
  // Lock screen or onPause

  if (mw_one->initMain) return;

  if (!mw_one->ui->frameSteps->isHidden()) {
    mw_one->ui->btnBackSteps->click();
    mw_one->m_Steps->isNeedRestoreUI = true;
  }

  qDebug() << "C++ JavaNotify_1";
}

static void JavaNotify_2() {
  // When the screen lights up.
  mw_one->m_Todo->refreshAlarm();
  mw_one->m_Steps->updateHardSensorSteps();

  qDebug() << "C++ JavaNotify_2";
}

static void JavaNotify_3() {
  mw_one->alertWindowsCount++;
  mw_one->m_Todo->refreshAlarm();

  qDebug() << "C++ JavaNotify_3";
}

static void JavaNotify_4() {
  mw_one->alertWindowsCount--;

  if (mw_one->alertWindowsCount == 0) {
    bool isBackMain = false;
    QJniObject activity =
        QJniObject(QNativeInterface::QAndroidApplication::context());
    if (activity.isValid()) {
      jboolean result = activity.callMethod<jboolean>("getIsBackMainUI", "()Z");
      activity.callMethod<void>("setIsBackMainUI", "(Z)V", false);
      isBackMain = result;
    }

    if (!isBackMain) {
      mw_one->setMini();
    }
  }

  qDebug() << "alertWindowsCount=" << mw_one->alertWindowsCount;
  qDebug() << "C++ JavaNotify_4";
}

static void JavaNotify_5() {
  mw_one->m_ReceiveShare->goReceiveShare();

  qDebug() << "C++ JavaNotify_5";
}

static void JavaNotify_6() {
  mw_one->m_Notes->javaNoteToQMLNote();

  qDebug() << "C++ JavaNotify_6";
}

static void JavaNotify_7() {
  mw_one->m_Notes->insertImage(privateDir + "receive_share_pic.png", true);

  qDebug() << "C++ JavaNotify_7";
}

static void JavaNotify_8() {
  if (isInitThemeEnd) {
    if (mw_one->m_Steps->isNeedRestoreUI) {
      QTimer::singleShot(1000, nullptr, []() { mw_one->execDeskShortcut(); });

    } else
      mw_one->execDeskShortcut();

  } else {
    isNeedExecDeskShortcut = true;
  }

  qDebug() << "C++ JavaNotify_8";
}

static void JavaNotify_9() {
  QSettings Reg(privateDir + "choice_book.ini", QSettings::IniFormat);

  QString file = Reg.value("book/file", "").toString();
  QString type = Reg.value("book/type", "filepicker").toString();
  if (QFile::exists(file)) {
    if (type == "defaultopen") {
      mw_one->m_ReceiveShare->closeAllChildWindows();
      mw_one->m_ReceiveShare->moveTaskToFront();
    }
    mw_one->m_Reader->startOpenFile(file);
  }

  qDebug() << "C++ JavaNotify_9";
}

static void JavaNotify_10() {
  // Open Book
  mw_one->on_btnOpen_clicked();

  qDebug() << "C++ JavaNotify_10";
}

static void JavaNotify_11() {
  // Books List
  mw_one->ui->btnReadList->click();

  qDebug() << "C++ JavaNotify_11";
}

static void JavaNotify_12() {
  if (isPDF && isAndroid) mw_one->m_Reader->openMyPDF(fileName);

  qDebug() << "C++ JavaNotify_12";
}

static void JavaNotify_13() {
  mw_one->m_Reader->openMyPDF(fileName);

  qDebug() << "C++ JavaNotify_13";
}

static void JavaNotify_14() {
  if (m_Method->getDateTimeFlag() == "todo") {
    mw_one->m_TodoAlarm->setDateTime();
  } else if (m_Method->getDateTimeFlag() == "gpslist") {
    mw_one->ui->btnGetGpsListData->click();
  } else {
    mw_one->m_DateSelector->ui->btnOk->click();
  }
  qDebug() << "C++ JavaNotify_14";
}

static void JavaNotify_15() {
  if (colorDlg != nullptr) {
    if (colorDlg->isVisible()) {
      colorDlg->close();
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

  if (m_Method->menuNoteBook != nullptr) {
    if (m_Method->menuNoteBook->isVisible()) {
      m_Method->menuNoteBook->close();
      return;
    }
  }

  if (m_Method->menuNoteList != nullptr) {
    if (m_Method->menuNoteList->isVisible()) {
      m_Method->menuNoteList->close();
      return;
    }
  }

  if (mw_one->m_NotesList->menuRecentOpen != nullptr) {
    if (mw_one->m_NotesList->menuRecentOpen->isVisible()) {
      mw_one->m_NotesList->menuRecentOpen->close();
      return;
    }
  }

  if (mw_one->m_NotesList->m_MoveTo != nullptr) {
    if (mw_one->m_NotesList->m_MoveTo->isVisible()) {
      mw_one->m_NotesList->m_MoveTo->ui->btnCancel->click();
      return;
    }
  }

  if (mw_one->m_NotesList->m_NewNoteBook != nullptr) {
    if (mw_one->m_NotesList->m_NewNoteBook->isVisible()) {
      mw_one->m_NotesList->m_NewNoteBook->ui->btnCancel->click();
      return;
    }
  }

  if (mw_one->textToolbar != nullptr) {
    if (mw_one->textToolbar->isVisible()) {
      mw_one->textToolbar->hide();
      return;
    }
  }

  if (mw_one->m_Preferences->textToolbarPreferences != nullptr) {
    if (mw_one->m_Preferences->textToolbarPreferences->isVisible()) {
      mw_one->m_Preferences->textToolbarPreferences->hide();
      return;
    }
  }

  if (m_ShowMessage != nullptr) {
    if (m_ShowMessage->isVisible()) {
      m_ShowMessage->close();
      return;
    }
  }

  if (mw_one->m_RenameDlg != nullptr) {
    if (mw_one->m_RenameDlg->isVisible()) {
      mw_one->m_RenameDlg->close();
      return;
    }
  }

  if (mw_one->m_Todo->textToolbarReeditTodo != nullptr) {
    if (mw_one->m_Todo->textToolbarReeditTodo->isVisible()) {
      mw_one->m_Todo->textToolbarReeditTodo->hide();
      return;
    }
  }

  if (mw_one->m_Todo->m_ReeditTodo != nullptr) {
    if (mw_one->m_Todo->m_ReeditTodo->isVisible()) {
      mw_one->m_Todo->m_ReeditTodo->close();
      return;
    }
  }

  if (mw_one->m_NotesList->textToolbarRenameNotes != nullptr) {
    if (mw_one->m_NotesList->textToolbarRenameNotes->isVisible()) {
      mw_one->m_NotesList->textToolbarRenameNotes->hide();
      return;
    }
  }

  if (mw_one->m_NotesList->m_RenameNotes != nullptr) {
    if (mw_one->m_NotesList->m_RenameNotes->isVisible()) {
      mw_one->m_NotesList->m_RenameNotes->close();
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

  if (mw_one->m_StepsOptions->isVisible()) {
    mw_one->m_StepsOptions->ui->btnBack->click();
    return;
  }

  if (mw_one->ui->f_ReaderSet->isVisible()) {
    mw_one->ui->btnBackReaderSet->click();
    return;
  }

  if (!mw_one->ui->frameReader->isHidden()) {
    if (mw_one->ui->qwCata->isVisible()) {
      mw_one->ui->btnCatalogue->click();
      return;

    } else if (mw_one->ui->qwBookmark->isVisible()) {
      mw_one->ui->btnShowBookmark->click();
      return;

    } else if (!mw_one->mydlgSetText->isHidden()) {
      mw_one->mydlgSetText->close();
      return;

    } else if (!mw_one->ui->textBrowser->isHidden()) {
      mw_one->ui->btnSelText->click();
      return;
    }

    else {
      mw_one->ui->btnBackReader->click();
      return;
    }
  }

  if (!mw_one->ui->frameImgView->isHidden()) {
    mw_one->ui->btnBackImg->click();
    return;
  }

  if (!mw_one->ui->frameMain->isHidden()) {
    if (!mw_one->ui->f_charts->isHidden()) {
      mw_one->ui->btnChart->click();
      return;
    }

    mw_one->setMini();

    return;
  }

  if (!mw_one->ui->frameOne->isHidden()) {
    mw_one->ui->btnBack_One->click();
    return;
  }

  if (!mw_one->ui->frameNoteRecycle->isHidden()) {
    mw_one->ui->btnBackNoteRecycle->click();
    return;
  }

  if (!mw_one->ui->frameNotesSearchResult->isHidden()) {
    mw_one->ui->btnBack_NotesSearchResult->click();
    return;
  }

  if (!mw_one->ui->frameNoteList->isHidden()) {
    mw_one->ui->btnBackNoteList->click();
    return;
  }

  if (!mw_one->ui->frameNotes->isHidden()) {
    mw_one->ui->btnBackNotes->click();
    return;
  }

  if (mw_one->m_TodoAlarm->isVisible()) {
    mw_one->m_TodoAlarm->ui->btnBack->click();
    return;
  }

  if (!mw_one->ui->frameTodo->isHidden()) {
    mw_one->ui->btnBackTodo->click();
    return;
  }

  if (!mw_one->ui->frameTodoRecycle->isHidden()) {
    mw_one->ui->btnReturnRecycle->click();
    return;
  }

  if (!mw_one->ui->frameTabRecycle->isHidden()) {
    mw_one->ui->btnBackTabRecycle->click();
    return;
  }

  if (!mw_one->ui->frameSteps->isHidden()) {
    mw_one->ui->btnBackSteps->click();
    return;
  }

  if (!mw_one->ui->frameViewCate->isHidden()) {
    mw_one->ui->frameViewCate->hide();
    mw_one->ui->frameReport->show();
    return;
  }

  if (!mw_one->ui->frameReport->isHidden()) {
    mw_one->ui->btnBack_Report->click();
    return;
  }

  if (!mw_one->ui->frameSearch->isHidden()) {
    mw_one->ui->btnBackSearch->click();
    return;
  }

  if (!mw_one->ui->frameBakList->isHidden()) {
    mw_one->ui->btnBackBakList->click();
    return;
  }

  if (!mw_one->ui->frameCategory->isHidden()) {
    mw_one->ui->btnCancelType->click();
    return;
  }

  if (!mw_one->ui->frameSetTab->isHidden()) {
    mw_one->ui->btnBackSetTab->click();
    return;
  }

  if (!mw_one->ui->frameEditRecord->isHidden()) {
    mw_one->ui->btnBackEditRecord->click();

    return;
  }

  if (!mw_one->ui->frameBookList->isHidden()) {
    mw_one->ui->btnBackBookList->click();
    return;
  }

  if (!mw_one->ui->frameNotesTree->isHidden()) {
    mw_one->ui->btnBack_Tree->click();
    return;
  }

  qDebug() << "C++ JavaNotify_15";
}

static const JNINativeMethod gMethods[] = {
    {"CallJavaNotify_0", "()V", (void *)JavaNotify_0},
    {"CallJavaNotify_1", "()V", (void *)JavaNotify_1},
    {"CallJavaNotify_2", "()V", (void *)JavaNotify_2},
    {"CallJavaNotify_3", "()V", (void *)JavaNotify_3},
    {"CallJavaNotify_4", "()V", (void *)JavaNotify_4},
    {"CallJavaNotify_5", "()V", (void *)JavaNotify_5},
    {"CallJavaNotify_6", "()V", (void *)JavaNotify_6},
    {"CallJavaNotify_7", "()V", (void *)JavaNotify_7},
    {"CallJavaNotify_8", "()V", (void *)JavaNotify_8},
    {"CallJavaNotify_9", "()V", (void *)JavaNotify_9},
    {"CallJavaNotify_10", "()V", (void *)JavaNotify_10},
    {"CallJavaNotify_11", "()V", (void *)JavaNotify_11},
    {"CallJavaNotify_12", "()V", (void *)JavaNotify_12},
    {"CallJavaNotify_13", "()V", (void *)JavaNotify_13},
    {"CallJavaNotify_14", "()V", (void *)JavaNotify_14}

};

static const JNINativeMethod gMethods15[] = {
    {"CallJavaNotify_15", "()V", (void *)JavaNotify_15}};

void RegJni(const char *myClassName) {
  QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
    QJniEnvironment Environment;
    const char *mClassName = myClassName;
    jclass j_class;
    j_class = Environment->FindClass(mClassName);
    if (j_class == nullptr) {
      qDebug() << "erro clazz";
      return;
    }
    jint mj = Environment->RegisterNatives(
        j_class, gMethods, sizeof(gMethods) / sizeof(gMethods[0]));
    if (mj != JNI_OK) {
      qDebug() << "register native method failed!";
      return;
    } else {
      qDebug() << "RegisterNatives success!";
    }
  });
  qDebug() << "++++++++++++++++++++++++";
}

void RegJni15(const char *myClassName) {
  QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
    QJniEnvironment Environment;
    const char *mClassName = myClassName;
    jclass j_class;
    j_class = Environment->FindClass(mClassName);
    if (j_class == nullptr) {
      qDebug() << "erro clazz";
      return;
    }
    jint mj = Environment->RegisterNatives(
        j_class, gMethods15, sizeof(gMethods15) / sizeof(gMethods15[0]));
    if (mj != JNI_OK) {
      qDebug() << "register native method failed!";
      return;
    } else {
      qDebug() << "RegisterNatives15 success!";
    }
  });
  qDebug() << "++++++++++++++++++++++++";
}

#endif

QString MainWindow::getYMD(QString date) {
  QStringList list = date.split(" ");
  QString str;
  if (list.count() == 4) {
    str = list.at(1) + list.at(2) + list.at(3);
  }
  return str;
}

void MainWindow::on_btnReader_clicked() {
  if (isPDF) {
    if (isAndroid) {
      ui->frameMain->hide();
      ui->frameBookList->show();

      m_Reader->getReadList();

      m_Reader->openMyPDF(fileName);
      return;
    }
  }

  ui->frameMain->hide();
  ui->frameReader->show();
  ui->f_ReaderFun->show();

  isReaderVisible = true;
  isMemoVisible = false;

  if (!isOne) {
    while (!ui->btnReader->isEnabled())
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    mwh = this->height();
    setFixedHeight(mwh);
    ui->qwReader->rootContext()->setContextProperty("myW", this->width());
    ui->qwReader->rootContext()->setContextProperty("myH", mwh);
  }

  if (!isOne) {
    isOne = true;
    m_Reader->setPageVPos();
  }
}

void MainWindow::on_btnBackReader_clicked() {
  ui->btnAutoStop->click();

  m_ReaderSet->close();

  if (m_Reader->isSelText) on_btnSelText_clicked();

  if (ui->f_ReaderSet->isVisible()) {
    on_btnBackReaderSet_clicked();
  }

  m_Reader->saveReader("", false);
  m_Reader->savePageVPos();

  ui->frameReader->hide();
  ui->frameMain->show();
}

void MainWindow::on_btnOpen_clicked() {
  ui->btnAutoStop->click();

  m_Reader->saveReader("", false);
  m_Reader->savePageVPos();

  if (ui->f_ReaderSet->isVisible()) {
    on_btnBackReaderSet_clicked();
  }
  if (ui->qwBookmark->isVisible()) {
    on_btnShowBookmark_clicked();
  }
  m_ReaderSet->close();
  m_Reader->closeSelText();
  m_Reader->on_btnOpen_clicked();
}

void MainWindow::on_btnPageUp_clicked() { m_Reader->goUpPage(); }

void MainWindow::on_btnPageNext_clicked() { m_Reader->goNextPage(); }

void MainWindow::on_btnPages_clicked() {
  ui->btnAutoStop->click();

  if (ui->qwCata->isVisible()) return;

  if (ui->f_ReaderSet->isHidden()) {
    ui->f_ReaderSet->show();

    m_Reader->closeSelText();
    if (ui->qwBookmark->isVisible()) {
      on_btnShowBookmark_clicked();
    }

    QStringList list = ui->btnPages->text().split("\n");
    if (list.count() == 2) {
      QString cur = list.at(0);
      QString total = list.at(1);
      ui->lblProg->setText(tr("Reading Progress") + " : " + cur + " -> " +
                           total);

      ui->hSlider->setMaximum(total.toInt());
      ui->hSlider->setMinimum(1);
      ui->hSlider->setValue(cur.toInt());
    }
  } else
    on_btnBackReaderSet_clicked();
}

void MainWindow::on_hSlider_sliderMoved(int position) {
  if (isText) {
    ui->btnPages->setText(QString::number(position) + "\n" +
                          QString::number(totalPages));
    ui->progReader->setMinimum(1);
    ui->progReader->setMaximum(totalPages);
    ui->progReader->setValue(position);
  }

  if (isEpub) {
    ui->btnPages->setText(QString::number(position) + "\n" +
                          QString::number(htmlFiles.count()));
    ui->progReader->setMinimum(1);
    ui->progReader->setMaximum(htmlFiles.count());
    if (position == 0) position = 1;
    ui->progReader->setValue(position);
  }

  m_ReaderSet->updateProgress();
}

void MainWindow::on_btnReadList_clicked() {
  ui->btnAutoStop->click();

  m_Reader->saveReader("", false);
  m_Reader->savePageVPos();

  if (isAndroid) m_Reader->closeMyPDF();

  if (ui->f_ReaderSet->isVisible()) {
    on_btnBackReaderSet_clicked();
  }

  if (mw_one->ui->qwBookmark->isVisible()) {
    mw_one->on_btnShowBookmark_clicked();
  }

  m_ReaderSet->close();
  m_Reader->closeSelText();

  if (ui->frameMain->isVisible()) ui->frameMain->hide();
  ui->frameReader->hide();
  ui->frameBookList->show();

  m_Reader->getReadList();
}

void MainWindow::on_btnBackDir_clicked() { m_Reader->backDir(); }

QString MainWindow::getTabText() {
  return tabData->tabText(tabData->currentIndex());
}

void MainWindow::refreshMainUI() {
  this->update();
  this->repaint();
  qApp->processEvents();
}

void MainWindow::on_btnSelText_clicked() {
  if (ui->f_ReaderSet->isVisible()) {
    on_btnBackReaderSet_clicked();
  }
  m_Reader->selectText();
}

void MainWindow::on_btnSignIn_clicked() {
  m_CloudBackup->on_pushButton_SignIn_clicked();
}

void MainWindow::on_btnSignOut_clicked() {
  m_CloudBackup->on_pushButton_SingOut_clicked();
}

void MainWindow::on_btnUpload_clicked() {
  if (!ui->btnReader->isEnabled()) return;
  m_CloudBackup->startBakData();
}

void MainWindow::on_btnDownload_clicked() {
  m_CloudBackup->on_pushButton_downloadFile_clicked();
}

void MainWindow::on_btnBack_One_clicked() {
  clearWidgetFocus();

  if (!ui->frameOne->isHidden()) {
    if (ui->f_OneFun->isHidden()) {
      ui->f_OneFun->show();
      ui->f_FunWeb->hide();

      m_CloudBackup->loadLogQML();
    } else {
      ui->frameOne->hide();
      ui->frameMain->show();
    }
  }

  iniPreferences->setValue("/webdav/url", ui->editWebDAV->text().trimmed());

  iniPreferences->setValue("/webdav/username",
                           ui->editWebDAVUsername->text().trimmed());
  QString password = ui->editWebDAVPassword->text().trimmed();
  QString aesStr = m_CloudBackup->aesEncrypt(password, aes_key, aes_iv);
  iniPreferences->setValue("/webdav/password", aesStr);

  iniPreferences->setValue("/cloudbak/onedrive", ui->chkOneDrive->isChecked());
  iniPreferences->setValue("/cloudbak/webdav", ui->chkWebDAV->isChecked());
  iniPreferences->setValue("/cloudbak/autosync", ui->chkAutoSync->isChecked());

  setEncSyncStatusTip();
}

void MainWindow::on_btnRefreshToken_clicked() {
  m_CloudBackup->on_pushButton_clicked();
}

void MainWindow::on_btnStorageInfo_clicked() {
  m_CloudBackup->on_pushButton_storageInfo_clicked();
}

void MainWindow::on_btnRefreshWeb_clicked() {}

void MainWindow::on_btnUserInfo_clicked() {
  QNetworkAccessManager *manager = new QNetworkAccessManager(this);
  qDebug() << manager->supportedSchemes();
  qDebug() << QSslSocket::sslLibraryBuildVersionString();

  m_CloudBackup->on_pushButton_GetUserInfo_clicked();
}

void MainWindow::on_btnBackNotes_clicked() {
  QFileInfo fi(currentMDFile);
  m_Notes->saveWebScrollPos(fi.baseName());

  ui->frameNotes->hide();
  ui->frameNoteList->show();
}

void MainWindow::on_btnEdit_clicked() { m_Notes->openEditUI(); }

void MainWindow::clearSelectBox() {
  QString tempFile = iniDir + "memo/texteditor.html";
  if (!mw_one->ui->frameReader->isHidden()) {
    mw_one->m_Reader->savePageVPos();
    bool isAni = false;
    mw_one->ui->qwReader->rootContext()->setContextProperty("isAni", isAni);
    QQuickItem *root = mw_one->ui->qwReader->rootObject();
    QMetaObject::invokeMethod((QObject *)root, "loadHtml",
                              Q_ARG(QVariant, tempFile));
    m_Method->Sleep(50);
    if (isEpub) {
      QMetaObject::invokeMethod(
          (QObject *)root, "loadHtml",
          Q_ARG(QVariant, mw_one->m_Reader->currentHtmlFile));
    } else {
      ui->qwReader->rootContext()->setContextProperty("strText",
                                                      m_Reader->currentTxt);
    }
    mw_one->m_Reader->setPageVPos();
  }

  if (!mw_one->ui->frameNotes->isHidden()) {
  }
}

void MainWindow::on_btnCopy_clicked() {
  QClipboard *clipboard = QApplication::clipboard();
  clipboard->setText(mydlgSetText->ui->lineEdit->text().trimmed());
  on_btnCancelSel_clicked();
}

QString MainWindow::getSelectedText() {
  QString str;
  QVariant returnedValue;
  QQuickItem *root = ui->qwReader->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "getSelectedText",
                            Q_RETURN_ARG(QVariant, returnedValue));
  str = returnedValue.toString();
  return str.trimmed();
}

void MainWindow::on_btnSearch_clicked() {
  QString str = mydlgSetText->ui->lineEdit->text().trimmed();
  if (str == "") return;

  QString strurl;
  strurl = "https://bing.com/search?q=" + str;

  QUrl url(strurl);
  QDesktopServices::openUrl(url);
  on_btnCancelSel_clicked();
}

void MainWindow::on_btnCancelSel_clicked() { ui->btnSelText->click(); }

void MainWindow::on_textBrowser_selectionChanged() {
  QString str = ui->textBrowser->textCursor().selectedText().trimmed();
  ui->editSetText->setText(str);
  mydlgSetText->ui->lineEdit->setText(str);
}

void MainWindow::on_SetReaderFunVisible() {
  if (!isTurnThePage) {
    if (ui->f_ReaderFun->isHidden())
      ui->f_ReaderFun->show();
    else {
      ui->f_ReaderFun->hide();
      m_ReaderSet->hide();
    }
  }
}

void MainWindow::on_timerMousePress() {
  if (!isMouseMove && isMousePress) on_btnSelText_clicked();
}

void MainWindow::on_btnNotesList_clicked() {
  ui->frameNotes->hide();
  ui->frameNoteList->show();
  m_NotesList->set_memo_dir();

  if (m_NotesList->tw->topLevelItemCount() == 0) {
    ui->lblNoteBook->setText(tr("Note Book"));
    ui->lblNoteList->setText(tr("Note List"));
    return;
  }

  m_NotesList->loadAllNoteBook();
  m_NotesList->localNotesItem();
  m_NotesList->setNoteLabel();
}

void MainWindow::on_btnBackImg_clicked() {
  ui->frameImgView->hide();
  if (isReaderVisible) ui->frameReader->show();
  if (isMemoVisible) ui->frameNotes->show();
}

void MainWindow::on_btnZoomIn_clicked() {
  QQuickItem *root = ui->qw_Img->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "zoomin");
}

void MainWindow::on_btnZoomOut_clicked() {
  QQuickItem *root = ui->qw_Img->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "zoomout");
}

void MainWindow::on_btnReport_clicked() {
  on_actionReport_triggered();
  ui->btnYear->setFixedHeight(ui->btnMonth->height());
}

void MainWindow::on_btnPasteCode_clicked() {
  QClipboard *clipboard = QApplication::clipboard();
  QString originalText = clipboard->text();
  ui->editCode->setPlainText(originalText);
}

void MainWindow::on_btnAdd_clicked() {
  m_EditRecord->monthSum();

  on_AddRecord();
}

void MainWindow::on_btnDel_clicked() {
  isMoveEntry = false;
  del_Data((QTreeWidget *)ui->tabWidget->currentWidget());
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  Q_UNUSED(event);
  ui->qwReader->rootContext()->setContextProperty("myW", this->width());
  ui->qwReader->rootContext()->setContextProperty("myH", this->height());
  ui->qwTodo->rootContext()->setContextProperty("isBtnVisible",
                                                QVariant(false));
  ui->qwSteps->rootContext()->setContextProperty("myW", this->width());

#ifdef Q_OS_ANDROID

#else
  if (!ui->frameTodo->isHidden()) {
    ui->qwTodo->rootContext()->setContextProperty("m_width", mw_one->width());
    m_Todo->init_Todo();
  }
#endif
}

void MainWindow::on_KVChanged() {}

void MainWindow::on_btnAddTodo_clicked() { m_Todo->on_btnAdd_clicked(); }

void MainWindow::on_btnBackTodo_clicked() { m_Todo->closeTodo(); }

void MainWindow::on_btnHigh_clicked() { m_Todo->on_btnHigh_clicked(); }

void MainWindow::on_btnLow_clicked() { m_Todo->on_btnLow_clicked(); }

void MainWindow::on_btnSetTime_clicked() { m_Todo->on_btnSetTime_clicked(); }

void MainWindow::on_btnRecycle_clicked() { m_Todo->on_btnRecycle_clicked(); }

void MainWindow::on_btnReturnRecycle_clicked() {
  m_Todo->on_btnReturn_clicked();
}

void MainWindow::on_btnClearRecycle_clicked() { m_Todo->on_btnClear_clicked(); }

void MainWindow::on_btnDelRecycle_clicked() { m_Todo->on_btnDel_clicked(); }

void MainWindow::on_btnRestoreRecycle_clicked() {
  m_Todo->on_btnRestore_clicked();
}

void MainWindow::on_editTodo_textChanged() {
  m_Todo->on_editTodo_textChanged();
}

void MainWindow::setItemHeight(int h) {
  QQuickItem *root = mw_one->ui->qwMainTab->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "setItemHeight",
                            Q_ARG(QVariant, h));
}

void MainWindow::addItem(QString text0, QString text1, QString text2,
                         QString text3, int itemH) {
  QQuickItem *root = mw_one->ui->qwMainTab->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "addItem", Q_ARG(QVariant, text0),
                            Q_ARG(QVariant, text1), Q_ARG(QVariant, text2),
                            Q_ARG(QVariant, text3), Q_ARG(QVariant, itemH));
}

QString MainWindow::getTop(int index) {
  QQuickItem *root = mw_one->ui->qwMainTab->rootObject();
  QVariant itemTime;
  QMetaObject::invokeMethod((QObject *)root, "getTop",
                            Q_RETURN_ARG(QVariant, itemTime),
                            Q_ARG(QVariant, index));
  return itemTime.toString();
}

QString MainWindow::getText0(int index) {
  QQuickItem *root = mw_one->ui->qwMainTab->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject *)root, "getText0",
                            Q_RETURN_ARG(QVariant, item),
                            Q_ARG(QVariant, index));
  return item.toString();
}

QString MainWindow::getText1(int index) {
  QQuickItem *root = mw_one->ui->qwMainTab->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject *)root, "getText1",
                            Q_RETURN_ARG(QVariant, item),
                            Q_ARG(QVariant, index));
  return item.toString();
}

QString MainWindow::getText2(int index) {
  QQuickItem *root = mw_one->ui->qwMainTab->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject *)root, "getText2",
                            Q_RETURN_ARG(QVariant, item),
                            Q_ARG(QVariant, index));
  return item.toString();
}

int MainWindow::getItemType(int index) {
  QQuickItem *root = mw_one->ui->qwMainTab->rootObject();
  QVariant itemType;
  QMetaObject::invokeMethod((QObject *)root, "getType",
                            Q_RETURN_ARG(QVariant, itemType),
                            Q_ARG(QVariant, index));
  return itemType.toInt();
}

void MainWindow::delItem(int index) {
  QQuickItem *root = mw_one->ui->qwMainTab->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "delItem", Q_ARG(QVariant, index));
}

int MainWindow::getCount() {
  QQuickItem *root = mw_one->ui->qwMainTab->rootObject();
  QVariant itemCount;
  QMetaObject::invokeMethod((QObject *)root, "getItemCount",
                            Q_RETURN_ARG(QVariant, itemCount));
  return itemCount.toInt();
}

void MainWindow::clearAll() {
  int count = getCount();
  for (int i = 0; i < count; i++) {
    delItem(0);
  }
}

void MainWindow::setCurrentIndex(int index) {
  QQuickItem *root = mw_one->ui->qwMainTab->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "setCurrentItem",
                            Q_ARG(QVariant, index));
}

void MainWindow::gotoEnd() {
  QQuickItem *root = mw_one->ui->qwMainTab->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "gotoEnd");
}

void MainWindow::gotoIndex(int index) {
  QQuickItem *root = mw_one->ui->qwMainTab->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "gotoIndex",
                            Q_ARG(QVariant, index));
}

int MainWindow::getCurrentIndex() {
  QQuickItem *root = mw_one->ui->qwMainTab->rootObject();
  QVariant itemIndex;
  QMetaObject::invokeMethod((QObject *)root, "getCurrentIndex",
                            Q_RETURN_ARG(QVariant, itemIndex));
  return itemIndex.toInt();
}

void MainWindow::setScrollBarPos(double pos) {
  QQuickItem *root = mw_one->ui->qwMainTab->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "setScrollBarPos",
                            Q_ARG(QVariant, pos));
}

void MainWindow::reloadMain() {
  bool isAniEffects;
  if (isDelItem || isEditItem)
    isAniEffects = false;
  else
    isAniEffects = true;

  ui->qwMainDate->rootContext()->setContextProperty("isAniEffects",
                                                    isAniEffects);
  ui->qwMainDate->rootContext()->setContextProperty("maindateWidth",
                                                    ui->qwMainDate->width());
  m_Method->clearAllBakList(ui->qwMainDate);

  // QFontMetrics fontMetrics(font());
  // int nFontHeight = fontMetrics.height();

  QTreeWidget *tw = get_tw(tabData->currentIndex());

  int total = tw->topLevelItemCount();

  if (total == 0) {
    m_Method->clearAllBakList(ui->qwMainEvent);
    return;
  }

  int a;

  if (total - days > 0)
    a = total - days;
  else
    a = 0;

  QString text0, text1, text2, text3, topitem;
  for (int i = a; i < total; i++) {
    QTreeWidgetItem *topItem = tw->topLevelItem(i);

    text0 = topItem->text(0) + "  " + topItem->text(3);
    text1 = topItem->text(1);
    text2 = topItem->text(2);

    topitem = text0;

    m_Method->addItemToQW(ui->qwMainDate, text0, text1, text2, text3, 0);
  }

  m_Method->gotoEnd(ui->qwMainDate);
  int count = m_Method->getCountFromQW(ui->qwMainDate);
  m_Method->setCurrentIndexFromQW(ui->qwMainDate, count - 1);
  m_Method->setScrollBarPos(ui->qwMainDate, 1.0);

  m_Method->clickMainDate();
}

void MainWindow::reeditData() {
  if (setTWCurrentItem()) {
    on_twItemDoubleClicked();
  }
}

void MainWindow::clickData() {
  if (setTWCurrentItem()) on_twItemClicked();
}

bool MainWindow::setTWCurrentItem() {
  bool isSel = false;
  int row = getCurrentIndex();
  if (row < 0) return false;

  // type==0 child; type==1 top
  int type = getItemType(row);

  QString textTop = getTop(row);
  QString text0 = getText0(row);
  QStringList list = text0.split(".");
  int childIndex = 0;
  if (list.count() > 0) {
    childIndex = list.at(0).toInt() - 1;
  }

  if (type == 0) {
    if (childIndex < 0) return false;
  }

  QTreeWidget *tw = get_tw(tabData->currentIndex());
  int count = tw->topLevelItemCount();
  for (int i = 0; i < count; i++) {
    QTreeWidgetItem *topItem = tw->topLevelItem(count - 1 - i);
    if (topItem->text(0) + "  " + topItem->text(3) == textTop) {
      if (type == 0) {
        QTreeWidgetItem *childItem = topItem->child(childIndex);
        tw->setCurrentItem(childItem, 0);
      }

      if (type == 1) {
        tw->setCurrentItem(topItem);
      }

      isSel = true;
      break;
    }
  }

  return isSel;
}

void MainWindow::on_btnBackSteps_clicked() { m_Steps->on_btnBack_clicked(); }

void MainWindow::on_btnReset_clicked() { m_Steps->on_btnReset_clicked(); }

void MainWindow::on_btnBack_Report_clicked() { m_Report->on_btnBack_clicked(); }

void MainWindow::on_btnYear_clicked() { m_Report->on_btnYear_clicked(); }

void MainWindow::on_btnMonth_clicked() { m_Report->on_btnMonth_clicked(); }

void MainWindow::on_btnCategory_clicked() {
  m_Report->on_btnCategory_clicked();
}

void MainWindow::on_btnSync_clicked() { on_btnUpload_clicked(); }

void MainWindow::on_btnPDF_clicked() { m_Notes->on_btnPDF_clicked(); }

void MainWindow::on_btnPasteTodo_clicked() { ui->editTodo->paste(); }

int MainWindow::getMaxDay(QString sy, QString sm) {
  int maxDay = 0;
  for (int i = 0; i < 50; i++) {
    QString strdate = sy + "-" + sm + "-" + QString::number(i + 1);
    QDate date = QDate::fromString(strdate, "yyyy-M-d");
    if (date.dayOfWeek() == 0) {
      maxDay = i;
      break;
    }
  }

  return maxDay;
}

void MainWindow::on_btnStartDate_clicked() {
  m_DateSelector->initStartEndDate("start");
}

void MainWindow::on_btnEndDate_clicked() {
  m_DateSelector->initStartEndDate("end");
}

void MainWindow::on_btnBackSearch_clicked() {
  clearWidgetFocus();
  ui->frameSearch->hide();
  ui->frameMain->show();
}

void MainWindow::on_btnClearSearchText_clicked() {
  ui->editSearchText->setText("");
  ui->editSearchText->setFocus();
}

void MainWindow::on_btnStartSearch_clicked() {
  searchStr = ui->editSearchText->text().trimmed();
  if (searchStr.length() == 0) return;

  showProgress();
  mySearchThread->start();
}

void MainWindow::on_btnBackBakList_clicked() {
  ui->frameBakList->hide();
  ui->frameMain->show();
}

void MainWindow::on_btnImportBakList_clicked() {
  if (m_Method->getCountFromQW(ui->qwBakList) == 0) return;

  int cur_index = m_Method->getCurrentIndexFromQW(ui->qwBakList);
  QString str = m_Method->getText3(ui->qwBakList, cur_index);
  zipfile = str.trimmed();

  if (!zipfile.isNull()) {
    m_Method->m_widget = new QWidget(mw_one);
    ShowMessage *m_ShowMsg = new ShowMessage(this);
    if (!m_ShowMsg->showMsg("Kont",
                            tr("Import this data?") + "\n" +
                                mw_one->m_Reader->getUriRealPath(zipfile),
                            2)) {
      isZipOK = false;
      return;
    }
  }

  isZipOK = true;
  ui->btnBackBakList->click();
  showProgress();

  isMenuImport = true;
  isDownData = false;

  myImportDataThread->start();
}

void MainWindow::on_btnOkViewCate_clicked() { m_Report->on_CateOk(); }

void MainWindow::on_btnBackTabRecycle_clicked() {
  ui->frameTabRecycle->hide();
  ui->frameMain->show();
}

void MainWindow::on_btnDelTabRecycle_clicked() {
  if (m_Method->getCountFromQW(ui->qwTabRecycle) == 0) return;
  int index = m_Method->getCurrentIndexFromQW(ui->qwTabRecycle);
  QString tab_file = m_Method->getText3(ui->qwTabRecycle, index);

  m_Method->m_widget = new QWidget(mw_one);
  ShowMessage *m_ShowMsg = new ShowMessage(this);
  if (!m_ShowMsg->showMsg("Knot",
                          tr("Whether to remove") + "  " + tab_file + " ? ", 2))
    return;

  QStringList list = tab_file.split("\n");
  QString rec_file;
  if (list.count() > 1) {
    for (int i = 0; i < list.count(); i++) {
      rec_file = list.at(i);
      QFile file(rec_file);
      file.remove();
    }
  } else {
    rec_file = tab_file;
    QFile file(rec_file);
    file.remove();
  }

  m_Method->delItemFromQW(ui->qwTabRecycle, index);

  ui->lblTitleTabRecycle->setText(
      tr("Tab Recycle") + "    " + tr("Total") + " : " +
      QString::number(m_Method->getCountFromQW(ui->qwTabRecycle)));
}

void MainWindow::on_btnRestoreTab_clicked() {
  if (m_Method->getCountFromQW(ui->qwTabRecycle) == 0) return;

  int count = ui->tabWidget->tabBar()->count();
  QString twName = m_Notes->getDateTimeStr() + "_" + QString::number(count + 1);

  int c_year = QDate::currentDate().year();
  int iniFileCount = c_year - 2025 + 1 + 1;

  int index = m_Method->getCurrentIndexFromQW(ui->qwTabRecycle);
  QString recycle = m_Method->getText3(ui->qwTabRecycle, index);
  QStringList recycleList = recycle.split("\n");

  QString ini_file;
  for (int i = 0; i < iniFileCount; i++) {
    if (i == 0)
      ini_file = iniDir + twName + ".ini";
    else {
      ini_file = iniDir + QString::number(2025 + i - 1) + "-" + twName + ".ini";
    }

    if (QFile(ini_file).exists()) QFile(ini_file).remove();
    QString recFile;
    if (recycleList.count() > 1)
      recFile = recycleList.at(i);
    else
      recFile = recycle;
    QFile::copy(recFile, ini_file);
  }

  QString tab_name = m_Method->getText0(ui->qwTabRecycle, index);
  QTreeWidget *tw = init_TreeWidget(twName);
  ui->tabWidget->addTab(tw, tab_name);

  addItem(tab_name, "", "", "", 0);
  setCurrentIndex(count);

  readData(tw);

  if (recycleList.count() > 1) {
    for (int i = 0; i < recycleList.count(); i++) {
      QFile recycle_file(recycle.split("\n").at(i));
      recycle_file.remove();
    }
  } else {
    QFile recycle_file(recycle);
    recycle_file.remove();
  }

  on_btnBackTabRecycle_clicked();

  saveTab();

  reloadMain();
  clickData();

  tabData->setCurrentIndex(count);

  strLatestModify = tr("Restore Tab") + "(" + tab_name + ")";
}

void MainWindow::on_btnDelBakFile_clicked() {
  if (m_Method->getCountFromQW(ui->qwBakList) == 0) return;

  int index = m_Method->getCurrentIndexFromQW(ui->qwBakList);
  QString bak_file = m_Method->getText3(ui->qwBakList, index);

  m_Method->m_widget = new QWidget(mw_one);
  ShowMessage *m_ShowMsg = new ShowMessage(this);
  if (!m_ShowMsg->showMsg("Knot",
                          tr("Whether to remove") + "  " + bak_file + " ? ", 2))
    return;

  QFile file(bak_file);
  file.remove();
  m_Method->delItemFromQW(ui->qwBakList, index);

  int newIndex = index - 1;
  if (newIndex < 0) newIndex = 0;

  m_Method->setCurrentIndexFromQW(ui->qwBakList, newIndex);

  ui->lblBakListTitle->setText(
      tr("Backup File List") + "    " + tr("Total") + " : " +
      QString::number(m_Method->getCountFromQW(ui->qwBakList)));
}

void MainWindow::on_btnBackNoteList_clicked() {
  clearWidgetFocus();
  ui->frameNoteList->hide();
  ui->frameMain->show();
  m_NotesList->saveNoteBookVPos();
  m_NotesList->saveCurrentNoteInfo();
  m_NotesList->saveNotesListIndex();
  m_Notes->updateMainnotesIniToSyncLists();

  m_Notes->syncToWebDAV();
}

void MainWindow::on_btnBackNoteRecycle_clicked() {
  ui->frameNoteRecycle->hide();
  ui->frameNoteList->show();

  if (ui->chkAutoSync->isChecked() && ui->chkWebDAV->isChecked()) {
    int count = m_NotesList->needDelWebDAVFiles.count();
    if (count > 0) {
      QStringList files;
      for (int i = 0; i < count; i++) {
        QString file = m_NotesList->needDelWebDAVFiles.at(i);
        file = file.replace(iniDir, "KnotData/");
        files.append(file);
      }
      m_CloudBackup->deleteWebDAVFiles(files);
    }
  }
}

void MainWindow::on_btnNoteRecycle_clicked() {
  ui->frameNoteList->hide();
  ui->frameNoteRecycle->show();

  m_NotesList->loadAllRecycle();
}

void MainWindow::on_btnDelNoteRecycle_clicked() {
  int count = m_Method->getCountFromQW(ui->qwNoteRecycle);
  if (count == 0) return;

  int index = m_Method->getCurrentIndexFromQW(ui->qwNoteRecycle);
  if (index < 0) return;

  m_NotesList->setTWRBCurrentItem();
  m_NotesList->on_btnDel_Recycle_clicked();
}

void MainWindow::on_btnRestoreNoteRecycle_clicked() {
  int count = m_Method->getCountFromQW(ui->qwNoteRecycle);
  if (count == 0) return;

  int index = m_Method->getCurrentIndexFromQW(ui->qwNoteRecycle);
  if (index < 0) return;

  if (m_NotesList->getNoteBookCount() == 0) return;

  m_NotesList->setTWRBCurrentItem();
  m_NotesList->on_btnRestore_clicked();
}

void MainWindow::on_btnFindNotes_clicked() {
  showProgress();
  m_NotesList->startFind(ui->editFindNote->text().trimmed());
}

void MainWindow::on_btnFindPreviousNote_clicked() { m_NotesList->goPrevious(); }

void MainWindow::on_btnFindNextNote_clicked() { m_NotesList->goNext(); }

void MainWindow::on_btnClearNoteFindText_clicked() {
  ui->editFindNote->setText("");
  ui->lblFindNoteCount->setText("0");
  ui->btnFindNextNote->setEnabled(false);
  ui->btnFindPreviousNote->setEnabled(false);
  ui->lblShowLineSn->setText("0");
}

void MainWindow::on_btnShowFindNotes_clicked() {
  m_NotesList->recycleNotesList.clear();
  int count = m_NotesList->ui->treeWidgetRecycle->topLevelItem(0)->childCount();
  for (int i = 0; i < count; i++) {
    QString file =
        iniDir +
        m_NotesList->ui->treeWidgetRecycle->topLevelItem(0)->child(i)->text(1);
    m_NotesList->recycleNotesList.append(file);
  }
  qDebug() << "recycle notes = " << m_NotesList->recycleNotesList;

  ui->frameNoteList->hide();
  ui->frameNotesSearchResult->show();
  ui->editNotesSearch->setFocus();

  m_NotesList->openSearch();
}

void MainWindow::on_btnNoteBookMenu_clicked() {
  m_Method->showNoteBookMenu(ui->qwNoteBook->x(), ui->qwNoteBook->y());
}

void MainWindow::on_btnNoteMenu_clicked() {
  m_Method->showNotsListMenu(ui->qwNoteList->x(), ui->qwNoteList->y());
}

void MainWindow::on_btnCancelType_clicked() {
  m_CategoryList->on_btnCancel_clicked();
}

void MainWindow::on_btnOkType_clicked() { m_CategoryList->on_btnOk_clicked(); }

void MainWindow::on_btnDelType_clicked() {
  m_CategoryList->on_btnDel_clicked();
}

void MainWindow::on_btnRenameType_clicked() {
  m_CategoryList->ui->editRename->setText(ui->editRenameType->text().trimmed());
  m_CategoryList->on_btnRename_clicked();
}

void MainWindow::on_btnBackSetTab_clicked() {
  ui->frameSetTab->hide();
  ui->frameMain->show();

  if (ui->btnTabMoveDown->isHidden()) {
    ui->btnTabMoveDown->show();
    ui->btnTabMoveUp->show();
    on_btnAdd_clicked();
    m_EditRecord->setCurrentValue();
  }
}

void MainWindow::on_btnBackEditRecord_clicked() {
  clearWidgetFocus();

  ui->frameEditRecord->hide();
  ui->frameMain->show();
}

void MainWindow::on_btnType_clicked() { m_EditRecord->on_btnCustom_clicked(); }

void MainWindow::on_btnOkEditRecord_clicked() {
  m_EditRecord->on_btnOk_clicked();
}

void MainWindow::on_btnClearType_clicked() { ui->editCategory->setText(""); }

void MainWindow::on_btnClearDetails_clicked() { ui->editDetails->setText(""); }

void MainWindow::on_btnClearAmount_clicked() { ui->editAmount->setText(""); }

void MainWindow::on_editAmount_textChanged(const QString &arg1) {
  m_EditRecord->on_editAmount_textChanged(arg1);
}

void MainWindow::on_editCategory_textChanged(const QString &arg1) {
  m_EditRecord->on_editCategory_textChanged(arg1);
}

void MainWindow::on_editDetails_textChanged() {
  m_EditRecord->on_editDetails_textChanged();
}

void MainWindow::on_hsH_valueChanged(int value) {
  m_EditRecord->on_hsH_valueChanged(value);
}

void MainWindow::on_hsM_valueChanged(int value) {
  m_EditRecord->on_hsM_valueChanged(value);
}

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

void MainWindow::on_btnBackBookList_clicked() {
  if (isPDF) {
    if (isAndroid) {
      ui->frameBookList->hide();
      ui->frameMain->show();
    }
  } else {
    ui->frameBookList->hide();
    ui->frameReader->show();
  }
}

void MainWindow::on_btnOkBookList_clicked() { m_Reader->openBookListItem(); }

void MainWindow::on_btnClearAllRecords_clicked() {
  m_Reader->clearAllReaderRecords();
}

void MainWindow::on_btnAnd_clicked() { ui->editSearchText->insert("&"); }

void MainWindow::on_btnClear_clicked() { ui->editTodo->clear(); }

void MainWindow::on_btnModify_clicked() { m_Todo->reeditText(); }

void MainWindow::on_btnChartMonth_clicked() {
  isTabChanged = true;
  tabChart->setCurrentIndex(0);
  m_Method->setToolButtonQss(ui->btnChartMonth, 5, 3, "#FF0000", "#FFFFFF",
                             "#FF0000", "#FFFFFF", "#FF5555", "#FFFFFF");
  m_Method->setToolButtonQss(ui->btnChartDay, 5, 3, "#455364", "#FFFFFF",
                             "#455364", "#FFFFFF", "#555364", "#FFFFFF");
}

void MainWindow::on_btnChartDay_clicked() {
  isTabChanged = true;
  tabChart->setCurrentIndex(1);
  m_Method->setToolButtonQss(ui->btnChartDay, 5, 3, "#FF0000", "#FFFFFF",
                             "#FF0000", "#FFFFFF", "#FF5555", "#FFFFFF");
  m_Method->setToolButtonQss(ui->btnChartMonth, 5, 3, "#455364", "#FFFFFF",
                             "#455364", "#FFFFFF", "#555364", "#FFFFFF");
}

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
    addItem(tabText, "", "", "", 0);
  }
  setCurrentIndex(tabData->currentIndex());
}

void MainWindow::on_btnChart_clicked() {
  axisY->setTickCount(7);
  axisY2->setTickCount(7);

  if (ui->f_charts->isHidden()) {
    ui->qwMainDate->hide();
    ui->qwMainEvent->hide();

    ui->f_charts->setMaximumHeight(this->height());
    ui->f_charts->show();
    ui->btnChartDay->show();
    ui->btnChartMonth->show();
    ui->rbAmount->show();
    ui->rbFreq->show();
    ui->rbSteps->show();
    ui->f_cw->show();

    ui->btnReport->hide();
    ui->btnFind->hide();
    ui->btnModifyRecord->hide();
    ui->btnMove->hide();
  } else {
    ui->f_charts->setMaximumHeight(0);
    ui->f_charts->hide();
    ui->rbAmount->hide();
    ui->rbFreq->hide();
    ui->rbSteps->hide();
    ui->btnChartDay->hide();
    ui->btnChartMonth->hide();

    ui->qwMainDate->show();
    ui->qwMainEvent->show();
    ui->btnReport->show();
    ui->btnFind->show();
    ui->btnModifyRecord->show();
    ui->btnMove->show();
  }
}

void MainWindow::on_btnManagement_clicked() {
  int x, y, w, h;
  x = geometry().x();
  y = geometry().y();
  w = width();
  h = height();
  m_NotesList->setGeometry(x, y, w, h);
  m_NotesList->show();
  m_NotesList->tw->setFocus();
}

void MainWindow::on_btnUpMove_clicked() {
  m_NotesList->setTWCurrentItem();
  m_NotesList->on_btnUp_clicked();
}

void MainWindow::on_btnDownMove_clicked() {
  m_NotesList->setTWCurrentItem();
  m_NotesList->on_btnDown_clicked();
}

void MainWindow::on_btnDelNote_NoteBook_clicked() {
  m_NotesList->setTWCurrentItem();
  m_NotesList->on_btnDel_clicked();
}

void MainWindow::on_btnMoveTo_clicked() {
  m_NotesList->setTWCurrentItem();
  m_NotesList->on_btnMoveTo_clicked();
}

void MainWindow::on_btnBack_Tree_clicked() {
  ui->frameNotesTree->hide();
  ui->frameNoteList->show();
}

void MainWindow::on_btnRename_clicked() {
  if (m_NotesList->getNoteBookCurrentIndex() < 0) return;

  m_NotesList->setTWCurrentItem();
  m_NotesList->on_btnRename_clicked();
}

void MainWindow::on_btnHideFind_clicked() { ui->f_FindNotes->hide(); }

void MainWindow::on_btnStepsOptions_clicked() {
  mw_one->m_StepsOptions->init();
}

void MainWindow::on_btnRecentOpen_clicked() {
  m_NotesList->genRecentOpenMenu();
}

void MainWindow::on_btnMenuReport_clicked() { m_Report->genReportMenu(); }

void MainWindow::on_btnCatalogue_clicked() {
  ui->btnAutoStop->click();

  if (ui->f_ReaderSet->isVisible()) {
    on_btnBackReaderSet_clicked();
  }
  m_Reader->showCatalogue();
}

void MainWindow::on_btnRemoveBookList_clicked() { m_Reader->removeBookList(); }

void MainWindow::on_btnShowBookmark_clicked() {
  ui->btnAutoStop->click();

  if (ui->f_ReaderSet->isVisible()) {
    on_btnBackReaderSet_clicked();
  }
  if (ui->qwBookmark->isHidden()) {
    ui->qwReader->hide();
    ui->qwBookmark->show();
    m_Reader->showBookmarkList();
    ui->btnCatalogue->setEnabled(false);
  } else {
    ui->qwBookmark->hide();
    ui->qwReader->show();
    ui->btnCatalogue->setEnabled(true);
  }
}

void MainWindow::stopTimerForPdf() {
  m_Reader->tmeShowEpubMsg->stop();
  ui->pEpubProg->hide();
  ui->lblEpubInfo->hide();
}

void MainWindow::on_btnShareImage_clicked() {
  m_ReceiveShare->shareImage(tr("Share to"), imgFileName, "image/png");
}

void MainWindow::on_btnHideKey_clicked() { pAndroidKeyboard->hide(); }

void MainWindow::on_btnDelImage_clicked() { m_Notes->delImage(); }

void MainWindow::on_btnBackReaderSet_clicked() {
  ui->f_ReaderSet->hide();
  qreal pos = m_Reader->getVPos();
  m_Reader->setVPos(pos + 0.01);
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

void MainWindow::on_editBackgroundColor_textChanged(const QString &arg1) {
  m_ReaderSet->on_editBackgroundColor_textChanged(arg1);
}

void MainWindow::on_editForegroundColor_textChanged(const QString &arg1) {
  m_ReaderSet->on_editForegroundColor_textChanged(arg1);
}

void MainWindow::on_btnStyle1_clicked() { m_ReaderSet->on_btnStyle1_clicked(); }

void MainWindow::on_btnStyle2_clicked() { m_ReaderSet->on_btnStyle2_clicked(); }

void MainWindow::on_btnStyle3_clicked() { m_ReaderSet->on_btnStyle3_clicked(); }

void MainWindow::on_btnGoPage_clicked() { m_ReaderSet->on_btnGoPage_clicked(); }

void MainWindow::on_hSlider_sliderReleased() {
  m_ReaderSet->on_hSlider_sliderReleased();
}

void MainWindow::on_DelayCloseProgressBar() {
  QTimer::singleShot(200, this, SLOT(on_CloseProgressBar()));
}

void MainWindow::on_CloseProgressBar() {
  mw_one->closeProgress();

  mw_one->ui->btnReader->setEnabled(true);
  mw_one->ui->f_ReaderFun->setEnabled(true);
}

void MainWindow::on_btnShareBook_clicked() { m_Reader->shareBook(); }

void MainWindow::on_btnAutoRun_clicked() {
  m_Reader->tmeAutoRun->start(50);
  ui->btnAutoRun->hide();
  ui->btnAutoStop->show();
}

void MainWindow::on_btnAutoStop_clicked() {
  m_Reader->tmeAutoRun->stop();
  ui->btnAutoStop->hide();
  ui->btnAutoRun->show();
}

void MainWindow::on_btnLessen_clicked() { m_ReaderSet->on_btnLessen_clicked(); }

void MainWindow::on_btnDefault_clicked() {
  m_ReaderSet->on_btnDefault_clicked();
}

void MainWindow::on_btnPlus_clicked() { m_ReaderSet->on_btnAdd_clicked(); }

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

void MainWindow::on_StartRecordAudio() {
  tmeStartRecordAudio->stop();
  if (m_Method->checkRecordAudio() == 0) {
    m_Method->showToastMessage(tr("Please enable permission to record audio!"));
    return;
  }
  m_Todo->startRecordVoice();
}

void MainWindow::on_sliderPlayAudio_sliderPressed() {
  m_Todo->tmePlayProgress->stop();
  m_Method->pausePlay();
}

void MainWindow::on_sliderPlayAudio_sliderReleased() {
  QString strPos = QString::number(ui->sliderPlayAudio->value());
  m_Method->seekTo(strPos);
  m_Method->startPlay();
  m_Todo->tmePlayProgress->start(m_Todo->nInterval);
}

void MainWindow::on_btnMove_clicked() {
  isMoveEntry = true;
  if (del_Data((QTreeWidget *)ui->tabWidget->currentWidget())) {
    ui->btnTabMoveDown->hide();
    ui->btnTabMoveUp->hide();
    on_btnSelTab_clicked();

    while (ui->frameEditRecord->isHidden())
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    ui->editCategory->setText(strCategory);
    ui->editDetails->setText(strDetails);
    ui->editAmount->setText(strAmount);

    on_btnOkEditRecord_clicked();
  }
}

void MainWindow::on_btnGPS_clicked() {
  if (ui->btnGPS->text() == tr("Start")) {
    m_Steps->startRecordMotion();

  } else if (ui->btnGPS->text() == tr("Stop")) {
    m_Steps->stopRecordMotion();
    ui->btnGPS->setText(tr("Start"));
  }
}

void MainWindow::on_btnSelGpsDate_clicked() { m_Steps->selGpsListYearMonth(); }

void MainWindow::on_btnGetGpsListData_clicked() {
  m_Steps->getGpsListDataFromYearMonth();
}

void MainWindow::on_rbCycling_clicked() {}

void MainWindow::on_rbHiking_clicked() {}

void MainWindow::on_rbRunning_clicked() {}

void MainWindow::on_btnOpenNote_clicked() {
  if (!QFile::exists(currentMDFile)) return;

  m_NotesList->setCurrentItemFromMDFile(currentMDFile);

  QString title = m_NotesList->noteTitle;
  m_NotesList->refreshRecentOpen(title);
  m_NotesList->saveRecentOpen();

  if (isAndroid) {
    m_Method->setMDTitle(title);

    m_Method->setMDFile(currentMDFile);
    m_Notes->openMDWindow();

    m_Notes->setAndroidNoteConfig("/cpos/currentMDFile",
                                  QFileInfo(currentMDFile).baseName());

    return;
  } else {
    m_Notes->MD2Html(currentMDFile);
    m_Notes->openBrowserOnce(privateDir + "memo.html");

    // m_Notes->loadNoteToQML();
    // ui->frameNoteList->hide();
    // ui->frameNotes->show();
  }
}

void MainWindow::on_btnEditNote_clicked() {
  m_NotesList->setCurrentItemFromMDFile(currentMDFile);
  m_Notes->openEditUI();
}

void MainWindow::on_btnToPDF_clicked() {
  if (!QFile::exists(currentMDFile)) return;
  ui->btnPDF->click();
}

void MainWindow::on_btnRecentOpen0_clicked() { on_btnRecentOpen_clicked(); }

void MainWindow::on_btnWebBack_clicked() {}

void MainWindow::on_btnWebDAVBackup_clicked() {
  if (!ui->btnReader->isEnabled()) return;
  m_CloudBackup->startBakData();
}

void MainWindow::on_btnWebDAVRestore_clicked() {
  QString filePath;
  filePath = bakfileDir + "memo.zip";

  if (QFile(filePath).exists()) QFile(filePath).remove();
  if (filePath.isEmpty()) return;

  ShowMessage *m_ShowMsg = new ShowMessage(this);
  if (!m_ShowMsg->showMsg(
          "WebDAV",
          tr("Downloading data?") + "\n\n" +
              tr("This action overwrites local files with files in the cloud."),
          2))
    return;
  m_CloudBackup->WEBDAV_URL = ui->editWebDAV->text().trimmed();
  m_CloudBackup->USERNAME = ui->editWebDAVUsername->text().trimmed();
  m_CloudBackup->APP_PASSWORD = ui->editWebDAVPassword->text().trimmed();
  m_CloudBackup->downloadFile("Knot/memo.zip", filePath);
  mw_one->ui->progressBar->setValue(0);
}

void MainWindow::on_chkWebDAV_clicked() {
  if (ui->chkWebDAV->isChecked())
    ui->chkOneDrive->setChecked(false);
  else
    ui->chkOneDrive->setChecked(true);
}

void MainWindow::on_chkOneDrive_clicked() {
  if (ui->chkOneDrive->isChecked())
    ui->chkWebDAV->setChecked(false);
  else
    ui->chkWebDAV->setChecked(true);
}

void MainWindow::on_btnBack_NotesSearchResult_clicked() {
  clearWidgetFocus();
  ui->frameNotesSearchResult->hide();
  ui->frameNoteList->show();
  isOpenSearchResult = false;
}

void MainWindow::on_editFindNote_returnPressed() { on_btnFindNotes_clicked(); }

void MainWindow::on_btnClearSearchResults_clicked() {
  ui->editNotesSearch->clear();
  ui->editNotesSearch->setFocus();
}

void MainWindow::on_btnOpenSearchResult_clicked() {
  QString mdFile = m_NotesList->getSearchResultQmlFile();
  if (!QFile::exists(mdFile)) return;
  isOpenSearchResult = true;
  currentMDFile = mdFile;
  mySearchText = ui->editNotesSearch->text().trimmed();
  on_btnEditNote_clicked();
}

void MainWindow::on_btnFindNotes2_clicked() {
  if (ui->f_FindNotes->isHidden())
    ui->f_FindNotes->show();
  else
    ui->f_FindNotes->hide();
}

void MainWindow::on_btnOpenEditFind_clicked() {
  isOpenSearchResult = true;
  mySearchText = ui->editFindNote->text().trimmed();
  on_btnEditNote_clicked();
}

void MainWindow::setEncSyncStatusTip() {
  ui->lblStats->setStyleSheet(labelNormalStyleSheet);

  if (m_Preferences->ui->chkZip->isChecked() && ui->chkAutoSync->isChecked() &&
      ui->chkWebDAV->isChecked())
    ui->lblStats->setStyleSheet(labelEnSyncStyleSheet);

  if (m_Preferences->ui->chkZip->isChecked() && !ui->chkAutoSync->isChecked() &&
      !ui->chkWebDAV->isChecked())
    ui->lblStats->setStyleSheet(labelEncStyleSheet);

  if (m_Preferences->ui->chkZip->isChecked() && !ui->chkAutoSync->isChecked() &&
      ui->chkWebDAV->isChecked())
    ui->lblStats->setStyleSheet(labelEncStyleSheet);

  if (m_Preferences->ui->chkZip->isChecked() && ui->chkAutoSync->isChecked() &&
      !ui->chkWebDAV->isChecked())
    ui->lblStats->setStyleSheet(labelEncStyleSheet);

  if (!m_Preferences->ui->chkZip->isChecked() && ui->chkAutoSync->isChecked() &&
      ui->chkWebDAV->isChecked())
    ui->lblStats->setStyleSheet(labelSyncStyleSheet);

  if (isAndroid) ui->lblVer->hide();
  ui->lblVer->setText("Knot   V:" + ver);
  ui->lblVer->setStyleSheet(ui->lblStats->styleSheet());
}

void MainWindow::on_btnTools_clicked() {
  if (ui->f_Tools->isHidden())
    ui->f_Tools->show();
  else
    ui->f_Tools->hide();
}
