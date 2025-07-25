#include "MainWindow.h"

#include "ui_MainWindow.h"

QString ver = "2.0.20";
QString appName = "Knot";

QString iniFile, iniDir, privateDir, bakfileDir, strDate, readDate, noteText,
    strStats, SaveType, strY, strM, btnYText, btnMText, btnDText, errorInfo,
    CurrentYearMonth, zipfile, txt, searchStr, currentMDFile, copyText,
    imgFileName, defaultFontFamily, customFontFamily, encPassword;

QList<QPointF> PointList;
QList<double> doubleList;
QStringList listM;

QSettings *iniPreferences;
CloudBackup *m_CloudBackup;
QTreeWidgetItem *parentItem;
MainWindow *mw_one;
Ui::MainWindow *mui;
Method *m_Method;
QTabWidget *tabData, *tabChart;

bool isrbFreq = true;
bool isEBook, isReport, isUpData, isZipOK, isMenuImport, isDownData, isEncrypt,
    isRemovedTopItem;
bool isAdd = false;
bool loading, isReadEnd, isReadTWEnd;
bool isReadEBookEnd = true;
bool isSaveEnd = true;
bool isBreak = false;
bool isDark = false;
bool isDelData = false;

int today, fontSize, red, currentTabIndex;

int chartMax = 5;

double yMaxMonth, yMaxDay;

QRegularExpression regxNumber("^-?[0-9.]*$");

extern bool isAndroid, isIOS, zh_cn, isEpub, isEpubError, isText, isPDF,
    isWholeMonth, isDateSection, isPasswordError, isInitThemeEnd,
    isNeedExecDeskShortcut;
extern QString btnYearText, btnMonthText, strPage, ebookFile, strTitle,
    fileName, strOpfPath, catalogueFile, strShowMsg;
extern QStringList readTextList, htmlFiles, listCategory;
extern int iPage, sPos, totallines, baseLines, htmlIndex, s_y1, s_m1, s_d1,
    s_y2, s_m2, s_d2, totalPages, currentPage;

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
extern CategoryList *m_CategoryList;
extern ReaderSet *m_ReaderSet;
extern ColorDialog *colorDlg;
extern PrintPDF *m_PrintPDF;

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
    mui->lblTitle->setText(tabData->tabText(tabData->currentIndex()));

    mui->btnCategory->hide();
    if (listCategory.count() > 0) mui->btnCategory->setHidden(false);

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

  mui->actionImport_Data->setEnabled(true);
  mui->actionExport_Data->setEnabled(true);
  mui->actionDel_Tab->setEnabled(true);
  mui->actionAdd_Tab->setEnabled(true);
  mui->actionView_App_Data->setEnabled(true);
  isReadTWEnd = true;

  mui->progBar->setMaximum(100);
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
    mui->lblStats->setText(strShowDetails);
  else
    mui->lblStats->setText(strStats);
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

  mui->progBar->setMaximum(100);

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
  mui = ui;
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
  init_Thread_Timer();
  m_MainHelper->init_UIWidget();
  init_ChartWidget();

  m_MainHelper->init_Theme();
  m_MainHelper->initQW();

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

  m_CloudBackup->init_CloudBacup();
  m_Preferences->setEncSyncStatusTip();

  if (QFile::exists(currentMDFile)) {
    m_Notes->MD2Html(currentMDFile);
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
}

void MainWindow::init_ChartWidget() {
  mui->centralwidget->layout()->setContentsMargins(1, 0, 1, 2);
  mui->centralwidget->layout()->setSpacing(1);
  mui->f_charts->setContentsMargins(0, 0, 0, 0);

  mui->f_charts->layout()->setContentsMargins(0, 0, 0, 0);
  mui->f_charts->layout()->setSpacing(0);
  frameChartHeight = 105;
  mui->f_charts->setFixedHeight(frameChartHeight);
  tabChart->setCurrentIndex(0);

  mui->glMonth->layout()->setContentsMargins(0, 0, 0, 0);
  mui->glMonth->layout()->setSpacing(0);
  mui->glDay->layout()->setContentsMargins(0, 0, 0, 0);
  mui->glDay->layout()->setSpacing(0);

  mui->f_charts->hide();
  mui->btnChartDay->hide();
  mui->btnChartMonth->hide();
  mui->rbAmount->hide();
  mui->rbFreq->hide();
  mui->rbSteps->hide();

  int a0 = 0;
  int a1 = -2;
  // Month
  chartMonth = new QChart();
  chartview = new QChartView(chartMonth);
  chartview->installEventFilter(this);
  mui->glMonth->addWidget(chartview);
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
  mui->glDay->addWidget(chartview1);
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
    QTreeWidget *tw = m_MainHelper->init_TreeWidget(name);

    QString tabText = RegTab
                          .value("TabName" + QString::number(i),
                                 tr("Tab") + QString::number(i + 1))
                          .toString();
    mui->tabWidget->addTab(tw, tabText);

    addItem(tabText, "", "", "", 0);

    RegTab.setValue("twName" + QString::number(i), name);
  }

  if (TabCount == 0) {
    QTreeWidget *tw = m_MainHelper->init_TreeWidget("20220303_101010_1");

    QString tabText = tr("Tab") + " " + QString::number(1);
    mui->tabWidget->addTab(tw, tabText);
    addItem(tabText, "", "", "", 0);

    mui->tabWidget->setTabToolTip(0, "");
  }

  m_EditRecord->init_MyCategory();

  currentTabIndex = RegTab.value("CurrentIndex").toInt();
  mui->tabWidget->setCurrentIndex(currentTabIndex);
  setCurrentIndex(currentTabIndex);
  QTreeWidget *twCur = (QTreeWidget *)tabData->currentWidget();
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
  if (keyType == "note") mui->btnNotes->click();
  ;
  if (keyType == "reader") m_Reader->ContinueReading();
  if (keyType == "add") mui->btnAdd->click();
  if (keyType == "exercise") {
    QTimer::singleShot(100, this, [this]() { mui->btnSteps->click(); });
  }
  if (keyType == "defaultopen") {
#ifdef Q_OS_ANDROID

    m_ReceiveShare->callJavaNotify9();

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
  delete mui;
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

    mui->progBar->setHidden(false);
    mui->progBar->setMaximum(0);

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
    if (mui->rbSteps->isChecked()) mui->rbFreq->click();
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
      item11->setText(3, mui->editDetails->toPlainText().trimmed());

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
    item11->setText(3, mui->editDetails->toPlainText().trimmed());

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
  m_MainHelper->sort_childItem(topItem->child(0));
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
        QString str = mui->tabWidget->tabText(mui->tabWidget->currentIndex());
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
    QString str = mui->tabWidget->tabText(mui->tabWidget->currentIndex());

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

  mui->lblTitleEditRecord->setText(tr("Add") + "  : " +
                                   tabData->tabText(tabData->currentIndex()));

  mui->hsH->setValue(QTime::currentTime().hour());
  mui->hsM->setValue(QTime::currentTime().minute());
  m_EditRecord->getTime(mui->hsH->value(), mui->hsM->value());

  mui->editDetails->clear();
  mui->editCategory->setText("");
  mui->editAmount->setText("");

  mui->frameMain->hide();
  mui->frameEditRecord->show();

  // tmeFlash->start(300);
}

