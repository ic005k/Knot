#include "MainHelper.h"

#include "src/MainWindow.h"
#include "ui_MainWindow.h"

extern MainWindow *mw_one;
extern Ui::MainWindow *mui;
extern Method *m_Method;
extern NotesList *m_NotesList;
extern QTabWidget *tabData, *tabChart;
extern QTreeWidgetItem *parentItem;

extern QList<QPointF> PointList;

extern QList<double> doubleList;

extern QStringList listM;

extern QString iniFile, iniDir, privateDir, bakfileDir, strDate, readDate,
    noteText, strStats, SaveType, strY, strM, btnYText, btnMText, btnDText,
    errorInfo, CurrentYearMonth, zipfile, txt, searchStr, currentMDFile,
    copyText, imgFileName, defaultFontFamily, customFontFamily, encPassword,
    ver, btnYearText, btnMonthText, strPage, ebookFile, strTitle, fileName,
    strOpfPath, catalogueFile, strShowMsg;

extern bool isAndroid, isReadEnd, isDark, isZipOK, isMenuImport, isDownData,
    isAdd, loading, isrbFreq, isPasswordError, isEncrypt, isBreak, isIOS,
    isZH_CN, isEpub, isEpubError, isText, isPDF, isWholeMonth, isDateSection,
    isInitThemeEnd, isNeedExecDeskShortcut;

extern int fontSize, red, chartMax, iPage, sPos, totallines, baseLines,
    htmlIndex, s_y1, s_m1, s_d1, s_y2, s_m2, s_d2, totalPages, currentPage;

extern double yMaxMonth, yMaxDay;

extern int deleteDirfile(QString dirName);

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
  if (mui->qwTabRecycle->source().isEmpty()) {
    mui->qwTabRecycle->rootContext()->setContextProperty("m_Report",
                                                         mw_one->m_Report);
    mui->qwTabRecycle->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/tabrecycle.qml")));
  }

  mui->frameMain->hide();
  mui->frameTabRecycle->show();

  m_Method->clearAllBakList(mui->qwTabRecycle);

  QString tab_name, tab_time;
  QStringList iniFiles;
  QStringList fmt;
  fmt.append("ini");
  m_NotesList->getAllFiles(iniDir, iniFiles, fmt);

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

void MainHelper::initMainQW() {
  qmlRegisterType<DocumentHandler>("MyModel2", 1, 0, "DocumentHandler");

  mui->qwMainTab->setFixedHeight(50);
  mui->qwMainTab->rootContext()->setContextProperty("isDark", isDark);
  mui->qwMainTab->rootContext()->setContextProperty("maintabHeight",
                                                    mui->qwMainTab->height());
  mui->qwMainTab->rootContext()->setContextProperty("mw_one", mw_one);
  mui->qwMainTab->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/maintab.qml")));

  mui->qwMainDate->rootContext()->setContextProperty("isDark", isDark);
  mui->qwMainDate->rootContext()->setContextProperty("isAniEffects", true);
  mui->qwMainDate->rootContext()->setContextProperty("maindateWidth",
                                                     mui->qwMainDate->width());
  mui->qwMainDate->rootContext()->setContextProperty("m_Method", m_Method);
  mui->qwMainDate->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/maindate.qml")));

  mui->qwMainEvent->rootContext()->setContextProperty("isDark", isDark);
  mui->qwMainEvent->rootContext()->setContextProperty("fontSize", fontSize);
  mui->qwMainEvent->rootContext()->setContextProperty("isAniEffects", true);
  mui->qwMainEvent->rootContext()->setContextProperty(
      "maineventWidth", mui->qwMainEvent->width());
  mui->qwMainEvent->rootContext()->setContextProperty("m_Method", m_Method);
  mui->qwMainEvent->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/mainevent.qml")));

  mui->qwNotesTree->rootContext()->setContextProperty("fontSize", fontSize);
  mui->qwNotesTree->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/tree_main.qml")));

  mui->qw_Img->rootContext()->setContextProperty("myW", mw_one->width());
  mui->qw_Img->rootContext()->setContextProperty("myH", mw_one->height());

  mui->qwReader->rootContext()->setContextProperty("myW", mw_one->width());
  mui->qwReader->rootContext()->setContextProperty("myH", mw_one->height());
  mui->qwReader->rootContext()->setContextProperty("m_Reader",
                                                   mw_one->m_Reader);
  mui->qwReader->rootContext()->setContextProperty("myBackgroundColor",
                                                   "#FFFFFF");
}

