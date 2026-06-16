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

void MainWindow::on_btnBackNoteDiff_pressed() { m_NotesList->closeNoteDiff(); }

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
  mui->frameNotesTree->show();
  mui->frameNoteList->hide();
}

void MainWindow::on_btnUpMove_pressed() {
  if (m_Method->getCountFromQW(mui->qwNoteBook) == 0) return;

  m_NotesList->on_btnUp_clicked();
}

void MainWindow::on_btnDownMove_pressed() {
  if (m_Method->getCountFromQW(mui->qwNoteBook) == 0) return;

  m_NotesList->on_btnDown_clicked();
}

void MainWindow::on_btnDelNote_NoteBook_pressed() {
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
    mui->qwCata->rootContext()->setContextProperty("m_Reader", m_Reader);
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
    mui->qwBookmark->rootContext()->setContextProperty("m_Reader", m_Reader);
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
  isStopMoveNote = false;
  m_NotesList->restoreNoteFromRecycle();

  m_NotesList->updateAllNoteIndexManager();
}

void MainWindow::on_btnFindNotes_clicked() {
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

void MainWindow::on_btnStartSearch_clicked() {
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

void MainWindow::on_btnWebDAVBackup_pressed() {
  if (!mui->btnReader->isEnabled()) return;
  m_CloudBackup->startBakData();
}

void MainWindow::on_btnWebDAVRestore_pressed() {
  m_CloudBackup->webDAVRestoreData();
}

void MainWindow::on_chkWebDAV_pressed() {}

void MainWindow::on_btnBack_NotesSearchResult_pressed() {
  clearWidgetFocus();
  mui->frameNotesSearchResult->hide();
  mui->frameNoteList->show();
  isOpenSearchResult = false;
}

void MainWindow::on_btnClearSearchResults_pressed() {
  mui->editNotesSearch->clear();
  mui->editNotesSearch->setFocus();
}

void MainWindow::on_btnOpenSearchResult_pressed() {
  QString mdFile = m_NotesList->getSearchResultQmlFile();
  if (!QFile::exists(mdFile)) return;
  isOpenSearchResult = true;
  currentMDFile = mdFile;
  mySearchText = mui->editNotesSearch->text().trimmed();
  on_btnEditNote_pressed();
  m_NotesList->setCurrentItemFromMDFile(mdFile);
}

void MainWindow::on_btnFindNotes2_pressed() {
  if (mui->f_FindNotes->isHidden())
    mui->f_FindNotes->show();
  else
    mui->f_FindNotes->hide();
}

void MainWindow::on_btnOpenEditFind_pressed() {
  isOpenSearchResult = true;
  mySearchText = mui->editFindNote->text().trimmed();
  on_btnEditNote_pressed();
}

void MainWindow::on_btnTools_pressed() {
  if (mui->f_Tools->isHidden())
    mui->f_Tools->show();
  else
    mui->f_Tools->hide();
}

void MainWindow::on_btnCopyNoteLink_pressed() {
  QString mdFile = m_NotesList->getSearchResultQmlFile();
  if (!QFile::exists(mdFile)) return;
  QString file = mdFile;
  file = file.replace(iniDir, "");
  QString name = m_Notes->m_NoteIndexManager->getNoteTitle(mdFile);
  QString strlink = "[" + name + "](" + file + ")";
  QClipboard* clipboard = QApplication::clipboard();
  clipboard->setText(strlink);

  auto msg = std::make_unique<ShowMessage>(this);
  msg->showMsg(appName, strlink, 1);
}

void MainWindow::on_btnRotation_pressed() {
  if (mui->lblBookName->text() == "Book Name") return;

  if (mui->qwViewBookNote->isVisible()) return;

  QQuickItem* rootItem = mui->qwReader->rootObject();
  QQuickItem* orientationButton =
      rootItem->findChild<QQuickItem*>("orientationButton");
  if (orientationButton) {
    QMetaObject::invokeMethod(orientationButton, "clicked");
    m_Reader->isLandscape = !m_Reader->isLandscape;
    m_Reader->readReadNote(m_Reader->cPage);
  }
}

void MainWindow::on_btnAddBookNote_pressed() { m_Reader->addBookNote(); }

void MainWindow::on_btnViewBookNote_pressed() { m_Reader->viewBookNote(); }

void MainWindow::on_btnMap_clicked() { m_Steps->openMapWindow(); }

void MainWindow::on_btnSportsChart_clicked() { m_Steps->showSportsChart(); }

void MainWindow::on_btnSpeak_pressed() {
  mui->btnSpeak->hide();
  mui->btnStopSpeak->show();

  isPlayBook = true;
  m_Reader->setAutoStopPlayTime();
  m_Reader->startSpeak();
}

void MainWindow::on_btnStopSpeak_pressed() {
  mui->btnStopSpeak->hide();
  mui->btnSpeak->show();
  m_Reader->stopSpeak();
}

void MainWindow::on_btnSteps_pressed() { m_Steps->openStepsUI(); }

void MainWindow::on_btnNotes_pressed() { m_Notes->openNotes(); }

void MainWindow::on_btnAdd_pressed() {
  // m_EditRecord->monthSum();

  m_MainHelper->on_AddRecord();
}

void MainWindow::on_btnDel_pressed() {
  isMoveEntry = false;
  del_Data((QTreeWidget*)mui->tabWidget->currentWidget());
}

void MainWindow::resizeEvent(QResizeEvent* event) {
  Q_UNUSED(event);
  mui->qwReader->rootContext()->setContextProperty("myW", mw_one->width());
  mui->qwReader->rootContext()->setContextProperty("myH", mw_one->height());
  mui->qwTodo->rootContext()->setContextProperty("isBtnVisible",
                                                 QVariant(false));
  mui->qwSteps->rootContext()->setContextProperty("myW", this->width());

#ifdef Q_OS_ANDROID

#else
  if (!mui->frameTodo->isHidden()) {
    mui->qwTodo->rootContext()->setContextProperty("m_width", mw_one->width());
    m_Todo->init_Todo();
  }

  if (!mui->frameNoteList->isHidden()) {
    m_NotesList->clickNoteBook();
  }
#endif
}

void MainWindow::on_btnBackTodo_pressed() { m_Todo->closeTodo(); }

void MainWindow::on_btnHigh() { m_Todo->on_btnHigh(); }

void MainWindow::on_btnLow() { m_Todo->on_btnLow(); }

void MainWindow::on_btnSetTime() { m_Todo->on_btnSetTime(); }

void MainWindow::on_btnRecycle() { m_Todo->on_btnRecycle(); }

void MainWindow::on_btnReturnRecycle_pressed() {
  m_Todo->on_btnReturn_clicked();
}

void MainWindow::on_btnClearRecycle_pressed() { m_Todo->on_btnClear_clicked(); }

void MainWindow::on_btnDelRecycle_pressed() { m_Todo->on_btnDel_clicked(); }

void MainWindow::on_btnRestoreRecycle_pressed() {
  m_Todo->on_btnRestore_clicked();
}

void MainWindow::on_btnSelText() {
  if (mui->f_ReaderSet->isVisible()) {
    on_btnBackReaderSet_pressed();
  }
  m_Reader->selectText();
}

void MainWindow::on_btnUpload_pressed() {
  if (!mui->btnReader->isEnabled() || !mui->btnWebDAVBackup->isEnabled() ||
      !mui->btnWebDAVRestore->isEnabled())
    return;

  m_CloudBackup->startBakData();
}

void MainWindow::on_btnDownload_pressed() {
  m_CloudBackup->on_pushButton_downloadFile_clicked();
}

void MainWindow::on_btnBack_One_pressed() { m_CloudBackup->backExit(); }

void MainWindow::on_btnBackNotesGraph_pressed() {
  mui->frameNotesGraph->hide();
  mui->frameNoteList->show();
  m_NotesList->clickNoteList();
}

void MainWindow::on_btnCopy_pressed() {
  QClipboard* clipboard = QApplication::clipboard();
  clipboard->setText(mui->editSetText->text().trimmed());
}

void MainWindow::on_btnCancelSel_pressed() {
  m_Reader->resetTextSelection();

  mui->f_ReaderNote->hide();

  mui->qwReader->show();
  mui->f_ReaderFun->show();
  m_Reader->isSelText = false;
}

void MainWindow::on_btnBackImg_pressed() {
  mui->frameImgView->hide();
  if (isReaderVisible) mui->frameReader->show();
  if (isMemoVisible) mui->frameNotesGraph->show();
}

void MainWindow::on_btnZoomIn_pressed() {
  QQuickItem* root = mui->qw_Img->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "zoomin");
}

void MainWindow::on_btnZoomOut_pressed() {
  QQuickItem* root = mui->qw_Img->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "zoomout");
}