void MainWindow::on_tmeFlash() {
  nFlashCount = nFlashCount + 1;
  if (nFlashCount % 2 == 0)
    mui->lblTitleEditRecord->setStyleSheet(m_Method->lblStyle0);
  else
    mui->lblTitleEditRecord->setStyleSheet(m_Method->lblStyle);
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
  if (mui->qwCata->isVisible()) {
    on_btnCatalogue_clicked();
    event->ignore();
    return;
  }

  if (mui->qwBookList->isVisible()) {
    on_btnBackBookList_clicked();
    event->ignore();
    return;
  }

  if (mui->frameReader->isVisible()) {
    on_btnBackReader_clicked();
    event->ignore();
    return;
  }

  if (!mui->frameImgView->isHidden()) {
    on_btnBackImg_clicked();
    event->ignore();
    return;
  }

  if (!mui->frameNoteRecycle->isHidden()) {
    on_btnBackNoteRecycle_clicked();
    event->ignore();
    return;
  }

  if (!mui->frameNotesSearchResult->isHidden()) {
    on_btnBack_NotesSearchResult_clicked();
    event->ignore();
    return;
  }

  if (!mui->frameNoteList->isHidden()) {
    on_btnBackNoteList_clicked();
    event->ignore();
    return;
  }

  if (!mui->frameNotes->isHidden()) {
    on_btnBackNotes_clicked();
    event->ignore();
    return;
  }

  if (!mui->frameTodoRecycle->isHidden()) {
    on_btnReturnRecycle_clicked();
    event->ignore();
    return;
  }

  if (!mui->frameTodo->isHidden()) {
    on_btnBackTodo_clicked();
    event->ignore();
    return;
  }

  if (!mui->frameBakList->isHidden()) {
    on_btnBackBakList_clicked();
    event->ignore();
    return;
  }

  if (!mui->frameOne->isHidden()) {
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
    if (mui->rbFreq->isChecked())
      chartMonth->setTitle(CurrentYear + "  Y:" + tr("Freq") +
                           "    X:" + tr("Days"));

    if (mui->rbAmount->isChecked())
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

  if (mui->rbFreq->isChecked())
    chartDay->setTitle(CurrentYear + "  Y:" + tr("Freq") +
                       "    X:" + tr("Time"));

  if (mui->rbAmount->isChecked())
    chartDay->setTitle(CurrentYear + "  Y:" + tr("Amount") +
                       "    X:" + tr("Time"));
}

void MainWindow::on_actionRename_triggered() {
  int index = mui->tabWidget->currentIndex();
  bool ok = false;

  QString text;

  if (m_RenameDlg != nullptr) delete m_RenameDlg;

  m_RenameDlg =
      m_Method->inputDialog(tr("Rename tab name : "), tr("Tab name : "),
                            mui->tabWidget->tabText(index));

  if (QDialog::Accepted == m_RenameDlg->exec()) {
    ok = true;
    text = m_RenameDlg->textValue();
    m_RenameDlg->close();
  } else {
    m_RenameDlg->close();
    return;
  }

  if (ok && !text.isEmpty()) {
    mui->tabWidget->setTabText(index, text);

    m_Method->modifyItemText0(mui->qwMainTab, index, text);

    updateMainTab();

    saveTab();
  }

  strLatestModify = tr("Rename Tab");
}

void MainWindow::on_actionAdd_Tab_triggered() {
  int count = mui->tabWidget->tabBar()->count();
  QString twName = m_Notes->getDateTimeStr() + "_" + QString::number(count + 1);
  QString ini_file = iniDir + twName + ".ini";
  if (QFile(ini_file).exists()) QFile(ini_file).remove();

  QTreeWidget *tw = m_MainHelper->init_TreeWidget(twName);

  QString tabText = tr("Tab") + " " + QString::number(count + 1);
  mui->tabWidget->addTab(tw, tabText);
  mui->tabWidget->setCurrentIndex(count);

  addItem(tabText, "", "", "", 0);
  setCurrentIndex(count);

  mui->tabCharts->setTabText(0, tr("Month"));
  mui->tabCharts->setTabText(1, tr("Day"));

  mui->btnChartMonth->setText(tabChart->tabText(0));
  mui->btnChartDay->setText(tabChart->tabText(1));

  on_actionRename_triggered();
  reloadMain();

  saveTab();

  strLatestModify = tr("Add Tab") + " ( " + getTabText() + " ) ";
}

void MainWindow::on_actionDel_Tab_triggered() {
  int index = mui->tabWidget->currentIndex();
  if (index < 0) return;

  QString tab_name = mui->tabWidget->tabText(index);

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

  int TabCount = mui->tabWidget->tabBar()->count();
  if (TabCount > 1) {
    mui->tabWidget->removeTab(index);
    delItem(index);
  }

  if (TabCount == 1) {
    QTreeWidget *tw = (QTreeWidget *)mui->tabWidget->currentWidget();
    tw->clear();
    tabData->setTabText(0, tr("Tab") + " 1");

    clearAll();
    addItem(tabData->tabText(0), "", "", "", 0);

    mui->tabWidget->setTabToolTip(0, "");

    reloadMain();
  }

  saveTab();
}

void MainWindow::on_twItemClicked() {
  QTreeWidget *tw = (QTreeWidget *)mui->tabWidget->currentWidget();
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
  mui->tabCharts->setTabText(0, stra.split(" ").at(1));
  mui->tabCharts->setTabText(1, stra.split(" ").at(2));

  mui->btnChartMonth->setText(tabChart->tabText(0));
  mui->btnChartDay->setText(tabChart->tabText(1));

  // top item
  if (item->childCount() > 0) {
    pItem = item;
    QString sy = pItem->text(3);
    QString sm = pItem->text(0).split(" ").at(1);
    max_day = getMaxDay(sy, sm);

    isShowDetails = false;

    mui->lblStats->setText(strStats);
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
  QTreeWidget *tw = (QTreeWidget *)mui->tabWidget->currentWidget();
  QTreeWidgetItem *item = tw->currentItem();
  QTreeWidgetItem *topItem = item->parent();
  QString newtime = mui->lblTime->text().trimmed();
  if (item->childCount() == 0 && item->parent()->childCount() > 0) {
    item->setText(0, newtime);
    QString sa = mui->editAmount->text().trimmed();
    if (sa == "")
      item->setText(1, "");
    else
      item->setText(1, QString("%1").arg(sa.toDouble(), 0, 'f', 2));
    item->setText(2, mui->editCategory->text().trimmed());
    item->setText(3, mui->editDetails->toPlainText().trimmed());
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
    m_MainHelper->sort_childItem(item);

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
    int row = m_Method->getCurrentIndexFromQW(mui->qwMainEvent);
    if (childRow0 - childRow1 == 0) newrow = row;
    if (childRow0 - childRow1 < 0) newrow = row + childRow1 - childRow0;
    if (childRow0 - childRow1 > 0) newrow = row - (childRow0 - childRow1);

    int maindateIndex = m_Method->getCurrentIndexFromQW(mui->qwMainDate);

    isEditItem = true;
    reloadMain();

    m_Method->setCurrentIndexFromQW(mui->qwMainDate, maindateIndex);
    isEditItem = true;
    m_Method->clickMainDate();
    m_Method->setCurrentIndexFromQW(mui->qwMainEvent, newrow);
  }
}

void MainWindow::on_twItemDoubleClicked() {
  m_EditRecord->monthSum();

  QTreeWidget *tw = (QTreeWidget *)mui->tabWidget->currentWidget();
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
    mui->lblTitleEditRecord->setText(tr("Modify") + "  : " +
                                     tabData->tabText(tabData->currentIndex()));

    mui->hsH->setValue(sh.toInt());
    mui->hsM->setValue(sm.toInt());

    mui->lblTime->setText(t.trimmed());

    QString str = item->text(1);
    if (str == "0.00")
      mui->editAmount->setText("");
    else
      mui->editAmount->setText(str);

    mui->editCategory->setText(item->text(2));
    mui->editDetails->setText(item->text(3));
    mui->f_Number->setFocus();

    isAdd = false;
    mui->frameMain->hide();
    mui->frameEditRecord->show();
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
  int count = mui->tabWidget->tabBar()->count();

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

  m_MainHelper->mainEventFilter(watch, evn);

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
  mui->frameMain->hide();
  mui->frameSearch->show();
  mui->editSearchText->setFocus();
  mui->btnClearSearchText->setFixedHeight(mui->btnStartSearch->height());
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
  QPalette pal = mui->btnFind->palette();
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
  m_Preferences->ui->sliderFontSize->setStyleSheet(mui->hsM->styleSheet());
  m_Preferences->ui->sliderFontSize->setValue(fontSize);
  m_Preferences->show();
  m_Preferences->initCheckStatus();
}

void MainWindow::on_tabCharts_currentChanged(int index) {
  if (mui->rbSteps->isChecked() || loading || index < 0) return;

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

  QString sm = get_Month(m_Method->setCurrentDateValue());
  QString strday, strmonth;
  for (int i = 0; i < count; i++) {
    QString strD = m_Steps->getDate(i);

    QStringList list = strD.split(" ").at(0).split("-");
    if (list.count() > 1) {
      strmonth = list.at(1);
      strday = list.at(2);
    }
    if (sm.toInt() == strmonth.toInt()) {
      int day = strday.toInt();
      int steps = m_Steps->getSteps(i);

      PointList.append(QPointF(day, steps));
      doubleList.append(steps);
    }
  }

  initChartMonth();
  chartMonth->setTitle("Y:" + tr("Steps") + "    X:" + tr("Days"));
}

void MainWindow::on_btnNotes_clicked() { m_Notes->openNotes(); }

void MainWindow::init_Instance() {
  mw_one = this;
  m_MainHelper = new MainHelper(this);
  CurrentYear = QString::number(QDate::currentDate().year());
  if (defaultFontFamily == "") defaultFontFamily = this->font().family();

  tabData = new QTabWidget;
  tabData = mui->tabWidget;

  tabChart = new QTabWidget;
  tabChart = mui->tabCharts;

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
}

void MainWindow::selTab() {
  int index = m_Method->getCurrentIndexFromQW(mui->qwSelTab);
  tabData->setCurrentIndex(index);
  on_btnBackSetTab_clicked();
  m_Method->clearAllBakList(mui->qwSelTab);

  if (mui->btnTabMoveDown->isHidden()) {
    mui->btnTabMoveDown->show();
    mui->btnTabMoveUp->show();
    on_btnAdd_clicked();
    m_EditRecord->setCurrentValue();
  }
}

void MainWindow::getMainTabs() {
  m_Method->clearAllBakList(mui->qwSelTab);
  int tab_count = tabData->tabBar()->count();
  for (int i = 0; i < tab_count; i++) {
    QString text0 = tabData->tabText(i);
    m_Method->addItemToQW(mui->qwSelTab, text0, "", "", "", 0);
  }

  int index = mui->tabWidget->currentIndex();
  m_Method->setCurrentIndexFromQW(mui->qwSelTab, index);

  mui->lblSelTabInfo->setText(tr("Total") + " : " + QString::number(tab_count) +
                              " ( " + QString::number(index + 1) + " ) ");
}

void MainWindow::on_btnSelTab_clicked() {
  mui->frameMain->hide();
  mui->frameSetTab->show();
  getMainTabs();
}

void MainWindow::on_openKnotBakDir() {
#ifdef Q_OS_ANDROID

  QJniObject m_activity = QNativeInterface::QAndroidApplication::context();
  m_activity.callMethod<void>("openKnotBakDir", "()V");

#endif
}

void MainWindow::on_actionOneDriveBackupData() {
  mui->frameMain->hide();
  mui->frameReader->hide();
  mui->frameOne->show();
}

void MainWindow::on_actionTabRecycle() { m_MainHelper->openTabRecycle(); }

void MainWindow::on_actionBakFileList() {
  m_MainHelper->startBackgroundTaskUpdateBakFileList();
}

void MainWindow::on_btnMenu_clicked() {
  mainMenu = new QMenu(this);
  m_MainHelper->init_Menu(mainMenu);
  int x = 0;
#ifdef Q_OS_ANDROID
  x = mw_one->geometry().x() + 2;
#else
  x = mw_one->geometry().x() + mui->btnMenu->x();
#endif
  int y = geometry().y() + mui->f_Menu->height() + 2;
  QPoint pos(x, y);
  mainMenu->exec(pos);
}

void MainWindow::stopJavaTimer() {
#ifdef Q_OS_ANDROID

  QJniObject jo = QNativeInterface::QAndroidApplication::context();
  jo.callStaticMethod<int>("com.x/MyService", "stopTimer", "()I");

#endif
}

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
      mui->frameMain->hide();
      mui->frameBookList->show();

      m_Reader->getReadList();

      m_Reader->openMyPDF(fileName);
      return;
    }
  }

  mui->frameMain->hide();
  mui->frameReader->show();
  mui->f_ReaderFun->show();

  isReaderVisible = true;
  isMemoVisible = false;

  if (!isOne) {
    while (!mui->btnReader->isEnabled())
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    mwh = this->height();
    setFixedHeight(mwh);
    mui->qwReader->rootContext()->setContextProperty("myW", this->width());
    mui->qwReader->rootContext()->setContextProperty("myH", mwh);
  }

  if (!isOne) {
    isOne = true;
    m_Reader->setPageVPos();
  }
}