void MainHelper::initNotesQW() {
  if (mui->qwNoteBook->source().isEmpty()) {
    mui->qwNoteBook->rootContext()->setContextProperty("m_NotesList",
                                                       m_NotesList);
    mui->qwNoteBook->rootContext()->setContextProperty("mw_one", mw_one);
    mui->qwNoteBook->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/notebook.qml")));

    if (isAndroid)
      mui->qwNoteList->rootContext()->setContextProperty("noteTimeFontSize",
                                                         12);
    else
      mui->qwNoteList->rootContext()->setContextProperty("noteTimeFontSize", 8);
    mui->qwNoteList->rootContext()->setContextProperty("m_NotesList",
                                                       m_NotesList);
    mui->qwNoteList->rootContext()->setContextProperty("mw_one", mw_one);
    mui->qwNoteList->rootContext()->setContextProperty("noteModel",
                                                       m_NotesList->noteModel);
    mui->qwNoteList->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/notelist.qml")));

    mui->qwNoteTools->rootContext()->setContextProperty("m_NotesList",
                                                        m_NotesList);
    mui->qwNoteTools->rootContext()->setContextProperty("m_Notes",
                                                        mw_one->m_Notes);
    mui->qwNoteTools->rootContext()->setContextProperty("mw_one", mw_one);
    mui->qwNoteTools->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/note_toolsbar.qml")));

    mui->qwNotesSearchResult->rootContext()->setContextProperty("fontSize",
                                                                fontSize);
    mui->qwNotesSearchResult->rootContext()->setContextProperty("m_NotesList",
                                                                m_NotesList);
    mui->qwNotesSearchResult->rootContext()->setContextProperty("mw_one",
                                                                mw_one);
    mui->qwNotesSearchResult->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/SearchResults.qml")));

    mui->qwNoteRecycle->rootContext()->setContextProperty("m_Method", m_Method);
    mui->qwNoteRecycle->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/noterecycle.qml")));
  }
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
    mui->cboxWebDAV->lineEdit()->installEventFilter(editFilter);
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
  mui->frameDiff->hide();

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

  mui->frameNotesGraph->hide();
  mui->frameNotesGraph->layout()->setContentsMargins(1, 1, 1, 1);

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

  sliderButton = new SliderButton(centralWidget);
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

  // Reader
  mui->btnBackReader->setStyleSheet("border:none; ");
  mui->btnCatalogue->setStyleSheet("border:none; ");
  mui->btnShowBookmark->setStyleSheet("border:none; ");
  mui->btnAutoRun->setStyleSheet("border:none; ");
  mui->btnAutoStop->setStyleSheet("border:none; ");
  mui->btnPages->setStyleSheet("border:none; ");
  mui->btnOpen->setStyleSheet("border:none;");
  mui->btnReadList->setStyleSheet("border:none; ");
  mui->btnRotation->setStyleSheet("border:none; ");
  // mui->f_ReaderFun->setStyleSheet("QFrame{background-color: #595959;}");

  if (isDark) {
    mui->btnBackReader->setIcon(QIcon(":/res/reader/exit_l.svg"));
    mui->btnRotation->setIcon(QIcon(":/res/reader/rotation_l.svg"));
    mui->btnCatalogue->setIcon(QIcon(":/res/reader/cata_l.svg"));
    mui->btnShowBookmark->setIcon(QIcon(":/res/reader/bookmark_l.svg"));
    mui->btnAutoRun->setIcon(QIcon(":/res/reader/run_l.svg"));
    mui->btnAutoStop->setIcon(QIcon(":/res/reader/stop_l.svg"));
    mui->btnOpen->setIcon(QIcon(":/res/reader/open_l.svg"));
    mui->btnReadList->setIcon(QIcon(":/res/reader/booklist_l.svg"));
  } else {
    mui->btnBackReader->setIcon(QIcon(":/res/reader/exit.svg"));
    mui->btnRotation->setIcon(QIcon(":/res/reader/rotation.svg"));
    mui->btnCatalogue->setIcon(QIcon(":/res/reader/cata.svg"));
    mui->btnShowBookmark->setIcon(QIcon(":/res/reader/bookmark.svg"));
    mui->btnAutoRun->setIcon(QIcon(":/res/reader/run.svg"));
    mui->btnAutoStop->setIcon(QIcon(":/res/reader/stop.svg"));
    mui->btnOpen->setIcon(QIcon(":/res/reader/open.svg"));
    mui->btnReadList->setIcon(QIcon(":/res/reader/booklist.svg"));
  }

  /*mui->btnPages->setStyleSheet(
      "color: rgb(255, 255, 255);background-color: #FF9933;border: "
      "0px solid "
      "rgb(255,0,0);border-radius: 4px;"
      "font-weight: bold;");*/

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
    mui->btnFind->setIcon(QIcon(":/res/find.svg"));
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

  mui->lblNoteGraphView->setWordWrap(true);
  mui->lblNoteGraphView->adjustSize();
  init_ButtonStyle();
}