void MainWindow::on_btnReport() {
  if (mui->qwReport->source().isEmpty()) {
    int f_size = 19;
    // if (fontSize <= f_size)
    f_size = fontSize;
    mui->qwReport->rootContext()->setContextProperty("maxFontSize", f_size);
    mui->qwReportSub->rootContext()->setContextProperty("maxFontSize", f_size);
    mui->qwReport->rootContext()->setContextProperty("m_Report",
                                                     mw_one->m_Report);
    mui->qwReport->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/report.qml")));
    mui->qwReportSub->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/details.qml")));
  }

  on_actionReport_triggered();
  mui->btnYear->setFixedHeight(mui->btnMonth->height());
}

void MainWindow::on_btnShowPassword_pressed() {
  m_Preferences->on_btnShowPassword_pressed();
}

void MainWindow::on_btnShowPassword_released() {
  m_Preferences->on_btnShowPassword_released();
}

void MainWindow::on_btnShowValidate_pressed() {
  m_Preferences->on_btnShowValidate_pressed();
}

void MainWindow::on_btnShowValidate_released() {
  m_Preferences->on_btnShowValidate_released();
}

void MainWindow::on_btnSendEmail() {
  if (m_Method->getCountFromQW(mui->qwBakList) == 0) return;

  int cur_index = m_Method->getCurrentIndexFromQW(mui->qwBakList);
  QString filePath = m_Method->getText3(mui->qwBakList, cur_index);

  if (m_Method->sendMailWithAttachment("", filePath)) {
    qDebug() << "The default email client of the system has been invoked.";
  } else {
    qDebug() << "The call to the email client failed!";
  }
}