void MainWindow::on_btnBackReader_clicked() {
  mui->btnAutoStop->click();

  m_ReaderSet->close();

  if (m_Reader->isSelText) on_btnSelText_clicked();

  if (mui->f_ReaderSet->isVisible()) {
    on_btnBackReaderSet_clicked();
  }

  m_Reader->saveReader("", false);
  m_Reader->savePageVPos();

  mui->frameReader->hide();
  mui->frameMain->show();
}

void MainWindow::on_btnOpen_clicked() {
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
  mui->btnAutoStop->click();

  if (mui->qwCata->isVisible()) return;

  if (mui->f_ReaderSet->isHidden()) {
    mui->f_ReaderSet->show();

    m_Reader->closeSelText();
    if (mui->qwBookmark->isVisible()) {
      on_btnShowBookmark_clicked();
    }

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

void MainWindow::on_hSlider_sliderMoved(int position) {
  if (isText) {
    mui->btnPages->setText(QString::number(position) + "\n" +
                           QString::number(totalPages));
    mui->progReader->setMinimum(1);
    mui->progReader->setMaximum(totalPages);
    mui->progReader->setValue(position);
  }

  if (isEpub) {
    mui->btnPages->setText(QString::number(position) + "\n" +
                           QString::number(htmlFiles.count()));
    mui->progReader->setMinimum(1);
    mui->progReader->setMaximum(htmlFiles.count());
    if (position == 0) position = 1;
    mui->progReader->setValue(position);
  }

  m_ReaderSet->updateProgress();
}

void MainWindow::on_btnReadList_clicked() {
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
  mui->frameBookList->show();

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
  if (mui->f_ReaderSet->isVisible()) {
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
  if (!mui->btnReader->isEnabled()) return;
  m_CloudBackup->startBakData();
}

void MainWindow::on_btnDownload_clicked() {
  m_CloudBackup->on_pushButton_downloadFile_clicked();
}

void MainWindow::on_btnBack_One_clicked() { m_CloudBackup->backExit(); }

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
  mui->frameNotes->hide();
  mui->frameNoteList->show();
}

void MainWindow::on_btnEdit_clicked() { m_Notes->openEditUI(); }

void MainWindow::clearSelectBox() {
  QString tempFile = iniDir + "memo/texteditor.html";
  if (!mui->frameReader->isHidden()) {
    mw_one->m_Reader->savePageVPos();
    bool isAni = false;
    mui->qwReader->rootContext()->setContextProperty("isAni", isAni);
    QQuickItem *root = mui->qwReader->rootObject();
    QMetaObject::invokeMethod((QObject *)root, "loadHtml",
                              Q_ARG(QVariant, tempFile));
    m_Method->Sleep(50);
    if (isEpub) {
      QMetaObject::invokeMethod(
          (QObject *)root, "loadHtml",
          Q_ARG(QVariant, mw_one->m_Reader->currentHtmlFile));
    } else {
      mui->qwReader->rootContext()->setContextProperty("strText",
                                                       m_Reader->currentTxt);
    }
    mw_one->m_Reader->setPageVPos();
  }

  if (!mui->frameNotes->isHidden()) {
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
  QQuickItem *root = mui->qwReader->rootObject();
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

void MainWindow::on_btnCancelSel_clicked() { mui->btnSelText->click(); }

void MainWindow::on_textBrowser_selectionChanged() {
  QString str = mui->textBrowser->textCursor().selectedText().trimmed();
  mui->editSetText->setText(str);
  mydlgSetText->ui->lineEdit->setText(str);
}

void MainWindow::on_SetReaderFunVisible() {
  if (!isTurnThePage) {
    if (mui->f_ReaderFun->isHidden())
      mui->f_ReaderFun->show();
    else {
      mui->f_ReaderFun->hide();
      m_ReaderSet->hide();
    }
  }
}

void MainWindow::on_timerMousePress() {
  if (!isMouseMove && isMousePress) on_btnSelText_clicked();
}

void MainWindow::on_btnNotesList_clicked() {
  mui->frameNotes->hide();
  mui->frameNoteList->show();
  m_NotesList->set_memo_dir();

  if (m_NotesList->tw->topLevelItemCount() == 0) {
    mui->lblNoteBook->setText(tr("Note Book"));
    mui->lblNoteList->setText(tr("Note List"));
    return;
  }

  m_NotesList->loadAllNoteBook();
  m_NotesList->localNotesItem();
  m_NotesList->setNoteLabel();
}

void MainWindow::on_btnBackImg_clicked() {
  mui->frameImgView->hide();
  if (isReaderVisible) mui->frameReader->show();
  if (isMemoVisible) mui->frameNotes->show();
}

void MainWindow::on_btnZoomIn_clicked() {
  QQuickItem *root = mui->qw_Img->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "zoomin");
}

void MainWindow::on_btnZoomOut_clicked() {
  QQuickItem *root = mui->qw_Img->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "zoomout");
}

void MainWindow::on_btnReport_clicked() {
  on_actionReport_triggered();
  mui->btnYear->setFixedHeight(mui->btnMonth->height());
}

void MainWindow::on_btnPasteCode_clicked() {
  QClipboard *clipboard = QApplication::clipboard();
  QString originalText = clipboard->text();
  mui->editCode->setPlainText(originalText);
}

void MainWindow::on_btnAdd_clicked() {
  m_EditRecord->monthSum();

  on_AddRecord();
}

void MainWindow::on_btnDel_clicked() {
  isMoveEntry = false;
  del_Data((QTreeWidget *)mui->tabWidget->currentWidget());
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  Q_UNUSED(event);
  mui->qwReader->rootContext()->setContextProperty("myW", this->width());
  mui->qwReader->rootContext()->setContextProperty("myH", this->height());
  mui->qwTodo->rootContext()->setContextProperty("isBtnVisible",
                                                 QVariant(false));
  mui->qwSteps->rootContext()->setContextProperty("myW", this->width());

#ifdef Q_OS_ANDROID

#else
  if (!mui->frameTodo->isHidden()) {
    mui->qwTodo->rootContext()->setContextProperty("m_width", mw_one->width());
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
  QQuickItem *root = mui->qwMainTab->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "setItemHeight",
                            Q_ARG(QVariant, h));
}

void MainWindow::addItem(QString text0, QString text1, QString text2,
                         QString text3, int itemH) {
  QQuickItem *root = mui->qwMainTab->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "addItem", Q_ARG(QVariant, text0),
                            Q_ARG(QVariant, text1), Q_ARG(QVariant, text2),
                            Q_ARG(QVariant, text3), Q_ARG(QVariant, itemH));
}

QString MainWindow::getTop(int index) {
  QQuickItem *root = mui->qwMainTab->rootObject();
  QVariant itemTime;
  QMetaObject::invokeMethod((QObject *)root, "getTop",
                            Q_RETURN_ARG(QVariant, itemTime),
                            Q_ARG(QVariant, index));
  return itemTime.toString();
}

QString MainWindow::getText0(int index) {
  QQuickItem *root = mui->qwMainTab->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject *)root, "getText0",
                            Q_RETURN_ARG(QVariant, item),
                            Q_ARG(QVariant, index));
  return item.toString();
}

