#include "MainWindow.h"

void MainWindow::on_btn7_pressed() { m_EditRecord->on_btn7_clicked(); }

void MainWindow::on_btn8_pressed() { m_EditRecord->on_btn8_clicked(); }

void MainWindow::on_btn9_pressed() { m_EditRecord->on_btn9_clicked(); }

void MainWindow::on_btn4_pressed() { m_EditRecord->on_btn4_clicked(); }

void MainWindow::on_btn5_pressed() { m_EditRecord->on_btn5_clicked(); }

void MainWindow::on_btn6_pressed() { m_EditRecord->on_btn6_clicked(); }

void MainWindow::on_btn1_pressed() { m_EditRecord->on_btn1_clicked(); }

void MainWindow::on_btn2_pressed() { m_EditRecord->on_btn2_clicked(); }

void MainWindow::on_btn3_pressed() { m_EditRecord->on_btn3_clicked(); }

void MainWindow::on_btn0_pressed() { m_EditRecord->on_btn0_clicked(); }

void MainWindow::on_btnDot_pressed() { m_EditRecord->on_btnDot_clicked(); }

void MainWindow::on_btnDel_Number_pressed() {
  m_EditRecord->on_btnDel_clicked();
}

void MainWindow::on_btnBackBookList_pressed() {
  if (isPDF) {
    if (isAndroid) {
      mui->frameMain->show();
      mui->frameBookList->hide();
    }
  } else {
    mui->frameReader->show();
    mui->frameBookList->hide();
  }
}

void MainWindow::on_btnOkBookList_pressed() { m_Reader->openBookListItem(); }

void MainWindow::on_btnClearAllRecords_pressed() {
  m_Reader->clearAllReaderRecords();
}

void MainWindow::on_btnAnd_pressed() { mui->editSearchText->insert("&"); }

void MainWindow::on_btnClear_pressed() { mui->editTodo->clear(); }

void MainWindow::on_btnModify_pressed() { m_Todo->reeditText(); }

void MainWindow::on_btnTabMoveUp_pressed() {
  if (tabData->count() == 0) return;
  int curIndex = tabData->currentIndex();
  if (curIndex > 0) {
    tabData->tabBar()->moveTab(curIndex, curIndex - 1);
    updateMainTab();
    saveTab();
    getMainTabs();
  }
}

void MainWindow::on_btnTabMoveDown_pressed() {
  if (tabData->count() == 0) return;
  int curIndex = tabData->currentIndex();
  if (curIndex <= tabData->count() - 2) {
    tabData->tabBar()->moveTab(curIndex, curIndex + 1);
    updateMainTab();
    saveTab();
    getMainTabs();
  }
}

void MainWindow::updateMainTab() {
  clearAll();
  for (int i = 0; i < tabData->count(); i++) {
    QString tabText = tabData->tabText(i);
    QTreeWidget* tw = get_tw(i);
    int isFlagToday = m_Method->getFlagToday(tw);
    addItem(tabText, "", "", "", isFlagToday);
  }
  setCurrentIndex(tabData->currentIndex());
}

void MainWindow::on_btnChart() { m_MainHelper->clickBtnChart(); }

void MainWindow::on_btnManagement_pressed() {
  int x, y, w, h;
  x = geometry().x();
  y = geometry().y();
  w = width();
  h = height();
  m_NotesList->setGeometry(x, y, w, h);
  m_NotesList->show();
  tw->setFocus();
}

void MainWindow::on_btnUpMove_pressed() {
  if (m_Method->getCountFromQW(mui->qwNoteBook) == 0) return;

  m_NotesList->setTWCurrentItem();
  m_NotesList->on_btnUp_clicked();
}

void MainWindow::on_btnDownMove_pressed() {
  if (m_Method->getCountFromQW(mui->qwNoteBook) == 0) return;

  m_NotesList->setTWCurrentItem();
  m_NotesList->on_btnDown_clicked();
}