void MainWindow::on_btnShareBakFile_pressed() {
  if (m_Method->getCountFromQW(mui->qwBakList) == 0) return;

  int cur_index = m_Method->getCurrentIndexFromQW(mui->qwBakList);
  QString filePath = m_Method->getText3(mui->qwBakList, cur_index);
  if (QFile::exists(filePath)) {
    mw_one->m_ReceiveShare->shareImage(tr("Share to"), filePath, "*/*");
  }
}

void MainWindow::on_btnNewNote_pressed() {
  m_NotesList->on_actionAdd_Note_triggered();
}

void MainWindow::on_btnShareBookText_pressed() {
  QString txt = mui->editSetText->text().trimmed();
  if (txt.length() > 0) {
    mw_one->m_ReceiveShare->shareString(tr("Share to"), txt);
  }
}

void MainWindow::on_btnBackReader_pressed() { m_Reader->closeReader(); }

void MainWindow::on_btnOpen_pressed() {
  if (mui->qwViewBookNote->isVisible()) return;

  mui->btnAutoStop->click();

  m_Reader->saveReader("", false);
  m_Reader->savePageVPos();

  if (mui->f_ReaderSet->isVisible()) {
    on_btnBackReaderSet_pressed();
  }
  if (mui->qwBookmark->isVisible()) {
    on_btnShowBookmark_pressed();
  }
  m_ReaderSet->close();
  m_Reader->closeSelText();
  m_Reader->on_btnOpen_clicked();
}