QString MainWindow::getText1(int index) {
  QQuickItem *root = mui->qwMainTab->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject *)root, "getText1",
                            Q_RETURN_ARG(QVariant, item),
                            Q_ARG(QVariant, index));
  return item.toString();
}

QString MainWindow::getText2(int index) {
  QQuickItem *root = mui->qwMainTab->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject *)root, "getText2",
                            Q_RETURN_ARG(QVariant, item),
                            Q_ARG(QVariant, index));
  return item.toString();
}

int MainWindow::getItemType(int index) {
  QQuickItem *root = mui->qwMainTab->rootObject();
  QVariant itemType;
  QMetaObject::invokeMethod((QObject *)root, "getType",
                            Q_RETURN_ARG(QVariant, itemType),
                            Q_ARG(QVariant, index));
  return itemType.toInt();
}

void MainWindow::delItem(int index) {
  QQuickItem *root = mui->qwMainTab->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "delItem", Q_ARG(QVariant, index));
}

int MainWindow::getCount() {
  QQuickItem *root = mui->qwMainTab->rootObject();
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
  QQuickItem *root = mui->qwMainTab->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "setCurrentItem",
                            Q_ARG(QVariant, index));
}

void MainWindow::gotoEnd() {
  QQuickItem *root = mui->qwMainTab->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "gotoEnd");
}