void MainHelper::sort_childItem(QTreeWidgetItem *item) {
  QStringList keys, list, keyTime, keysNew;
  int childCount = item->parent()->childCount();

  for (int i = 0; i < childCount; i++) {
    QString txt0 = item->parent()->child(i)->text(0);
    QStringList list0 = txt0.split(".");
    if (list0.count() == 2) {
      txt0 = list0.at(1);
      txt0 = txt0.trimmed();
    }
    QString txt1 = item->parent()->child(i)->text(1);
    QString txt2 = item->parent()->child(i)->text(2);
    QString txt3 = item->parent()->child(i)->text(3);
    keys.append(txt0 + "|===|" + txt1 + "|===|" + txt2 + "|===|" + txt3);
    keyTime.append(txt0);
  }

  std::sort(keyTime.begin(), keyTime.end(),
            [](const QString &s1, const QString &s2) { return s1 < s2; });

  for (int i = 0; i < keyTime.count(); i++) {
    QString time = keyTime.at(i);
    for (int n = 0; n < keys.count(); n++) {
      QString str1 = keys.at(n);
      QStringList l0 = str1.split("|===|");
      if (time == l0.at(0)) {
        keysNew.append(str1);
        break;
      }
    }
  }

  for (int i = 0; i < childCount; i++) {
    QTreeWidgetItem *childItem = item->parent()->child(i);
    QString str = keysNew.at(i);
    list.clear();
    list = str.split("|===|");
    if (list.count() == 4) {
      int number = i + 1;
      QString strChildCount = QString::number(childCount);
      QString strNum;
      strNum = QString("%1").arg(number, strChildCount.length(), 10,
                                 QLatin1Char('0'));
      childItem->setText(0, strNum + ". " + list.at(0).trimmed());
      childItem->setText(1, list.at(1).trimmed());
      childItem->setText(2, list.at(2).trimmed());
      childItem->setText(3, list.at(3).trimmed());
    }
  }
}

void MainHelper::on_AddRecord() {
  isAdd = true;

  mui->lblTitleEditRecord->setText(tr("Add") + "  : " +
                                   tabData->tabText(tabData->currentIndex()));

  mui->hsH->setValue(QTime::currentTime().hour());
  mui->hsM->setValue(QTime::currentTime().minute());
  mw_one->m_EditRecord->getTime(mui->hsH->value(), mui->hsM->value());

  mui->editDetails->clear();
  mui->editCategory->setText("");
  mui->editAmount->setText("");

  mui->frameMain->hide();
  mui->frameEditRecord->show();

  // tmeFlash->start(300);
}

void MainHelper::initChartMonth() {
  if (loading) return;

  int count = PointList.count();
  if (count == 0) {
    return;
  }

  mw_one->barSeries->clear();
  mw_one->series->clear();
  mw_one->m_scatterSeries->clear();
  bool isOne = true;

  QBarSet *setY = new QBarSet("Y");
  QStringList categories;

  for (int i = 0; i < count; i++) {
    if (PointList.at(i).y() != 1) isOne = false;
  }

  if (isOne) {
    mw_one->series->clear();
    mw_one->m_scatterSeries->clear();
    QList<QPointF> tempPointList;
    for (int i = 0; i < count; i++) {
      double y0 = 0.0;
      QString str = listM.at(i);
      QStringList list1 = str.split(".");
      if (list1.count() == 2) {
        QStringList list = list1.at(1).split(":");
        int t = 0;
        if (list.count() == 3) {
          QString a, b, c;
          a = list.at(0);
          b = list.at(1);
          c = list.at(2);
          int a1, b1;
          a1 = a.toInt();
          b1 = b.toInt();
          t = a1 * 3600 + b1 * 60 + c.toInt();
        }
        y0 = (double)t / 3600;
      }

      tempPointList.append(QPointF(PointList.at(i).x(), y0));
    }
    PointList.clear();
    PointList = tempPointList;
  }

  double maxValue = *std::max_element(doubleList.begin(), doubleList.end());
  double max;
  if (isrbFreq) {
    max = chartMax;
    if (maxValue >= max) {
      max = maxValue;
    }

  } else {
    max = 50.00;
    if (maxValue >= max) max = maxValue;
  }

  yMaxMonth = max;

  QList<double> dList, tempDList;
  for (int i = 0; i < PointList.count(); i++) {
    tempDList.append(PointList.at(i).y());
    categories.append(QString::number(PointList.at(i).x()));
  }
  for (int i = 0; i < mw_one->max_day; i++) {
    dList.append(0);
  }
  for (int i = 0; i < categories.count(); i++) {
    for (int n = 0; n < mw_one->max_day; n++) {
      if (categories.at(i) == QString::number(n + 1)) {
        dList.removeAt(n);
        dList.insert(n, PointList.at(i).y());
      }
    }
  }

  for (int i = 0; i < mw_one->max_day; i++) setY->append(dList.at(i));
  categories.clear();
  for (int i = 0; i < mw_one->max_day; i++)
    categories.append(QString::number(i + 1));
  mw_one->barSeries->append(setY);
  mw_one->axisX->setRange("", QString::number(mw_one->max_day));
  mw_one->axisX->append(categories);
  mw_one->axisY->setRange(0, yMaxMonth);

  if (isOne) {
    mw_one->axisY->setRange(0, 24);
    mw_one->chartMonth->setTitle(mw_one->CurrentYear + "  Y:" + tr("Time") +
                                 "    X:" + tr("Days"));
  } else {
    mw_one->axisY->setRange(0, yMaxMonth);
    if (mui->rbFreq->isChecked())
      mw_one->chartMonth->setTitle(mw_one->CurrentYear + "  Y:" + tr("Freq") +
                                   "    X:" + tr("Days"));

    if (mui->rbAmount->isChecked())
      mw_one->chartMonth->setTitle(mw_one->CurrentYear + "  Y:" + tr("Amount") +
                                   "    X:" + tr("Days"));
  }
}