void MainWindow::on_btnPageUp_pressed() { m_Reader->goUpPage(); }

void MainWindow::on_btnPageNext_pressed() { m_Reader->goNextPage(); }

void MainWindow::on_btnPages_pressed() {
  if (mui->qwViewBookNote->isVisible()) return;

  mui->btnAutoStop->click();

  if (mui->qwCata->isVisible()) return;

  if (mui->f_ReaderSet->isHidden()) {
    mui->lblTotalReading->setText(tr("Total Reading: ") +
                                  m_Reader->getReadTotalTime() + " h");
    mui->f_ReaderSet->show();

    m_Reader->closeSelText();
    if (mui->qwBookmark->isVisible()) {
      on_btnShowBookmark_pressed();
    }

    QStringList list = mui->btnPages->text().split("\n");
    if (list.count() == 2) {
      QString cur = list.at(0);
      QString total = list.at(1);
      mui->lblProg->setText(tr("Reading Progress") + " : " + cur + " -> " +
                            total);

      mui->hSlider->setMaximum(total.toInt());
      mui->hSlider->setMinimum(1);
      mui->hSlider->setValue(cur.toInt());
    }
  } else
    on_btnBackReaderSet_pressed();
}

void MainWindow::on_btnOpenNote_pressed() { m_Notes->previewNote(); }

void MainWindow::on_btnEditNote_pressed() { m_Notes->openEditUI(); }

void MainWindow::on_btnToPDF_pressed() {
  if (!QFile::exists(currentMDFile)) return;

  m_Notes->on_btnPDF_clicked();
}

void MainWindow::on_btnPause_pressed() {
  if (!isRunPaused) {
    isRunPaused = true;
    mui->btnPause->setIcon(QIcon(":/res/erun.svg"));
    mui->lblGpsInfo->setStyleSheet(m_Steps->lblPausedStyle);
  } else {
    isRunPaused = false;
    mui->btnPause->setIcon(QIcon(":/res/epaused.svg"));
    mui->lblGpsInfo->setStyleSheet(m_Steps->lblStartStyle);
  }
}

void MainWindow::on_btnTestWebDav_pressed() {
  auto msg = std::make_unique<ShowMessage>(this);
  if (!m_CloudBackup->checkWebDAVConnection()) {
    msg->showMsg(appName,
                 tr("WebDAV connection failed. Please check the network, "
                    "website address or login information."),
                 1);
  } else {
    msg->showMsg(appName, tr("WebDav connection successful."), 1);
  }
}

void MainWindow::on_btnReader_pressed() { m_Reader->openReader(); }

void MainWindow::on_btnFind_pressed() {
  mui->frameMain->hide();
  mui->frameSearch->show();
  mui->editSearchText->setFocus();
  mui->btnClearSearchText->setFixedHeight(mui->btnStartSearch->height());
}

void MainWindow::on_btnTodo_pressed() { m_Todo->openTodo(); }

void MainWindow::on_btnHome_pressed() {
  mui->qwMainTab->show();
  mui->qwMainDate->hide();
  mui->qwMainEvent->hide();
  mui->lblStats->hide();
  mui->lblTabTitle->hide();
}

void MainWindow::on_btnReadList_pressed() {
  if (mui->qwViewBookNote->isVisible()) return;

  mui->btnAutoStop->click();

  m_Reader->saveReader("", false);
  m_Reader->savePageVPos();

  if (isAndroid) m_Reader->closeMyPDF();

  if (mui->f_ReaderSet->isVisible()) {
    on_btnBackReaderSet_pressed();
  }

  if (mui->qwBookmark->isVisible()) {
    mw_one->on_btnShowBookmark_pressed();
  }

  m_ReaderSet->close();
  m_Reader->closeSelText();

  if (mui->frameMain->isVisible()) mui->frameMain->hide();
  mui->frameReader->hide();
  mui->frameBookList->show();

  m_Reader->getReadList();
}

