#include "MainHelper.h"

#include "src/MainWindow.h"
#include "ui_MainWindow.h"

extern MainWindow *mw_one;
extern Ui::MainWindow *mui;
extern Method *m_Method;
extern QTabWidget *tabData;
extern QString iniDir, searchStr, currentMDFile, privateDir, encPassword,
    errorInfo, ver;
extern bool isAndroid;
extern int fontSize;

MainHelper::MainHelper(QWidget *parent) : QDialog{parent} {}

bool MainHelper::mainEventFilter(QObject *watch, QEvent *evn) {
  QMouseEvent *event = static_cast<QMouseEvent *>(evn);  // 将之转换为鼠标事件

  if (evn->type() == QEvent::ToolTip) {
    QToolTip::hideText();
    evn->ignore();
    return true;
  }

  if (watch == mui->lblStats) {
    if (event->type() == QEvent::MouseButtonDblClick) {
      mw_one->on_btnSelTab_clicked();
      return true;
    }
  }

  if (watch == mui->lblTitleEditRecord) {
    if (event->type() == QEvent::MouseButtonPress) {
      QString title = mui->lblTitleEditRecord->text();
      title = title.mid(0, 4);
      if (!title.contains(QObject::tr("Add"))) return true;

      mui->btnTabMoveDown->hide();
      mui->btnTabMoveUp->hide();

      mw_one->m_EditRecord->saveCurrentValue();
      mw_one->on_btnBackEditRecord_clicked();
      mw_one->on_btnSelTab_clicked();

      return true;
    }
  }

  if (watch == mui->lblNoteName) {
    if (event->type() == QEvent::MouseButtonPress) {
      mw_one->on_btnNotesList_clicked();
      return true;
    }
  }

  if (isAndroid)
    mw_one->m_Reader->eventFilterReaderAndroid(watch, evn);
  else
    mw_one->m_Reader->eventFilterReader(watch, evn);

  mw_one->m_Notes->eventFilterQwNote(watch, evn);

  if (watch == mui->textBrowser->viewport()) {
    if (event->type() == QEvent::MouseButtonPress) {
      mw_one->isMousePress = true;
    }

    if (event->type() == QEvent::MouseButtonRelease) {
      mw_one->isMousePress = false;

      QString str = mui->textBrowser->textCursor().selectedText().trimmed();
      if (str == "") {
        mw_one->mydlgSetText->close();
      } else {
        int y1;
        int a = 30;
        if (event->globalPosition().y() - a - mw_one->mydlgSetText->height() >=
            0)
          y1 = event->globalPosition().y() - a - mw_one->mydlgSetText->height();
        else
          y1 = event->globalPosition().y() + a;

        mw_one->mydlgSetText->setFixedWidth(mw_one->width() - 4);
        mw_one->mydlgSetText->init(
            mw_one->geometry().x() +
                (mw_one->width() - mw_one->mydlgSetText->width()) / 2,
            y1, mw_one->mydlgSetText->width(), mw_one->mydlgSetText->height());
      }
    }

    if (event->type() == QEvent::MouseMove) {
      if (mw_one->isMousePress) {
        QString str = mui->textBrowser->textCursor().selectedText().trimmed();
        if (str != "") {
          int y1;
          int a = 30;
          if (event->globalPosition().y() - a -
                  mw_one->mydlgSetText->height() >=
              0)
            y1 = event->globalPosition().y() - a -
                 mw_one->mydlgSetText->height();
          else
            y1 = event->globalPosition().y() + a;

          mw_one->mydlgSetText->setFixedWidth(mw_one->width() - 4);
          mw_one->mydlgSetText->init(
              mw_one->geometry().x() +
                  (mw_one->width() - mw_one->mydlgSetText->width()) / 2,
              y1, mw_one->mydlgSetText->width(),
              mw_one->mydlgSetText->height());
        }
      }
    }

    if (event->type() == QEvent::MouseButtonDblClick) {
      QTextCursor cursor = mui->textBrowser->textCursor();
      cursor.setPosition(cursor.anchor());
      mui->textBrowser->setTextCursor(cursor);

      return true;
    }
  }

  if (watch == mw_one->chartview || watch == mw_one->chartview1) {
    if (event->type() == QEvent::MouseButtonDblClick) {
      mw_one->on_btnChart_clicked();
    }
  }

  if (evn->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(evn);

    if (watch == mui->editSearchText && keyEvent->key() == Qt::Key_Return) {
      mw_one->on_btnStartSearch_clicked();
      return true;
    }

    if (keyEvent->key() == Qt::Key_Escape) {
      if (mui->frameReader->isVisible()) mw_one->on_btnBackReader_clicked();
      return true;
    }
  }

  return true;
}

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
  QTreeWidget *tw = init_TreeWidget(twName);
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

QTreeWidget *MainHelper::init_TreeWidget(QString name) {
  QTreeWidget *tw = new QTreeWidget(mw_one);
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
  tw->installEventFilter(mw_one);
  tw->viewport()->installEventFilter(mw_one);
  tw->setUniformRowHeights(true);  // 加快展开速度
  connect(tw, &QTreeWidget::itemClicked, mw_one, &MainWindow::on_twItemClicked);
  connect(tw, &QTreeWidget::itemDoubleClicked, mw_one,
          &MainWindow::on_twItemDoubleClicked);
  connect(tw, &QTreeWidget::itemPressed, [=]() {});

  connect(tw->verticalScrollBar(), &QScrollBar::valueChanged, [=]() {});

  QScrollBar *SB = tw->verticalScrollBar();
  SB->setStyleSheet(m_Method->vsbarStyleSmall);
  tw->setStyleSheet(mw_one->treeStyle);
  tw->setVerticalScrollMode(QTreeWidget::ScrollPerPixel);
  QScroller::grabGesture(tw, QScroller::LeftMouseButtonGesture);
  m_Method->setSCrollPro(tw);
  return tw;
}

void MainHelper::init_Menu(QMenu *mainMenu) {
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

  connect(actAddTab, &QAction::triggered, mw_one,
          &MainWindow::on_actionAdd_Tab_triggered);
  connect(actDelTab, &QAction::triggered, mw_one,
          &MainWindow::on_actionDel_Tab_triggered);
  connect(actRenameTab, &QAction::triggered, mw_one,
          &MainWindow::on_actionRename_triggered);

  connect(actBakFileList, &QAction::triggered, mw_one,
          &MainWindow::on_actionBakFileList);

  connect(actTabRecycle, &QAction::triggered, mw_one,
          &MainWindow::on_actionTabRecycle);

  connect(actOpenKnotBakDir, &QAction::triggered, mw_one,
          &MainWindow::on_openKnotBakDir);
  connect(actReport, &QAction::triggered, mw_one,
          &MainWindow::on_actionReport_triggered);

  connect(actExportData, &QAction::triggered, mw_one,
          &MainWindow::on_actionExport_Data_triggered);

  connect(actImportData, &QAction::triggered, mw_one,
          &MainWindow::on_actionImport_Data_triggered);
  connect(actPreferences, &QAction::triggered, mw_one,
          &MainWindow::on_actionPreferences_triggered);

  connect(actOneDrive, &QAction::triggered, mw_one,
          &MainWindow::on_actionOneDriveBackupData);
  connect(actAbout, &QAction::triggered, mw_one, &MainWindow::on_actionAbout);
  connect(actShareFile, &QAction::triggered, mw_one,
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
  if (!mw_one->m_Preferences->devMode) {
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