void MainHelper::initChartDay() {
  if (loading) return;
  mw_one->series2->clear();
  mw_one->m_scatterSeries2->clear();
  mw_one->m_scatterSeries2_1->clear();

  int count = PointList.count();
  if (count == 0) return;
  for (int i = 0; i < count; i++) {
    mw_one->series2->append(PointList.at(i));
    mw_one->m_scatterSeries2->append(PointList.at(i));
    mw_one->m_scatterSeries2_1->append(PointList.at(i));
  }

  mw_one->axisX2->setRange(0, 24);
  mw_one->axisX2->setTickType(QValueAxis::TicksFixed);
  mw_one->axisX2->setTickCount(7);

  mw_one->axisY2->setRange(0, yMaxDay + 1);

  if (mui->rbFreq->isChecked())
    mw_one->chartDay->setTitle(mw_one->CurrentYear + "  Y:" + tr("Freq") +
                               "    X:" + tr("Time"));

  if (mui->rbAmount->isChecked())
    mw_one->chartDay->setTitle(mw_one->CurrentYear + "  Y:" + tr("Amount") +
                               "    X:" + tr("Time"));
}

////////////////////////////////////////////////////////////////////////////////

bool MainWindow::importBakData(QString fileName) {
  if (fileName.isNull()) return false;

  deleteDirfile(privateDir + "gps");
  m_Reader->copyDirectoryFiles(iniDir + "memo/gps", privateDir + "gps", true);

  QString zipPath = bakfileDir + "memo.zip";
  if (fileName != zipPath) {
    QFile::remove(zipPath);
    QFile::copy(fileName, zipPath);
  }

  QString dec_file;
  if (isEncrypt) {
    dec_file = zipPath + ".dec";
    bool result = false;
    result = m_Method->decryptFile(zipPath, dec_file, encPassword);

    while (result == false && isPasswordError == false)
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
  } else
    dec_file = zipPath;

  deleteDirfile(bakfileDir + "KnotData");
  bool unzipResult = false;
  unzipResult =
      m_Method->decompressWithPassword(dec_file, bakfileDir, encPassword);

  while (unzipResult == false && isPasswordError == false)
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

  if (isPasswordError) {
    return false;
  }

  isZipOK = true;

  QString bakFileFrom;
  QSettings Reg(bakfileDir + "KnotData/osflag.ini", QSettings::IniFormat);
  bakFileFrom = Reg.value("os", "mobile").toString();
  qDebug() << "bakFileFrom=" << bakFileFrom;

  if (bakFileFrom == "desktop") {
    deleteDirfile(iniDir + "memo");
    QFile::remove(iniDir + "todo.ini");
    QFile::remove(iniDir + "mainnotes.ini");

    m_Reader->copyDirectoryFiles(bakfileDir + "KnotData/memo", iniDir + "memo",
                                 true);
    QFile::copy(bakfileDir + "KnotData/todo.ini", iniDir + "todo.ini");
    QFile::copy(bakfileDir + "KnotData/mainnotes.ini",
                iniDir + "mainnotes.ini");

    deleteDirfile(iniDir + "memo/gps");
    m_Reader->copyDirectoryFiles(bakfileDir + "KnotData/memo/gps",
                                 iniDir + "memo/gps", true);
  }

  if (bakFileFrom == "mobile") {
    deleteDirfile(iniDir);
    m_Reader->copyDirectoryFiles(bakfileDir + "KnotData", iniDir, true);
  }

  return true;
}

