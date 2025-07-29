#include "MyThread.h"

#include "src/MainWindow.h"
#include "ui_MainWindow.h"

extern int currentTabIndex;

extern bool isReadTWEnd, isBreak, isReadEnd, isSaveEnd, isReadEBookEnd, isEBook,
    isReport, isMenuImport, isDownData, isUpData, isPasswordError, isZipOK,
    loading;

extern QString SaveType, ebookFile, zipfile, strStats, errorInfo, iniDir,
    strDate;

extern QStringList readTextList, htmlFiles, listCategory;

extern MainWindow *mw_one;
extern Ui::MainWindow *mui;
extern Method *m_Method;
extern QTabWidget *tabData, *tabChart;
extern CloudBackup *m_CloudBackup;

MyThread::MyThread() {}

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

void MainWindow::readChartDone() {
  if (tabChart->currentIndex() == 0) {
    m_MainHelper->initChartMonth();
  }
  if (tabChart->currentIndex() == 1) {
    m_MainHelper->initChartDay();
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

SearchThread::SearchThread(QObject *parent) : QThread{parent} {}
void SearchThread::run() {
  m_Method->startSearch();

  emit isDone();
}

void MainWindow::searchDone() {
  m_Method->initSearchResults();
  mw_one->closeProgress();
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