void MainWindow::gotoIndex(int index) {
  QQuickItem *root = mui->qwMainTab->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "gotoIndex",
                            Q_ARG(QVariant, index));
}

int MainWindow::getCurrentIndex() {
  QQuickItem *root = mui->qwMainTab->rootObject();
  QVariant itemIndex;
  QMetaObject::invokeMethod((QObject *)root, "getCurrentIndex",
                            Q_RETURN_ARG(QVariant, itemIndex));
  return itemIndex.toInt();
}

void MainWindow::setScrollBarPos(double pos) {
  QQuickItem *root = mui->qwMainTab->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "setScrollBarPos",
                            Q_ARG(QVariant, pos));
}

void MainWindow::reloadMain() {
  bool isAniEffects;
  if (isDelItem || isEditItem)
    isAniEffects = false;
  else
    isAniEffects = true;

  mui->qwMainDate->rootContext()->setContextProperty("isAniEffects",
                                                     isAniEffects);
  mui->qwMainDate->rootContext()->setContextProperty("maindateWidth",
                                                     mui->qwMainDate->width());
  m_Method->clearAllBakList(mui->qwMainDate);

  QTreeWidget *tw = get_tw(tabData->currentIndex());

  int total = tw->topLevelItemCount();

  if (total == 0) {
    m_Method->clearAllBakList(mui->qwMainEvent);
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

    m_Method->addItemToQW(mui->qwMainDate, text0, text1, text2, text3, 0);
  }

  m_Method->gotoEnd(mui->qwMainDate);
  int count = m_Method->getCountFromQW(mui->qwMainDate);
  m_Method->setCurrentIndexFromQW(mui->qwMainDate, count - 1);
  m_Method->setScrollBarPos(mui->qwMainDate, 1.0);

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

void MainWindow::on_btnPasteTodo_clicked() { mui->editTodo->paste(); }

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
  mui->frameSearch->hide();
  mui->frameMain->show();
}