void MainWindow::reloadMain() {
  bool isAniEffects;
  if (isDelItem || isEditItem)
    isAniEffects = false;
  else
    isAniEffects = true;

  mui->qwMainDate->rootContext()->setContextProperty("isAniEffects",
                                                     isAniEffects);
  mui->qwMainDate->rootContext()->setContextProperty("maindateWidth",
                                                     mui->qwMainDate->width());
  m_Method->clearAllBakList(mui->qwMainDate);

  QTreeWidget *tw = get_tw(tabData->currentIndex());

  int total = tw->topLevelItemCount();

  if (total == 0) {
    m_Method->clearAllBakList(mui->qwMainEvent);
    return;
  }

  int a;

  if (total - days > 0)
    a = total - days;
  else
    a = 0;

  QString text0, text1, text2, text3, topitem;
  for (int i = a; i < total; i++) {
    QTreeWidgetItem *topItem = tw->topLevelItem(i);

    text0 = topItem->text(0) + "  " + topItem->text(3);
    text1 = topItem->text(1);
    text2 = topItem->text(2);

    topitem = text0;

    m_Method->addItemToQW(mui->qwMainDate, text0, text1, text2, text3, 0);
  }

  m_Method->gotoEnd(mui->qwMainDate);
  int count = m_Method->getCountFromQW(mui->qwMainDate);
  m_Method->setCurrentIndexFromQW(mui->qwMainDate, count - 1);
  m_Method->setScrollBarPos(mui->qwMainDate, 1.0);

  m_Method->clickMainDate();
}

void MainWindow::on_rbSteps_clicked() {
  int count = m_Steps->getCount();
  if (count <= 0) return;

  tabChart->setCurrentIndex(0);
  tabChart->setTabEnabled(1, false);

  PointList.clear();
  doubleList.clear();

  QString sm = get_Month(m_Method->setCurrentDateValue());
  QString strday, strmonth;
  for (int i = 0; i < count; i++) {
    QString strD = m_Steps->getDate(i);

    QStringList list = strD.split(" ").at(0).split("-");
    if (list.count() > 1) {
      strmonth = list.at(1);
      strday = list.at(2);
    }
    if (sm.toInt() == strmonth.toInt()) {
      int day = strday.toInt();
      int steps = m_Steps->getSteps(i);

      PointList.append(QPointF(day, steps));
      doubleList.append(steps);
    }
  }

  m_MainHelper->initChartMonth();
  chartMonth->setTitle("Y:" + tr("Steps") + "    X:" + tr("Days"));
}

QStringList MainWindow::get_MonthList(QString strY, QString strM) {
  QStringList listMonth;
  if (loading) return listMonth;
  // 格式：记录第一个子项的时间
  PointList.clear();
  doubleList.clear();

  QTreeWidget *tw = (QTreeWidget *)tabData->currentWidget();
  for (int i = 0; i < tw->topLevelItemCount(); i++) {
    if (isBreak) break;
    QTreeWidgetItem *topItem = tw->topLevelItem(i);
    QString str0 = topItem->text(0) + " " + topItem->text(3);
    QString y, m, d;
    y = get_Year(str0);
    m = get_Month(str0);
    d = QString::number(get_Day(str0));
    double x0 = d.toDouble();
    if (y == strY) {
      if (m == strM) {
        if (isrbFreq) {
          if (topItem->childCount() > 0)
            listMonth.append(
                topItem->child(0)->text(0));  // 记录第一个子项的时间

          double y0 = topItem->text(1).toDouble();
          doubleList.append(y0);

          PointList.append(QPointF(x0, y0));
        } else {
          double y0 = topItem->text(2).toDouble();
          doubleList.append(y0);

          PointList.append(QPointF(x0, y0));
        }
      }
    }
  }

  return listMonth;
}