void MainWindow::on_btnDelNote_NoteBook_pressed() {
  m_NotesList->setTWCurrentItem();
  m_NotesList->on_btnDel_clicked();

  m_NotesList->updateAllNoteIndexManager();
}

void MainWindow::on_btnMoveTo_pressed() {
  m_NotesList->setTWCurrentItem();
  m_NotesList->on_btnMoveTo_clicked();
}

void MainWindow::on_btnBack_Tree_pressed() {
  mui->frameNotesTree->hide();
  mui->frameNoteList->show();
}

void MainWindow::on_btnRename_pressed() { m_Notes->renameTitle(false); }

void MainWindow::on_btnHideFind_pressed() {
  closeTextToolBar();
  mui->f_FindNotes->hide();
}

void MainWindow::on_btnStepsOptions_clicked() { m_StepsOptions->init(); }

void MainWindow::on_btnRecentOpen_pressed() {
  m_NotesList->genRecentOpenMenu();
}

void MainWindow::on_btnMenuReport_pressed() { m_Report->genReportMenu(); }

void MainWindow::on_btnCatalogue_pressed() {
  if (mui->lblBookName->text() == "Book Name") return;

  if (mui->qwViewBookNote->isVisible()) return;
  if (mui->qwBookmark->isVisible()) return;

  if (mui->qwCata->source().isEmpty()) {
    mui->qwCata->rootContext()->setContextProperty("m_Reader",
                                                   mw_one->m_Reader);
    mui->qwCata->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/epub_cata.qml")));
  }

  mui->btnAutoStop->click();

  if (mui->f_ReaderSet->isVisible()) {
    on_btnBackReaderSet_pressed();
  }
  m_Reader->showCatalogue();
}

void MainWindow::on_btnRemoveBookList_pressed() { m_Reader->removeBookList(); }

void MainWindow::on_btnShowBookmark_pressed() {
  if (mui->qwViewBookNote->isVisible()) return;
  if (mui->qwCata->isVisible()) return;

  if (mui->qwBookmark->source().isEmpty()) {
    mui->qwBookmark->rootContext()->setContextProperty("m_Reader",
                                                       mw_one->m_Reader);
    mui->qwBookmark->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/bookmark.qml")));
  }

  m_Reader->showOrHideBookmark();
}

void MainWindow::on_btnShareImage_pressed() {
  m_ReceiveShare->shareImage(tr("Share to"), imgFileName, "image/png");
}

void MainWindow::on_btnDelImage_pressed() {}

void MainWindow::on_btnBackReaderSet_pressed() {
  closeTextToolBar();
  mui->f_ReaderSet->hide();
  qreal pos = m_Reader->getVPos();
  m_Reader->setVPos(pos + 0.01);

  QSettings Reg(privateDir + "reader.ini", QSettings::IniFormat);
  Reg.setValue("/Reader/editAutoStopTTS", mui->editAutoStopTTS->text());
  Reg.setValue("/Reader/chkAutoStopTTS", mui->chkAutoStopTTS->isChecked());
}

void MainWindow::on_btnSetBookmark_pressed() {
  mw_one->on_btnBackReaderSet_pressed();
  QTimer::singleShot(200, this, SLOT(slotSetBookmark()));
}

void MainWindow::slotSetBookmark() { m_ReaderSet->on_btnSetBookmark_clicked(); }

void MainWindow::on_btnFontLess_pressed() {
  m_ReaderSet->on_btnFontLess_clicked();
}

void MainWindow::on_btnFontPlus_pressed() {
  m_ReaderSet->on_btnFontPlus_clicked();
}

void MainWindow::on_btnFont_pressed() { m_ReaderSet->on_btnFont_clicked(); }

void MainWindow::on_btnBackgroundColor_pressed() {
  m_ReaderSet->on_btnBackgroundColor_clicked();
}

