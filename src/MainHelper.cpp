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

  mui->qwReader->rootContext()->setContextProperty("myW", this->width());
  mui->qwReader->rootContext()->setContextProperty("myH", this->height());
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

  mui->qw_Img->rootContext()->setContextProperty("myW", this->width());
  mui->qw_Img->rootContext()->setContextProperty("myH", this->height());

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
  mui->qwSteps->rootContext()->setContextProperty("myW", this->width());
  mui->qwSteps->rootContext()->setContextProperty("text0", "");
  mui->qwSteps->rootContext()->setContextProperty("text1", "");
  mui->qwSteps->rootContext()->setContextProperty("text2", "");
  mui->qwSteps->rootContext()->setContextProperty("text3", "");

  mui->qwSpeed->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/Speedometer.qml")));

  mui->qwGpsList->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/gps_list.qml")));
  mui->qwGpsList->rootContext()->setContextProperty("myW", this->width());
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
