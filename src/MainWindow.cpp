#include "MainWindow.h"

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

  init_UIWidget();

  initMainQW();

  init_TotalData();

  loading = false;

  QTreeWidget* tw = (QTreeWidget*)tabData->currentWidget();
  startRead(strDate);
  get_Today(tw);
  init_Stats(tw);

  resetWinPos();

  m_Reader->initReader();

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
  on_btnSync_pressed();
}

void MainWindow::startSyncData() {
  if (initMain) return;
  timerSyncData->start(10000);
}

MainWindow::~MainWindow() {
  delete mui;

  if (m_EditRecord != nullptr) {
    delete m_EditRecord;
    m_EditRecord = nullptr;
  }

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

  // ========== 1. 处理一次性线程（仅执行一次，自动退出） ==========
  // 模板函数：统一处理一次性非子对象线程（wait()等待完成 + delete释放）
  auto releaseOneShotThread = [](auto& thread) {  // 用auto&替代QThread*&
    if (thread) {
      if (thread->isRunning()) {
        thread->wait();
      }
      delete thread;
      thread = nullptr;
    }
  };

  // 逐个处理所有一次性线程
  releaseOneShotThread(mySaveThread);
  releaseOneShotThread(myReadChartThread);
  releaseOneShotThread(m_ReadTWThread);
  releaseOneShotThread(myReadEBookThread);
  releaseOneShotThread(myBakDataThread);
  releaseOneShotThread(myImportDataThread);
  releaseOneShotThread(mySearchThread);
  releaseOneShotThread(myUpdateGpsMapThread);

  // ========== 2. 处理循环线程（GetGpsDataThread，已有stop()） ==========
  if (myGetGpsDataThread) {
    if (myGetGpsDataThread->isRunning()) {
      myGetGpsDataThread->stop();  // 停止循环
      myGetGpsDataThread->wait();  // 等待退出
    }
    delete myGetGpsDataThread;  // 释放内存
    myGetGpsDataThread = nullptr;
  }

  delete iniPreferences;
}

void MainWindow::startSave(QString str_type) {
  if (!isSaveEnd) {
    isBreak = true;
    mySaveThread->quit();
    mySaveThread->wait();

    while (!isSaveEnd) {
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
      QThread::msleep(1);
    }
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

    while (!isReadEnd) {
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
      QThread::msleep(1);
    }
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
    on_btnCatalogue_pressed();
    event->ignore();
    return;
  }

  if (mui->qwBookList->isVisible()) {
    on_btnBackBookList_pressed();
    event->ignore();
    return;
  }

  if (mui->frameReader->isVisible()) {
    on_btnBackReader_pressed();
    event->ignore();
    return;
  }

  if (mui->qwMainChart->isVisible()) {
    on_btnChart();
    event->ignore();
    return;
  }

  if (!mui->frameImgView->isHidden()) {
    on_btnBackImg_pressed();
    event->ignore();
    return;
  }

  if (!mui->frameNoteRecycle->isHidden()) {
    on_btnBackNoteRecycle_clicked();
    event->ignore();
    return;
  }

  if (!mui->frameNotesSearchResult->isHidden()) {
    on_btnBack_NotesSearchResult_pressed();
    event->ignore();
    return;
  }

  if (!mui->frameNoteList->isHidden()) {
    on_btnBackNoteList_pressed();
    event->ignore();
    return;
  }

  if (!mui->frameDiff->isHidden()) {
    on_btnBackNoteDiff_pressed();
    event->ignore();
    return;
  }

  if (!mui->frameNotesGraph->isHidden()) {
    on_btnBackNotesGraph_pressed();
    event->ignore();
    return;
  }

  if (!mui->frameTodoRecycle->isHidden()) {
    on_btnReturnRecycle_pressed();
    event->ignore();
    return;
  }

  if (m_Todo->isTodoAlarmShow) {
    m_Todo->closeTodoAlarm();
    event->ignore();
    return;
  }

  if (!mui->frameTodo->isHidden()) {
    on_btnBackTodo_pressed();
    event->ignore();
    return;
  }

  if (!mui->frameBakList->isHidden()) {
    on_btnBackBakList_pressed();
    event->ignore();
    return;
  }

  if (!mui->frameOne->isHidden()) {
    on_btnBack_One_pressed();
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

// ===== 主动释放共享内存 =====
#ifndef Q_OS_ANDROID
  sharedMemory.setKey(uniqueKey);
  if (sharedMemory.attach()) {
    sharedMemory.detach();
  }
#endif
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
  if (count > 0) {
    for (int i = 0; i < count; i++) {
      QString note = RegSync.value("note" + QString::number(i), "").toString();
      if (QFile::exists(note)) {
        m_Notes->appendToSyncList(note);
      }
    }

    if (m_Notes->notes_sync_files.count() > 0)
      m_Notes->syncToWebDAV();
    else
      emit m_Notes->syncFinished();
  } else {
    emit m_Notes->syncFinished();
  }
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

    QTimer::singleShot(100, mw_one, [this, maindateIndex, newrow]() {
      m_Method->setCurrentIndexFromQW(mui->qwMainDate, maindateIndex);
      isEditItem = true;
      m_Method->clickMainDate();
      m_Method->setCurrentIndexFromQW(mui->qwMainEvent, newrow);
    });
  }
}