void MainWindow::on_btnClearSearchText_clicked() {
  mui->editSearchText->setText("");
  mui->editSearchText->setFocus();
}

void MainWindow::on_btnStartSearch_clicked() {
  searchStr = mui->editSearchText->text().trimmed();
  if (searchStr.length() == 0) return;

  showProgress();
  mySearchThread->start();
}

void MainWindow::on_btnBackBakList_clicked() {
  mui->frameBakList->hide();
  mui->frameMain->show();
}

void MainWindow::on_btnImportBakList_clicked() {
  m_MainHelper->importBakFileList();
}

void MainWindow::on_btnOkViewCate_clicked() { m_Report->on_CateOk(); }

void MainWindow::on_btnBackTabRecycle_clicked() {
  mui->frameTabRecycle->hide();
  mui->frameMain->show();
}

void MainWindow::on_btnDelTabRecycle_clicked() {
  m_MainHelper->delTabRecycleFile();
}

void MainWindow::on_btnRestoreTab_clicked() {
  m_MainHelper->clickBtnRestoreTab();
}

void MainWindow::on_btnDelBakFile_clicked() { m_MainHelper->delBakFile(); }

void MainWindow::on_btnBackNoteList_clicked() {
  clearWidgetFocus();
  mui->frameNoteList->hide();
  mui->frameMain->show();
  m_NotesList->saveNoteBookVPos();
  m_NotesList->saveCurrentNoteInfo();
  m_NotesList->saveNotesListIndex();
  m_Notes->updateMainnotesIniToSyncLists();

  m_Notes->syncToWebDAV();
}

