#include "MyThread.h"

#include "src/MainWindow.h"
#include "src/defines.h"
#include "ui_MainWindow.h"

MyThread::MyThread() {}

ReadTWThread::ReadTWThread(QObject* parent) : QThread{parent} {}
void ReadTWThread::run() {
  isReadTWEnd = false;
  MainWindow::readDataInThread(currentTabIndex);
  emit isDone();
}

void MainWindow::readTWDone() {
  for (int i = 0; i < tabData->tabBar()->count(); i++) {
    QTreeWidget* tw = (QTreeWidget*)tabData->widget(i);
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

ReadChartThread::ReadChartThread(QObject* parent) : QThread{parent} {}
void ReadChartThread::run() {
  if (isBreak) {
    emit isDone();
    return;
  }
  isReadEnd = false;
  MainWindow::ReadChartData();
  emit isDone();
}

void MainWindow::readChartDone() {
  if (isShowDetails)
    mui->lblStats->setText(strShowDetails);
  else
    mui->lblStats->setText(strStats);
  isReadEnd = true;

  // qDebug() << "Read Chart End ..." << freqPointList << amountList;

  max_day = getMaxDay(QString::number(QDate::currentDate().year()),
                      QString::number(QDate::currentDate().month()));
  // 1. 初始化完整数组（补0）
  QList<double> freqValues(max_day, 0.0);  // 长度max_day，全0
  QList<double> amountValues(max_day, 0.0);

  // 2. 填充频次数据（有数据的日期替换0）
  // 频次循环
  for (int i = 0; i < freqPointList.count(); ++i) {
    const QPointF& p = freqPointList[i];
    int day = qRound(p.x());
    if (day >= 1 && day <= max_day) {
      freqValues[day - 1] = p.y();
    }
  }

  // 3. 金额循环
  for (int i = 0; i < amountList.count(); ++i) {
    const QPointF& p = amountList[i];
    int day = qRound(p.x());
    if (day >= 1 && day <= max_day) {
      amountValues[day - 1] = p.y();
    }
  }

  // 4. 准备x轴分类（1~max_day的字符串列表，比如["1","2",..."31"]）
  chartCategories.clear();
  for (int i = 1; i <= max_day; ++i) {
    chartCategories.append(QString::number(i));
  }

  // 5. 转成QML兼容的QList<QVariant>（BarSet.values需要这个类型）
  qmlFreqValues.clear();
  qmlAmountValues.clear();
  qmlFreqValues = QVariant::fromValue(freqValues).toList();
  qmlAmountValues = QVariant::fromValue(amountValues).toList();

  // 6. 直接暴露给QML
  mui->qwMainChart->rootContext()->setContextProperty("chartCategories",
                                                      chartCategories);
  mui->qwMainChart->rootContext()->setContextProperty("chartFreqValues",
                                                      qmlFreqValues);
  mui->qwMainChart->rootContext()->setContextProperty("chartAmountValues",
                                                      qmlAmountValues);

  // qDebug() << "chartCategories=" << chartCategories
  //          << "qmlFreqValues=" << qmlFreqValues
  //          << "qmlAmountValues=" << qmlAmountValues;
}

SaveThread::SaveThread(QObject* parent) : QThread{parent} {}
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

UpdateGpsMapThread::UpdateGpsMapThread(QObject* parent) : QThread{parent} {}
void UpdateGpsMapThread::run() {
  m_Steps->updateGpsTrack();

  emit isDone();
}

void MainWindow::updateGpsMapDone() {
  m_Steps->updateGpsMapUi();
  closeProgress();
}

ReadEBookThread::ReadEBookThread(QObject* parent) : QThread{parent} {}
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
    isEBook = false;

    if (!isOpen) {
      on_DelayCloseProgressBar();
      return;
    }

    m_Reader->readBookDone();
  }

  if (isReport) {
    QTimer::singleShot(100, this,  // 上下文直接用this（等价于mw_one）
                       [this]() {  // 仅捕获this（MainWindow的指针）
                         // 空指针检查：确保m_Report不为空
                         if (this->m_Report) {
                           this->m_Report->updateTable();
                         }
                       });

    mui->lblTitle->setText(tabData->tabText(tabData->currentIndex()));

    mui->btnCategory->hide();
    if (listCategory.count() > 0) mui->btnCategory->setHidden(false);

    isReport = false;
    closeProgress();
  }

  isReadEBookEnd = true;
}

SearchThread::SearchThread(QObject* parent) : QThread{parent} {}
void SearchThread::run() {
  m_Method->startSearch();

  emit isDone();
}

void MainWindow::searchDone() {
  m_Method->initSearchResults();
  mw_one->closeProgress();
}

ImportDataThread::ImportDataThread(QObject* parent) : QThread{parent} {}
void ImportDataThread::run() {
  if (isMenuImport || isDownData) mw_one->importBakData(zipfile);

  emit isDone();
}

void MainWindow::importDataDone() {
  m_Method->setOSFlag();

  if (isPasswordError) {
    closeProgress();
    auto msg = std::make_unique<ShowMessage>(this);
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

    m_NotesList->startBackgroundTaskUpdateFilesIndex();
  }

  closeProgress();

  if (isMenuImport) {
    if (!isZipOK) {
      auto msg = std::make_unique<ShowMessage>(this);
      msg->showMsg("Knot",
                   tr("Invalid data file.") + "\n\n" +
                       tr("Or the operation is canceled by the user."),
                   1);

      return;
    }
  }

  auto msg = std::make_unique<ShowMessage>(this);
  msg->showMsg("Knot", tr("Data import was successful."), 1);
}

BakDataThread::BakDataThread(QObject* parent) : QThread{parent} {}
void BakDataThread::run() {
  mw_one->bakData();

  emit isDone();
}

void MainWindow::bakDataDone() {
  // 适配 Qt 6.6.3：替换 uiThread() 为兼容写法
  QThread* mainThread = QCoreApplication::instance()->thread();
  if (QThread::currentThread() != mainThread) {
    QMetaObject::invokeMethod(this, &MainWindow::bakDataDone,
                              Qt::QueuedConnection);
    return;
  }

  closeProgress();

  if (!errorInfo.isEmpty()) {
    ShowMessage* msg = new ShowMessage(this);
    msg->showMsg("Knot", errorInfo, 1);
    msg->setAttribute(Qt::WA_DeleteOnClose);
    return;
  }

  if (isUpData) {
    m_CloudBackup->uploadData();
  } else {
    QFile zipFile(zipfile);
    if (zipFile.exists()) {
      QString fileSize = m_Method->getFileSize(zipFile.size(), 2);
      // 使用多参数版 arg()
      QString bakInfo =
          QString("%1\n%2\n%3")
              .arg(QDateTime::currentDateTime().toString("yyyy-M-d HH:mm:ss"),
                   strLatestModify, fileSize);

      m_Preferences->appendBakFile(bakInfo, zipfile);

      ShowMessage* msg = new ShowMessage(this);
      QString msgContent = tr("The data was exported successfully.") + "\n\n" +
                           zipfile + "\n\n" + fileSize;
      msg->showMsg("Knot", msgContent, 1);
      msg->setAttribute(Qt::WA_DeleteOnClose);
    }
  }

  isUpData = false;
}
