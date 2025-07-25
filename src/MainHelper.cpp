#include "MainHelper.h"

#include "src/MainWindow.h"
#include "ui_MainWindow.h"

extern MainWindow *mw_one;
extern Ui::MainWindow *mui;
extern Method *m_Method;
extern QTabWidget *tabData;
extern QString iniDir, searchStr, currentMDFile, privateDir, encPassword,
    errorInfo, ver, strDate, zipfile;
extern bool isAndroid, isReadEnd, isDark, isZipOK, isMenuImport, isDownData;
extern int fontSize, red;

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

void MainHelper::openTabRecycle() {
  mui->frameMain->hide();
  mui->frameTabRecycle->show();

  m_Method->clearAllBakList(mui->qwTabRecycle);

  QString tab_name, tab_time;
  QStringList iniFiles;
  QStringList fmt;
  fmt.append("ini");
  mw_one->m_NotesList->getAllFiles(iniDir, iniFiles, fmt);

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

  // Qt6 新写法（直接通过迭代器构造）
  QSet<QString> set(lastList.begin(), lastList.end());  // 直接构造 QSet
  QStringList uniqueList(set.begin(), set.end());       // 直接构造 QStringList

  for (int i = 0; i < uniqueList.count(); i++) {
    QString str = uniqueList.at(i);
    tab_name = str.split("-=-").at(0);
    tab_time = str.split("-=-").at(1);
    iniTotal = str.split("-=-").at(2);
    iniTotal = iniTotal.trimmed();
    m_Method->addItemToQW(mui->qwTabRecycle, tab_name, tab_time, "", iniTotal,
                          0);
  }

  int t_count = m_Method->getCountFromQW(mui->qwTabRecycle);
  if (t_count > 0) {
    m_Method->setCurrentIndexFromQW(mui->qwTabRecycle, 0);
  }

  mui->lblTitleTabRecycle->setText(tr("Tab Recycle") + "    " + tr("Total") +
                                   " : " + QString::number(t_count));
}