void MainWindow::init_Options() {
  QSettings Reg2(iniDir + "ymd.ini", QSettings::IniFormat);

  btnYText = Reg2.value("/YMD/btnYText", 2022).toString();

  btnMText = Reg2.value("/YMD/btnMText", tr("Month")).toString();

  btnDText = Reg2.value("/YMD/btnDText", 1).toString();

  btnYearText = Reg2.value("/YMD/btnYearText", "2022").toString();
  mui->btnYear->setText(btnYearText);
  btnMonthText = Reg2.value("/YMD/btnMonthText", "01").toString();
  mui->btnMonth->setText(btnMonthText);

  s_y1 = Reg2.value("/YMD/Y1", 2022).toInt();
  s_y2 = Reg2.value("/YMD/Y2", 2022).toInt();
  s_m1 = Reg2.value("/YMD/M1", 1).toInt();
  s_m2 = Reg2.value("/YMD/M2", 12).toInt();
  s_d1 = Reg2.value("/YMD/D1", 1).toInt();
  s_d2 = Reg2.value("/YMD/D2", 1).toInt();

  mui->btnStartDate->setText(QString::number(s_y1) + "  " +
                             QString("%1").arg(s_m1, 2, 10, QLatin1Char('0')) +
                             "  " +
                             QString("%1").arg(s_d1, 2, 10, QLatin1Char('0')));
  mui->btnEndDate->setText(QString::number(s_y2) + "  " +
                           QString("%1").arg(s_m2, 2, 10, QLatin1Char('0')) +
                           "  " +
                           QString("%1").arg(s_d2, 2, 10, QLatin1Char('0')));

  isWholeMonth = Reg2.value("/YMD/isWholeMonth", 1).toBool();
  isDateSection = Reg2.value("/YMD/isDateSection", 0).toBool();

  // time machine
  QSettings RegTime(privateDir + "timemachine.ini", QSettings::IniFormat);

  int countTime = RegTime.value("/TimeLines/Count", 0).toInt();
  for (int i = 0; i < countTime; i++)
    timeLines.append(
        RegTime.value("/TimeLines/Files" + QString::number(i)).toString());

  m_Preferences->initOptions();
  m_Preferences->ui->btnReStart->hide();
}

void MainWindow::init_ChartWidget() {
  mui->centralwidget->layout()->setContentsMargins(1, 0, 1, 2);
  mui->centralwidget->layout()->setSpacing(1);
  mui->f_charts->setContentsMargins(0, 0, 0, 0);

  mui->f_charts->layout()->setContentsMargins(0, 0, 0, 0);
  mui->f_charts->layout()->setSpacing(0);
  frameChartHeight = 105;
  mui->f_charts->setFixedHeight(frameChartHeight);
  tabChart->setCurrentIndex(0);

  mui->glMonth->layout()->setContentsMargins(0, 0, 0, 0);
  mui->glMonth->layout()->setSpacing(0);
  mui->glDay->layout()->setContentsMargins(0, 0, 0, 0);
  mui->glDay->layout()->setSpacing(0);

  mui->f_charts->hide();
  mui->btnChartDay->hide();
  mui->btnChartMonth->hide();
  mui->rbAmount->hide();
  mui->rbFreq->hide();
  mui->rbSteps->hide();

  int a0 = 0;
  int a1 = -2;
  // Month
  chartMonth = new QChart();
  chartview = new QChartView(chartMonth);
  chartview->installEventFilter(this);
  mui->glMonth->addWidget(chartview);
  chartview->setRenderHint(QPainter::Antialiasing);
  chartMonth->legend()->hide();
  chartMonth->setMargins(QMargins(a0, a0, a0, a0));
  chartMonth->setContentsMargins(a1, a1, a1, a1);
  chartMonth->setAnimationOptions(QChart::SeriesAnimations);

  barSeries = new QBarSeries();
  series = new QSplineSeries();
  series->setPen(QPen(Qt::blue, 3, Qt::SolidLine));
  m_scatterSeries = new QScatterSeries();
  m_scatterSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);
  m_scatterSeries->setMarkerSize(10);
  chartMonth->addSeries(barSeries);
  chartMonth->addSeries(series);
  chartMonth->addSeries(m_scatterSeries);

  // Day
  chartDay = new QChart();
  chartview1 = new QChartView(chartDay);
  chartview1->installEventFilter(this);
  mui->glDay->addWidget(chartview1);
  chartview1->setRenderHint(QPainter::Antialiasing);
  chartDay->legend()->hide();
  chartDay->setMargins(QMargins(a0, a0, a0, a0));
  chartDay->setContentsMargins(a1, a1, a1, a1);
  chartDay->setAnimationOptions(QChart::SeriesAnimations);

  series2 = new QSplineSeries(chartDay);
  series2->setPen(QPen(Qt::blue, 2, Qt::SolidLine));
  m_scatterSeries2 = new QScatterSeries();
  m_scatterSeries2_1 = new QScatterSeries();

  // 散点图(用于边框)
  m_scatterSeries2->setMarkerShape(
      QScatterSeries::MarkerShapeCircle);                 // 圆形的点
  m_scatterSeries2->setBorderColor(QColor(255, 0, 0));    // 边框颜色
  m_scatterSeries2->setBrush(QBrush(QColor(255, 0, 0)));  // 背景颜色
  m_scatterSeries2->setMarkerSize(5);                     // 点大小

  // 散点图(用于中心)
  m_scatterSeries2_1->setMarkerShape(
      QScatterSeries::MarkerShapeCircle);         // 圆形的点
  m_scatterSeries2_1->setBorderColor(Qt::red);    // 边框颜色
  m_scatterSeries2_1->setBrush(QBrush(Qt::red));  // 背景颜色
  m_scatterSeries2_1->setMarkerSize(4);           // 点大小
  connect(m_scatterSeries2_1, &QScatterSeries::hovered, this,
          &MainWindow::slotPointHoverd);  // 用于鼠标移动到点上显示数值
  m_valueLabel = new QLabel(this);
  m_valueLabel->adjustSize();
  m_valueLabel->setHidden(true);

  chartDay->addSeries(series2);
  chartDay->addSeries(m_scatterSeries2);
  chartDay->addSeries(m_scatterSeries2_1);

  // chartMonth->createDefaultAxes();
  axisX = new QBarCategoryAxis();
  chartMonth->addAxis(axisX, Qt::AlignBottom);
  barSeries->attachAxis(axisX);

  axisY = new QValueAxis();
  chartMonth->addAxis(axisY, Qt::AlignLeft);
  barSeries->attachAxis(axisY);

  // chartDay->createDefaultAxes();
  axisX2 = new QValueAxis();
  chartDay->addAxis(axisX2, Qt::AlignBottom);
  series2->attachAxis(axisX2);

  axisY2 = new QValueAxis();
  chartDay->addAxis(axisY2, Qt::AlignLeft);
  series2->attachAxis(axisY2);

  m_scatterSeries2->attachAxis(axisX2);
  m_scatterSeries2->attachAxis(axisY2);
  m_scatterSeries2_1->attachAxis(axisX2);
  m_scatterSeries2_1->attachAxis(axisY2);
}