void MainWindow::on_btnForegroundColor_pressed() {
  m_ReaderSet->on_btnForegroundColor_clicked();
}

void MainWindow::on_editBackgroundColor_textChanged(const QString& arg1) {
  m_ReaderSet->on_editBackgroundColor_textChanged(arg1);
}

void MainWindow::on_editForegroundColor_textChanged(const QString& arg1) {
  m_ReaderSet->on_editForegroundColor_textChanged(arg1);
}

void MainWindow::on_btnStyle1_pressed() { m_ReaderSet->on_btnStyle1_clicked(); }

void MainWindow::on_btnStyle2_pressed() { m_ReaderSet->on_btnStyle2_clicked(); }

void MainWindow::on_btnStyle3_pressed() { m_ReaderSet->on_btnStyle3_clicked(); }

void MainWindow::on_btnGoPage_pressed() { m_ReaderSet->on_btnGoPage_clicked(); }

void MainWindow::on_btnShareBook_pressed() { m_Reader->shareBook(); }

void MainWindow::on_btnAutoRun_pressed() {
  if (mui->qwViewBookNote->isVisible()) return;

  if (!m_Reader->isAutoRun) {
    mui->qwReader->rootContext()->setContextProperty("isAutoRun", true);
    mui->btnAutoRun->hide();
    mui->btnAutoStop->show();
    m_Reader->isAutoRun = true;
  }
}

void MainWindow::on_btnAutoStop_pressed() {
  if (m_Reader->isAutoRun) {
    mui->qwReader->rootContext()->setContextProperty("isAutoRun",
                                                     QVariant(false));
    mui->btnAutoStop->hide();
    mui->btnAutoRun->show();
    m_Reader->isAutoRun = false;
  }
}

void MainWindow::on_btnLessen_pressed() { m_ReaderSet->on_btnLessen_clicked(); }

void MainWindow::on_btnDefault_pressed() {
  m_ReaderSet->on_btnDefault_clicked();
}

void MainWindow::on_btnPlus_pressed() { m_ReaderSet->on_btnAdd_clicked(); }

void MainWindow::on_btnAddTodo_clicked() { m_Todo->on_btnAdd_clicked(); }

void MainWindow::on_btnAddTodo_pressed() {
  m_Todo->isRecordVoice = false;
  tmeStartRecordAudio->start(750);
}

void MainWindow::on_btnAddTodo_released() {
  tmeStartRecordAudio->stop();
  m_Todo->stopRecordVoice();
}

void MainWindow::on_btnClearReaderFont_pressed() {
  m_ReaderSet->on_btnClear_clicked();
}

void MainWindow::on_btnMove() {
  isMoveEntry = true;
  if (del_Data((QTreeWidget*)mui->tabWidget->currentWidget())) {
    mui->btnTabMoveDown->hide();
    mui->btnTabMoveUp->hide();
    on_btnSelTab_pressed();

    while (mui->frameEditRecord->isHidden()) {
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
      QThread::msleep(1);
    }

    mw_one->m_EditRecord->isNoShowSuggestions = true;
    mui->editCategory->setText(strCategory);
    mui->editDetails->setText(strDetails);
    mui->editAmount->setText(strAmount);

    on_btnOkEditRecord_pressed();
  }
}

void MainWindow::on_btnGPS_pressed() {
  if (mui->btnGPS->text() == tr("Start")) {
    m_Steps->startRecordMotion();

  } else if (mui->btnGPS->text() == tr("Stop")) {
    m_Steps->stopRecordMotion();
    mui->btnGPS->setText(tr("Start"));
  }
}

void MainWindow::on_btnSelGpsDate_pressed() { m_Steps->selGpsListYearMonth(); }

void MainWindow::on_btnGetGpsListData_pressed() {
  m_Steps->getGpsListDataFromYearMonth();
}

void MainWindow::on_btnBackBakList_pressed() {
  mui->frameMain->show();
  mui->frameBakList->hide();
}

