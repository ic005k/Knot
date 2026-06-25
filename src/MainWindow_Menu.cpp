#include "MainWindow.h"
#include "src/Comm/loglogger.h"

void MainWindow::on_actionAbout() {
  QString str = "\n" + appName + "  Ver: " + ver + "\n\n" + tr("Startup Time") +
                ": " + strStartTotalTime + " s" + "\n" + loginTime + "\n" +
                "(c) 2022-" + QString::number(QDate::currentDate().year()) +
                " The Knot Authors\n";

  m_AboutThis->ui->lblAbout->setText(str);
  m_AboutThis->ui->frameAbout->show();

  int x, y;
  if (!isAndroid) {
    m_AboutThis->setMaximumWidth(320);
    m_AboutThis->setFixedHeight(mw_one->geometry().height() - 30);

    x = mw_one->geometry().x() +
        (mw_one->geometry().width() - m_AboutThis->width()) / 2;
    y = mw_one->geometry().y() +
        (mw_one->geometry().height() - m_AboutThis->height()) / 2;
  } else {
    m_AboutThis->setFixedWidth(this->width());
    m_AboutThis->setFixedHeight(this->height());
    x = this->geometry().x();
    y = this->geometry().y();
  }
  m_AboutThis->setGeometry(x, y, this->width(), this->height());

  m_AboutThis->show();
}

void MainWindow::on_actionFind_triggered() { on_btnFind_pressed(); }

/*void MainWindow::on_actionAdd_Tab_triggered() {
  int count = mui->tabWidget->tabBar()->count();
  QString defaultTabName = tr("Tab") + " " + QString::number(count + 1);

  // 1. 创建可自定义样式的 QInputDialog 实例
  QInputDialog inputDialog(this);
  inputDialog.setWindowTitle(tr("New Tab"));
  inputDialog.setLabelText(tr("Please enter tab name:"));
  inputDialog.setTextValue(defaultTabName);
  inputDialog.setInputMode(QInputDialog::TextInput);

  // 2. 适配移动端/桌面端的样式
  int dialogWidth =
      qMin(300, mw_one->width() - 40);  // 主窗口宽度减边距，最大300px
  inputDialog.setFixedSize(dialogWidth, 200);

  // 适配安卓/桌面端按钮文本
  inputDialog.setOkButtonText(tr("Done"));
  inputDialog.setCancelButtonText(tr("Cancel"));

  // 3. 显示对话框并处理结果
  if (inputDialog.exec() == QDialog::Accepted) {
    QString customTabText = inputDialog.textValue().trimmed();
    if (customTabText.isEmpty()) {
      return;
    }

    // 后续创建标签页的逻辑
    QString twName =
        m_Notes->getDateTimeStr() + "_" + QString::number(count + 1);
    QTreeWidget* tw = init_TreeWidget(twName);

    mui->tabWidget->addTab(tw, customTabText);
    mui->tabWidget->setCurrentIndex(count);
    addItem(customTabText, "", "", "", 0);
    setCurrentIndex(count);

    reloadMain();
    saveTab();
    strLatestModify = tr("Add Tab") + " ( " + customTabText + " ) ";
  }
}*/