void MainWindow::on_btnMenu_pressed() {
  int x = 0;
  int y = 0;

#ifdef Q_OS_ANDROID
  int statusBarHeight = 36;

  x = mw_one->geometry().x() + 2;
  y = geometry().y() + mui->f_Menu->height() + 2 + statusBarHeight;
#else
  x = mw_one->geometry().x() + mui->btnMenu->x() + 2;
  y = geometry().y() + mui->f_Menu->height() + 2;
#endif

  QPoint pos(x, y);
  mainMenu->exec(pos);
}

void MainWindow::on_btnModifyRecord() { m_Method->reeditMainEventData(); }

void MainWindow::on_btnSelTab_pressed() {
  if (mui->qwSelTab->source().isEmpty()) {
    mui->qwSelTab->rootContext()->setContextProperty("mw_one", mw_one);
    mui->qwSelTab->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/seltab.qml")));
  }

  mui->frameMain->hide();
  mui->frameSetTab->show();
  getMainTabs();
}

void MainWindow::on_btnSearch_pressed() {
  QString str = mui->editSetText->text().trimmed();
  if (str == "") return;

  QString strurl;
  strurl = "https://bing.com/search?q=" + str;

  QUrl url(strurl);
  QDesktopServices::openUrl(url);
}

void MainWindow::on_btnShowCboxList_clicked() { mui->cboxWebDAV->showPopup(); }