void MainWindow::on_btnBackNoteRecycle_clicked() {
  mui->frameNoteRecycle->hide();
  mui->frameNoteList->show();

  if (mui->chkAutoSync->isChecked() && mui->chkWebDAV->isChecked()) {
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
  m_NotesList->on_btnDel_Recycle_clicked();
}

void MainWindow::on_btnRestoreNoteRecycle_clicked() {
  m_NotesList->restoreNoteFromRecycle();
}

void MainWindow::on_btnFindNotes_clicked() {
  showProgress();
  m_NotesList->startFind(mui->editFindNote->text().trimmed());
}

void MainWindow::on_btnFindPreviousNote_clicked() { m_NotesList->goPrevious(); }

void MainWindow::on_btnFindNextNote_clicked() { m_NotesList->goNext(); }

void MainWindow::on_btnClearNoteFindText_clicked() {
  mui->editFindNote->setText("");
  mui->lblFindNoteCount->setText("0");
  mui->btnFindNextNote->setEnabled(false);
  mui->btnFindPreviousNote->setEnabled(false);
  mui->lblShowLineSn->setText("0");
}

void MainWindow::on_btnShowFindNotes_clicked() { m_NotesList->showFindNotes(); }

void MainWindow::on_btnNoteBookMenu_clicked() {
  m_Method->showNoteBookMenu(mui->qwNoteBook->x(), mui->qwNoteBook->y());
}

void MainWindow::on_btnNoteMenu_clicked() {
  m_Method->showNotsListMenu(mui->qwNoteList->x(), mui->qwNoteList->y());
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
  m_CategoryList->on_btnRename_clicked();
}

void MainWindow::on_btnBackSetTab_clicked() {
  mui->frameSetTab->hide();
  mui->frameMain->show();

  if (mui->btnTabMoveDown->isHidden()) {
    mui->btnTabMoveDown->show();
    mui->btnTabMoveUp->show();
    on_btnAdd_clicked();
    m_EditRecord->setCurrentValue();
  }
}

void MainWindow::on_btnBackEditRecord_clicked() {
  clearWidgetFocus();

  mui->frameEditRecord->hide();
  mui->frameMain->show();
}

void MainWindow::on_btnType_clicked() { m_EditRecord->on_btnCustom_clicked(); }

void MainWindow::on_btnOkEditRecord_clicked() {
  m_EditRecord->on_btnOk_clicked();
}

void MainWindow::on_btnClearType_clicked() { mui->editCategory->setText(""); }

void MainWindow::on_btnClearDetails_clicked() { mui->editDetails->setText(""); }

void MainWindow::on_btnClearAmount_clicked() { mui->editAmount->setText(""); }

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
      mui->frameBookList->hide();
      mui->frameMain->show();
    }
  } else {
    mui->frameBookList->hide();
    mui->frameReader->show();
  }
}

void MainWindow::on_btnOkBookList_clicked() { m_Reader->openBookListItem(); }

void MainWindow::on_btnClearAllRecords_clicked() {
  m_Reader->clearAllReaderRecords();
}

void MainWindow::on_btnAnd_clicked() { mui->editSearchText->insert("&"); }

void MainWindow::on_btnClear_clicked() { mui->editTodo->clear(); }

void MainWindow::on_btnModify_clicked() { m_Todo->reeditText(); }

void MainWindow::on_btnChartMonth_clicked() {
  isTabChanged = true;
  tabChart->setCurrentIndex(0);
  m_Method->setToolButtonQss(mui->btnChartMonth, 5, 3, "#FF0000", "#FFFFFF",
                             "#FF0000", "#FFFFFF", "#FF5555", "#FFFFFF");
  m_Method->setToolButtonQss(mui->btnChartDay, 5, 3, "#455364", "#FFFFFF",
                             "#455364", "#FFFFFF", "#555364", "#FFFFFF");
}