void MainHelper::initQW() {
  qmlRegisterType<File>("MyModel1", 1, 0, "File");
  qmlRegisterType<DocumentHandler>("MyModel2", 1, 0, "DocumentHandler");

  int f_size = 19;
  if (fontSize <= f_size) f_size = fontSize;
  mui->qwReport->rootContext()->setContextProperty("maxFontSize", f_size);
  mui->qwReportSub->rootContext()->setContextProperty("maxFontSize", f_size);

  mui->qwNotesTree->rootContext()->setContextProperty("fontSize", fontSize);
  mui->qwNotesTree->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/tree_main.qml")));

  mui->qwReader->rootContext()->setContextProperty("myW", mw_one->width());
  mui->qwReader->rootContext()->setContextProperty("myH", mw_one->height());
  mui->qwReader->rootContext()->setContextProperty("m_Reader",
                                                   mw_one->m_Reader);
  mui->qwReader->rootContext()->setContextProperty("myBackgroundColor",
                                                   "#FFFFFF");

  mui->qwCata->rootContext()->setContextProperty("m_Reader", mw_one->m_Reader);
  mui->qwCata->setSource(QUrl(QStringLiteral("qrc:/src/qmlsrc/epub_cata.qml")));

  mui->qwBookmark->rootContext()->setContextProperty("m_Reader",
                                                     mw_one->m_Reader);
  mui->qwBookmark->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/bookmark.qml")));

  mui->qw_Img->rootContext()->setContextProperty("myW", mw_one->width());
  mui->qw_Img->rootContext()->setContextProperty("myH", mw_one->height());

  mui->qwTodo->rootContext()->setContextProperty("maxFontSize", f_size);
  mui->qwTodo->rootContext()->setContextProperty("isBtnVisible",
                                                 QVariant(false));
  mui->qwTodo->rootContext()->setContextProperty("m_Todo", mw_one->m_Todo);
  mui->qwTodo->rootContext()->setContextProperty("FontSize", fontSize);
  mui->qwTodo->setSource(QUrl(QStringLiteral("qrc:/src/qmlsrc/todo.qml")));

  mui->qwRecycle->rootContext()->setContextProperty("FontSize", fontSize);
  mui->qwRecycle->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/todorecycle.qml")));

  mui->qwSteps->setSource(QUrl(QStringLiteral("qrc:/src/qmlsrc/steps.qml")));
  mui->qwSteps->rootContext()->setContextProperty("maxFontSize", f_size);
  mui->qwSteps->rootContext()->setContextProperty("myW", mw_one->width());
  mui->qwSteps->rootContext()->setContextProperty("text0", "");
  mui->qwSteps->rootContext()->setContextProperty("text1", "");
  mui->qwSteps->rootContext()->setContextProperty("text2", "");
  mui->qwSteps->rootContext()->setContextProperty("text3", "");

  mui->qwSpeed->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/Speedometer.qml")));

  mui->qwGpsList->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/gps_list.qml")));
  mui->qwGpsList->rootContext()->setContextProperty("myW", mw_one->width());
  mui->qwGpsList->rootContext()->setContextProperty("m_Steps", mw_one->m_Steps);

  mui->qwMap->setResizeMode(QQuickWidget::SizeRootObjectToView);
  mui->qwMap->setFocusPolicy(Qt::StrongFocus);  // 关键设置
  mui->qwMap->setClearColor(Qt::transparent);   // 避免渲染冲突
  mui->qwMap->setAttribute(Qt::WA_AcceptTouchEvents, true);
  mui->qwMap->setAttribute(Qt::WA_TouchPadAcceptSingleTouchEvents, true);
  mui->qwMap->setSource(QUrl(QStringLiteral("qrc:/src/qmlsrc/map.qml")));

  mui->qwReport->rootContext()->setContextProperty("m_Report",
                                                   mw_one->m_Report);
  mui->qwReport->setSource(QUrl(QStringLiteral("qrc:/src/qmlsrc/report.qml")));
  mui->qwReportSub->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/details.qml")));

  mui->qwSearch->rootContext()->setContextProperty("m_Method", m_Method);
  mui->qwSearch->setSource(QUrl(QStringLiteral("qrc:/src/qmlsrc/search.qml")));

  mui->qwBakList->rootContext()->setContextProperty("m_Method", m_Method);
  mui->qwBakList->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/baklist.qml")));

  mui->qwViewCate->rootContext()->setContextProperty("m_Report",
                                                     mw_one->m_Report);
  mui->qwViewCate->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/viewcate.qml")));

  mui->qwTabRecycle->rootContext()->setContextProperty("m_Report",
                                                       mw_one->m_Report);
  mui->qwTabRecycle->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/tabrecycle.qml")));

  mui->qwNoteBook->rootContext()->setContextProperty("m_NotesList",
                                                     mw_one->m_NotesList);
  mui->qwNoteBook->rootContext()->setContextProperty("mw_one", mw_one);
  mui->qwNoteBook->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/notebook.qml")));

  if (isAndroid)
    mui->qwNoteList->rootContext()->setContextProperty("noteTimeFontSize", 12);
  else
    mui->qwNoteList->rootContext()->setContextProperty("noteTimeFontSize", 8);
  mui->qwNoteList->rootContext()->setContextProperty("m_NotesList",
                                                     mw_one->m_NotesList);
  mui->qwNoteList->rootContext()->setContextProperty("mw_one", mw_one);
  mui->qwNoteList->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/notelist.qml")));

  mui->qwNotesSearchResult->rootContext()->setContextProperty("fontSize",
                                                              fontSize);
  mui->qwNotesSearchResult->rootContext()->setContextProperty(
      "m_NotesList", mw_one->m_NotesList);
  mui->qwNotesSearchResult->rootContext()->setContextProperty("mw_one", mw_one);
  mui->qwNotesSearchResult->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/SearchResults.qml")));

  mui->qwNoteRecycle->rootContext()->setContextProperty("m_Method", m_Method);
  mui->qwNoteRecycle->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/noterecycle.qml")));

  mui->qwMainTab->setFixedHeight(50);
  mui->qwMainTab->rootContext()->setContextProperty("maintabHeight",
                                                    mui->qwMainTab->height());
  mui->qwMainTab->rootContext()->setContextProperty("mw_one", mw_one);
  mui->qwMainTab->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/maintab.qml")));

  mui->qwMainDate->rootContext()->setContextProperty("isAniEffects", true);
  mui->qwMainDate->rootContext()->setContextProperty("maindateWidth",
                                                     mui->qwMainDate->width());
  mui->qwMainDate->rootContext()->setContextProperty("m_Method", m_Method);
  mui->qwMainDate->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/maindate.qml")));

  mui->qwMainEvent->rootContext()->setContextProperty("fontSize", fontSize);
  mui->qwMainEvent->rootContext()->setContextProperty("isAniEffects", true);
  mui->qwMainEvent->rootContext()->setContextProperty(
      "maineventWidth", mui->qwMainEvent->width());
  mui->qwMainEvent->rootContext()->setContextProperty("m_Method", m_Method);
  mui->qwMainEvent->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/mainevent.qml")));

  mui->qwCategory->rootContext()->setContextProperty("m_Method", m_Method);
  mui->qwCategory->setSource(QUrl(QStringLiteral("qrc:/src/qmlsrc/type.qml")));

  mui->qwSelTab->rootContext()->setContextProperty("mw_one", mw_one);
  mui->qwSelTab->setSource(QUrl(QStringLiteral("qrc:/src/qmlsrc/seltab.qml")));

  mui->qwBookList->rootContext()->setContextProperty("fontSize", fontSize);
  mui->qwBookList->rootContext()->setContextProperty("m_Reader",
                                                     mw_one->m_Reader);
  mui->qwBookList->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/booklist.qml")));
}