void MainWindow::on_btnImportBakList_pressed() {
  m_MainHelper->importBakFileList();
}

void MainWindow::on_btnOkViewCate_pressed() { m_Report->on_CateOk(); }

void MainWindow::on_btnBackTabRecycle_pressed() {
  mui->frameMain->show();
  mui->frameTabRecycle->hide();
}

void MainWindow::on_btnDelTabRecycle_pressed() {
  m_MainHelper->delTabRecycleFile();
}

void MainWindow::on_btnRestoreTab_pressed() {
  m_MainHelper->clickBtnRestoreTab();
}

void MainWindow::on_btnDelBakFile_pressed() { m_MainHelper->delBakFile(); }

void MainWindow::on_btnBackNoteList_pressed() {
  clearWidgetFocus();

  m_NotesList->saveCurrentNoteInfo();
  m_NotesList->saveNotesListIndex();
  m_Notes->updateMainnotesIniToSyncLists();

  saveNeedSyncNotes();

  mui->frameMain->show();
  mui->frameNoteList->hide();

  if (m_Notes->checkAndUpdateCleanDate())
    qDebug() << "已到自动清理服务器文件时间，过去3个月内的文件将被清理";
  else
    qDebug() << "不到日期，不用自动清理服务器上的旧文件";

  // 先清空旧连接，避免重复触发
  disconnect(m_Notes, &Notes::syncFinished, this, nullptr);

  // 绑定：等 sync 全部结束 → 再删除
  disconnect(m_Notes, &Notes::syncFinished, this, nullptr);

  connect(
      m_Notes, &Notes::syncFinished, this,
      [this]() { m_NotesList->delRemoteWebDAVFiles(); },
      Qt::ConnectionType(Qt::QueuedConnection | Qt::SingleShotConnection));

  // 现在才开始同步
  m_Notes->syncToWebDAV();
}

void MainWindow::on_btnBackNoteRecycle_clicked() {
  mui->frameNoteRecycle->hide();
  mui->frameNoteList->show();

  if (m_NotesList->isDelNoteRecycle) {
    m_Notes->startBackgroundTaskDelAndClear();
  }
}

void MainWindow::on_btnNoteRecycle_pressed() {
  mui->frameNoteList->hide();
  mui->frameNoteRecycle->show();

  m_NotesList->loadAllRecycle();
}

void MainWindow::on_btnDelNoteRecycle_clicked() {
  int count = m_Method->getCountFromQW(mui->qwNoteRecycle);
  if (count == 0) return;

  int index = m_Method->getCurrentIndexFromQW(mui->qwNoteRecycle);
  if (index < 0) return;

  m_NotesList->setTWRBCurrentItem();
  m_NotesList->on_btnBatchDel_Recycle_clicked();
}

void MainWindow::on_btnRestoreNoteRecycle_clicked() {
  m_NotesList->restoreNoteFromRecycle();

  m_NotesList->updateAllNoteIndexManager();
}

void MainWindow::on_btnFindNotes_pressed() {
  QString str = mui->editFindNote->text().trimmed();
  if (str.length() == 0) return;
  m_NotesList->startFind(str);
}

void MainWindow::on_btnFindPreviousNote_pressed() { m_NotesList->goPrevious(); }

void MainWindow::on_btnFindNextNote_pressed() { m_NotesList->goNext(); }

void MainWindow::on_btnClearNoteFindText_pressed() {
  mui->editFindNote->setText("");
  mui->lblFindNoteCount->setText("0");
  mui->btnFindNextNote->setEnabled(false);
  mui->btnFindPreviousNote->setEnabled(false);
  mui->lblShowLineSn->setText("0");
}

void MainWindow::on_btnShowFindNotes_pressed() { m_NotesList->showFindNotes(); }

void MainWindow::on_btnNoteBookMenu_pressed() {
  m_NotesList->showNoteBookMenu(mui->qwNoteBook->x(), mui->qwNoteBook->y());
}