void MainWindow::on_btnChartDay_clicked() {
  isTabChanged = true;
  tabChart->setCurrentIndex(1);
  m_Method->setToolButtonQss(mui->btnChartDay, 5, 3, "#FF0000", "#FFFFFF",
                             "#FF0000", "#FFFFFF", "#FF5555", "#FFFFFF");
  m_Method->setToolButtonQss(mui->btnChartMonth, 5, 3, "#455364", "#FFFFFF",
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

void MainWindow::on_btnChart_clicked() { m_MainHelper->clickBtnChart(); }

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
  mui->frameNotesTree->hide();
  mui->frameNoteList->show();
}

void MainWindow::on_btnRename_clicked() {
  if (m_NotesList->getNoteBookCurrentIndex() < 0) return;

  m_NotesList->setTWCurrentItem();
  m_NotesList->on_btnRename_clicked();
}

void MainWindow::on_btnHideFind_clicked() { mui->f_FindNotes->hide(); }

void MainWindow::on_btnStepsOptions_clicked() {
  mw_one->m_StepsOptions->init();
}

void MainWindow::on_btnRecentOpen_clicked() {
  m_NotesList->genRecentOpenMenu();
}

void MainWindow::on_btnMenuReport_clicked() { m_Report->genReportMenu(); }

void MainWindow::on_btnCatalogue_clicked() {
  mui->btnAutoStop->click();

  if (mui->f_ReaderSet->isVisible()) {
    on_btnBackReaderSet_clicked();
  }
  m_Reader->showCatalogue();
}

void MainWindow::on_btnRemoveBookList_clicked() { m_Reader->removeBookList(); }

void MainWindow::on_btnShowBookmark_clicked() {
  m_Reader->showOrHideBookmark();
}

void MainWindow::stopTimerForPdf() {
  m_Reader->tmeShowEpubMsg->stop();
  mui->pEpubProg->hide();
  mui->lblEpubInfo->hide();
}

void MainWindow::on_btnShareImage_clicked() {
  m_ReceiveShare->shareImage(tr("Share to"), imgFileName, "image/png");
}

void MainWindow::on_btnHideKey_clicked() { pAndroidKeyboard->hide(); }

void MainWindow::on_btnDelImage_clicked() { m_Notes->delImage(); }

void MainWindow::on_btnBackReaderSet_clicked() {
  mui->f_ReaderSet->hide();
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

  mui->btnReader->setEnabled(true);
  mui->f_ReaderFun->setEnabled(true);
}

void MainWindow::on_btnShareBook_clicked() { m_Reader->shareBook(); }

void MainWindow::on_btnAutoRun_clicked() {
  m_Reader->tmeAutoRun->start(50);
  mui->btnAutoRun->hide();
  mui->btnAutoStop->show();
}

void MainWindow::on_btnAutoStop_clicked() {
  m_Reader->tmeAutoRun->stop();
  mui->btnAutoStop->hide();
  mui->btnAutoRun->show();
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
  QString strPos = QString::number(mui->sliderPlayAudio->value());
  m_Method->seekTo(strPos);
  m_Method->startPlay();
  m_Todo->tmePlayProgress->start(m_Todo->nInterval);
}

void MainWindow::on_btnMove_clicked() {
  isMoveEntry = true;
  if (del_Data((QTreeWidget *)mui->tabWidget->currentWidget())) {
    mui->btnTabMoveDown->hide();
    mui->btnTabMoveUp->hide();
    on_btnSelTab_clicked();

    while (mui->frameEditRecord->isHidden())
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

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

void MainWindow::on_rbCycling_clicked() {}

void MainWindow::on_rbHiking_clicked() {}

void MainWindow::on_rbRunning_clicked() {}

void MainWindow::on_btnOpenNote_clicked() { m_Notes->previewNote(); }

void MainWindow::on_btnEditNote_clicked() {
  m_NotesList->setCurrentItemFromMDFile(currentMDFile);
  m_Notes->openEditUI();
}

void MainWindow::on_btnToPDF_clicked() {
  if (!QFile::exists(currentMDFile)) return;
  mui->btnPDF->click();
}

void MainWindow::on_btnRecentOpen0_clicked() { on_btnRecentOpen_clicked(); }

void MainWindow::on_btnWebBack_clicked() {}

void MainWindow::on_btnWebDAVBackup_clicked() {
  if (!mui->btnReader->isEnabled()) return;
  m_CloudBackup->startBakData();
}

void MainWindow::on_btnWebDAVRestore_clicked() {
  m_CloudBackup->webDAVRestoreData();
}

void MainWindow::on_chkWebDAV_clicked() {
  if (mui->chkWebDAV->isChecked())
    mui->chkOneDrive->setChecked(false);
  else
    mui->chkOneDrive->setChecked(true);
}

void MainWindow::on_chkOneDrive_clicked() {
  if (mui->chkOneDrive->isChecked())
    mui->chkWebDAV->setChecked(false);
  else
    mui->chkWebDAV->setChecked(true);
}

void MainWindow::on_btnBack_NotesSearchResult_clicked() {
  clearWidgetFocus();
  mui->frameNotesSearchResult->hide();
  mui->frameNoteList->show();
  isOpenSearchResult = false;
}

void MainWindow::on_editFindNote_returnPressed() { on_btnFindNotes_clicked(); }

void MainWindow::on_btnClearSearchResults_clicked() {
  mui->editNotesSearch->clear();
  mui->editNotesSearch->setFocus();
}

void MainWindow::on_btnOpenSearchResult_clicked() {
  QString mdFile = m_NotesList->getSearchResultQmlFile();
  if (!QFile::exists(mdFile)) return;
  isOpenSearchResult = true;
  currentMDFile = mdFile;
  mySearchText = mui->editNotesSearch->text().trimmed();
  on_btnEditNote_clicked();
}

void MainWindow::on_btnFindNotes2_clicked() {
  if (mui->f_FindNotes->isHidden())
    mui->f_FindNotes->show();
  else
    mui->f_FindNotes->hide();
}

void MainWindow::on_btnOpenEditFind_clicked() {
  isOpenSearchResult = true;
  mySearchText = mui->editFindNote->text().trimmed();
  on_btnEditNote_clicked();
}

void MainWindow::on_btnTools_clicked() {
  if (mui->f_Tools->isHidden())
    mui->f_Tools->show();
  else
    mui->f_Tools->hide();
}