void MainWindow::on_twItemDoubleClicked() {
  // m_EditRecord->monthSum();

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

    mw_one->m_EditRecord->isNoShowSuggestions = true;
    mui->editCategory->setText(item->text(2));
    mw_one->m_EditRecord->isNoShowSuggestions = false;
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

  mui->qwMainDate->show();
  mui->qwMainEvent->show();
  mui->lblStats->show();

  mui->lblTabTitle->setText(mui->tabWidget->tabBar()->tabText(index));
  mui->lblTabTitle->show();

  mui->qwMainTab->hide();

  if (isSelectTab) {
    on_btnAdd_pressed();
    m_EditRecord->setCurrentValue();
    isSelectTab = false;
  }
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

  mui->lblTabTitle->setText(mui->tabWidget->tabBar()->tabText(index));

  reloadMain();

  isTabChanged = true;

  m_Method->clickMainDateData();
}

bool MainWindow::eventFilter(QObject* watch, QEvent* evn) {
  if (loading) return QWidget::eventFilter(watch, evn);

  m_MainHelper->mainEventFilter(watch, evn);

  /*if (evn->type() == QEvent::KeyPress) {
    QKeyEvent* key = static_cast<QKeyEvent*>(evn);
    if (key->key() == Qt::Key_Back) {
      onAndroidBackHandle();

      return true;
    }
  }*/

  /*if (watch == mui->qwMainTab && evn->type() == QEvent::Show) {
    QTimer::singleShot(200, this, [this]() {
      sendFakeTouch();

      QQuickItem* root = mui->qwMainTab->rootObject();
      if (root) {
        QMetaObject::invokeMethod(root, "forceActivateUI",
                                  Qt::QueuedConnection);
      }
    });
  }*/

  return QWidget::eventFilter(watch, evn);
}

void MainWindow::sendFakeTouch() {
  QWidget* w = mui->frameMain;
  if (!w) return;

  auto send = [&](const QPoint& p) {
    const QPointF pf(p);

    QMouseEvent press(QEvent::MouseButtonPress, pf, pf, Qt::LeftButton,
                      Qt::LeftButton, Qt::NoModifier,
                      QPointingDevice::primaryPointingDevice());
    QApplication::sendEvent(w, &press);

    QMouseEvent release(QEvent::MouseButtonRelease, pf, pf, Qt::LeftButton,
                        Qt::LeftButton, Qt::NoModifier,
                        QPointingDevice::primaryPointingDevice());
    QApplication::sendEvent(w, &release);

    qDebug() << pf << "被点击";
  };

  send(QPoint(0, 0));
  send(QPoint(1, 1));
}

