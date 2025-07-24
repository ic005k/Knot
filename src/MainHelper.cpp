#include "MainHelper.h"

#include "src/MainWindow.h"
#include "ui_MainWindow.h"

extern MainWindow *mw_one;
extern Ui::MainWindow *mui;
extern Method *m_Method;
extern QTabWidget *tabData;
extern QString iniDir, searchStr, currentMDFile, privateDir, encPassword,
    errorInfo;

MainHelper::MainHelper(QWidget *parent) : QWidget{parent} {}

void MainHelper::clickBtnChart() {
  mw_one->axisY->setTickCount(7);
  mw_one->axisY2->setTickCount(7);

  if (mui->f_charts->isHidden()) {
    mui->qwMainDate->hide();
    mui->qwMainEvent->hide();

    mui->f_charts->setMaximumHeight(mw_one->height());
    mui->f_charts->show();
    mui->btnChartDay->show();
    mui->btnChartMonth->show();
    mui->rbAmount->show();
    mui->rbFreq->show();
    mui->rbSteps->show();
    mui->f_cw->show();

    mui->btnReport->hide();
    mui->btnFind->hide();
    mui->btnModifyRecord->hide();
    mui->btnMove->hide();
  } else {
    mui->f_charts->setMaximumHeight(0);
    mui->f_charts->hide();
    mui->rbAmount->hide();
    mui->rbFreq->hide();
    mui->rbSteps->hide();
    mui->btnChartDay->hide();
    mui->btnChartMonth->hide();

    mui->qwMainDate->show();
    mui->qwMainEvent->show();
    mui->btnReport->show();
    mui->btnFind->show();
    mui->btnModifyRecord->show();
    mui->btnMove->show();
  }
}

void MainHelper::clickBtnRestoreTab() {
  if (m_Method->getCountFromQW(mui->qwTabRecycle) == 0) return;

  int count = mui->tabWidget->tabBar()->count();
  QString twName =
      mw_one->m_Notes->getDateTimeStr() + "_" + QString::number(count + 1);

  int c_year = QDate::currentDate().year();
  int iniFileCount = c_year - 2025 + 1 + 1;

  int index = m_Method->getCurrentIndexFromQW(mui->qwTabRecycle);
  QString recycle = m_Method->getText3(mui->qwTabRecycle, index);
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

  QString tab_name = m_Method->getText0(mui->qwTabRecycle, index);
  QTreeWidget *tw = mw_one->init_TreeWidget(twName);
  mui->tabWidget->addTab(tw, tab_name);

  mw_one->addItem(tab_name, "", "", "", 0);
  mw_one->setCurrentIndex(count);

  mw_one->readData(tw);

  if (recycleList.count() > 1) {
    for (int i = 0; i < recycleList.count(); i++) {
      QFile recycle_file(recycle.split("\n").at(i));
      recycle_file.remove();
    }
  } else {
    QFile recycle_file(recycle);
    recycle_file.remove();
  }

  mw_one->on_btnBackTabRecycle_clicked();

  mw_one->saveTab();

  mw_one->reloadMain();
  mw_one->clickData();

  tabData->setCurrentIndex(count);

  mw_one->strLatestModify = tr("Restore Tab") + "(" + tab_name + ")";
}