void MainHelper::init_UIWidget() {
  QFontMetrics fontMetrics(font());
  int nFontHeight = fontMetrics.height();
  int nHeight = nFontHeight * 1.5;
  mui->tabWidget->tabBar()->setFixedHeight(nHeight);
  mui->tabWidget->setStyleSheet(mui->tabCharts->styleSheet());
  mui->tabWidget->setFixedHeight(mui->tabWidget->tabBar()->height() + 0);
  if (nHeight <= 36) nHeight = 36;
  mui->qwMainTab->setFixedHeight(nHeight);
  mui->tabWidget->hide();

  mw_one->loginTime = m_Method->setCurrentDateTimeValue();
  strDate = m_Method->setCurrentDateValue();
  isReadEnd = true;

  mw_one->installEventFilter(mw_one);

  if (isAndroid) {
    mw_one->textToolbar = new TextEditToolbar(mw_one);
    EditEventFilter *editFilter =
        new EditEventFilter(mw_one->textToolbar, mw_one);
    mui->editCategory->installEventFilter(editFilter);
    mui->editDetails->installEventFilter(editFilter);
    mui->editTodo->installEventFilter(editFilter);
    mui->editDetails->viewport()->installEventFilter(editFilter);
    mui->editTodo->viewport()->installEventFilter(editFilter);
    mui->editWebDAV->installEventFilter(editFilter);
    mui->editWebDAVPassword->installEventFilter(editFilter);
    mui->editWebDAVUsername->installEventFilter(editFilter);
    mui->editFindNote->installEventFilter(editFilter);
    mui->editNotesSearch->installEventFilter(editFilter);
    mui->editSearchText->installEventFilter(editFilter);
  }

  mui->menubar->hide();
  mui->statusbar->hide();
  mui->frameReader->hide();
  mui->frameTodo->hide();
  mui->frameTodoRecycle->hide();
  mui->frameSteps->hide();
  mui->frameReport->hide();
  mui->frameSearch->hide();
  mui->frameBakList->hide();

  mui->frameViewCate->hide();
  mui->frameTabRecycle->hide();
  mui->frameNoteList->hide();
  mui->frameNotesSearchResult->hide();
  mui->frameNoteRecycle->hide();
  mui->f_FindNotes->hide();
  mui->btnFindNextNote->setEnabled(false);
  mui->btnFindPreviousNote->setEnabled(false);
  mui->frameNotesTree->hide();
  mui->qwCata->hide();
  mui->qwBookmark->hide();

  mui->frameCategory->hide();
  mui->frameSetTab->hide();
  mui->frameEditRecord->hide();
  mui->frameBookList->hide();
  mui->f_ReaderSet->hide();

  mui->frameReader->layout()->setContentsMargins(0, 0, 0, 1);
  mui->frameReader->setContentsMargins(0, 0, 0, 1);
  mui->frameReader->layout()->setSpacing(1);
  mui->frameImgView->hide();

  mui->frameMain->layout()->setContentsMargins(1, 0, 1, 0);
  mui->frameMain->setContentsMargins(1, 0, 1, 0);
  mui->frameMain->layout()->setSpacing(1);

  mui->frameOne->hide();
  mui->f_FunWeb->hide();
  mui->btnStorageInfo->hide();
  mui->editCode->setLineWrapMode(QTextEdit::NoWrap);
  mui->lblEpubInfo->hide();
  mui->pEpubProg->hide();

  mui->frameNotes->hide();
  mui->frameNotes->layout()->setContentsMargins(1, 1, 1, 1);

  mui->btnSetKey->hide();
  mui->btnNotesList->hide();
  mui->btnWebBack->hide();
  mui->btnRecentOpen0->hide();

  mui->chkOneDrive->setStyleSheet(mw_one->m_Preferences->chkStyle);
  mui->chkWebDAV->setStyleSheet(mw_one->m_Preferences->chkStyle);
  mui->chkAutoSync->setStyleSheet(mw_one->m_Preferences->chkStyle);
  mui->twCloudBackup->setCurrentIndex(1);
  mui->twCloudBackup->setTabVisible(0, false);
  mui->chkWebDAV->hide();
  mui->lblWebDAV->hide();

  mui->editWebDAVPassword->setEchoMode(QLineEdit::EchoMode::Password);
  mui->lblWebDAV->setStyleSheet(mw_one->labelNormalStyleSheet);
  mui->lblTitleEditRecord->setStyleSheet(clickableLabelButtonStyle);

  mui->textBrowser->installEventFilter(mw_one);
  mui->textBrowser->setMouseTracking(true);
  mui->textBrowser->viewport()->installEventFilter(mw_one);
  mui->textBrowser->viewport()->setMouseTracking(true);
  mui->qwReader->installEventFilter(mw_one);

  mui->tabWidget->tabBar()->installEventFilter(mw_one);
  mui->tabWidget->installEventFilter(mw_one);
  mui->tabWidget->setMouseTracking(true);
  mui->lblStats->installEventFilter(mw_one);
  mui->editSearchText->installEventFilter(mw_one);
  mui->editFindNote->installEventFilter(mw_one);

  mui->lblTitleEditRecord->installEventFilter(mw_one);
  mui->lblNoteName->installEventFilter(mw_one);

  mui->lblStats->adjustSize();
  mui->lblStats->setWordWrap(true);

  mui->lblNoteTitle->adjustSize();
  mui->lblNoteTitle->setWordWrap(true);
  mui->lblNoteTitle->hide();
  mui->f_Tools->hide();

  mui->progBar->setMaximumHeight(4);
  mui->progBar->hide();
  mui->progBar->setStyleSheet(
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
  mui->progReader->setStyleSheet(mui->progBar->styleSheet());
  mui->progReader->setFixedHeight(4);

  mui->tabCharts->tabBar()->hide();
  m_Method->setToolButtonQss(mui->btnChartMonth, 5, 3, "#FF0000", "#FFFFFF",
                             "#FF0000", "#FFFFFF", "#FF5555", "#FFFFFF");
  m_Method->setToolButtonQss(mui->btnChartDay, 5, 3, "#455364", "#FFFFFF",
                             "#455364", "#FFFFFF", "#555364", "#FFFFFF");

  int nIConFontSize;
#ifdef Q_OS_ANDROID
  nIConFontSize = 12;
#else
  nIConFontSize = 9;
#endif
  QFont f = mw_one->font();
  f.setPointSize(nIConFontSize);
  mui->btnTodo->setFont(f);
  mui->btnSteps->setFont(f);
  mui->btnChart->setFont(f);
  mui->btnReader->setFont(f);
  mui->btnNotes->setFont(f);
  mui->btnSelTab->setFont(f);

  f.setPointSize(nIConFontSize + 0);
  mui->btnMenu->setFont(f);
  mui->btnAdd->setFont(f);
  mui->btnDel->setFont(f);
  mui->btnSync->setFont(f);

  mui->btnReport->setFont(f);
  mui->btnFind->setFont(f);
  mui->btnModifyRecord->setFont(f);
  mui->btnMove->setFont(f);

  f.setBold(true);
  mui->lblSyncNote->setFont(f);
  mui->lblShowLineSn->setFont(f);
  mui->lblShowLineSn->setWordWrap(true);
  mui->lblShowLineSn->adjustSize();

  QString lblStyle = mw_one->labelNormalStyleSheet;
  mui->lblTotal->setStyleSheet(lblStyle);
  mui->lblDetails->setStyleSheet(lblStyle);
  mui->lblTitle->setStyleSheet(lblStyle);
  mui->lblTitle_Report->setStyleSheet(lblStyle);

  mui->tabMotion->setCornerWidget(mui->btnBackSteps, Qt::TopRightCorner);
  mui->tabMotion->setCurrentIndex(1);
  QString rbStyle = mui->rbCycling->styleSheet();
  mui->rbHiking->setStyleSheet(rbStyle);
  mui->rbRunning->setStyleSheet(rbStyle);
  QSettings Reg(iniDir + "gpslist.ini", QSettings::IniFormat);

  mui->rbCycling->setChecked(Reg.value("/GPS/isCycling", 0).toBool());
  mui->rbHiking->setChecked(Reg.value("/GPS/isHiking", 0).toBool());
  mui->rbRunning->setChecked(Reg.value("/GPS/isRunning", 0).toBool());

  mui->btnGPS->setStyleSheet(mw_one->m_Steps->btnRoundStyle);
  mui->btnGPS->hide();
  mui->frame_btnGps->setFixedHeight(80);
  QWidget *centralWidget = new QWidget(mw_one);
  QVBoxLayout *layout = new QVBoxLayout(centralWidget);

  SliderButton *sliderButton = new SliderButton(centralWidget);
  sliderButton->setTipText(tr("Slide Right to Start or Stop."));
  layout->addWidget(sliderButton);

  QObject::connect(sliderButton, &SliderButton::sliderMovedToEnd, mw_one,
                   [&]() { mui->btnGPS->click(); });
  mui->frame_btnGps->layout()->addWidget(centralWidget);
}

void MainHelper::startBackgroundTaskUpdateBakFileList() {
  mw_one->showProgress();

  mui->frameMain->hide();
  mui->frameBakList->show();

  QFuture<void> future = QtConcurrent::run([=]() {
    bakFileList = mw_one->m_Preferences->getBakFilesList();
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
  QFutureWatcher<void> *watcher = new QFutureWatcher<void>(mw_one);
  connect(watcher, &QFutureWatcher<void>::finished, mw_one, [=]() {
    m_Method->clearAllBakList(mui->qwBakList);
    int bakCount = bakFileList.count();
    for (int i = 0; i < bakCount; i++) {
      QString action, bakfile;
      QString str = bakFileList.at(bakCount - 1 - i);
      action = str.split("-===-").at(0);
      bakfile = str.split("-===-").at(1);
      m_Method->addItemToQW(mui->qwBakList, action, "", "", bakfile, 0);
    }

    if (m_Method->getCountFromQW(mui->qwBakList) > 0)
      m_Method->setCurrentIndexFromQW(mui->qwBakList, 0);

    mui->lblBakListTitle->setText(
        tr("Backup File List") + "    " + tr("Total") + " : " +
        QString::number(m_Method->getCountFromQW(mui->qwBakList)));

    qDebug() << "BakFileList update completed";
    watcher->deleteLater();

    mw_one->closeProgress();
  });
  watcher->setFuture(future);
}

void MainHelper::init_ButtonStyle() {
  m_Method->set_ToolButtonStyle(mw_one);
  mui->btnMenu->setStyleSheet("border:none");
  mui->btnModifyRecord->setStyleSheet("border:none");
  mui->btnMove->setStyleSheet("border:none");

  mui->btnTodo->setStyleSheet("border:none");
  mui->btnSteps->setStyleSheet("border:none");
  mui->btnChart->setStyleSheet("border:none");
  mui->btnReader->setStyleSheet("border:none");
  mui->btnNotes->setStyleSheet("border:none");
  mui->btnAdd->setStyleSheet("border:none");
  mui->btnDel->setStyleSheet("border:none");
  mui->btnPasteTodo->setStyleSheet("border:none");
  mui->btnSync->setStyleSheet("border:none");
  mui->btnFind->setStyleSheet("border:none");
  mui->btnReport->setStyleSheet("border:none");
  mui->btnSelTab->setStyleSheet("border:none");

  if (isDark) {
    mui->f_ReaderFun->setStyleSheet("QFrame{background-color: #2874AC;}");
    mui->btnOpen->setStyleSheet("border:none; background-color:#2874AC;");
    mui->btnBackReader->setStyleSheet("border:none; background-color:#2874AC;");
    mui->btnCatalogue->setStyleSheet("border:none; background-color:#2874AC;");
    mui->btnBackDir->setStyleSheet("border:none; background-color:#2874AC;");

    mui->btnReadList->setStyleSheet("border:none; background-color:#2874AC;");
    mui->btnShowBookmark->setStyleSheet(
        "border:none; background-color:#2874AC;");
    mui->btnPages->setStyleSheet("border:none; background-color:#2874AC;");
    mui->btnAutoRun->setStyleSheet("border:none; background-color:#2874AC;");
    mui->btnAutoStop->setStyleSheet("border:none; background-color:#2874AC;");

    mui->btnPages->setStyleSheet(
        "color: rgb(255, 255, 255);background-color: #2874AC; "
        "border: "
        "0px solid "
        "rgb(255,0,0);border-radius: 0px;"
        "font-weight: bold;");
  } else {
    mui->f_ReaderFun->setStyleSheet("QFrame{background-color: #3498DB;}");
    mui->btnOpen->setStyleSheet("border:none; background-color:#3498DB;");
    mui->btnBackReader->setStyleSheet("border:none; background-color:#3498DB;");
    mui->btnCatalogue->setStyleSheet("border:none; background-color:#3498DB;");
    mui->btnBackDir->setStyleSheet("border:none; background-color:#3498DB;");

    mui->btnReadList->setStyleSheet("border:none; background-color:#3498DB;");
    mui->btnShowBookmark->setStyleSheet(
        "border:none; background-color:#3498DB;");
    mui->btnPages->setStyleSheet("border:none; background-color:#3498DB;");
    mui->btnAutoRun->setStyleSheet("border:none; background-color:#3498DB;");
    mui->btnAutoStop->setStyleSheet("border:none; background-color:#3498DB;");

    mui->btnPages->setStyleSheet(
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
  mw_one->m_Preferences->ui->btnReStart->setStyleSheet(style);
}

void MainHelper::delBakFile() {
  if (m_Method->getCountFromQW(mui->qwBakList) == 0) return;

  int index = m_Method->getCurrentIndexFromQW(mui->qwBakList);
  QString bak_file = m_Method->getText3(mui->qwBakList, index);

  m_Method->m_widget = new QWidget(mw_one);
  ShowMessage *m_ShowMsg = new ShowMessage(mw_one);
  if (!m_ShowMsg->showMsg("Knot",
                          tr("Whether to remove") + "  " + bak_file + " ? ", 2))
    return;

  QFile file(bak_file);
  file.remove();
  m_Method->delItemFromQW(mui->qwBakList, index);

  int newIndex = index - 1;
  if (newIndex < 0) newIndex = 0;

  m_Method->setCurrentIndexFromQW(mui->qwBakList, newIndex);

  mui->lblBakListTitle->setText(
      tr("Backup File List") + "    " + tr("Total") + " : " +
      QString::number(m_Method->getCountFromQW(mui->qwBakList)));
}

void MainHelper::delTabRecycleFile() {
  if (m_Method->getCountFromQW(mui->qwTabRecycle) == 0) return;
  int index = m_Method->getCurrentIndexFromQW(mui->qwTabRecycle);
  QString tab_file = m_Method->getText3(mui->qwTabRecycle, index);

  m_Method->m_widget = new QWidget(mw_one);
  ShowMessage *m_ShowMsg = new ShowMessage(mw_one);
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

  m_Method->delItemFromQW(mui->qwTabRecycle, index);

  mui->lblTitleTabRecycle->setText(
      tr("Tab Recycle") + "    " + tr("Total") + " : " +
      QString::number(m_Method->getCountFromQW(mui->qwTabRecycle)));
}

void MainHelper::importBakFileList() {
  if (m_Method->getCountFromQW(mui->qwBakList) == 0) return;

  int cur_index = m_Method->getCurrentIndexFromQW(mui->qwBakList);
  QString str = m_Method->getText3(mui->qwBakList, cur_index);
  zipfile = str.trimmed();

  if (!zipfile.isNull()) {
    m_Method->m_widget = new QWidget(mw_one);
    ShowMessage *m_ShowMsg = new ShowMessage(mw_one);
    if (!m_ShowMsg->showMsg("Kont",
                            tr("Import this data?") + "\n" +
                                mw_one->m_Reader->getUriRealPath(zipfile),
                            2)) {
      isZipOK = false;
      return;
    }
  }

  isZipOK = true;
  mui->btnBackBakList->click();
  mw_one->showProgress();

  isMenuImport = true;
  isDownData = false;

  mw_one->myImportDataThread->start();
}

void MainHelper::init_Theme() {
  // Get the background color to fit the dark mode
  QPalette pal = mw_one->palette();
  QBrush brush = pal.window();
  red = brush.color().red();

  qDebug() << "red=" << red;

  mui->qwMainTab->rootContext()->setContextProperty("isDark", isDark);
  mui->qwMainDate->rootContext()->setContextProperty("isDark", isDark);
  mui->qwMainEvent->rootContext()->setContextProperty("isDark", isDark);
  mui->qwTodo->rootContext()->setContextProperty("isDark", isDark);
  mui->qwRecycle->rootContext()->setContextProperty("isDark", isDark);
  mui->qwNoteBook->rootContext()->setContextProperty("isDark", isDark);
  mui->qwNoteList->rootContext()->setContextProperty("isDark", isDark);

  mui->qwNotesSearchResult->rootContext()->setContextProperty("isDark", isDark);
  mui->qwSearch->rootContext()->setContextProperty("isDark", isDark);
  mui->qwBakList->rootContext()->setContextProperty("isDark", isDark);
  mui->qwViewCate->rootContext()->setContextProperty("isDark", isDark);
  mui->qwTabRecycle->rootContext()->setContextProperty("isDark", isDark);
  mui->qwNoteRecycle->rootContext()->setContextProperty("isDark", isDark);
  mui->qwCategory->rootContext()->setContextProperty("isDark", isDark);
  mui->qwSelTab->rootContext()->setContextProperty("isDark", isDark);
  mui->qwBookList->rootContext()->setContextProperty("isDark", isDark);
  mui->qwReportSub->rootContext()->setContextProperty("isDark", isDark);
  mui->qwSteps->rootContext()->setContextProperty("isDark", isDark);
  mui->qwGpsList->rootContext()->setContextProperty("isDark", isDark);
  mui->qwReport->rootContext()->setContextProperty("isDark", isDark);

  mui->qwCata->rootContext()->setContextProperty("isDark", isDark);
  mui->qwBookmark->rootContext()->setContextProperty("isDark", isDark);
  mui->qwReader->rootContext()->setContextProperty("isDark", isDark);

  if (!isDark) {
    mui->f_Menu->setStyleSheet("background-color: rgb(243,243,243);");
    mui->f_Btn->setStyleSheet("background-color: rgb(243,243,243);");
    mui->f_cw->setStyleSheet("background-color: rgb(243,243,243);");
    mui->f_charts->setStyleSheet("background-color: rgb(243,243,243);");

    mw_one->chartMonth->setTheme(QChart::ChartThemeLight);
    mw_one->chartDay->setTheme(QChart::ChartThemeLight);

    mui->btnAddTodo->setIcon(QIcon(":/res/plus_l.svg"));
    mui->btnClear->setIcon(QIcon(":/res/clear.png"));

    mui->btnModifyRecord->setIcon(QIcon(":/res/edit.svg"));
    mui->btnMove->setIcon(QIcon(":/res/move.svg"));

    mui->btnReader->setIcon(QIcon(":/res/reader.svg"));
    mui->btnTodo->setIcon(QIcon(":/res/todo.svg"));
    mui->btnSteps->setIcon(QIcon(":/res/steps.svg"));
    mui->btnNotes->setIcon(QIcon(":/res/note.svg"));
    mui->btnChart->setIcon(QIcon(":/res/chart.svg"));
    mui->btnFind->setIcon(QIcon(":/res/find.png"));
    mui->btnReport->setIcon(QIcon(":/res/report.svg"));
    mui->btnSelTab->setIcon(QIcon(":/res/tab.svg"));

    mui->btnMenu->setIcon(QIcon(":/res/mainmenu.svg"));
    mui->btnAdd->setIcon(QIcon(":/res/additem.svg"));
    mui->btnDel->setIcon(QIcon(":/res/delitem.svg"));
    mui->btnSync->setIcon(QIcon(":/res/upload.svg"));

    m_Method->setEditLightMode(mui->editTodo);

    mui->editDetails->setStyleSheet(mui->editTodo->styleSheet());

    mui->editTodo->verticalScrollBar()->setStyleSheet(
        m_Method->lightScrollbarStyle);
    mui->editDetails->verticalScrollBar()->setStyleSheet(
        m_Method->lightScrollbarStyle);

    mw_one->chartMonth->setTheme(QChart::ChartThemeLight);
    mw_one->chartDay->setTheme(QChart::ChartThemeLight);

  } else {
    mui->f_Menu->setStyleSheet("background-color: #19232D;");
    mui->f_Btn->setStyleSheet("background-color: #19232D;");
    mui->f_cw->setStyleSheet("background-color: #19232D;");
    mui->f_charts->setStyleSheet("background-color: #19232D;");

    mw_one->chartMonth->setTheme(QChart::ChartThemeDark);
    mw_one->chartDay->setTheme(QChart::ChartThemeDark);

    mui->btnAddTodo->setIcon(QIcon(":/res/plus_l.svg"));
    mui->btnClear->setIcon(QIcon(":/res/clear.png"));

    mui->btnReport->setIcon(QIcon(":/res/report_l.svg"));
    mui->btnFind->setIcon(QIcon(":/res/find_l.png"));
    mui->btnModifyRecord->setIcon(QIcon(":/res/edit_l.svg"));
    mui->btnMove->setIcon(QIcon(":/res/move_l.svg"));

    mui->btnReader->setIcon(QIcon(":/res/reader_l.svg"));
    mui->btnTodo->setIcon(QIcon(":/res/todo_l.png"));
    mui->btnSteps->setIcon(QIcon(":/res/steps_l.svg"));
    mui->btnNotes->setIcon(QIcon(":/res/note_l.svg"));
    mui->btnChart->setIcon(QIcon(":/res/chart_l.svg"));
    mui->btnSelTab->setIcon(QIcon(":/res/tab_l.svg"));

    mui->btnMenu->setIcon(QIcon(":/res/mainmenu_l.svg"));
    mui->btnAdd->setIcon(QIcon(":/res/additem_l.svg"));
    mui->btnDel->setIcon(QIcon(":/res/delitem_l.svg"));
    mui->btnSync->setIcon(QIcon(":/res/upload_l.svg"));

    m_Method->setEditDarkMode(mui->editTodo);

    mui->editDetails->setStyleSheet(mui->editTodo->styleSheet());

    mui->editTodo->verticalScrollBar()->setStyleSheet(
        m_Method->darkScrollbarStyle);
    mui->editDetails->verticalScrollBar()->setStyleSheet(
        m_Method->darkScrollbarStyle);

    mw_one->chartMonth->setTheme(QChart::ChartThemeDark);
    mw_one->chartDay->setTheme(QChart::ChartThemeDark);
  }

  // Edit Record UI
  int nH = mui->editCategory->height();
  if (isDark) {
    m_Method->setQLabelImage(mui->lblCategory, nH, nH, ":/res/fl_l.svg");
    m_Method->setQLabelImage(mui->lblDetailsType, nH, nH, ":/res/xq_l.svg");
    m_Method->setQLabelImage(mui->lblAmount, nH, nH, ":/res/je_l.svg");
  } else {
    m_Method->setQLabelImage(mui->lblCategory, nH, nH, ":/res/fl.svg");
    m_Method->setQLabelImage(mui->lblDetailsType, nH, nH, ":/res/xq.svg");
    m_Method->setQLabelImage(mui->lblAmount, nH, nH, ":/res/je.svg");
  }

  mw_one->m_EditRecord->on_editAmount_textChanged(mui->editAmount->text());
  mw_one->m_EditRecord->on_editCategory_textChanged(mui->editCategory->text());
  mw_one->m_EditRecord->on_editDetails_textChanged();

  // Todo
  mw_one->m_Todo->changeTodoIcon(mw_one->m_Todo->isToday);

  // Android
  m_Method->setDark(isDark);

  // Notes Editor
  mw_one->m_Notes->init_md();

  // Chart
  QFont font1;
#ifdef Q_OS_ANDROID
  font1.setPointSize(10);
#else
  font1.setPointSize(10);
#endif
  font1.setBold(true);
  mw_one->chartMonth->setTitleFont(font1);
  mw_one->chartDay->setTitleFont(font1);
  mw_one->axisX->setLabelsFont(font1);
  mw_one->axisY->setLabelsFont(font1);
  mw_one->axisY->setTickCount(mw_one->yScale);
  mw_one->axisX2->setLabelsFont(font1);
  mw_one->axisY2->setLabelsFont(font1);
  mw_one->axisY2->setTickCount(mw_one->yScale);

  mui->lblNoteName->setStyleSheet(
      "QLabel{background:lightyellow;color:black;}");

  init_ButtonStyle();
}