void MainWindow::clearWidgetFocus() {
  // InputMethodReset::instance().fullReset();
  if (QWidget* focused = focusWidget()) {
    focused->clearFocus();
  }

  closeTextToolBar();
}

void MainWindow::hideEvent(QHideEvent* event) { QWidget::hideEvent(event); }

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

void MainWindow::showProgress() {
  if (dlgProg) closeProgress();  // 先关旧的
  dlgProg = m_Method->getProgBar();
  if (!initMain) dlgProg->show();
}

void MainWindow::closeProgress() {
  if (!initMain && dlgProg) {
    dlgProg->close();
    dlgProg->deleteLater();
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

void MainWindow::startInitReport() {
  showProgress();

  isReport = true;
  myReadEBookThread->start();
}

void MainWindow::changeEvent(QEvent* event) {
  if (event->type() == QEvent::WindowStateChange) {
  }
}

void MainWindow::selTab() {
  int index = m_Method->getCurrentIndexFromQW(mui->qwSelTab);
  tabData->setCurrentIndex(index);
  on_btnBackSetTab_pressed();
  m_Method->clearAllBakList(mui->qwSelTab);

  if (mui->btnTabMoveDown->isHidden()) {
    mui->btnTabMoveDown->show();
    mui->btnTabMoveUp->show();
    on_btnAdd_pressed();
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

void MainWindow::on_openKnotBakDir() {
#ifdef Q_OS_ANDROID

  QJniObject m_activity = QNativeInterface::QAndroidApplication::context();
  m_activity.callMethod<void>("openKnotBakDir", "()V");

#endif
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

QString MainWindow::getTabText() {
  return tabData->tabText(tabData->currentIndex());
}

void MainWindow::refreshMainUI() {
  this->update();
  this->repaint();
  qApp->processEvents();
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

void MainWindow::on_timerMousePress() {
  if (!isMouseMove && isMousePress) on_btnSelText();
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

void MainWindow::on_StartRecordAudio() {
  tmeStartRecordAudio->stop();

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

void MainWindow::on_rbCycling_pressed() {}

void MainWindow::on_rbHiking_pressed() {}

void MainWindow::on_rbRunning_pressed() {}

void MainWindow::on_editFindNote_returnPressed() { on_btnFindNotes_clicked(); }

void MainWindow::on_cboxWebDAV_currentTextChanged(const QString& arg1) {
  m_CloudBackup->changeComBoxWebDAV(arg1);
}

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

void MainWindow::on_chkPlayRunVoice_clicked(bool checked) {
  m_Steps->isChkPlayRunVoice = checked;
  if (checked) {
    isPlayBook = false;
    m_Method->stopPlayMyText();
    m_Method->playMyText(mui->lblDirection->text());
  }
}

void MainWindow::on_tabMotion_currentChanged(int index) {
  if (index == 0)
    m_Steps->tmeRefreshSteps->start(3000);
  else
    m_Steps->tmeRefreshSteps->stop();
}

void MainWindow::on_chkZip_clicked() { m_Preferences->on_chkZip_clicked(); }

void MainWindow::on_editPassword_textChanged(const QString& arg1) {
  m_Preferences->on_editPassword_textChanged(arg1);
}

void MainWindow::on_editValidate_textChanged(const QString& arg1) {
  m_Preferences->on_editValidate_textChanged(arg1);
}

void MainWindow::on_editAutoStopTTS_textChanged(const QString& arg1) {
  if (arg1.length() > 0) m_Reader->setAutoStopPlayTime();
}

void MainWindow::on_chkAutoStopTTS_clicked(bool checked) {
  if (checked)
    m_Reader->setAutoStopPlayTime();
  else
    m_Reader->m_autoStopDeadline = QDateTime();
}

void MainWindow::showEvent(QShowEvent* event) { QMainWindow::showEvent(event); }