void MainWindow::onAndroidBackHandle() {
  if (textToolbarDynamic != nullptr && textToolbarDynamic->isVisible()) {
    closeTextToolBar();
    return;
  }

  if (textToolbar != nullptr && textToolbar->isVisible()) {
    closeTextToolBar();
    return;
  }

  if (colorDlg != nullptr) {
    if (colorDlg->isVisible()) {
      colorDlg->close();
      return;
    }
  }

  if (mainMenu != nullptr) {
    if (mainMenu->isVisible()) {
      mainMenu->close();
      return;
    }
  }

  if (m_NotesList->menuNoteBook) {
    if (m_NotesList->menuNoteBook->isVisible()) {
      m_NotesList->menuNoteBook->close();
      return;
    }
  }

  if (m_NotesList->menuNoteList) {
    if (m_NotesList->menuNoteList->isVisible()) {
      m_NotesList->menuNoteList->close();
      return;
    }
  }

  if (m_NotesList->menuRecentOpen) {
    if (m_NotesList->menuRecentOpen->isVisible()) {
      m_NotesList->menuRecentOpen->close();
      return;
    }
  }

  if (m_MsgBox != nullptr) {
    if (m_MsgBox->isVisible()) {
      m_MsgBox->close();
      return;
    }
  }

  if (m_Method->m_EnColorPicker != nullptr) {
    if (m_Method->m_EnColorPicker->isVisible()) {
      m_Method->m_EnColorPicker->close();
      return;
    }
  }

  if (m_PrintPDF != nullptr) {
    if (m_PrintPDF->isVisible()) {
      m_PrintPDF->close();
      return;
    }
  }

  if (mw_one->mainMenu != nullptr) {
    if (mw_one->mainMenu->isVisible()) {
      mw_one->mainMenu->close();
      return;
    }
  }

  if (mw_one->m_Report->m_Menu != nullptr) {
    if (mw_one->m_Report->m_Menu->isVisible()) {
      mw_one->m_Report->m_Menu->close();
      return;
    }
  }

  if (m_NotesList->menuNoteBook != nullptr) {
    if (m_NotesList->menuNoteBook->isVisible()) {
      m_NotesList->menuNoteBook->close();
      return;
    }
  }

  if (m_NotesList->menuNoteList != nullptr) {
    if (m_NotesList->menuNoteList->isVisible()) {
      m_NotesList->menuNoteList->close();
      return;
    }
  }

  if (m_NotesList->menuRecentOpen != nullptr) {
    if (m_NotesList->menuRecentOpen->isVisible()) {
      m_NotesList->menuRecentOpen->close();
      return;
    }
  }

  if (m_NotesList->m_MoveTo != nullptr) {
    if (m_NotesList->m_MoveTo->isVisible()) {
      m_NotesList->m_MoveTo->ui->btnCancel->click();
      return;
    }
  }

  if (m_NotesList->m_NewNoteBook != nullptr) {
    if (m_NotesList->m_NewNoteBook->isVisible()) {
      m_NotesList->m_NewNoteBook->ui->btnCancel->click();
      return;
    }
  }

  if (mw_one->m_RenameDlg != nullptr) {
    if (mw_one->m_RenameDlg->isVisible()) {
      mw_one->m_RenameDlg->close();
      return;
    }
  }

  if (mw_one->m_Todo->m_ReeditTodo != nullptr) {
    if (mw_one->m_Todo->m_ReeditTodo->isVisible()) {
      mw_one->m_Todo->m_ReeditTodo->close();
      return;
    }
  }

  if (m_NotesList->m_RenameNotes != nullptr) {
    if (m_NotesList->m_RenameNotes->isVisible()) {
      if (textToolbarDynamic != nullptr && textToolbarDynamic->isVisible()) {
        closeTextToolBar();
        return;
      }

      m_NotesList->m_RenameNotes->close();
      m_Method->closeGrayWindows();
      return;
    }
  }

  if (mw_one->m_Preferences->isVisible()) {
    mw_one->m_Preferences->ui->btnBack->click();
    return;
  }

  if (mw_one->m_AboutThis->isVisible()) {
    mw_one->m_AboutThis->ui->btnBack_About->click();
    return;
  }

  if (m_StepsOptions->isVisible()) {
    m_StepsOptions->ui->btnBack->click();
    return;
  }

  // Reader
  if (m_Reader->dlgAddBookNote != nullptr) {
    if (m_Reader->dlgAddBookNote->isVisible()) {
      m_Reader->dlgAddBookNote->close();
      return;
    }
  }

  if (m_Reader->dlgEditBookNote != nullptr) {
    if (m_Reader->dlgEditBookNote->isVisible()) {
      m_Reader->dlgEditBookNote->close();
      return;
    }
  }

  if (mui->qwViewBookNote->isVisible()) {
    QTimer::singleShot(100, mw_one, []() { m_Reader->closeViewBookNote(); });

    return;
  }

  if (mui->f_ReaderNote->isVisible()) {
    mw_one->on_btnCancelSel_pressed();
    return;
  }

  if (mui->f_ReaderSet->isVisible()) {
    mw_one->on_btnBackReaderSet_pressed();
    return;
  }

  if (mui->qwCata->isVisible()) {
    mw_one->on_btnCatalogue_pressed();
    return;
  }

  if (mui->qwBookmark->isVisible()) {
    mw_one->on_btnShowBookmark_pressed();
    return;
  }

  if (!mui->frameReader->isHidden()) {
    on_btnBackReader_pressed();
    return;
  }

  if (!mui->frameImgView->isHidden()) {
    on_btnBackImg_pressed();
    return;
  }

  if (!mui->qwMainChart->isHidden()) {
    QTimer::singleShot(100, mw_one, []() { mw_one->on_btnChart(); });
    return;
  }

  if (!mui->frameOne->isHidden()) {
    on_btnBack_One_pressed();
    return;
  }

  if (!mui->frameNoteRecycle->isHidden()) {
    on_btnBackNoteRecycle_clicked();
    return;
  }

  if (!mui->frameNotesSearchResult->isHidden()) {
    on_btnBack_NotesSearchResult_pressed();
    return;
  }

  if (mui->f_FindNotes->isVisible()) {
    mui->f_FindNotes->hide();
    return;
  }

  if (!mui->frameNoteList->isHidden()) {
    QTimer::singleShot(100, mw_one,
                       []() { mw_one->on_btnBackNoteList_pressed(); });
    return;
  }

  if (!mui->frameDiff->isHidden()) {
    on_btnBackNoteDiff_pressed();
    return;
  }

  if (!mui->frameNotesGraph->isHidden()) {
    on_btnBackNotesGraph_pressed();
    return;
  }

  if (mw_one->m_TodoAlarm->isVisible()) {
    mw_one->m_TodoAlarm->ui->btnBack->click();
    return;
  }

  if (mw_one->m_Todo->isTodoAlarmShow) {
    QTimer::singleShot(100, mw_one, []() { mw_one->m_Todo->closeTodoAlarm(); });
    return;
  }

  if (!mui->frameTodoRecycle->isHidden()) {
    on_btnReturnRecycle_pressed();
    return;
  }

  if (!mui->frameTodo->isHidden()) {
    on_btnBackTodo_pressed();
    return;
  }

  if (!mui->frameTabRecycle->isHidden()) {
    on_btnBackTabRecycle_pressed();
    return;
  }

  if (m_Steps->isRouteShow()) {
    m_Steps->closeRouteDialog();
    return;
  }

  if (m_Steps->m_remarksDialog != nullptr) {
    if (m_Steps->m_remarksDialog->isVisible()) {
      m_Steps->m_remarksDialog->close();
      return;
    }
  }

  if (m_Steps->statsDialog != nullptr) {
    m_Steps->statsDialog->close();

    return;
  }

  if (!mui->frameSteps->isHidden()) {
    on_btnBackSteps_clicked();
    return;
  }

  if (!mui->frameViewCate->isHidden()) {
    QTimer::singleShot(100, mw_one, []() {
      mui->frameViewCate->hide();
      mui->frameReport->show();
    });

    return;
  }

  if (!mui->frameReport->isHidden()) {
    on_btnBack_Report_pressed();
    return;
  }

  if (!mui->frameSearch->isHidden()) {
    on_btnBackSearch_pressed();
    return;
  }

  if (!mui->frameBakList->isHidden()) {
    on_btnBackBakList_pressed();
    return;
  }

  if (!mui->frameCategory->isHidden()) {
    on_btnCancelType_pressed();
    return;
  }

  if (!mui->frameSetTab->isHidden()) {
    on_btnBackSetTab_pressed();
    return;
  }

  if (!mui->frameEditRecord->isHidden()) {
    on_btnBackEditRecord_pressed();

    return;
  }

  if (!mui->frameBookList->isHidden()) {
    on_btnBackBookList_pressed();
    return;
  }

  if (!mui->frameNotesTree->isHidden()) {
    mui->btnBack_Tree->click();

    return;
  }

  if (!mui->qwMainDate->isHidden()) {
    mui->btnHome->pressed();

    // QQuickItem* root = mui->qwMainTab->rootObject();
    // if (root) {
    //   QMetaObject::invokeMethod(root, "forceActivateUI",
    //   Qt::QueuedConnection);
    // }

    return;
  }

  if (!mui->frameMain->isHidden()) {
    mw_one->setMini();

    return;
  }
}