void MainWindow::on_btnNoteMenu_pressed() {
  m_NotesList->showNotsListMenu(mui->qwNoteList->x(), mui->qwNoteList->y());
}

void MainWindow::on_btnCancelType_pressed() {
  m_CategoryList->on_btnCancel_clicked();
}

void MainWindow::on_btnOkType_pressed() { m_CategoryList->on_btnOk_clicked(); }

void MainWindow::on_btnDelType_pressed() {
  m_CategoryList->on_btnDel_clicked();
}

void MainWindow::on_btnRenameType_pressed() {
  m_CategoryList->ui->editRename->setText(
      mui->editRenameType->text().trimmed());
  m_CategoryList->on_btnRename_clicked();
}

void MainWindow::on_btnBackSetTab_pressed() {
  mui->frameMain->show();
  mui->frameSetTab->hide();

  if (mui->btnTabMoveDown->isHidden()) {
    mui->btnTabMoveDown->show();
    mui->btnTabMoveUp->show();
    on_btnAdd_pressed();
    m_EditRecord->setCurrentValue();
  }
}

void MainWindow::on_btnBackSearch_pressed() {
  clearWidgetFocus();

  mui->frameMain->show();
  mui->frameSearch->hide();
}

void MainWindow::on_btnClearSearchText_pressed() {
  mui->editSearchText->setText("");
  mui->editSearchText->setFocus();
}

void MainWindow::on_btnStartSearch_pressed() {
  mui->editSearchText->clearFocus();

  searchStr = mui->editSearchText->text().trimmed();
  if (searchStr.length() == 0) return;

  showProgress();
  m_Method->data_for_search = m_Method->exportAllDataForSearch();

  qDebug() << "ExportAllDataForSearch:" << m_Method->data_for_search.size();

  // 安卓安全的跨线程调用
  QMetaObject::invokeMethod(
      m_searchWorker,
      [=]() {
        m_searchWorker->startSearch(m_Method->data_for_search, searchStr);
      },
      Qt::QueuedConnection);
}

void MainWindow::on_btnStartDate_pressed() {
  m_DateSelector->initStartEndDate("start");
}

void MainWindow::on_btnEndDate_pressed() {
  m_DateSelector->initStartEndDate("end");
}

void MainWindow::on_btnBackEditRecord_pressed() {
  clearWidgetFocus();

  m_EditRecord->hideSuggestions();

  mui->frameMain->show();
  mui->frameEditRecord->hide();
}

void MainWindow::on_btnType_pressed() { m_EditRecord->on_btnType_clicked(); }

void MainWindow::on_btnOkEditRecord_pressed() {
  m_EditRecord->on_btnOk_clicked();
}

void MainWindow::on_btnClearType_pressed() { mui->editCategory->setText(""); }

void MainWindow::on_btnClearDetails_pressed() { mui->editDetails->setText(""); }

void MainWindow::on_btnClearAmount_pressed() { mui->editAmount->setText(""); }

void MainWindow::on_btnBackSteps_clicked() { m_Steps->closeSteps(); }

void MainWindow::on_btnReset_pressed() { m_Steps->on_btnReset_clicked(); }

void MainWindow::on_btnBack_Report_pressed() { m_Report->on_btnBack_clicked(); }

void MainWindow::on_btnYear_pressed() { m_Report->on_btnYear_clicked(); }

void MainWindow::on_btnMonth_pressed() { m_Report->on_btnMonth_clicked(); }

void MainWindow::on_btnCategory_pressed() {
  m_Report->on_btnCategory_clicked();
}

void MainWindow::on_btnSync_pressed() { on_btnUpload_pressed(); }

void MainWindow::on_btnPasteTodo_pressed() { mui->editTodo->paste(); }

void MainWindow::on_btnBackDir_pressed() { m_Reader->backDir(); }
