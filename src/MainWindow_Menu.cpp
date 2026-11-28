#include "MainWindow.h"
#include "defines.h"
#include "src/Comm/AppLogger.h"

void MainWindow::on_actionAbout() {
  QString str = "\n" + appName + "  Ver: " + ver + "\n\n" + tr("Startup Time") +
                ": " + strStartTotalTime + " s" + "\n" + loginTime + "\n" +
                "(c) 2022-" + QString::number(QDate::currentDate().year()) +
                " The Knot Authors\n";

  m_AboutThis->ui->lblAbout->setText(str);
  m_AboutThis->ui->frameAbout->show();

  int x, y;
  if (!isAndroid) {
    m_Method->setWinPos(m_AboutThis, 350, 3);
  } else {
    m_AboutThis->setFixedWidth(this->width());
    m_AboutThis->setFixedHeight(this->height());
    x = this->geometry().x();
    y = this->geometry().y();
    m_AboutThis->setGeometry(x, y, this->width(), this->height());
  }

  m_AboutThis->show();
}

void MainWindow::addMyTab(const QString &customTabText) {
  int count = mw_one->ui->tabWidget->tabBar()->count();

  QString twName = m_Notes->getDateTimeStr() + "_" + QString::number(count + 1);
  QTreeWidget *tw = init_TreeWidget(twName);

  mw_one->ui->tabWidget->addTab(tw, customTabText);
  mw_one->ui->tabWidget->setCurrentIndex(count);

  saveTab();
  strLatestModify = tr("Add Tab") + " ( " + customTabText + " ) ";
}

void MainWindow::delTab() {
  int index = mw_one->ui->tabWidget->currentIndex();
  if (index < 0)
    return;

  QString tab_name = mw_one->ui->tabWidget->tabText(index);

  strLatestModify = tr("Del Tab") + " ( " + tab_name + " ) ";

  QString date_time = m_Notes->getDateTimeStr();
  m_Method->saveRecycleTabName(date_time, tab_name);

  QTreeWidget *tw = (QTreeWidget *)tabData->currentWidget();
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

  int TabCount = ui->tabWidget->tabBar()->count();
  if (TabCount > 1) {
    ui->tabWidget->removeTab(index);
  }

  if (TabCount == 1) {
    ui->tabWidget->removeTab(0);
    QString tw_name = m_Notes->getDateTimeStr() + "_" + QString::number(1);
    QTreeWidget *tw = init_TreeWidget(tw_name);
    QString tabText = tr("Tab 1");
    ui->tabWidget->addTab(tw, tabText);
  }

  saveTab();
}

void MainWindow::on_actionBakFileList() { m_BakFileList->loadBakFile(); }

void MainWindow::on_actionOneDriveBackupData() {
  mw_one->ui->frameMain->hide();

  if (isAndroid) {
    mw_one->ui->twCloudBackup->setFixedWidth(mw_one->width() - 24);
  }
  mw_one->ui->frameOne->show();
}

void MainWindow::on_actionExport_Data_triggered() {
  if (!isSaveEnd)
    return;

  isUpData = false;
  showProgress();

  myBakDataThread->start();
}

void MainWindow::on_actionImport_Data_triggered() {
  if (!isSaveEnd)
    return;

  QFileDialog *dialog = new QFileDialog(this, tr("KnotBak"), bakfileDir);

#ifdef Q_OS_ANDROID
  dialog->setNameFilter(tr("Zip File (*.*)"));
#else
  dialog->setNameFilter(tr("Zip File (*.zip);;All(*.*)"));
#endif

  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  // ✅ 关键：用 accepted 替代 fileSelected
  // accepted 在 Android SAF 返回结果时触发可靠性远高于 fileSelected
  connect(dialog, &QFileDialog::accepted, this, [this, dialog]() {
    // 主动从 dialog 获取选中的文件列表
    QStringList files = dialog->selectedFiles();
    if (!files.isEmpty()) {
      zipfile = files.first();
      qDebug() << "Android file accepted:" << zipfile;

      // ✅ 关键：延迟 300ms 执行，让 Android Activity 完全完成 onResume 和
      // UI 重建
      QTimer::singleShot(300, this, [this]() {
        qDebug() << "Executing import after delay...";
        mw_one->m_MainHelper->importBakFileList(zipfile);
      });
    }
  });

  connect(dialog, &QFileDialog::rejected, this, [this]() {
    qDebug() << "User cancelled file selection";
    zipfile = "";
  });

  dialog->open();
}

void MainWindow::on_actionPreferences_triggered() {
  m_Preferences->openPreferences();
}

void MainWindow::renameTab(const QString &newName) {
  int index = ui->tabWidget->currentIndex();
  if (index < 0)
    return;

  ui->tabWidget->setTabText(index, newName);
  saveTab();
  strLatestModify = tr("Rename Tab");
}

void MainWindow::init_Menu(QMenu *mainMenu) {
  QAction *actPreferences = new QAction(tr("Preferences"));
  QAction *actOneDrive = new QAction(tr("Cloud Backup"));
  QAction *actBakFileList = new QAction(tr("Local Backup"));
  QAction *actAbout = new QAction(tr("About") + " (" + ver + ")");

  connect(actPreferences, &QAction::triggered, mw_one,
          &MainWindow::on_actionPreferences_triggered);
  connect(actOneDrive, &QAction::triggered, mw_one,
          &MainWindow::on_actionOneDriveBackupData);
  connect(actBakFileList, &QAction::triggered, mw_one,
          &MainWindow::on_actionBakFileList);
  connect(actAbout, &QAction::triggered, mw_one, &MainWindow::on_actionAbout);

  mainMenu->addAction(actPreferences);
  mainMenu->addAction(actOneDrive);
  mainMenu->addAction(actBakFileList);
  mainMenu->addAction(actAbout);

  mainMenu->setStyleSheet(m_Method->qssMenu);
}