void MainWindow::on_actionAdd_Tab_triggered() {
  int count = mui->tabWidget->tabBar()->count();
  QString defaultTabName = tr("Tab") + " " + QString::number(count + 1);

  // 直接调用全局封装的输入框
  QInputDialog* dlg = m_Method->inputDialog(
      tr("New Tab"), tr("Please enter tab name:"), defaultTabName);
  dlg->setOkButtonText(tr("Done"));

  connect(dlg, &QDialog::accepted, this, [=]() {
    QString customTabText = dlg->textValue().trimmed();
    dlg->deleteLater();
    if (customTabText.isEmpty()) return;

    // 创建标签页逻辑不变
    QString twName =
        m_Notes->getDateTimeStr() + "_" + QString::number(count + 1);
    QTreeWidget* tw = init_TreeWidget(twName);

    mui->tabWidget->addTab(tw, customTabText);
    mui->tabWidget->setCurrentIndex(count);
    addItem(customTabText, "", "", "", 0);
    setCurrentIndex(count);

    reloadMain();
    saveTab();
    strLatestModify = tr("Add Tab") + " ( " + customTabText + " ) ";
  });

  connect(dlg, &QDialog::rejected, this, [=]() { dlg->deleteLater(); });
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
    QTreeWidget* tw = init_TreeWidget(tw_name);
    QString tabText = tr("Tab 1");
    mui->tabWidget->addTab(tw, tabText);

    clearAll();
    addItem(tabData->tabText(0), "", "", "", 0);

    reloadMain();
  }

  saveTab();
}

void MainWindow::on_actionBakFileList() {
  // 【安全】界面切换必须在主线程执行
  mui->frameBakList->show();
  mui->frameMain->hide();

  m_MainHelper->startBackgroundTaskUpdateBakFileList();
}

void MainWindow::on_actionOneDriveBackupData() {
  mui->frameMain->hide();
  mui->frameReader->hide();
  if (isAndroid) mui->f_WebDAV->setFixedWidth(mw_one->width() - 40);
  mui->frameOne->show();
}

void MainWindow::on_actionTabRecycle() { m_MainHelper->openTabRecycle(); }

void MainWindow::on_actionExport_Data_triggered() {
  if (!isSaveEnd) return;

  isUpData = false;
  showProgress();

  myBakDataThread->start();
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
    if (!m_ShowMsg->showMsg(
            "Kont",
            tr("Import this data?") + "\n" + m_Reader->getUriRealPath(zipfile),
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

void MainWindow::on_actionPreferences_triggered() {
  m_Preferences->openPreferences();
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
    mui->lblTabTitle->setText(mui->tabWidget->tabBar()->tabText(index));

    updateMainTab();

    saveTab();
  }

  strLatestModify = tr("Rename Tab");
}

void MainWindow::on_actionReport_triggered() {
  if (isEBook || !isSaveEnd || !isReadEBookEnd) return;

  if (isReadEBookEnd) {
    m_Report->init();
    startInitReport();
  }
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

void MainWindow::init_Menu(QMenu* mainMenu) {
  QAction* actAddTab = new QAction(tr("Add Tab"));
  QAction* actDelTab = new QAction(tr("Del Tab"));
  QAction* actRenameTab = new QAction(tr("Rename Tab"));

  QAction* actOpenKnotBakDir = new QAction(tr("Open KnotBak Dir"));

  QAction* actReport = new QAction(tr("Report"));
  actReport->setVisible(false);

  QAction* actExportData = new QAction(tr("Export Data"));
  QAction* actImportData = new QAction(tr("Import Data"));

  QAction* actPreferences = new QAction(tr("Preferences"));

  QAction* actCopyLog = new QAction(tr("Copy Log to Clipboard"));

  QAction* actAbout = new QAction(tr("About") + " (" + ver + ")");
  QAction* actOneDrive = new QAction(tr("Cloud Backup and Restore Data"));

  QAction* actBakFileList = new QAction(tr("Backup File List"));
  QAction* actTabRecycle = new QAction(tr("Tab Recycle"));
  QAction* actShareFile = new QAction(tr("Share File"));

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

  connect(actCopyLog, &QAction::triggered, this, [=]() {
    AppLogger::instance().copyTodayLogToClipboard();
    auto m_ShowMsg = std::make_unique<ShowMessage>(this);
    m_ShowMsg->showMsg(tr("Success"),
                       tr("Today's log has been copied to clipboard, you "
                          "can paste it anywhere."),
                       1);
  });

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
  mainMenu->addAction(actCopyLog);
  mainMenu->addAction(actAbout);

  mainMenu->setStyleSheet(m_Method->qssMenu);
}