void MainWindow::readData(QTreeWidget *tw) {
  tw->clear();

  QStringList myTopStrList;

  int iniFileCount = QDate::currentDate().year() - 2025 + 1 + 1;
  QString name = tw->objectName();
  QString ini_file;
  for (int p = 0; p < iniFileCount; p++) {
    if (p == 0) {
      ini_file = iniDir + name + ".ini";
    } else {
      QString strYear = QString::number(2025 + p - 1);
      ini_file = iniDir + strYear + "-" + name + ".ini";
    }

    qDebug() << "read ini_file=" << ini_file;

    if (QFile::exists(ini_file)) {
      QSettings Reg(ini_file, QSettings::IniFormat);

      QString group = Reg.childGroups().at(0);

      int rowCount = Reg.value("/" + group + "/TopCount").toInt();
      for (int i = 0; i < rowCount; i++) {
        int childCount = Reg.value("/" + group + "/" + QString::number(i + 1) +
                                   "-childCount")
                             .toInt();

        // 不显示子项为0的数据
        if (childCount > 0) {
          QTreeWidgetItem *topItem = new QTreeWidgetItem;
          QString strD0 =
              Reg.value("/" + group + "/" + QString::number(i + 1) + "-topDate")
                  .toString();

          QStringList lista = strD0.split(" ");
          if (lista.count() == 4) {
            QString a0 = lista.at(0) + " " + lista.at(1) + " " + lista.at(2);
            topItem->setText(0, a0);
            topItem->setText(3, lista.at(3));
          } else {
            topItem->setText(0, strD0);
            QString year = Reg.value("/" + group + "/" +
                                     QString::number(i + 1) + "-topYear")
                               .toString();
            topItem->setText(3, year);
          }

          topItem->setTextAlignment(1, Qt::AlignHCenter | Qt::AlignVCenter);
          topItem->setTextAlignment(2, Qt::AlignRight | Qt::AlignVCenter);

          topItem->setText(1, Reg.value("/" + group + "/" +
                                        QString::number(i + 1) + "-topFreq")
                                  .toString());
          topItem->setText(2, Reg.value("/" + group + "/" +
                                        QString::number(i + 1) + "-topAmount")
                                  .toString());

          // 移除异项（时间相同，但频次处于累加的异常情况）
          int lastTopIndex = tw->topLevelItemCount() - 1;
          if (lastTopIndex >= 0) {
            QTreeWidgetItem *lastTopItem = tw->topLevelItem(lastTopIndex);
            // 时间相同但频次或金额不同
            if ((lastTopItem->text(0) == topItem->text(0)) &&
                (lastTopItem->text(2) != topItem->text(2) ||
                 lastTopItem->text(1) != topItem->text(1))) {
              tw->takeTopLevelItem(lastTopIndex);
            }
          }

          QString topStr = QString("%1|%2|%3|%4")
                               .arg(topItem->text(0), topItem->text(1),
                                    topItem->text(2), topItem->text(3));

          if (myTopStrList.contains(topStr)) {
            for (int x = 0; x < tw->topLevelItemCount(); x++) {
              QString top, top0, top1, top2, top3;
              top0 = tw->topLevelItem(x)->text(0);
              top1 = tw->topLevelItem(x)->text(1);
              top2 = tw->topLevelItem(x)->text(2);
              top3 = tw->topLevelItem(x)->text(3);
              top = QString("%1|%2|%3|%4").arg(top0, top1, top2, top3);
              if (top == topStr) {
                tw->takeTopLevelItem(x);
                tw->insertTopLevelItem(x, topItem);
                for (int j = 0; j < childCount; j++) {
                  QTreeWidgetItem *item11 = new QTreeWidgetItem(topItem);
                  item11->setText(
                      0, Reg.value("/" + group + "/" + QString::number(i + 1) +
                                   "-childTime" + QString::number(j))
                             .toString());
                  item11->setText(
                      1, Reg.value("/" + group + "/" + QString::number(i + 1) +
                                   "-childAmount" + QString::number(j))
                             .toString());
                  item11->setText(
                      2, Reg.value("/" + group + "/" + QString::number(i + 1) +
                                   "-childDesc" + QString::number(j))
                             .toString());
                  item11->setText(
                      3, Reg.value("/" + group + "/" + QString::number(i + 1) +
                                   "-childDetails" + QString::number(j))
                             .toString());

                  item11->setTextAlignment(1,
                                           Qt::AlignRight | Qt::AlignVCenter);
                }
                break;
              }
            }
          }

          if (!myTopStrList.contains(topStr)) {
            tw->addTopLevelItem(topItem);
            myTopStrList.append(topStr);

            for (int j = 0; j < childCount; j++) {
              QTreeWidgetItem *item11 = new QTreeWidgetItem(topItem);
              item11->setText(
                  0, Reg.value("/" + group + "/" + QString::number(i + 1) +
                               "-childTime" + QString::number(j))
                         .toString());
              item11->setText(
                  1, Reg.value("/" + group + "/" + QString::number(i + 1) +
                               "-childAmount" + QString::number(j))
                         .toString());
              item11->setText(
                  2, Reg.value("/" + group + "/" + QString::number(i + 1) +
                               "-childDesc" + QString::number(j))
                         .toString());
              item11->setText(
                  3, Reg.value("/" + group + "/" + QString::number(i + 1) +
                               "-childDetails" + QString::number(j))
                         .toString());

              item11->setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);
            }
          }
        }
      }
    }
  }
}