void MainWindow::setToolButtonAnimation(QToolButton* btn, bool setMyStyle) {
  if (setMyStyle) {
    // 根据明暗主题区分tooltip样式
    QString tipStyle;
    if (!isAndroid) {
      if (isDark) {
        tipStyle = R"(
        QToolTip {
            background-color: #2b2b2b;
            color: #f0f0f0;
            border: 1px solid #555555;
            padding: 4px;
        }
        )";
      } else {
        tipStyle = R"(
        QToolTip {
            background-color: white;
            color: black;
            border: 1px solid #aaa;
            padding: 4px;
        }
        )";
      }
    }

    // 拼接按钮基础样式 + 动态tooltip样式，存入变量sheet
    QString sheet = QString(R"(
        QToolButton {
            border: none;
            background: transparent;
        }
        %1
    )")
                        .arg(tipStyle);

    // 把拼接好的完整样式赋值给按钮
    btn->setStyleSheet(sheet);
  }

  if (btn->property("__anim_installed").toBool()) return;
  btn->setProperty("__anim_installed", true);

  QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(btn);
  effect->setOpacity(1);
  btn->setGraphicsEffect(effect);

  class ButtonAnimFilter : public QObject {
   public:
    explicit ButtonAnimFilter(QObject* parent) : QObject(parent) {}

    bool eventFilter(QObject* obj, QEvent* event) override {
      QToolButton* btn = qobject_cast<QToolButton*>(obj);
      if (!btn) return false;

      if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton) {
          // 按下变暗
          QPropertyAnimation* anim =
              new QPropertyAnimation(btn->graphicsEffect(), "opacity", btn);
          anim->setDuration(80);
          anim->setStartValue(1);
          anim->setEndValue(0.65);

          connect(anim, &QPropertyAnimation::finished, btn, [=]() {
            QPropertyAnimation* back =
                new QPropertyAnimation(btn->graphicsEffect(), "opacity", btn);
            back->setDuration(80);
            back->setStartValue(0.65);
            back->setEndValue(1);
            back->start(QAbstractAnimation::DeleteWhenStopped);
            emit btn->pressed();
          });

          anim->start(QAbstractAnimation::DeleteWhenStopped);
          return true;
        }
      }
      return false;
    }
  };

  ButtonAnimFilter* filter = new ButtonAnimFilter(btn);
  btn->installEventFilter(filter);
}

