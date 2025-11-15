#include "MainWindow.h"

#include "src/defines.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget* parent)
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

  m_MainHelper->initMainQW();

  init_TotalData();

  loading = false;

  QTreeWidget* tw = (QTreeWidget*)tabData->currentWidget();
  startRead(strDate);
  get_Today(tw);
  init_Stats(tw);

  resetWinPos();

  QTimer::singleShot(10, this, [this]() {
    reloadMain();
    clickData();
    updateMainTab();
  });

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

  initMain = false;
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
    QTreeWidget* tw = m_MainHelper->init_TreeWidget(name);

    QString tabText = RegTab
                          .value("TabName" + QString::number(i),
                                 tr("Tab") + QString::number(i + 1))
                          .toString();
    mui->tabWidget->addTab(tw, tabText);

    addItem(tabText, "", "", "", 0);
  }

  if (TabCount == 0) {
    QString tw_name = m_Notes->getDateTimeStr() + "_" + QString::number(1);
    QTreeWidget* tw = m_MainHelper->init_TreeWidget(tw_name);

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

void MainWindow::readDataInThread(int ExceptIndex) {
  int count = tabData->tabBar()->count();
  for (int i = 0; i < count; i++) {
    if (i != ExceptIndex) {
      QTreeWidget* tw = (QTreeWidget*)tabData->widget(i);
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
    QTimer::singleShot(100, this, []() { mui->btnSteps->click(); });
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

  // 停止所有定时器
  timer->stop();
  timerSyncData->stop();
  timerMousePress->stop();
  tmeFlash->stop();
  tmeStartRecordAudio->stop();

  // 释放定时器
  delete timer;
  delete timerSyncData;
  delete timerMousePress;
  delete tmeFlash;
  delete tmeStartRecordAudio;

  mySaveThread->quit();
  mySaveThread->wait();

  myReadChartThread->quit();
  myReadChartThread->wait();

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
    myReadChartThread->quit();
    myReadChartThread->wait();

    while (!isReadEnd)
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
  }

  if (isReadEnd) {
    isBreak = false;
    myReadChartThread->start();
  }
}

void MainWindow::add_Data(QTreeWidget* tw, QString strTime, QString strAmount,
                          QString strDesc) {
  bool isYes = false;

  strDate = m_Method->setCurrentDateValue();

  int topc = tw->topLevelItemCount();
  for (int i = 0; i < topc; i++) {
    QString str = tw->topLevelItem(topc - 1 - i)->text(0) + " " +
                  tw->topLevelItem(topc - 1 - i)->text(3);
    if (getYMD(str) == getYMD(strDate)) {
      isYes = true;

      QTreeWidgetItem* topItem = tw->topLevelItem(topc - 1 - i);
      QTreeWidgetItem* item11 = new QTreeWidgetItem(topItem);
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
    QTreeWidgetItem* topItem = new QTreeWidgetItem;

    QStringList lista = strDate.split(" ");
    if (lista.count() == 4) {
      QString a = lista.at(0) + " " + lista.at(1) + " " + lista.at(2);
      topItem->setText(0, a);
      topItem->setText(3, lista.at(3));
    }

    tw->addTopLevelItem(topItem);
    QTreeWidgetItem* item11 = new QTreeWidgetItem(topItem);
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
  QTreeWidgetItem* topItem = tw->topLevelItem(topCount - 1);
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

bool MainWindow::del_Data(QTreeWidget* tw) {
  if (tw->topLevelItemCount() == 0) return false;

  bool isTodayData = false;
  isRemovedTopItem = false;

  strDate = m_Method->setCurrentDateValue();
  for (int i = 0; i < tw->topLevelItemCount(); i++) {
    QString str =
        tw->topLevelItem(i)->text(0) + " " + tw->topLevelItem(i)->text(3);
    if (getYMD(str) == getYMD(strDate)) {
      isTodayData = true;
      QTreeWidgetItem* topItem = tw->topLevelItem(i);
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
          strTip = tr("The last record of today will be moved.");
        else
          strTip = tr("The last record of today will be deleted.");

        auto m_ShowMsg = std::make_unique<ShowMessage>(this);
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

    auto m_ShowMsg = std::make_unique<ShowMessage>(this);
    m_ShowMsg->showMsg(str, strTip, 1);

    return false;
  }

  int topCount = tw->topLevelItemCount();
  if (topCount > 0) {
    QTreeWidgetItem* topItem = tw->topLevelItem(topCount - 1);
    tw->setCurrentItem(topItem);
  }

  isDelData = true;
  startSave("tab");
  return true;
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
  QString tempFile = iniDir + "tab.tmp";
  QString endFile = iniDir + "tab.ini";
  QSettings Reg(tempFile, QSettings::IniFormat);

  int TabCount = tabData->tabBar()->count();
  Reg.setValue("TabCount", TabCount);
  int CurrentIndex = tabData->currentIndex();
  Reg.setValue("CurrentIndex", CurrentIndex);
  for (int i = 0; i < TabCount; i++) {
    if (isBreak) break;
    Reg.setValue("TabName" + QString::number(i), tabData->tabText(i));

    QTreeWidget* tw = (QTreeWidget*)tabData->widget(i);
    Reg.setValue("twName" + QString::number(i), tw->objectName());
  }

  Reg.sync();
  m_Method->upIniFile(tempFile, endFile);
}

void MainWindow::get_Today(QTreeWidget* tw) {
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

void MainWindow::closeEvent(QCloseEvent* event) {
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

  if (mui->qwMainChart->isVisible()) {
    on_btnChart_clicked();
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

  if (!mui->frameDiff->isHidden()) {
    on_btnBackNoteDiff_clicked();
    event->ignore();
    return;
  }

  if (!mui->frameNotesGraph->isHidden()) {
    on_btnBackNotesGraph_clicked();
    event->ignore();
    return;
  }

  if (!mui->frameTodoRecycle->isHidden()) {
    on_btnReturnRecycle_clicked();
    event->ignore();
    return;
  }

  if (m_Todo->isTodoAlarmShow) {
    m_Todo->closeTodoAlarm();
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

  saveNeedSyncNotes();

  QSettings Reg(privateDir + "winpos.ini", QSettings::IniFormat);
  Reg.setValue("x", this->geometry().x());
  Reg.setValue("y", this->geometry().y());
  Reg.setValue("w", this->geometry().width());
  Reg.setValue("h", this->geometry().height());

  event->accept();
#endif
}

void MainWindow::saveNeedSyncNotes() {
  int m_count = m_Notes->notes_sync_files.count();
  QString tempFile = privateDir + "need_sync_" +
                     QUuid::createUuid().toString(QUuid::WithoutBraces) +
                     ".tmp";
  QString endFile = privateDir + "need_sync.ini";
  QSettings RegSync(tempFile, QSettings::IniFormat);
  RegSync.setValue("count", m_count);
  for (int i = 0; i < m_count; i++) {
    RegSync.setValue("note" + QString::number(i),
                     m_Notes->notes_sync_files.at(i));
  }
  RegSync.sync();
  m_Method->upIniFile(tempFile, endFile);
}

void MainWindow::execNeedSyncNotes() {
  QSettings RegSync(privateDir + "need_sync.ini", QSettings::IniFormat);
  int count = RegSync.value("count", 0).toInt();
  qDebug() << "==================================";
  qDebug() << "Need exec sync >>" << count;
  qDebug() << "==================================";
  for (int i = 0; i < count; i++) {
    QString note = RegSync.value("note" + QString::number(i), "").toString();
    if (QFile::exists(note)) {
      m_Notes->appendToSyncList(note);
    }
  }

  m_Notes->syncToWebDAV();
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

  QTreeWidget* tw = m_MainHelper->init_TreeWidget(twName);

  QString tabText = tr("Tab") + " " + QString::number(count + 1);
  mui->tabWidget->addTab(tw, tabText);
  mui->tabWidget->setCurrentIndex(count);

  addItem(tabText, "", "", "", 0);
  setCurrentIndex(count);

  on_actionRename_triggered();
  reloadMain();

  saveTab();

  strLatestModify = tr("Add Tab") + " ( " + getTabText() + " ) ";
}

void MainWindow::on_actionDel_Tab_triggered() {
  int index = mui->tabWidget->currentIndex();
  if (index < 0) return;

  QString tab_name = mui->tabWidget->tabText(index);

  auto m_ShowMsg = std::make_unique<ShowMessage>(this);
  if (!m_ShowMsg->showMsg("Knot",
                          tr("Whether to remove") + "  " + tab_name + " ? ", 2))
    return;

  strLatestModify = tr("Del Tab") + " ( " + tab_name + " ) ";

  QString date_time = m_Notes->getDateTimeStr();
  m_Method->saveRecycleTabName(date_time, tab_name);

  QTreeWidget* tw = (QTreeWidget*)tabData->currentWidget();
  QString twName = tw->objectName();
  int c_year = QDate::currentDate().year();

  bool isFileExists = false;
  for (int i = 2022; i <= c_year; i++) {
    QString file = iniDir + QString::number(i) + "-" + twName + ".json";
    if (QFile::exists(file)) {
      isFileExists = true;
      QFileInfo fi(file);
      QString fn = fi.fileName();
      QString newFile = iniDir + "recycle_" + tab_name + "_" + fn;
      QFile::rename(file, newFile);
    }
  }

  if (!isFileExists) {  // ini files
    int iniFileCount = c_year - 2025 + 1 + 1;
    for (int i = 0; i < iniFileCount; i++) {
      QString tab_file;
      if (i == 0)
        tab_file = iniDir + twName + ".ini";
      else {
        tab_file =
            iniDir + QString::number(2025 + i - 1) + "-" + twName + ".ini";
      }

      if (QFile::exists(tab_file)) {
        QFile::copy(tab_file, iniDir + "recycle_name" + "_" + date_time + "-" +
                                  QString::number(i) + ".ini");
        QFile file(tab_file);
        file.remove();
      }
    }
  }

  int TabCount = mui->tabWidget->tabBar()->count();
  if (TabCount > 1) {
    mui->tabWidget->removeTab(index);
    delItem(index);
  }

  if (TabCount == 1) {
    mui->tabWidget->removeTab(0);
    QString tw_name = m_Notes->getDateTimeStr() + "_" + QString::number(1);
    QTreeWidget* tw = m_MainHelper->init_TreeWidget(tw_name);
    QString tabText = tr("Tab 1");
    mui->tabWidget->addTab(tw, tabText);

    clearAll();
    addItem(tabData->tabText(0), "", "", "", 0);

    reloadMain();
  }

  saveTab();
}

void MainWindow::on_twItemClicked() {
  QTreeWidget* tw = (QTreeWidget*)mui->tabWidget->currentWidget();
  if (!tw->currentIndex().isValid()) return;

  QTreeWidgetItem* item = tw->currentItem();
  if (item->parent() == NULL && item->childCount() == 0) return;

  QTreeWidgetItem* pItem = NULL;

  QString stra;
  if (item->parent() == NULL) {
    CurrentYear = item->text(3);
    stra = item->text(0);
  } else {
    CurrentYear = item->parent()->text(3);
    stra = item->parent()->text(0);
  }
  tw->headerItem()->setText(0, "" + tr("Date") + "  " + CurrentYear);

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

  if (nMainChartType == 0) {
    QString str = stra + " " + CurrentYear;
    QString strYearMonth = get_Year(str) + get_Month(str);
    if (!isTabChanged) {
      if (strYearMonth == CurrentYearMonth) return;
    } else
      isTabChanged = false;
    startRead(strDate);
  }

  if (nMainChartType == 1) {
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
  QTreeWidget* tw = (QTreeWidget*)mui->tabWidget->currentWidget();
  QTreeWidgetItem* item = tw->currentItem();
  QTreeWidgetItem* topItem = item->parent();
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
      QTreeWidgetItem* childItem = topItem->child(i);

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

  QTreeWidget* tw = (QTreeWidget*)mui->tabWidget->currentWidget();
  QTreeWidgetItem* item = tw->currentItem();
  if (item->childCount() == 0 && item->parent()->childCount() > 0) {
    if (item->parent()->text(3).toInt() != QDate::currentDate().year()) {
      auto msg = std::make_unique<ShowMessage>(this);
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

  QTreeWidget* tw = (QTreeWidget*)tabData->widget(index);
  tw->setFocus();

  if (!loading) {
    QSettings Reg(iniDir + "tab.ini", QSettings::IniFormat);

    Reg.setValue("CurrentIndex", index);
  }

  reloadMain();

  isTabChanged = true;

  m_Method->clickMainDateData();
}

void MainWindow::on_btnModifyRecord_clicked() {
  m_Method->reeditMainEventData();
}

bool MainWindow::eventFilter(QObject* watch, QEvent* evn) {
  if (loading) return QWidget::eventFilter(watch, evn);

  m_MainHelper->mainEventFilter(watch, evn);

  return QWidget::eventFilter(watch, evn);
}

void MainWindow::clearWidgetFocus() {
  // InputMethodReset::instance().fullReset();
  if (QWidget* focused = focusWidget()) {
    focused->clearFocus();
  }

  closeTextToolBar();
}

void MainWindow::hideEvent(QHideEvent* event) { QWidget::hideEvent(event); }

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
    auto m_ShowMsg = std::make_unique<ShowMessage>(this);
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

void MainWindow::showProgress() {
  if (dlgProg) closeProgress();  // 先关旧的
  dlgProg = m_Method->getProgBar();
  if (!initMain) dlgProg->show();
}

void MainWindow::closeProgress() {
  if (!initMain && dlgProg) {
    dlgProg->close();  // 自动销毁，不需要 delete
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

QTreeWidget* MainWindow::get_tw(int tabIndex) {
  QTreeWidget* tw = (QTreeWidget*)tabData->widget(tabIndex);
  return tw;
}

void MainWindow::on_actionAbout() {
  QTextBrowser* textBrowser = new QTextBrowser;
  textBrowser->append("");
  textBrowser->append(appName + "  Ver: " + ver);

  textBrowser->append("");
  textBrowser->append(tr("Startup Time") + ": " + strStartTotalTime + " s" +
                      "\n" + loginTime + "\n" + "(c) 2022-" +
                      QString::number(QDate::currentDate().year()) +
                      " The Knot Authors");
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
  if (mui->qwSearch->source().isEmpty()) {
    mui->qwSearch->rootContext()->setContextProperty("m_Method", m_Method);
    mui->qwSearch->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/search.qml")));
  }

  mui->frameMain->hide();
  mui->frameSearch->show();
  mui->editSearchText->setFocus();
  mui->btnClearSearchText->setFixedHeight(mui->btnStartSearch->height());
}

void MainWindow::on_actionFind_triggered() { on_btnFind_clicked(); }

void MainWindow::on_btnTodo_clicked() { m_Todo->openTodo(); }

void MainWindow::on_rbFreq_clicked() {}

void MainWindow::on_rbAmount_clicked() {}

void MainWindow::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);

  // 获取背景色
  QPalette pal = mui->btnFind->palette();
  QBrush brush = pal.window();
  int c_red = brush.color().red();

  if (c_red != red) {
    red = c_red;
    if (red < 55) {
    }

    else {
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
  m_Preferences->openPreferences();
}

void MainWindow::on_tabCharts_currentChanged(int index) {}

void MainWindow::on_btnSteps_clicked() { m_Steps->openStepsUI(); }

void MainWindow::changeEvent(QEvent* event) {
  if (event->type() == QEvent::WindowStateChange) {
  }
}

void MainWindow::on_btnNotes_clicked() { m_Notes->openNotes(); }

void MainWindow::init_Instance() {
  mw_one = this;
  m_MainHelper = new MainHelper(this);
  CurrentYear = QString::number(QDate::currentDate().year());
  if (defaultFontFamily == "") defaultFontFamily = this->font().family();

  tabData = mui->tabWidget;

  m_Method = new Method(this);

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
  m_ReaderSet = new ReaderSet(this);

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
  if (mui->qwSelTab->source().isEmpty()) {
    mui->qwSelTab->rootContext()->setContextProperty("mw_one", mw_one);
    mui->qwSelTab->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/seltab.qml")));
  }

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
  if (mui->qwBakList->source().isEmpty()) {
    mui->qwBakList->rootContext()->setContextProperty("m_Method", m_Method);
    mui->qwBakList->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/baklist.qml")));
  }

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

void MainWindow::on_btnReader_clicked() { m_Reader->openReader(); }

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
  if (mui->qwViewBookNote->isVisible()) return;

  mui->btnAutoStop->click();

  if (mui->qwCata->isVisible()) return;

  if (mui->f_ReaderSet->isHidden()) {
    mui->lblTotalReading->setText(tr("Total Reading: ") +
                                  m_Reader->getReadTotalTime() + " h");
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
    m_Reader->updateReaderProperty(position, totalPages);
    mui->progReader->setMinimum(1);
    mui->progReader->setMaximum(totalPages);
    mui->progReader->setValue(position);
  }

  if (isEpub) {
    mui->btnPages->setText(QString::number(position) + "\n" +
                           QString::number(htmlFiles.count()));
    m_Reader->updateReaderProperty(position, htmlFiles.count());
    mui->progReader->setMinimum(1);
    mui->progReader->setMaximum(htmlFiles.count());
    if (position == 0) position = 1;
    mui->progReader->setValue(position);
  }

  m_ReaderSet->updateProgress();
}

void MainWindow::on_btnReadList_clicked() {
  if (mui->qwViewBookNote->isVisible()) return;

  if (mui->qwBookList->source().isEmpty()) {
    mui->qwBookList->rootContext()->setContextProperty("fontSize", fontSize);
    mui->qwBookList->rootContext()->setContextProperty("m_Reader",
                                                       mw_one->m_Reader);
    mui->qwBookList->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/booklist.qml")));
  }

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

void MainWindow::on_btnUpload_clicked() {
  if (!mui->btnReader->isEnabled()) return;
  m_CloudBackup->startBakData();
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

QString MainWindow::getSelectedText() {
  QString str;
  QVariant returnedValue;
  QQuickItem* root = mui->qwReader->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "getSelectedText",
                            Q_RETURN_ARG(QVariant, returnedValue));
  str = returnedValue.toString();
  return str.trimmed();
}

void MainWindow::on_btnSearch_clicked() {
  QString str = mui->editSetText->text().trimmed();
  if (str == "") return;

  QString strurl;
  strurl = "https://bing.com/search?q=" + str;

  QUrl url(strurl);
  QDesktopServices::openUrl(url);
}

void MainWindow::on_btnCancelSel_clicked() {
  m_Reader->resetTextSelection();

  mui->f_ReaderNote->hide();

  mui->qwReader->show();
  mui->f_ReaderFun->show();
  m_Reader->isSelText = false;
}

void MainWindow::on_timerMousePress() {
  if (!isMouseMove && isMousePress) on_btnSelText_clicked();
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

void MainWindow::on_btnReport_clicked() {
  if (mui->qwReport->source().isEmpty()) {
    int f_size = 19;
    if (fontSize <= f_size) f_size = fontSize;
    mui->qwReport->rootContext()->setContextProperty("maxFontSize", f_size);
    mui->qwReportSub->rootContext()->setContextProperty("maxFontSize", f_size);
    mui->qwReport->rootContext()->setContextProperty("m_Report",
                                                     mw_one->m_Report);
    mui->qwReport->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/report.qml")));
    mui->qwReportSub->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/details.qml")));
  }

  on_actionReport_triggered();
  mui->btnYear->setFixedHeight(mui->btnMonth->height());
}

void MainWindow::on_btnAdd_clicked() {
  m_EditRecord->monthSum();

  m_MainHelper->on_AddRecord();
}

void MainWindow::on_btnDel_clicked() {
  isMoveEntry = false;
  del_Data((QTreeWidget*)mui->tabWidget->currentWidget());
}

void MainWindow::resizeEvent(QResizeEvent* event) {
  Q_UNUSED(event);
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
  QQuickItem* root = mui->qwMainTab->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "setItemHeight",
                            Q_ARG(QVariant, h));
}

void MainWindow::addItem(QString text0, QString text1, QString text2,
                         QString text3, int itemH) {
  QQuickItem* root = mui->qwMainTab->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "addItem", Q_ARG(QVariant, text0),
                            Q_ARG(QVariant, text1), Q_ARG(QVariant, text2),
                            Q_ARG(QVariant, text3), Q_ARG(QVariant, itemH));
}

QString MainWindow::getTop(int index) {
  QQuickItem* root = mui->qwMainTab->rootObject();
  QVariant itemTime;
  QMetaObject::invokeMethod((QObject*)root, "getTop",
                            Q_RETURN_ARG(QVariant, itemTime),
                            Q_ARG(QVariant, index));
  return itemTime.toString();
}

QString MainWindow::getText0(int index) {
  QQuickItem* root = mui->qwMainTab->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject*)root, "getText0",
                            Q_RETURN_ARG(QVariant, item),
                            Q_ARG(QVariant, index));
  return item.toString();
}

QString MainWindow::getText1(int index) {
  QQuickItem* root = mui->qwMainTab->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject*)root, "getText1",
                            Q_RETURN_ARG(QVariant, item),
                            Q_ARG(QVariant, index));
  return item.toString();
}

QString MainWindow::getText2(int index) {
  QQuickItem* root = mui->qwMainTab->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject*)root, "getText2",
                            Q_RETURN_ARG(QVariant, item),
                            Q_ARG(QVariant, index));
  return item.toString();
}

int MainWindow::getItemType(int index) {
  QQuickItem* root = mui->qwMainTab->rootObject();
  QVariant itemType;
  QMetaObject::invokeMethod((QObject*)root, "getType",
                            Q_RETURN_ARG(QVariant, itemType),
                            Q_ARG(QVariant, index));
  return itemType.toInt();
}

void MainWindow::delItem(int index) {
  QQuickItem* root = mui->qwMainTab->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "delItem", Q_ARG(QVariant, index));
}

int MainWindow::getCount() {
  QQuickItem* root = mui->qwMainTab->rootObject();
  QVariant itemCount;
  QMetaObject::invokeMethod((QObject*)root, "getItemCount",
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
  QQuickItem* root = mui->qwMainTab->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "setCurrentItem",
                            Q_ARG(QVariant, index));
}

void MainWindow::gotoEnd() {
  QQuickItem* root = mui->qwMainTab->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "gotoEnd");
}

void MainWindow::gotoIndex(int index) {
  QQuickItem* root = mui->qwMainTab->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "gotoIndex",
                            Q_ARG(QVariant, index));
}

int MainWindow::getCurrentIndex() {
  QQuickItem* root = mui->qwMainTab->rootObject();
  QVariant itemIndex;
  QMetaObject::invokeMethod((QObject*)root, "getCurrentIndex",
                            Q_RETURN_ARG(QVariant, itemIndex));
  return itemIndex.toInt();
}

void MainWindow::setScrollBarPos(double pos) {
  QQuickItem* root = mui->qwMainTab->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "setScrollBarPos",
                            Q_ARG(QVariant, pos));
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

  QTreeWidget* tw = get_tw(tabData->currentIndex());
  int count = tw->topLevelItemCount();
  for (int i = 0; i < count; i++) {
    QTreeWidgetItem* topItem = tw->topLevelItem(count - 1 - i);
    if (topItem->text(0) + "  " + topItem->text(3) == textTop) {
      if (type == 0) {
        QTreeWidgetItem* childItem = topItem->child(childIndex);
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

void MainWindow::on_btnBackSteps_clicked() { m_Steps->closeSteps(); }

void MainWindow::on_btnReset_clicked() { m_Steps->on_btnReset_clicked(); }

void MainWindow::on_btnBack_Report_clicked() { m_Report->on_btnBack_clicked(); }

void MainWindow::on_btnYear_clicked() { m_Report->on_btnYear_clicked(); }

void MainWindow::on_btnMonth_clicked() { m_Report->on_btnMonth_clicked(); }

void MainWindow::on_btnCategory_clicked() {
  m_Report->on_btnCategory_clicked();
}

void MainWindow::on_btnSync_clicked() { on_btnUpload_clicked(); }

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

  mui->frameMain->show();
  mui->frameSearch->hide();
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
  clearWidgetFocus();

  mui->frameMain->show();
  mui->frameNoteList->hide();

  m_NotesList->saveCurrentNoteInfo();
  m_NotesList->saveNotesListIndex();
  m_Notes->updateMainnotesIniToSyncLists();

  saveNeedSyncNotes();

  m_Notes->syncToWebDAV();

  if (mui->chkAutoSync->isChecked() && mui->chkWebDAV->isChecked()) {
    int count = m_NotesList->needDelWebDAVFiles.count();
    if (count > 0) m_Notes->delRemoteFile(m_NotesList->needDelWebDAVFiles);
    m_Method->setAccessCount(m_NotesList->needDelWebDAVFiles.count());
  }
}

void MainWindow::on_btnBackNoteRecycle_clicked() {
  mui->frameNoteRecycle->hide();
  mui->frameNoteList->show();

  if (m_NotesList->isDelNoteRecycle) {
    m_Notes->startBackgroundTaskDelAndClear();
    m_NotesList->isDelNoteRecycle = false;
  }
}

void MainWindow::on_btnNoteRecycle_clicked() {
  mui->frameNoteList->hide();
  mui->frameNoteRecycle->show();

  m_NotesList->needDelWebDAVFiles.clear();

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

  m_NotesList->updateAllNoteIndexManager();
}

void MainWindow::on_btnFindNotes_clicked() {
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
  mui->frameMain->show();
  mui->frameSetTab->hide();

  if (mui->btnTabMoveDown->isHidden()) {
    mui->btnTabMoveDown->show();
    mui->btnTabMoveUp->show();
    on_btnAdd_clicked();
    m_EditRecord->setCurrentValue();
  }
}

void MainWindow::on_btnBackEditRecord_clicked() {
  clearWidgetFocus();

  mui->frameMain->show();
  mui->frameEditRecord->hide();
}

void MainWindow::on_btnType_clicked() { m_EditRecord->on_btnCustom_clicked(); }

void MainWindow::on_btnOkEditRecord_clicked() {
  m_EditRecord->on_btnOk_clicked();
}

void MainWindow::on_btnClearType_clicked() { mui->editCategory->setText(""); }

void MainWindow::on_btnClearDetails_clicked() { mui->editDetails->setText(""); }

void MainWindow::on_btnClearAmount_clicked() { mui->editAmount->setText(""); }

void MainWindow::on_editAmount_textChanged(const QString& arg1) {
  m_EditRecord->on_editAmount_textChanged(arg1);
}

void MainWindow::on_editCategory_textChanged(const QString& arg1) {
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
      mui->frameMain->show();
      mui->frameBookList->hide();
    }
  } else {
    mui->frameReader->show();
    mui->frameBookList->hide();
  }
}

void MainWindow::on_btnOkBookList_clicked() { m_Reader->openBookListItem(); }

void MainWindow::on_btnClearAllRecords_clicked() {
  m_Reader->clearAllReaderRecords();
}

void MainWindow::on_btnAnd_clicked() { mui->editSearchText->insert("&"); }

void MainWindow::on_btnClear_clicked() { mui->editTodo->clear(); }

void MainWindow::on_btnModify_clicked() { m_Todo->reeditText(); }

void MainWindow::on_btnChartMonth_clicked() {}

void MainWindow::on_btnChartDay_clicked() {}

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

void MainWindow::on_btnChart_clicked() { m_MainHelper->clickBtnChart(); }

void MainWindow::on_btnManagement_clicked() {
  int x, y, w, h;
  x = geometry().x();
  y = geometry().y();
  w = width();
  h = height();
  m_NotesList->setGeometry(x, y, w, h);
  m_NotesList->show();
  tw->setFocus();
}

void MainWindow::on_btnUpMove_clicked() {
  if (m_Method->getCountFromQW(mui->qwNoteBook) == 0) return;

  m_NotesList->setTWCurrentItem();
  m_NotesList->on_btnUp_clicked();
}

void MainWindow::on_btnDownMove_clicked() {
  if (m_Method->getCountFromQW(mui->qwNoteBook) == 0) return;

  m_NotesList->setTWCurrentItem();
  m_NotesList->on_btnDown_clicked();
}

void MainWindow::on_btnDelNote_NoteBook_clicked() {
  m_NotesList->setTWCurrentItem();
  m_NotesList->on_btnDel_clicked();

  m_NotesList->updateAllNoteIndexManager();
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

void MainWindow::on_btnHideFind_clicked() {
  closeTextToolBar();
  mui->f_FindNotes->hide();
}

void MainWindow::on_btnStepsOptions_clicked() { m_StepsOptions->init(); }

void MainWindow::on_btnRecentOpen_clicked() {
  m_NotesList->genRecentOpenMenu();
}

void MainWindow::on_btnMenuReport_clicked() { m_Report->genReportMenu(); }

void MainWindow::on_btnCatalogue_clicked() {
  if (mui->qwViewBookNote->isVisible()) return;

  if (mui->qwCata->source().isEmpty()) {
    mui->qwCata->rootContext()->setContextProperty("m_Reader",
                                                   mw_one->m_Reader);
    mui->qwCata->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/epub_cata.qml")));
  }

  mui->btnAutoStop->click();

  if (mui->f_ReaderSet->isVisible()) {
    on_btnBackReaderSet_clicked();
  }
  m_Reader->showCatalogue();
}

void MainWindow::on_btnRemoveBookList_clicked() { m_Reader->removeBookList(); }

void MainWindow::on_btnShowBookmark_clicked() {
  if (mui->qwViewBookNote->isVisible()) return;

  if (mui->qwBookmark->source().isEmpty()) {
    mui->qwBookmark->rootContext()->setContextProperty("m_Reader",
                                                       mw_one->m_Reader);
    mui->qwBookmark->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/bookmark.qml")));
  }

  m_Reader->showOrHideBookmark();
}

void MainWindow::on_btnShareImage_clicked() {
  m_ReceiveShare->shareImage(tr("Share to"), imgFileName, "image/png");
}

void MainWindow::on_btnHideKey_clicked() { pAndroidKeyboard->hide(); }

void MainWindow::on_btnDelImage_clicked() {}

void MainWindow::on_btnBackReaderSet_clicked() {
  closeTextToolBar();
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
  if (mui->qwViewBookNote->isVisible()) return;

  if (!m_Reader->isAutoRun) {
    // m_Reader->tmeAutoRun->start(50);

    mui->qwReader->rootContext()->setContextProperty("isAutoRun", true);
    mui->btnAutoRun->hide();
    mui->btnAutoStop->show();
    m_Reader->isAutoRun = true;
  }
}

void MainWindow::on_btnAutoStop_clicked() {
  if (m_Reader->isAutoRun) {
    // m_Reader->tmeAutoRun->stop();

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
  if (del_Data((QTreeWidget*)mui->tabWidget->currentWidget())) {
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

void MainWindow::on_btnEditNote_clicked() { m_Notes->openEditUI(); }

void MainWindow::on_btnToPDF_clicked() {
  if (!QFile::exists(currentMDFile)) return;

  m_Notes->on_btnPDF_clicked();
}

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
  m_NotesList->setCurrentItemFromMDFile(mdFile);
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

void MainWindow::on_btnCopyNoteLink_clicked() {
  QString mdFile = m_NotesList->getSearchResultQmlFile();
  if (!QFile::exists(mdFile)) return;
  QString file = mdFile;
  file = file.replace(iniDir, "");
  QString name = m_Notes->m_NoteIndexManager->getNoteTitle(mdFile);
  QString strlink = "[" + name + "](" + file + ")";
  QClipboard* clipboard = QApplication::clipboard();
  clipboard->setText(strlink);

  auto msg = std::make_unique<ShowMessage>(this);
  msg->showMsg(appName, strlink, 1);
}

void MainWindow::on_cboxWebDAV_currentTextChanged(const QString& arg1) {
  m_CloudBackup->changeComBoxWebDAV(arg1);
}

void MainWindow::on_btnShowCboxList_clicked() { mui->cboxWebDAV->showPopup(); }

void MainWindow::on_btnRotation_clicked() {
  if (mui->qwViewBookNote->isVisible()) return;

  QQuickItem* rootItem = mui->qwReader->rootObject();
  QQuickItem* orientationButton =
      rootItem->findChild<QQuickItem*>("orientationButton");
  if (orientationButton) {
    QMetaObject::invokeMethod(orientationButton, "clicked");
    m_Reader->isLandscape = !m_Reader->isLandscape;
    m_Reader->readReadNote(m_Reader->cPage);
  }
}

void MainWindow::on_btnBackNoteDiff_clicked() { m_NotesList->closeNoteDiff(); }

void MainWindow::ReadChartData() {
  int index = tabData->currentIndex();
  QTreeWidget* tw = (QTreeWidget*)tabData->widget(index);

  strY = get_Year(readDate);
  strM = get_Month(readDate);

  if (nMainChartType == 0) {
    drawMonthChart();
  }
  if (nMainChartType == 1) {
    drawDayChart();
  }

  get_Today(tw);
  init_Stats(tw);
}

void MainWindow::SaveFile(QString SaveType) {
  if (SaveType == "tab") {
    EditRecord::saveCurrentYearData();

    saveTab();

    EditRecord::saveMyClassification();

    isAdd = false;
    isDelData = false;
  }
}

void MainWindow::on_btnSendEmail_clicked() {
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

void MainWindow::on_btnAddBookNote_clicked() { m_Reader->addBookNote(); }

void MainWindow::on_btnViewBookNote_clicked() { m_Reader->viewBookNote(); }

void MainWindow::on_btnMap_clicked() { m_Steps->openMapWindow(); }

void MainWindow::on_btnSportsChart_clicked() { m_Steps->showSportsChart(); }