void MainWindow::drawMonthChart() {
  listM.clear();
  listM = get_MonthList(strY, strM);
  CurrentYearMonth = strY + strM;
}

void MainWindow::drawDayChart() {
  QTreeWidget *tw = (QTreeWidget *)tabData->currentWidget();
  if (loading) return;
  PointList.clear();

  int topCount = tw->topLevelItemCount();
  if (topCount == 0) {
    return;
  }

  if (topCount > 0) {
    if (!tw->currentIndex().isValid()) {
      QTreeWidgetItem *topItem = tw->topLevelItem(topCount - 1);
      tw->setCurrentItem(topItem);
    }
  }

  QTreeWidgetItem *item = tw->currentItem();
  bool child;
  int childCount;
  if (item->parent() == NULL)
    child = false;
  else
    child = true;

  QString month;

  if (child) {
    childCount = item->parent()->childCount();
    parentItem = item->parent();
    month = get_Month(item->parent()->text(0));
  } else {
    childCount = item->childCount();
    parentItem = item;
    month = get_Month(item->text(0));
  }

  QList<double> dList;
  double x, y;
  QString str, str1;
  int step = 1;
  if (childCount > 500) step = 100;

  for (int i = 0; i < childCount; i = i + step) {
    if (isBreak) break;

    if (child) {
      str = item->parent()->child(i)->text(0);
      str1 = item->parent()->child(i)->text(1);
    } else {
      str = item->child(i)->text(0);
      str1 = item->child(i)->text(1);
    }
    QStringList l0 = str.split(".");
    if (l0.count() == 2) str = l0.at(1);
    QStringList list = str.split(":");
    int t = 0;
    if (list.count() == 3) {
      QString a, b, c;
      a = list.at(0);
      b = list.at(1);
      c = list.at(2);
      int a1, b1;
      a1 = a.toInt();
      b1 = b.toInt();
      t = a1 * 3600 + b1 * 60 + c.toInt();
    }
    x = (double)t / 3600;
    if (isrbFreq)
      y = i + 1;
    else {
      y = str1.toDouble();
      dList.append(y);
    }

    PointList.append(QPointF(x, y));
  }

  if (isrbFreq) {
    int a = chartMax;
    if (childCount > a)
      yMaxDay = childCount;
    else
      yMaxDay = a;
  } else {
    yMaxDay = *std::max_element(dList.begin(), dList.end());
  }
}