// 备用：图标放大、缩小版
/*void MainWindow::setToolButtonAnimation(QToolButton* btn, bool setMyStyle) {
  if (setMyStyle) {
    // 根据明暗主题区分tooltip样式
    QString tipStyle;
    if (isDark) {
      tipStyle = R"(
        QToolTip {
            background-color: #2b2b2b;
            color: #f0f0f0;
            border: 1px solid #555555;
            padding: 4px;
        }
        )";
    } else {
      tipStyle = R"(
        QToolTip {
            background-color: white;
            color: black;
            border: 1px solid #aaa;
            padding: 4px;
        }
        )";
    }

    // 拼接按钮基础样式 + 动态tooltip样式，存入变量sheet
    QString sheet = QString(R"(
        QToolButton {
            border: none;
            background: transparent;
        }
        %1
    )")
                        .arg(tipStyle);

    // 把拼接好的完整样式赋值给按钮
    btn->setStyleSheet(sheet);
  }

  QSize originalSize = btn->iconSize();

  // 防止重复安装
  if (btn->property("__anim_installed").toBool()) {
    return;
  }
  btn->setProperty("__anim_installed", true);

  // ================================
  // 核心：给按钮安装事件过滤器
  // ================================
  class ButtonAnimFilter : public QObject {
   public:
    QSize originalSize;
    explicit ButtonAnimFilter(QObject* parent = nullptr) : QObject(parent) {}

    bool eventFilter(QObject* obj, QEvent* event) override {
      QToolButton* btn = qobject_cast<QToolButton*>(obj);
      if (!btn) return false;

      // 拦截 鼠标左键按下
      if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton) {
          // 1. 立即播放缩小动画（必现反馈）
          btn->setIconSize(originalSize);
          QPropertyAnimation* anim =
              new QPropertyAnimation(btn, "iconSize", btn);
          anim->setDuration(100);
          anim->setStartValue(originalSize);
          anim->setEndValue(originalSize * 0.9);

          // 2. 动画结束 → 恢复大小 → 手动触发 pressed 业务
          connect(anim, &QPropertyAnimation::finished, btn, [=]() {
            btn->setIconSize(originalSize);
            // 关键：动画完成后才触发业务
            emit btn->pressed();
          });

          anim->start(QAbstractAnimation::DeleteWhenStopped);

          // 拦截原生事件，不让按钮自己触发 pressed
          return true;
        }
      }

      return false;
    }
  };

  // 创建过滤器并绑定到按钮
  ButtonAnimFilter* filter = new ButtonAnimFilter(btn);
  filter->originalSize = originalSize;
  btn->installEventFilter(filter);

  // ================================
  // 安卓兜底：防止按钮卡住不回弹
  // ================================
  connect(btn, &QToolButton::released, this,
          [=]() { btn->setIconSize(originalSize); });
}*/
