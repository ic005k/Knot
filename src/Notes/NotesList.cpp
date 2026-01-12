#include "NotesList.h"

#include "src/MainWindow.h"
#include "src/defines.h"
#include "ui_MainWindow.h"
#include "ui_NewNoteBook.h"
#include "ui_NotesList.h"

QString strNoteNameIndexFile = "";

NotesList::NotesList(QWidget* parent) : QDialog(parent), ui(new Ui::NotesList) {
  ui->setupUi(this);
  this->installEventFilter(this);

  strNoteNameIndexFile = privateDir + "MyNoteNameIndex";

  connect(pAndroidKeyboard, &QInputMethod::visibleChanged, this,
          &NotesList::on_KVChanged);

  tw = ui->treeWidget;
  twrb = ui->treeWidgetRecycle;

  noteModel = new NoteListModel(this);

  mui->f_FindNotes->setStyleSheet(
      "QFrame{background-color: #455364;color: #FFFFFF;border-radius:10px; "
      "border:0px solid gray;}");
  mui->f_Tools->setStyleSheet(mui->f_FindNotes->styleSheet());
  mui->qwNoteTools->setFixedHeight(60);

  mui->btnBackNoteList->hide();
  mui->btnEditNote->hide();
  mui->btnOpenNote->hide();
  mui->btnShowFindNotes->hide();
  mui->btnTools->hide();

  if (isAndroid) {
    QFont font = this->font();
    font.setPointSize(13);
    mui->lblNoteBook->setFont(font);
    mui->lblNoteList->setFont(font);
  }

  setModal(true);
  this->layout()->setSpacing(5);
  this->layout()->setContentsMargins(2, 2, 2, 2);
  ui->frame1->hide();

  tw->headerItem()->setText(0, tr("Notebook"));
  tw->setColumnHidden(1, false);
  tw->setColumnWidth(0, 300);
  tw->setColumnWidth(1, 300);
  twrb->header()->hide();
  twrb->setColumnHidden(1, true);
  twrb->setColumnWidth(0, 180);

  set_memo_dir();

  ui->btnImport->hide();
  ui->btnExport->hide();

  initNotesList();
  initRecycle();

  // 连接搜索框
  connect(mui->editNotesSearch, &QLineEdit::textChanged, this,
          &NotesList::onSearchTextChanged);
  mui->qwNotesSearchResult->rootContext()->setContextProperty("searchModel",
                                                              &m_searchModel);
  initSerachDatabase();

  loadNotesListIndex();

  // 初始化笔记关系图谱功能
  initNoteGraphView();  // 关键：注册控制器到QML引擎
}

NotesList::~NotesList() { delete ui; }

void NotesList::initSerachDatabase() {
  QString databaseFile = privateDir + "md_database_v3.db";
  m_dbManager.initDatabase(databaseFile);
  QFile m_dfile(databaseFile);
  if (m_dfile.size() < 100000) {
    startBackgroundTaskUpdateFilesIndex();
  }
}

void NotesList::startBackgroundTaskUpdateFilesIndex() {
  QString fullPath = iniDir + "memo";  // 先构造完整路径

  QFuture<void> future = QtConcurrent::run([=]() {
    m_dbManager.updateFilesIndex(fullPath);  // 值捕获保证线程安全
  });

  // 可选：使用 QFutureWatcher 监控进度
  QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
  connect(watcher, &QFutureWatcher<void>::finished, this, [=]() {
    qDebug() << "Database update completed.";
    mw_one->closeProgress();

    QStringList cycleFiles = getRecycleNoteFiles();
    if (cycleFiles.count() > 0) startBackgroundTaskDelFilesIndex(cycleFiles);

    watcher->deleteLater();
  });
  watcher->setFuture(future);
}

void NotesList::startBackgroundTaskDelFilesIndex(const QStringList& files) {
  QStringList m_files = files;

  QFuture<void> future = QtConcurrent::run([=]() {
    for (int i = 0; i < m_files.count(); i++) {
      QString filePath = m_files.at(i);
      m_dbManager.deleteFileIndex(filePath);
    }
  });

  QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
  connect(watcher, &QFutureWatcher<void>::finished, this, [=]() {
    qDebug() << "Database del files completed.";
    watcher->deleteLater();
  });
  watcher->setFuture(future);
}

void NotesList::set_memo_dir() {
  QString path = iniDir + "memo/";
  QDir dir(path);
  if (!dir.exists()) dir.mkdir(path);
}

bool NotesList::eventFilter(QObject* watch, QEvent* evn) {
  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
      if (!ui->frame1->isHidden()) {
        on_btnBack_clicked();
        return true;
      }

      if (!ui->frame0->isHidden()) {
        on_btnClose_clicked();
        return true;
      }
    }

    if (keyEvent->key() == Qt::Key_Return) {
      QTreeWidgetItem* item = tw->currentItem();
      on_treeWidget_itemClicked(item, 0);
      return true;
    }
  }

  return QWidget::eventFilter(watch, evn);
}

void NotesList::on_btnClose_clicked() { this->close(); }

void NotesList::on_btnNewNoteBook_clicked() {
  QTreeWidgetItem* item = new QTreeWidgetItem();
  item->setText(0, notebookName);
  item->setText(2, "#FF0000");
  item->setForeground(0, Qt::red);
  item->setIcon(0, QIcon(":/res/nb.png"));

  if (rootIndex == 0) {
    ui->treeWidget->addTopLevelItem(item);
    ui->treeWidget->setCurrentItem(item);
  } else {
    QTreeWidgetItem* topItem = tw->topLevelItem(rootIndex - 1);
    tw->setCurrentItem(topItem);
    topItem->addChild(item);
    tw->setCurrentItem(item);
  }
}

void NotesList::on_btnNewNote_clicked() {
  if (ui->treeWidget->topLevelItemCount() == 0) return;

  int rand = QRandomGenerator::global()->generate();
  if (rand < 0) rand = 0 - rand;

  QString noteFile =
      "memo/" + m_Notes->getDateTimeStr() + "_" + QString::number(rand) + ".md";
  QTreeWidgetItem* parentitem = ui->treeWidget->currentItem();

  QTreeWidgetItem* item1 = new QTreeWidgetItem(parentitem);
  item1->setText(0, "");
  item1->setText(1, noteFile);
  item1->setIcon(0, QIcon(":/res/n.png"));

  QTextEdit* edit = new QTextEdit();
  edit->append("");
  TextEditToFile(edit, iniDir + noteFile);
  delete edit;

  ui->treeWidget->setCurrentItem(item1);
  on_treeWidget_itemClicked(item1, 0);

  pNoteItems.clear();
  int count = parentitem->childCount();
  for (int i = 0; i < count; i++) {
    pNoteItems.append(parentitem->child(i));
  }
}

void NotesList::on_treeWidget_itemClicked(QTreeWidgetItem* item, int column) {
  Q_UNUSED(column);

  if (ui->treeWidget->topLevelItemCount() == 0) return;

  noteName = item->text(0);

  return;

  if (item->parent() != NULL) {
    if (tw->currentIndex().row() == 0) {
      if (tw->currentIndex().parent().row() == 0) {
      }
    }

    QSettings Reg(iniDir + "curmd.ini", QSettings::IniFormat);

    QString curmd = item->text(1);
    Reg.setValue("/MainNotes/currentItem", curmd);

    currentMDFile = iniDir + curmd;

    setNoteName(item->text(0));
  }
}

QString NotesList::getCurrentMDFile() {
  QSettings Reg(iniDir + "curmd.ini", QSettings::IniFormat);

  QString curmd = Reg.value("/MainNotes/currentItem", "memo/xxx.md").toString();
  QString title = Reg.value("/MainNotes/NoteName", tr("Note Name")).toString();

  noteTitle = title;

  return iniDir + curmd;
}

void NotesList::on_btnRename_clicked() {
  QTreeWidgetItem* item = ui->treeWidget->currentItem();
  if (item == NULL) return;

  m_RenameNotes = new QDialog(this);
  QVBoxLayout* vbox0 = new QVBoxLayout;
  m_RenameNotes->setLayout(vbox0);
  vbox0->setContentsMargins(5, 5, 5, 5);
  if (!isAndroid) m_RenameNotes->setModal(true);
  m_RenameNotes->setWindowFlag(Qt::FramelessWindowHint);

  QFrame* frame = new QFrame(this);
  vbox0->addWidget(frame);

  QVBoxLayout* vbox = new QVBoxLayout;

  frame->setLayout(vbox);
  vbox->setContentsMargins(25, 6, 25, 10);
  vbox->setSpacing(10);

  QLabel* lblTitle = new QLabel(this);
  lblTitle->adjustSize();
  lblTitle->setWordWrap(true);
  lblTitle->setText(tr("Rename"));
  vbox->addWidget(lblTitle);

  QFrame* hframe = new QFrame(this);
  hframe->setFrameShape(QFrame::HLine);
  hframe->setStyleSheet("QFrame{background:red;min-height:2px}");
  vbox->addWidget(hframe);
  hframe->hide();

  QTextEdit* edit = new QTextEdit(this);
  edit->setAcceptRichText(false);

  initTextToolbarDynamic(m_RenameNotes);
  EditEventFilter* editFilter =
      new EditEventFilter(textToolbarDynamic, m_RenameNotes);
  editFilter->setParent(m_RenameNotes);
  edit->installEventFilter(editFilter);
  edit->viewport()->installEventFilter(editFilter);

  vbox->addWidget(edit);
  edit->setPlainText(item->text(0));
  QScroller::grabGesture(edit, QScroller::LeftMouseButtonGesture);
  edit->horizontalScrollBar()->setHidden(true);
  edit->verticalScrollBar()->setStyleSheet(
      mui->editDetails->verticalScrollBar()->styleSheet());

  if (edit->toPlainText().trimmed() == "无标题笔记" ||
      edit->toPlainText().trimmed() == "Untitled Note") {
    edit->setPlainText(m_Notes->new_title);
  }

  QToolButton* btnCancel = new QToolButton(m_RenameNotes);
  QToolButton* btnPaste = new QToolButton(m_RenameNotes);
  QToolButton* btnCopy = new QToolButton(m_RenameNotes);
  QToolButton* btnOk = new QToolButton(m_RenameNotes);
  btnCancel->setText(tr("Cancel"));
  btnPaste->setText(tr("Paste"));
  btnCopy->setText(tr("Copy"));
  btnOk->setText(tr("OK"));

  btnOk->setFixedHeight(35);
  btnCancel->setFixedHeight(35);
  btnCopy->setFixedHeight(35);
  btnPaste->setFixedHeight(35);

  QHBoxLayout* hbox = new QHBoxLayout;
  hbox->addWidget(btnCancel);
  hbox->addWidget(btnPaste);
  hbox->addWidget(btnCopy);
  hbox->addWidget(btnOk);

  btnPaste->hide();
  btnCopy->hide();

  btnCancel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  btnPaste->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  btnCopy->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  btnOk->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

  QSpacerItem* sparcer_item =
      new QSpacerItem(0, 60, QSizePolicy::Fixed, QSizePolicy::Expanding);
  vbox->addItem(sparcer_item);

  vbox->addLayout(hbox, 0);

  connect(btnCancel, &QToolButton::clicked, m_RenameNotes,
          [=]() mutable { m_RenameNotes->close(); });
  connect(m_RenameNotes, &QDialog::rejected, m_RenameNotes,
          [=]() mutable { m_Method->closeGrayWindows(); });
  connect(m_RenameNotes, &QDialog::accepted, m_RenameNotes,
          [=]() mutable { m_Method->closeGrayWindows(); });
  connect(btnPaste, &QToolButton::clicked, m_RenameNotes,
          [=]() mutable { edit->paste(); });
  connect(btnCopy, &QToolButton::clicked, m_RenameNotes, [=]() mutable {
    edit->selectAll();
    edit->copy();
    m_RenameNotes->close();
  });
  connect(btnOk, &QToolButton::clicked, m_RenameNotes, [=]() mutable {
    renameCurrentItem(edit->toPlainText().trimmed());

    saveNotesList();
    m_RenameNotes->close();
  });

  connect(m_RenameNotes, &QDialog::finished, this, [this](int result) {
    Q_UNUSED(result);
    closeTextToolBar();
    m_Method->closeGrayWindows();
  });

  int x, y, w, h;
  h = 300;
  if (isAndroid) {
    w = mw_one->width() - 2;
    y = mw_one->geometry().y();
  } else {
    w = 320;
    y = mw_one->geometry().y() + (mw_one->height() - h) / 2;
  }

  x = mw_one->geometry().x() + (mw_one->width() - w) / 2;
  m_RenameNotes->setGeometry(x, y, w, h);

  m_Method->set_ToolButtonStyle(m_RenameNotes);

  m_Method->showGrayWindows();
  m_RenameNotes->show();
}

void NotesList::renameCurrentItem(QString title) {
  QTreeWidgetItem* item = ui->treeWidget->currentItem();
  if (item == NULL) return;

  item->setText(0, title.trimmed());
  if (item->parent() != NULL && !item->text(1).isEmpty()) {
    setNoteName(item->text(0));

    m_Notes->m_NoteIndexManager->setNoteTitle(iniDir + item->text(1),
                                              item->text(0));
    m_Notes->m_NoteIndexManager->saveIndex(strNoteNameIndexFile);

    for (int i = 0; i < listRecentOpen.count(); i++) {
      QString str = listRecentOpen.at(i);
      if (str.split("===").at(1) == item->text(1)) {
        QString newStr = item->text(0) + "===" + item->text(1);
        listRecentOpen.removeAt(i);
        listRecentOpen.insert(i, newStr);
        saveRecentOpen();
        break;
      }
    }
  }
  resetQML_List();
}

void NotesList::setNoteName(QString name) { noteTitle = name; }

void NotesList::on_btnDel_clicked() {
  if (tw->topLevelItemCount() == 0) return;

  QTreeWidgetItem* item = tw->currentItem();

  if (item == NULL) return;

  QString strFlag;
  if (item->text(1).isEmpty())
    strFlag = tr("NoteBook");
  else
    strFlag = tr("Note");

  auto m_ShowMsg = std::make_unique<ShowMessage>(this);
  if (!m_ShowMsg->showMsg("Knot",
                          tr("Move to the recycle bin?") + "\n\n" + strFlag +
                              " : " + item->text(0),
                          2)) {
    return;
  }

  QStringList delFilesIndex;
  QString str0, str1;
  // Top Item
  if (item->parent() == NULL) {
    int count = item->childCount();
    int index = tw->currentIndex().row();

    for (int i = 0; i < count; i++) {
      QTreeWidgetItem* childItem = new QTreeWidgetItem;

      str0 = item->child(i)->text(0);
      str1 = item->child(i)->text(1);

      delFilesIndex.append(iniDir + str1);

      // Child Notes
      if (!str1.isEmpty()) {
        childItem->setText(0, str0);
        childItem->setText(1, str1);
        addItem(twrb, childItem);

        // Child NoteBook
      } else {
        auto* child_Item = item->child(i);
        int count1 = child_Item->childCount();
        delete childItem;
        childItem = nullptr;

        for (int j = 0; j < count1; j++) {
          str0 = item->child(i)->child(j)->text(0);
          str1 = item->child(i)->child(j)->text(1);
          QTreeWidgetItem* childItem = new QTreeWidgetItem;
          childItem->setText(0, str0);
          childItem->setText(1, str1);
          addItem(twrb, childItem);
        }
      }
    }

    tw->takeTopLevelItem(index);

    // Child Item
  } else {
    // Notes
    if (!item->text(1).isEmpty()) {
      str0 = item->text(0);
      str1 = item->text(1);
      QTreeWidgetItem* childItem = new QTreeWidgetItem;
      childItem->setText(0, str0);
      childItem->setText(1, str1);
      addItem(twrb, childItem);

      delFilesIndex.append(iniDir + str1);

      // Child NoteBook
    } else {
      int count = item->childCount();
      for (int n = 0; n < count; n++) {
        str0 = item->child(n)->text(0);
        str1 = item->child(n)->text(1);
        QTreeWidgetItem* childItem = new QTreeWidgetItem;
        childItem->setText(0, str0);
        childItem->setText(1, str1);
        addItem(twrb, childItem);
      }
    }

    item->parent()->removeChild(item);
  }

  if (tw->topLevelItemCount() > 0) {
    if (tw->currentItem()->childCount() == 0) {
      m_Notes->loadEmptyNote();
    }
  } else {
    m_Notes->loadEmptyNote();
    mui->lblNoteBook->setText(tr("Note Book"));
    mui->lblNoteList->setText(tr("Note List"));
  }

  startBackgroundTaskDelFilesIndex(delFilesIndex);

  tw->setFocus();

  saveNotesList();

  resetQML_List();
}

void NotesList::addItem(QTreeWidget* tw, QTreeWidgetItem* item) {
  item->setIcon(0, QIcon(":/res/n.png"));
  tw->setFocus();
  if (tw == twrb) {
    tw->setCurrentItem(tw->topLevelItem(0));
    QTreeWidgetItem* curItem = tw->currentItem();
    curItem->addChild(item);
  } else {
    QTreeWidgetItem* curItem = tw->currentItem();
    curItem->addChild(item);
  }

  tw->expandAll();
}

bool NotesList::delFile(QString file) {
  bool isOk = false;
  QFile _file(file);
  if (_file.exists()) {
    _file.remove();
    isOk = true;
  } else {
    isOk = false;
  }

  _file.close();
  return isOk;
}

int NotesList::on_btnImport_clicked() {
  if (ui->treeWidget->topLevelItemCount() == 0) return 0;

  QStringList fileNames;
  fileNames =
      QFileDialog::getOpenFileNames(this, tr("Knot"), "", tr("MD File (*.*)"));

  qDebug() << "Import Files:" << fileNames;

  if (fileNames.count() == 0) {
    isImportFilesEnd = true;
    return 0;
  }

  QStringList MDFileList;

  QTreeWidgetItem* item = ui->treeWidget->currentItem();

  for (int i = 0; i < fileNames.count(); i++) {
    QString fileName = fileNames.at(i);

    bool isMD = false;
    QString strFile = fileName;
    strFile = strFile.toLower();
    isMD = strFile.contains(".md") || strFile.contains(".txt");

    if (!isMD) {
      qDebug() << tr("Invalid Markdown file.") + "\n\n" + fileName;

    } else {
      MDFileList.append(fileName);
    }
  }

  mw_one->showProgress();
  isImportFilesEnd = false;

  if (MDFileList.count() > 1000) {
    MDFileList.resize(10);
    auto msg = std::make_unique<ShowMessage>(this);
    msg->showMsg(appName,
                 tr("A maximum of 10 files can be imported at a time."), 1);
  }

  QFuture<void> future = QtConcurrent::run([this, MDFileList, item]() {
    for (int i = 0; i < MDFileList.count(); i++) {
      QString fileName = MDFileList.at(i);

      if (QFile(fileName).exists()) {
        QTreeWidgetItem* item1;

        QFileInfo fi(fileName);
        QString name = fi.baseName();

        item1 = new QTreeWidgetItem(item);
        item1->setText(0, name);

        QString a = "memo/" + m_Notes->getDateTimeStr() + "_" +
                    QString::number(i) + ".md";
        currentMDFile = iniDir + a;

        // QString strNoteText = loadText(fileName);
        // StringToFile(strNoteText, currentMDFile);

        QFile::copy(fileName, currentMDFile);

        item1->setText(1, a);

        qDebug() << fileName << a;

        m_Notes->updateMDFileToSyncLists();

      } else {
        isImportFilesEnd = true;
      }
    }
  });

  // 可选：使用 QFutureWatcher 监控进度
  QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
  connect(watcher, &QFutureWatcher<void>::finished, this,
          [this, watcher, MDFileList]() {
            qDebug() << "Import note completed:" +
                            QString::number(MDFileList.count());

            m_Notes->startBackgroundTaskUpdateNoteIndexes(MDFileList);

            isImportNotes = true;
            isImportFilesEnd = true;
            watcher->deleteLater();
          });
  watcher->setFuture(future);

  return MDFileList.count();
}

void NotesList::on_btnExport_clicked() {
  if (ui->treeWidget->topLevelItemCount() == 0) return;

  QTreeWidgetItem* item = tw->currentItem();
  if (item->parent() == NULL) return;

  QString name = item->text(0);
  name = name + ".md";
  QString fileName;
  QFileDialog fd;
  fileName = fd.getSaveFileName(this, name, name, tr("MD File(*.*)"));

  if (fileName == "") return;

  QString mdfile = iniDir + item->text(1);

  QString str = loadText(mdfile);
  QTextEdit* edit = new QTextEdit();
  edit->setAcceptRichText(false);
  edit->setPlainText(str);

  TextEditToFile(edit, fileName);
}

void NotesList::closeEvent(QCloseEvent* event) { Q_UNUSED(event); }

void NotesList::resetQML_List() {
  if (tw->topLevelItemCount() == 0) {
    m_Method->clearAllBakList(mui->qwNoteBook);
    m_Method->clearAllBakList(mui->qwNoteList);
    return;
  }

  loadAllNoteBook();

  int index = 0;
  QTreeWidgetItem* item = tw->currentItem();
  QTreeWidgetItem* pNoteBook = NULL;
  QTreeWidgetItem* pNote = NULL;
  if (item == NULL)
    index = 0;
  else {
    // NoteBook
    if (item->text(1).isEmpty()) {
      if (item->parent() == NULL) {
        index = tw->indexOfTopLevelItem(item);
        pNoteBook = item;
      } else {
        index = tw->indexOfTopLevelItem(item->parent());
        pNoteBook = item;
      }
    }

    // Notes
    if (!item->text(1).isEmpty()) {
      if (item->parent()->parent() == NULL) {
        index = tw->indexOfTopLevelItem(item->parent());
        pNoteBook = item->parent();
        pNote = item;
      }

      else {
        if (item->parent()->parent()->parent() == NULL) {
          index = tw->indexOfTopLevelItem(item->parent()->parent());
          pNoteBook = item->parent();
          pNote = item;
        }
      }
    }
  }

  int count = m_Method->getCountFromQW(mui->qwNoteBook);

  for (int i = 0; i < count; i++) {
    if (pNoteBookItems.at(i) == pNoteBook) {
      index = i;
      break;
    }
  }

  setNoteBookCurrentIndex(index);
  clickNoteBook();

  if (pNote == NULL) {
    setNotesListCurrentIndex(-1);
  } else {
    int countNotes = pNoteItems.count();
    for (int i = 0; i < countNotes; i++) {
      if (pNote == pNoteItems.at(i)) {
        setNotesListCurrentIndex(i);
        break;
      }
    }
  }
}

void NotesList::resetQML_Recycle() {
  loadAllRecycle();

  int index = 0;
  QTreeWidgetItem* item = twrb->currentItem();
  if (item->parent() == NULL) {
    index = -1;
  } else {
    index = twrb->currentIndex().row();
  }

  m_Method->setCurrentIndexFromQW(mui->qwNoteRecycle, index);
}

void NotesList::initRecentOpen() {
  QSettings iniFile(iniDir + "recentopen.ini", QSettings::IniFormat);

  listRecentOpen.clear();
  int count = iniFile.value("/RecentOpen/count", 0).toInt();
  for (int i = 0; i < count; i++) {
    QString list =
        iniFile.value("/RecentOpen/list" + QString::number(i)).toString();
    QFile file(iniDir + list.split("===").at(1));
    if (file.exists()) {
      listRecentOpen.append(list);
    }
  }

  saveRecentOpen();
}

void NotesList::saveRecentOpen() {
  QSettings iniFile(iniDir + "recentopen.ini", QSettings::IniFormat);

  iniFile.setValue("/RecentOpen/count", listRecentOpen.count());
  for (int i = 0; i < listRecentOpen.count(); i++) {
    iniFile.setValue("/RecentOpen/list" + QString::number(i),
                     listRecentOpen.at(i));
  }
}

void NotesList::refreshRecentOpen(QString name) {
  QString strmd = currentMDFile;
  strmd = strmd.replace(iniDir, "").trimmed();

  listRecentOpen.insert(0, name + "===" + strmd);
  listRecentOpen = m_Method->removeDuplicatesFromQStringList(listRecentOpen);

  int count = listRecentOpen.count();
  if (count > 15) {
    listRecentOpen.removeAt(count - 1);
  }
}

void NotesList::saveCurrentNoteInfo() {
  QSettings Reg(iniDir + "curmd.ini", QSettings::IniFormat);

  QString str = currentMDFile;
  QString iniName = str.replace(iniDir, "");

  Reg.setValue("/MainNotes/currentItem", iniName);
  Reg.setValue("/MainNotes/NoteName", noteTitle);
  Reg.sync();
}

void NotesList::saveNotesList() {
  QFuture<void> future = QtConcurrent::run([=]() { saveNotesListToFile(); });

  // 可选：使用 QFutureWatcher 监控进度
  QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
  connect(watcher, &QFutureWatcher<void>::finished, this, [=]() {
    mw_one->strLatestModify = tr("Modi Notes List");
    m_Notes->isSaveNotesConfig = true;
    qDebug() << "NotesList save completed.";

    watcher->deleteLater();
  });
  watcher->setFuture(future);
}

void NotesList::saveNotesListToFile() {
  QString tempFile0 = iniDir + "temp.json";
  QString endFile0 = iniDir + "mainnotes.json";

  QJsonObject rootObj;

  // 当前 MD 文件
  QString md = currentMDFile;
  md = md.replace(iniDir, "");
  rootObj["CurrentMD"] = md;

  // 主笔记树
  QJsonArray mainNotesArray;
  int count = tw->topLevelItemCount();
  for (int i = 0; i < count; i++) {
    QTreeWidgetItem* topItem = tw->topLevelItem(i);
    QJsonObject topObj;
    topObj["name"] = topItem->text(0);
    topObj["colorFlag"] = topItem->text(2);

    QJsonArray childrenArray;
    int childCount = topItem->childCount();
    int n_less = 0;
    for (int j = 0; j < childCount; j++) {
      QTreeWidgetItem* childItem = topItem->child(j);
      QString strChild1 = childItem->text(1);
      QString md_file = iniDir + strChild1;

      if (QFile::exists(md_file) && !strChild1.isEmpty()) {
        QJsonObject childObj;
        childObj["name"] = childItem->text(0);
        childObj["file"] = strChild1;
        childrenArray.append(childObj);
      } else {
        n_less++;
      }
    }
    topObj["children"] = childrenArray;
    mainNotesArray.append(topObj);
  }
  rootObj["mainNotes"] = mainNotesArray;

  // 回收站
  QJsonArray recycleBinArray;
  if (twrb->topLevelItemCount() > 0) {
    QTreeWidgetItem* rbTopItem = twrb->topLevelItem(0);
    int rbChildCount = rbTopItem->childCount();
    for (int j = 0; j < rbChildCount; j++) {
      QTreeWidgetItem* childItem = rbTopItem->child(j);
      QJsonObject rbObj;
      rbObj["name"] = childItem->text(0);
      rbObj["file"] = childItem->text(1);
      recycleBinArray.append(rbObj);
    }
  }
  rootObj["recycleBin"] = recycleBinArray;

  // 待删除文件
  needDelFiles.removeDuplicates();
  QJsonArray needDelArray;
  for (const QString& file : std::as_const(needDelFiles)) {
    needDelArray.append(file);
  }
  rootObj["needDelNotes"] = needDelArray;

  // 保存 JSON 到临时文件
  QJsonDocument doc(rootObj);
  QFile f(tempFile0);
  if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
    f.write(doc.toJson(QJsonDocument::Indented));
    f.close();
  }

  // 覆盖到最终文件
  m_Method->upIniFile(tempFile0, endFile0);

  // Save Note Name
  saveCurrentNoteInfo();
}

void NotesList::initNotesList() {
  tw->clear();
  noteFiles.clear();

  QString jsonFile = iniDir + "mainnotes.json";

  if (QFile::exists(jsonFile)) {
    QFile f(jsonFile);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
      return;  // 文件不存在或无法打开
    }

    QByteArray data = f.readAll();
    f.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
      return;  // JSON 解析失败
    }

    QJsonObject rootObj = doc.object();

    // 获取 mainNotes 数组
    QJsonArray mainNotesArray = rootObj["mainNotes"].toArray();

    int nNoteBook = mainNotesArray.size();
    int notesTotal = 0;

    for (int i = 0; i < mainNotesArray.size(); ++i) {
      QJsonObject topObj = mainNotesArray[i].toObject();

      QString strTop = topObj["name"].toString();
      QString strTopColorFlag = topObj["colorFlag"].toString("#FF0000");

      QTreeWidgetItem* topItem = new QTreeWidgetItem;
      topItem->setText(0, strTop);
      topItem->setText(2, strTopColorFlag);
      topItem->setForeground(0, Qt::red);

      QFont font = this->font();
      font.setBold(true);
      topItem->setFont(0, font);
      topItem->setIcon(0, QIcon(":/res/nb.png"));

      // 子节点
      QJsonArray childrenArray = topObj["children"].toArray();
      notesTotal += childrenArray.size();

      for (int j = 0; j < childrenArray.size(); ++j) {
        QJsonObject childObj = childrenArray[j].toObject();
        QString str0 = childObj["name"].toString();
        QString str1 = childObj["file"].toString();

        if (!str1.isEmpty()) {
          QTreeWidgetItem* childItem = new QTreeWidgetItem(topItem);
          childItem->setText(0, str0);
          childItem->setText(1, str1);
          childItem->setIcon(0, QIcon(":/res/n.png"));

          QString md = iniDir + str1;
          m_Notes->m_NoteIndexManager->setNoteTitle(md, str0);
          updateNoteIndexManager(md, i, j);

          noteFiles.append(md);
        }
      }

      tw->addTopLevelItem(topItem);
    }

    tw->headerItem()->setText(
        0, tr("Notebook") + " : " + QString::number(nNoteBook) + "  " +
               tr("Notes") + " : " + QString::number(notesTotal));
    tw->expandAll();

  } else {
    QSettings* iniNotes =
        new QSettings(iniDir + "mainnotes.ini", QSettings::IniFormat, NULL);

    int topCount = iniNotes->value("/MainNotes/topItemCount").toInt();
    int nNoteBook = topCount;

    int notesTotal = 0;
    QString str0, str1;
    for (int i = 0; i < topCount; i++) {
      QString strTop =
          iniNotes->value("/MainNotes/strTopItem" + QString::number(i))
              .toString();
      QString strTopColorFlag =
          iniNotes
              ->value("/MainNotes/strTopColorFlag" + QString::number(i),
                      "#FF0000")
              .toString();
      int childCount =
          iniNotes->value("/MainNotes/childCount" + QString::number(i)).toInt();
      notesTotal = notesTotal + childCount;

      QTreeWidgetItem* topItem = new QTreeWidgetItem;
      topItem->setText(0, strTop);
      topItem->setText(2, strTopColorFlag);
      topItem->setForeground(0, Qt::red);
      QFont font = this->font();
      font.setBold(true);
      topItem->setFont(0, font);
      topItem->setIcon(0, QIcon(":/res/nb.png"));

      for (int j = 0; j < childCount; j++) {
        str0 = iniNotes
                   ->value("/MainNotes/childItem0_" + QString::number(i) + "_" +
                           QString::number(j))
                   .toString();
        str1 = iniNotes
                   ->value("/MainNotes/childItem1_" + QString::number(i) + "_" +
                           QString::number(j))
                   .toString();

        if (!str1.isEmpty()) {
          QTreeWidgetItem* childItem = new QTreeWidgetItem(topItem);
          childItem->setText(0, str0);
          childItem->setText(1, str1);
          childItem->setIcon(0, QIcon(":/res/n.png"));

          QString md = iniDir + str1;
          m_Notes->m_NoteIndexManager->setNoteTitle(md, str0);
          updateNoteIndexManager(md, i, j);

          noteFiles.append(md);
        }
      }
      tw->addTopLevelItem(topItem);
    }

    tw->headerItem()->setText(
        0, tr("Notebook") + " : " + QString::number(nNoteBook) + "  " +
               tr("Notes") + " : " + QString::number(notesTotal));
    tw->expandAll();
  }

  m_Notes->m_NoteIndexManager->saveIndex(strNoteNameIndexFile);

  initRecentOpen();
}

void NotesList::initRecycle() {
  twrb->clear();
  recycleFiles.clear();

  QString jsonFile = iniDir + "mainnotes.json";

  if (QFile::exists(jsonFile)) {
    QFile f(jsonFile);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
      return;  // 文件不存在或无法打开
    }

    QByteArray data = f.readAll();
    f.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
      return;  // JSON 解析失败
    }

    QJsonObject rootObj = doc.object();

    // 获取回收站数组
    QJsonArray recycleBinArray = rootObj["recycleBin"].toArray();

    // 创建顶层项
    QTreeWidgetItem* topItem = new QTreeWidgetItem;
    topItem->setText(0, tr("Notes Recycle Bin"));

    // 遍历回收站数组
    for (QJsonArray::const_iterator it = recycleBinArray.cbegin();
         it != recycleBinArray.cend(); ++it) {
      const QJsonValue& val = *it;  // 明确使用const引用
      QJsonObject obj = val.toObject();
      QString str0 = obj["name"].toString();
      QString str1 = obj["file"].toString();

      QTreeWidgetItem* childItem = new QTreeWidgetItem(topItem);
      childItem->setText(0, str0);
      childItem->setText(1, str1);
      childItem->setIcon(0, QIcon(":/res/n.png"));

      recycleFiles.append(iniDir + str1);
    }

    twrb->addTopLevelItem(topItem);
    twrb->expandAll();

  } else {
    QSettings iniNotes(iniDir + "mainnotes.ini", QSettings::IniFormat);

    int i = 0;
    QTreeWidgetItem* topItem = new QTreeWidgetItem;
    topItem->setText(0, tr("Notes Recycle Bin"));

    int childCount =
        iniNotes.value("/MainNotes/rbchildCount" + QString::number(i)).toInt();
    for (int j = 0; j < childCount; j++) {
      QString str0, str1;
      str0 = iniNotes
                 .value("/MainNotes/rbchildItem0" + QString::number(i) +
                        QString::number(j))
                 .toString();
      str1 = iniNotes
                 .value("/MainNotes/rbchildItem1" + QString::number(i) +
                        QString::number(j))
                 .toString();

      QTreeWidgetItem* childItem = new QTreeWidgetItem(topItem);
      childItem->setText(0, str0);
      childItem->setText(1, str1);
      childItem->setIcon(0, QIcon(":/res/n.png"));

      recycleFiles.append(iniDir + str1);
    }
    twrb->addTopLevelItem(topItem);

    twrb->expandAll();
  }
}

void NotesList::initUnclassified() {
  QStringList dirFiles = m_Method->getMdFilesInDir(iniDir + "memo/", true);

  QStringList excludeFiles = noteFiles + recycleFiles;
  excludeFiles.removeDuplicates();

  QStringList result;
  foreach (const QString& file, dirFiles) {
    if (!excludeFiles.contains(file)) {
      result.append(file);
    }
  }

  int count = result.count();
  if (count == 0) return;

  int topCount = tw->topLevelItemCount();

  QTreeWidgetItem* topItem = new QTreeWidgetItem;
  topItem->setText(0, tr("Unclassified"));
  topItem->setText(2, "#FF0000");
  topItem->setForeground(0, Qt::red);
  QFont font = this->font();
  font.setBold(true);
  topItem->setFont(0, font);
  topItem->setIcon(0, QIcon(":/res/nb.png"));

  TitleGenerator generator;

  for (int i = 0; i < count; i++) {
    QString mdFile = result.at(i);
    QString str0, str1;
    str0 = generator.genNewTitle(loadText(mdFile));
    str1 = mdFile;
    str1 = str1.replace(iniDir, "");
    QTreeWidgetItem* childItem = new QTreeWidgetItem(topItem);
    childItem->setText(0, str0);
    childItem->setText(1, str1);
    childItem->setIcon(0, QIcon(":/res/n.png"));

    m_Notes->m_NoteIndexManager->setNoteTitle(mdFile, str0);
    updateNoteIndexManager(mdFile, topCount, i);
  }

  tw->addTopLevelItem(topItem);
  tw->expandAll();

  qDebug() << result;
}

void NotesList::on_btnRecycle() {
  ui->frame0->hide();
  ui->frame1->show();
  setWinPos();
  twrb->setFocus();
  twrb->setCurrentItem(twrb->topLevelItem(0));
}

void NotesList::on_btnBack_clicked() {
  ui->frame1->hide();
  ui->frame0->show();
  setWinPos();
  tw->setFocus();
}

void NotesList::on_btnRestore_clicked() {
  QTreeWidgetItem* curItem = twrb->currentItem();
  if (curItem->parent() == NULL) return;

  if (moveItem(twrb)) {
    resetQML_List();
    clickNoteList();
    if (!ui->frame1->isHidden()) {
      on_btnBack_clicked();
    }
    if (!mui->frameNoteRecycle->isHidden()) {
      mw_one->on_btnBackNoteRecycle_clicked();
    }
  } else
    return;

  saveNotesList();
}

void NotesList::on_btnDel_Recycle_clicked() {
  QTreeWidgetItem* curItem = twrb->currentItem();
  if (curItem->parent() == NULL) {
    return;
  } else {
    auto m_ShowMsg = std::make_unique<ShowMessage>(this);
    if (!m_ShowMsg->showMsg(
            "Knot", tr("Whether to remove") + "  " + curItem->text(0) + " ? ",
            2)) {
      return;
    }

    QString md = iniDir + curItem->text(1);
    needDelWebDAVFiles.append(md + ".zip");
    QStringList imagesInMD = extractLocalImagesFromMarkdown(md);
    for (int i = 0; i < imagesInMD.count(); i++) {
      QString image_file = imagesInMD.at(i);
      image_file = "KnotData/memo/" + image_file;
      needDelWebDAVFiles.append(image_file);
    }

    delFile(md);
    QString json = m_Notes->getCurrentJSON(md);
    delFile(json);

    QStringList tempList = m_Notes->notes_sync_files;
    for (int i = 0; i < tempList.count(); i++) {
      QString file = tempList.at(i);
      QString baseFlag = m_Method->getBaseFlag(file);
      if (file.contains(baseFlag)) m_Notes->notes_sync_files.removeOne(file);
    }

    setDelNoteFlag(curItem->text(1));

    curItem->parent()->removeChild(curItem);

    isDelNoteRecycle = true;
  }

  saveNotesList();

  resetQML_Recycle();
}

void NotesList::setDelNoteFlag(QString mdFile) {
  needDelFiles.insert(0, mdFile);
  if (needDelFiles.count() > 500) {
    needDelFiles.removeAt(500);  // 移除老的
  }
}

void NotesList::needDelNotes() {
  QString jsonFile0 = iniDir + "mainnotes.json";
  int count = 0;

  if (QFile::exists(jsonFile0)) {
    QFile f(jsonFile0);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qWarning() << "打开 mainnotes.json 失败:" << f.errorString();
      return;  // 文件不存在或无法打开
    }

    QByteArray data = f.readAll();
    f.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
      qWarning() << "解析 mainnotes.json 失败（格式无效）";
      return;  // JSON 解析失败
    }

    QJsonObject rootObj = doc.object();

    if (!rootObj.contains("needDelNotes") ||
        !rootObj["needDelNotes"].isArray()) {
      qWarning() << "JSON中缺少needDelNotes数组";
      return;
    }

    // 获取待删除文件数组
    QJsonArray needDelArray = rootObj["needDelNotes"].toArray();

    needDelFiles.clear();
    count = needDelArray.size();
    for (int i = 0; i < count; ++i) {
      QString str1 = needDelArray[i].toString();
      if (str1.isEmpty()) continue;
      needDelFiles.append(str1);
    }

    qDebug() << "needDelFiles=" << needDelFiles.count();

  } else {
    QSettings iniNotes(iniDir + "mainnotes.ini", QSettings::IniFormat);
    count = iniNotes.value("/NeedDelNotes/Count", 0).toInt();
    if (count == 0) return;

    needDelFiles.clear();
    for (int i = 0; i < count; i++) {
      QString str1 =
          iniNotes.value("/NeedDelNotes/Note-" + QString::number(i), "")
              .toString();
      if (str1.isEmpty()) continue;
      needDelFiles.append(str1);
    }
  }

  // 定义保存的文件路径
  QString jsonPath = privateDir + "LocalDelList.json";

  // 读取上次保存的删除列表
  QStringList lastDelList;

  QFile jsonFile(jsonPath);
  if (jsonFile.exists() && jsonFile.open(QIODevice::ReadOnly)) {
    QByteArray data = jsonFile.readAll();
    jsonFile.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isArray()) {
      QJsonArray arr = doc.array();
      for (auto it = arr.begin(); it != arr.end(); ++it) {
        if (it->isString()) {
          lastDelList.append(it->toString());
        }
      }
    }
  }

  // 比较本次服务器下发的列表(直接比较集合）
  if (QSet<QString>(needDelFiles.begin(), needDelFiles.end()) !=
      QSet<QString>(lastDelList.begin(), lastDelList.end())) {
    // del files
    bool isDelMDOk, isDelJSONOk;
    for (int i = 0; i < needDelFiles.count(); i++) {
      QString mdFile = iniDir + needDelFiles.at(i);
      QString strFile = m_Notes->getCurrentJSON(mdFile);
      isDelMDOk = delFile(mdFile);
      isDelJSONOk = delFile(strFile);

      qDebug() << "Need Del Note: " << mdFile << isDelMDOk;
      qDebug() << "Need Del Note: " << strFile << isDelJSONOk;
    }

    // 保存本次的删除列表到 JSON 文件
    QJsonArray newArr;
    for (const QString& fileName : std::as_const(needDelFiles)) {
      newArr.append(fileName);
    }

    QJsonDocument newDoc(newArr);
    if (jsonFile.open(QIODevice::WriteOnly)) {
      jsonFile.write(
          newDoc.toJson(QJsonDocument::Indented));  // 缩进格式，方便查看
      jsonFile.close();
    } else {
      qWarning() << "无法打开文件进行写入:" << jsonFile.errorString();
      return;
    }
  } else {
    qDebug() << "删除列表无变化，跳过删除。";
  }
}

void NotesList::setWinPos() {
  int w = mw_one->width();
  int x = mw_one->geometry().x();
  this->setGeometry(x, mw_one->geometry().y(), w, mw_one->height());
  mui->btnBackNotesGraph->hide();
}

void NotesList::clearFiles() {
  QFile::remove(iniDir + "memo.zip");

  QString tempDir = iniDir;
  knot_all_files.clear();
  QStringList fmt = QString("zip;md;html;jpg;bmp;png;ini").split(';');
  getAllFiles(tempDir, knot_all_files, fmt);

  int count = knot_all_files.count();
  for (int i = 0; i < count; i++) {
    QString filePath = knot_all_files.at(i);

    QFile file(filePath);
    if (filePath.contains(".sync-conflict-")) {
      file.remove();
    }
  }

  clearMD_Pic();
}

void NotesList::clearMD_Pic() {
  // 获取所有MD文件和图片文件
  QStringList allmdFiles = m_Method->getMdFilesInDir(iniDir + "memo/", true);
  QStringList allimgFiles;
  QStringList fmt = QString("jpg;bmp;png").split(';');
  getAllFiles(iniDir + "memo/images/", allimgFiles, fmt);

  // 存储所有被引用的图片文件名
  QStringList usedImageNames;

  // 使用静态QRegularExpression对象，避免重复创建
  static const QRegularExpression regex(
      R"(!\[.*?\]\(([\w\-./\\]+\.(jpg|jpeg|bmp|png|gif))\))",
      QRegularExpression::CaseInsensitiveOption);

  // 1. 解析所有Markdown文件，提取引用的图片
  foreach (const QString& mdFilePath, allmdFiles) {
    QFile mdFile(mdFilePath);
    if (!mdFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qWarning() << "无法打开Markdown文件:" << mdFilePath;
      continue;
    }

    // 逐行读取，降低内存占用
    QTextStream in(&mdFile);
    QString line;
    while (in.readLineInto(&line)) {
      QRegularExpressionMatchIterator it = regex.globalMatch(line);
      while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString imagePath = match.captured(1);
        // 处理相对路径：基于当前MD文件所在目录解析图片路径
        QFileInfo mdFileInfo(mdFilePath);
        QFileInfo imageInfo(mdFileInfo.dir(), imagePath);
        usedImageNames << imageInfo.fileName();
      }
    }
    mdFile.close();
  }

  // 去重处理
  usedImageNames.removeDuplicates();

  // 2. 找出并删除未被引用的图片
  int deletedCount = 0;
  int failedCount = 0;

  foreach (const QString& imgFilePath, allimgFiles) {
    QFileInfo imgInfo(imgFilePath);
    QString imgFileName = imgInfo.fileName();

    // 检查图片是否未被引用
    if (!usedImageNames.contains(imgFileName)) {
      QFile imgFile(imgFilePath);
      if (imgFile.remove()) {
        qDebug() << "已删除未使用的图片:" << imgFilePath;
        deletedCount++;
      } else {
        qWarning() << "删除失败:" << imgFilePath
                   << "原因:" << imgFile.errorString();
        failedCount++;
      }
    }
  }

  qDebug() << "图片清理完成 - 已删除:" << deletedCount
           << "个, 删除失败:" << failedCount << "个";
}

void NotesList::getAllFiles(const QString& foldPath, QStringList& folds,
                            const QStringList& formats) {
  QDirIterator it(foldPath, QDir::Files | QDir::NoDotAndDotDot,
                  QDirIterator::Subdirectories);
  while (it.hasNext()) {
    it.next();
    QFileInfo fileInfo = it.fileInfo();
    if (formats.contains(fileInfo.suffix())) {  // 检测格式，按需保存
      folds << fileInfo.absoluteFilePath();
    }
  }
}

// 文件搜索实现
QStringList findMarkdownFiles(const QString& dirPath) {
  Q_UNUSED(dirPath);

  QList<QString> paths;
  QDir dir(iniDir + "memo/");
  const QStringList files = dir.entryList(QStringList() << "*.md", QDir::Files);
  for (const QString& file : files) {
    QFileInfo info(dir.absoluteFilePath(file));
    QString canonicalPath = info.canonicalFilePath();  // 规范化路径
    if (!canonicalPath.isEmpty() && info.exists()) {
      paths.append(canonicalPath);
    }
  }
  // 使用 QSet 去重
  return QSet<QString>(paths.begin(), paths.end()).values();
}

MySearchResult searchInFile(const QString& filePath,
                            const QRegularExpression& regex) {
  MySearchResult result;
  QFile file(filePath);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream in(&file);
    int lineNum = 0;
    while (!in.atEnd()) {
      ++lineNum;
      if (regex.match(in.readLine()).hasMatch()) {
        result.filePath = filePath;
        result.lineNumbers.append(lineNum);
      }
    }
  }
  return result;
}

// 结果归并
void reduceResults(ResultsMap& result, const MySearchResult& partial) {
  if (!partial.filePath.isEmpty()) {
    result[partial.filePath] = partial;
  }
}

QFuture<ResultsMap> NotesList::performSearchAsync(const QString& dirPath,
                                                  const QString& keyword) {
  return QtConcurrent::run([this, dirPath, keyword]() {
    QStringList files = findMarkdownFiles(dirPath);
    QStringList cycleFiles = m_NotesList->getRecycleNoteFiles();

    files.removeIf([&cycleFiles](const QString& file) {
      return cycleFiles.contains(file);
    });

    QRegularExpression regex(
        keyword, QRegularExpression::CaseInsensitiveOption |
                     QRegularExpression::UseUnicodePropertiesOption);

    // 注意这里用 blockingMappedReduced
    return QtConcurrent::blockingMappedReduced(
        files, SearchMapper(regex), reduceResults,
        QtConcurrent::ReduceOption::UnorderedReduce);
  });
}

void NotesList::displayResults(const ResultsMap& results) {
  for (auto it = results.begin(); it != results.end(); ++it) {
    qDebug() << "文件：" << it.key();
    qDebug() << "匹配行号：" << it.value().lineNumbers;
    QString file = it.key();
    QList<int> lineNumbersList = it.value().lineNumbers;
    QString strLine;
    for (int i = 0; i < lineNumbersList.count(); i++) {
      strLine = strLine + " " + QString::number(lineNumbersList.at(i));
    }
    m_NotesList->searchResultList.append(file + "-==-" + strLine.trimmed());
  }
  qDebug() << m_NotesList->searchResultList;

  m_NotesList->showNotesSearchResult();
}

void NotesList::showNotesSearchResult() {
  if (searchResultList.count() == 0) {
    mw_one->closeProgress();
    return;
  }

  mui->frameNoteList->hide();
  mui->frameNotesSearchResult->show();

  m_Method->clearAllBakList(mui->qwNotesSearchResult);
  for (int i = 0; i < searchResultList.count(); i++) {
    QStringList list = searchResultList.at(i).split("-==-");
    QString note_name = getCurrentNoteNameFromMDFile(list.at(0));
    if (note_name != "")
      m_Method->addItemToQW(mui->qwNotesSearchResult, note_name, list.at(0),
                            list.at(1), "", 0);
  }

  mw_one->closeProgress();
}

void NotesList::onSearchFinished() {
  if (!watcher) return;

  // ▶️ 正式获取结果
  const ResultsMap results = watcher->result();

  // ▶️ 数据安全检查
  if (results.isEmpty()) {
    mw_one->closeProgress();
    searchResultList.clear();
    mui->btnFindNextNote->setEnabled(false);
    mui->btnFindPreviousNote->setEnabled(false);
    mui->lblShowLineSn->setText("0");
    mui->lblFindNoteCount->setText("0");
    auto msg = std::make_unique<ShowMessage>(this);
    msg->showMsg("Knot", tr("No match was found."), 1);

    return;
  }

  m_Method->clearAllBakList(mui->qwNotesSearchResult);

  // ▶️ 处理搜索结果

  for (auto it = results.constBegin(); it != results.constEnd(); ++it) {
    const QString& filePath = it.key();
    const QList<int> lines = it.value().lineNumbers;

    qDebug() << "文件：" << it.key();
    qDebug() << "匹配行号：" << it.value().lineNumbers;

    QString strLineSn;
    int linesCount = lines.count();
    for (int i = 0; i < linesCount; i++) {
      strLineSn = strLineSn + " " + QString::number(lines.at(i));
    }
    strLineSn = strLineSn.trimmed();

    if (!recycleNotesList.contains(filePath))
      searchResultList.append(filePath + "-==-" + strLineSn + "-==-" +
                              QString::number(linesCount));
  }

  // ▶️ 释放资源
  watcher->deleteLater();

  if (searchResultList.count() > 0) {
    mui->btnFindNextNote->setEnabled(true);
    mui->btnFindPreviousNote->setEnabled(true);
    mui->lblFindNoteCount->setText(QString::number(searchResultList.count()));

    goNext();
  }

  mw_one->closeProgress();
}

void NotesList::startFind(QString strFind) {
  mw_one->showProgress();

  QString directory = iniDir + "memo/";
  QString keyword = strFind;
  searchResultList.clear();
  findCount = -1;

  watcher = new QFutureWatcher<ResultsMap>(this);
  auto future = performSearchAsync(directory, keyword);
  watcher->setFuture(future);
  connect(watcher, &QFutureWatcher<ResultsMap>::finished, this,
          &NotesList::onSearchFinished);
}

// 通用导航函数：step 为 -1 表示上一个，1 表示下一个
void NotesList::navigateFindResult(int step) {
  // 检查搜索结果列表是否为空
  if (searchResultList.isEmpty()) {
    mui->lblFindNoteCount->setText("0 -> 0");
    return;
  }

  // 更新当前索引（根据步长调整）
  findCount += step;

  // 处理循环边界（索引超出范围时循环到首尾）
  int listCount = searchResultList.count();
  if (findCount < 0) {
    findCount = listCount - 1;
  } else if (findCount >= listCount) {
    findCount = 0;
  }

  // 解析当前结果项（带格式校验）
  QStringList list = searchResultList.at(findCount).split("-==-");
  if (list.size() >= 3) {  // 确保分割后有足够的元素
    QString md_file = list.at(0);
    mui->lblShowLineSn->setText(list.at(2));
    setCurrentItemFromMDFile(md_file);
    clickNoteList();
  }

  // 更新计数标签
  mui->lblFindNoteCount->setText(QString::number(findCount + 1) + " -> " +
                                 QString::number(listCount));

  // 隐藏安卓键盘（如果可见）
  if (pAndroidKeyboard->isVisible()) {
    pAndroidKeyboard->hide();
  }
}

// 上一个结果（复用通用导航函数，步长为 -1）
void NotesList::goPrevious() { navigateFindResult(-1); }

// 下一个结果（复用通用导航函数，步长为 1）
void NotesList::goNext() { navigateFindResult(1); }

void NotesList::goFindResult(int index) {
  if (findResult.count() == 0) return;

  findCount = index;
  QString str = findResult.at(index);
  QStringList list = str.split("===");
  if (list.at(1) == "NoteBook") {
    int indexNoteBook = list.at(0).toInt();
    setNoteBookCurrentIndex(indexNoteBook);
    clickNoteBook();
    setNotesListCurrentIndex(-1);
  } else {
    int index0, index1;
    index0 = list.at(0).toInt();
    index1 = list.at(1).toInt();
    setNoteBookCurrentIndex(index0);
    clickNoteBook();
    setNotesListCurrentIndex(index1);
  }

  mui->lblFindNoteCount->setText(QString::number(findCount + 1) + " -> " +
                                 QString::number(findResult.count()));
}

void NotesList::localItem() {
  QTreeWidgetItem* item = tw->currentItem();
  // NoteBook

  if (item->childCount() > 0) {
    if (item->parent() == NULL) {
      int topIndex = tw->indexOfTopLevelItem(item);
      setNoteBookCurrentIndex(topIndex);
    } else {
      int index = tw->indexOfTopLevelItem(item->parent());

      int childCount = item->parent()->childCount();
      int newRow = 0;
      for (int i = 0; i < childCount; i++) {
        QTreeWidgetItem* item1 = item->parent()->child(i);
        if (item1 == item) break;

        if (item1->text(1).isEmpty()) newRow++;
      }

      setNoteBookCurrentIndex(index + newRow + 1);
    }

    clickNoteBook();
    setNotesListCurrentIndex(-1);

    // Notes
  } else {
    QTreeWidgetItem* top_item = item->parent()->parent();
    if (top_item == NULL) {
      int topIndex = tw->indexOfTopLevelItem(item->parent());
      int childIndex = tw->currentIndex().row();
      setNoteBookCurrentIndex(topIndex);
      clickNoteBook();
      setNotesListCurrentIndex(childIndex);
    } else {
      int index = tw->indexOfTopLevelItem(top_item);

      int childCount = top_item->childCount();
      int newRow = 0;
      for (int i = 0; i < childCount; i++) {
        QTreeWidgetItem* item1 = top_item->child(i);
        if (item1 == item->parent()) break;

        if (item1->text(1).isEmpty()) newRow++;
      }

      setNoteBookCurrentIndex(index + newRow + 1);

      clickNoteBook();
      setNotesListCurrentIndex(item->parent()->indexOfChild(item));
    }
  }
}

QString NotesList::getNoteBookText0(int index) {
  return m_Method->getText0(mui->qwNoteBook, index);
}

QString NotesList::getNotesListText0(int index) {
  return m_Method->getText0(mui->qwNoteList, index);
}

void NotesList::modifyNoteBookText0(QString text0, int index) {
  m_Method->modifyItemText0(mui->qwNoteBook, index, text0);
}

void NotesList::modifyNotesListText0(QString text0, int index) {
  m_Method->modifyItemText0(mui->qwNoteList, index, text0);
}

void NotesList::on_KVChanged() {
  if (!pAndroidKeyboard->isVisible()) {
  } else {
  }
}

void NotesList::moveBy(int ud) {
  QTreeWidgetItem* item = tw->currentItem();

  if (item == NULL) return;

  if (item->parent() != NULL) {
    QTreeWidgetItem* parentItem = item->parent();
    int index = parentItem->indexOfChild(item);
    if (ud == -1) {
      if (index - 1 >= 0) {
        int n = 0;
        // NoteBook
        if (item->text(1).isEmpty()) {
          for (int i = index - 1; i >= 0; i--) {
            n++;
            if (parentItem->child(i)->text(1).isEmpty()) {
              break;
            }
          }
        }
        // Note
        if (!item->text(1).isEmpty()) {
          for (int i = index - 1; i >= 0; i--) {
            n++;
            if (!parentItem->child(i)->text(1).isEmpty()) {
              break;
            }
          }
        }

        parentItem->removeChild(item);
        parentItem->insertChild(index - n, item);
        tw->setCurrentItem(item);
        tw->scrollToItem(item);
      }
    }
    if (ud == 1) {
      int count = parentItem->childCount() - 1;
      int n = 0;
      // NoteBook
      if (item->text(1).isEmpty()) {
        for (int i = index + 1; i <= count; i++) {
          n++;
          if (parentItem->child(i)->text(1).isEmpty()) {
            break;
          }
        }
      }
      // Note
      if (!item->text(1).isEmpty()) {
        for (int i = index + 1; i <= count; i++) {
          n++;
          if (!parentItem->child(i)->text(1).isEmpty()) {
            break;
          }
        }
      }

      if (index + n <= count) {
        parentItem->removeChild(item);
        parentItem->insertChild(index + n, item);
        tw->setCurrentItem(item);
        tw->scrollToItem(item);
      }
    }
  } else {
    // top item
    int index = tw->indexOfTopLevelItem(item);
    if (ud == -1) {
      if (index - 1 >= 0) {
        tw->takeTopLevelItem(index);
        tw->insertTopLevelItem(index - 1, item);
        tw->setCurrentItem(item);
        tw->scrollToItem(item);
        tw->expandAll();
      }
    }
    if (ud == 1) {
      if (index + 1 <= tw->topLevelItemCount() - 1) {
        tw->takeTopLevelItem(index);
        tw->insertTopLevelItem(index + 1, item);
        tw->setCurrentItem(item);
        tw->scrollToItem(item);
        tw->expandAll();
      }
    }
  }

  resetQML_List();
  saveNotesList();

  updateAllNoteIndexManager();
}

void NotesList::on_btnUp_clicked() {
  moveBy(-1);

  bool isParent = false;
  QTreeWidgetItem* item = tw->currentItem();
  if (item->parent() == NULL) {
    isParent = true;

  } else {
    int index = m_Method->getCurrentIndexFromQW(mui->qwNoteList);
    setNotesListCurrentIndex(index - 1);
    clickNoteList();
    isParent = false;
  }

  if (isParent) {
    isMouseClick = true;
  }
}

void NotesList::on_btnDown_clicked() {
  moveBy(1);

  bool isParent = false;
  QTreeWidgetItem* item = tw->currentItem();
  if (item->parent() == NULL) {
    isParent = true;

  } else {
    int index = m_Method->getCurrentIndexFromQW(mui->qwNoteList);
    setNotesListCurrentIndex(index + 1);
    clickNoteList();
    isParent = false;
  }

  if (isParent) {
    isMouseClick = true;
  }
}

void NotesList::setTWRBCurrentItem() {
  int index = m_Method->getCurrentIndexFromQW(mui->qwNoteRecycle);
  QTreeWidgetItem* topItem = twrb->topLevelItem(0);
  twrb->setCurrentItem(topItem->child(index));
}

void NotesList::setTWCurrentItem() {
  int index0, index1;
  index0 = getNoteBookCurrentIndex();
  index1 = getNotesListCurrentIndex();
  if (index0 >= 0) tw->setCurrentItem(pNoteBookItems.at(index0));
  if (index1 >= 0) tw->setCurrentItem(pNoteItems.at(index1));

  tw->scrollToItem(tw->currentItem());
}

void NotesList::setNoteBookCurrentItem() {
  int index = getNoteBookCurrentIndex();
  if (index < 0) return;

  QString text1 = m_Method->getText1(mui->qwNoteBook, index);
  QString text2 = m_Method->getText2(mui->qwNoteBook, index);
  if (text2.isEmpty()) {
    tw->setCurrentItem(tw->topLevelItem(text1.toInt()));

  } else {
    QStringList list = text1.split("===");
    int indexMain, indexChild;
    if (list.count() == 2) {
      indexMain = list.at(0).toInt();
      indexChild = list.at(1).toInt();

      QTreeWidgetItem* topItem = tw->topLevelItem(indexMain);
      QTreeWidgetItem* childItem = topItem->child(indexChild);
      tw->setCurrentItem(childItem);
    }
  }
}

void NotesList::on_actionAdd_NoteBook_triggered() {
  QString text;

  m_NewNoteBook = new NewNoteBook(mw_one);
  m_NewNoteBook->showDialog();
  text = m_NewNoteBook->notebookName;
  if (m_NewNoteBook->isOk && !text.isEmpty()) {
    rootIndex = m_NewNoteBook->rootIndex;
    notebookName = text;
    on_btnNewNoteBook_clicked();

    loadAllNoteBook();

    int count = m_Method->getCountFromQW(mui->qwNoteBook);
    int index = 0;
    for (int i = 0; i < count; i++) {
      if (pNoteBookItems.at(i) == tw->currentItem()) {
        index = i;
        break;
      }
    }
    setNoteBookCurrentIndex(index);
    clickNoteBook();
  }

  saveNotesList();
}

void NotesList::on_actionDel_NoteBook_triggered() {
  int index = getNoteBookCurrentIndex();
  if (index < 0) return;

  tw->setCurrentItem(pNoteBookItems.at(index));
  on_btnDel_clicked();

  loadAllNoteBook();

  int count = getNoteBookCount();
  for (int i = 0; i < count; i++) {
    if (tw->currentItem() == pNoteBookItems.at(i)) {
      index = i;
      break;
    }
  }
  setNoteBookCurrentIndex(index);

  setNoteLabel();

  saveNotesList();

  if (count == 0) {
    m_Notes->loadEmptyNote();
  }
}

void NotesList::on_actionRename_NoteBook_triggered() {
  int index = getNoteBookCurrentIndex();
  if (index < 0) return;

  on_btnRename_clicked();
}

int NotesList::getNoteBookCount() {
  int count = m_Method->getCountFromQW(mui->qwNoteBook);
  return count;
}

int NotesList::getNotesListCount() {
  int count = m_Method->getCountFromQW(mui->qwNoteList);
  return count;
}

int NotesList::getNoteBookCurrentIndex() {
  int index = m_Method->getCurrentIndexFromQW(mui->qwNoteBook);
  return index;
}

int NotesList::getNotesListCurrentIndex() {
  int index = m_Method->getCurrentIndexFromQW(mui->qwNoteList);
  return index;
}

void NotesList::setNoteBookCurrentIndex(int index) {
  m_Method->setCurrentIndexFromQW(mui->qwNoteBook, index);
}

void NotesList::setNotesListCurrentIndex(int index) {
  m_Method->setCurrentIndexFromQW(mui->qwNoteList, index);
}

void NotesList::on_actionMoveUp_NoteBook_triggered() {
  int index = getNoteBookCurrentIndex();
  if (index <= 0) return;

  setNoteBookCurrentItem();
  on_btnUp_clicked();

  int newIndex = getNoteBookIndex_twToqml();

  loadAllNoteBook();
  setNoteBookCurrentIndex(newIndex);
  clickNoteBook();
}

void NotesList::on_actionMoveDown_NoteBook_triggered() {
  int index = getNoteBookCurrentIndex();

  if (index < 0) return;

  if (index == getNoteBookCount() - 1) return;

  setNoteBookCurrentItem();
  on_btnDown_clicked();

  int newIndex = getNoteBookIndex_twToqml();

  loadAllNoteBook();
  setNoteBookCurrentIndex(newIndex);
  clickNoteBook();
}

int NotesList::getNoteBookIndex_twToqml() {
  QTreeWidgetItem* item = tw->currentItem();
  int index = 0;
  if (item->parent() == NULL) {
    index = tw->indexOfTopLevelItem(item);
  } else {
    int index0 = tw->indexOfTopLevelItem(item->parent());
    int index1 = 0;
    int count = item->parent()->childCount();
    for (int i = 0; i < count; i++) {
      QTreeWidgetItem* item0 = item->parent()->child(i);
      if (item0 == item) break;

      if (item0->text(1).isEmpty()) index1++;
    }

    index = index0 + index1 + 1;
  }

  return index;
}

void NotesList::loadAllNoteBook() {
  pNoteBookItems.clear();

  m_Method->clearAllBakList(mui->qwNoteBook);
  m_Method->clearAllBakList(mui->qwNoteList);
  int count = tw->topLevelItemCount();
  for (int i = 0; i < count; i++) {
    QTreeWidgetItem* topItem = tw->topLevelItem(i);
    QString str = topItem->text(0);
    QString strtopcolorflag = topItem->text(2);

    int sum = topItem->childCount();
    int child_count = sum;
    for (int n = 0; n < child_count; n++) {
      if (topItem->child(n)->text(1).isEmpty()) sum--;
    }

    QString strSum = QString::number(sum);
    addItemToQW(mui->qwNoteBook, str, QString::number(i), "", strSum,
                strtopcolorflag, fontSize);
    pNoteBookItems.append(topItem);

    int childCount = topItem->childCount();
    for (int j = 0; j < childCount; j++) {
      QTreeWidgetItem* childItem = topItem->child(j);
      if (childItem->text(1).isEmpty()) {
        addItemToQW(mui->qwNoteBook, childItem->text(0),
                    QString::number(i) + "===" + QString::number(j),
                    "isNoteBook", QString::number(childItem->childCount()), "",
                    fontSize - 1);

        pNoteBookItems.append(childItem);
      }
    }
  }
}

void NotesList::addItemToQW(QQuickWidget* qw, QString text0, QString text1,
                            QString text2, QString text3, QString text4,
                            int itemH) {
  QQuickItem* root = qw->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "addItem", Q_ARG(QVariant, text0),
                            Q_ARG(QVariant, text1), Q_ARG(QVariant, text2),
                            Q_ARG(QVariant, text3), Q_ARG(QVariant, text4),
                            Q_ARG(QVariant, itemH));
}

void NotesList::setColorFlag(QString strColor) {
  QQuickItem* root = mui->qwNoteBook->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "setColorFlag",
                            Q_ARG(QVariant, strColor));
}

void NotesList::init_NoteBookMenu(QMenu* mainMenu) {
  QAction* actNew = new QAction(tr("New NoteBook"));
  QAction* actDel = new QAction(tr("Del NoteBook"));
  QAction* actRename = new QAction(tr("Rename NoteBook"));
  QAction* actMoveUp = new QAction(tr("Move Up"));
  QAction* actMoveDown = new QAction(tr("Move Down"));
  QAction* actSetColorFlag = new QAction(tr("Set Color Marker"));
  actSetColorFlag->setEnabled(isActColorFlagStatus);
  QAction* actStatistics = new QAction(tr("Statistics"));
  QAction* actRebuildSearchIndex =
      new QAction(tr("Rebuild Search Database Index"));

  connect(actNew, &QAction::triggered, this,
          &NotesList::on_actionAdd_NoteBook_triggered);
  connect(actDel, &QAction::triggered, this,
          &NotesList::on_actionDel_NoteBook_triggered);
  connect(actRename, &QAction::triggered, this,
          &NotesList::on_actionRename_NoteBook_triggered);

  connect(actMoveUp, &QAction::triggered, this,
          &NotesList::on_actionMoveUp_NoteBook_triggered);
  connect(actMoveDown, &QAction::triggered, this,
          &NotesList::on_actionMoveDown_NoteBook_triggered);
  connect(actSetColorFlag, &QAction::triggered, this,
          &NotesList::on_actionSetColorFlag);

  connect(actStatistics, &QAction::triggered, this,
          &NotesList::on_actionStatistics);

  connect(actRebuildSearchIndex, &QAction::triggered, this, [this]() {
    auto msg = std::make_unique<ShowMessage>(this);
    if (msg->showMsg(
            appName,
            tr("Rebuilding the index will take some time. Click OK to start."),
            2)) {
      mw_one->showProgress();
      QString databaseFile = privateDir + "md_database_v3.db";
      if (m_dbManager.deleteDatabaseFile(databaseFile))
        initSerachDatabase();
      else
        mw_one->closeProgress();
    }
  });

  mainMenu->addAction(actNew);
  mainMenu->addAction(actRename);
  mainMenu->addAction(actDel);

  mainMenu->addAction(actMoveUp);
  mainMenu->addAction(actMoveDown);

  mainMenu->addAction(actStatistics);
  mainMenu->addAction(actRebuildSearchIndex);
  mainMenu->addAction(actSetColorFlag);

  actRename->setVisible(false);
  actDel->setVisible(false);
  actMoveUp->setVisible(false);
  actMoveDown->setVisible(false);
  actStatistics->setVisible(true);

  mainMenu->setStyleSheet(m_Method->qssMenu);
}

void NotesList::on_actionSetColorFlag() {
  QString color_0;
  color_0 = m_Method->getCustomColor();
  if (color_0.isNull()) return;

  QTreeWidgetItem* itemTop;
  QTreeWidgetItem* item = tw->currentItem();
  if (item->parent() == nullptr) itemTop = item;
  if (item->parent() != nullptr) itemTop = item->parent();
  if (item->parent()->parent() != nullptr) itemTop = item->parent()->parent();
  itemTop->setText(2, color_0);
  setColorFlag(color_0);
  saveNotesList();
}

void NotesList::on_actionStatistics() {
  // 1. UI线程：显示进度条
  mw_one->showProgress();

  // 2. 提前计算所有需要的变量（UI线程非耗时操作，值捕获给后台Lambda）
  int countNoteBook = tw->topLevelItemCount();
  int totalNotes = 0;
  for (int i = 0; i < countNoteBook; i++) {
    totalNotes += tw->topLevelItem(i)->childCount();
  }
  int webDAVCount = m_Method->getAccessCount();
  QString memoDir = iniDir + "memo/images/";
  QString localAppName = appName;  // 单独赋值，便于值捕获

  // 3. 【核心】后台任务：极简Lambda，仅执行耗时的图片统计，用局部变量存储结果
  // 定义一个可被Lambda捕获的变量（用于存储后台统计结果）
  int* imgCountPtr = new int(0);  // 用堆内存存储，避免栈变量生命周期问题

  QFuture<void> future = QtConcurrent::run([=]() {
    // 【后台线程执行】仅做耗时操作，不操作任何UI，结果存入堆内存指针
    *imgCountPtr = countMdFilesImages(memoDir);
  });

  // 4. 【核心】监控任务完成：与你的参考示例结构完全一致
  QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
  connect(watcher, &QFutureWatcher<void>::finished, this, [=]() {
    qDebug() << "Statistics task completed.";

    // 【UI线程执行】处理结果，安全操作UI
    QString strAccessCount =
        tr("Access WebDAV:") + QString::number(webDAVCount) + "t/30min";

    // 关闭进度条
    mw_one->closeProgress();

    // 弹出统计消息框（使用后台统计的结果）
    auto msg = std::make_unique<ShowMessage>(this);
    msg->showMsg(localAppName,
                 tr("NoteBook") + ": " + QString::number(countNoteBook) +
                     "    " + tr("Notes") + ": " + QString::number(totalNotes) +
                     "\n\n" + tr("Images") + ": " +
                     QString::number(*imgCountPtr) + "\n\n" + strAccessCount,
                 1);

    // 【关键】释放资源：避免内存泄漏（堆内存指针+watcher）
    delete imgCountPtr;
    watcher->deleteLater();
  });
  watcher->setFuture(future);
}

int NotesList::countMdFilesImages(const QString& dirPath) {
  // 1. 构造QDir对象，指定目标目录路径
  QDir targetDir(dirPath);

  // 2. 仅需调用QDir::exists()，即可校验目录是否存在且为有效目录
  // （QDir::exists() 对目录路径返回true，对文件/无效路径返回false）
  if (!targetDir.exists()) {
    return 0;  // 无效目录（或文件、不存在路径）返回0个文件
  }

  // 3. 设置文件筛选规则：仅获取文件（排除子目录、符号链接等）
  QFileInfoList fileInfoList =
      targetDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

  // 4. 返回文件列表的长度，即该目录下直接文件的数量
  return fileInfoList.count();
}

void NotesList::on_actionAdd_Note_triggered() {
  int notebookIndex = getNoteBookCurrentIndex();

  if (notebookIndex < 0) {
    auto msg = std::make_unique<ShowMessage>(this);
    msg->showMsg(
        "Knot",
        tr("Please create a new notebook first, and then create new notes."),
        1);
    return;
  }

  tw->setCurrentItem(tw->topLevelItem(notebookIndex));

  on_btnNewNote_clicked();

  QTreeWidgetItem* childItem = tw->currentItem();
  QString text3 = childItem->text(1);
  m_Method->addItemToQW(mui->qwNoteList, "", "", "", text3, 0);

  int count = getNotesListCount();
  setNotesListCurrentIndex(count - 1);

  clickNoteList();
  m_Notes->updateMDFileToSyncLists();
  mw_one->on_btnEditNote_clicked();

  setNoteLabel();

  renameCurrentItem(tr("Untitled Note"));
  saveNotesList();
  updateAllNoteIndexManager();
}

void NotesList::on_actionDel_Note_triggered() {
  if (getNotesListCount() == 0) return;

  int notebookIndex = getNoteBookCurrentIndex();
  int notelistIndex = getNotesListCurrentIndex();

  if (notebookIndex < 0) return;
  if (notelistIndex < 0) return;

  on_btnDel_clicked();

  int count = getNotesListCount();

  setNoteLabel();

  saveNotesList();

  if (count == 0) {
    m_Notes->loadEmptyNote();
  }
}

void NotesList::on_actionRename_Note_triggered() {
  int notebookIndex = getNoteBookCurrentIndex();
  int noteIndex = getNotesListCurrentIndex();
  if (notebookIndex < 0) return;
  if (noteIndex < 0) return;

  on_btnRename_clicked();
}

void NotesList::on_actionMoveUp_Note_triggered() {
  int indexBook = getNoteBookCurrentIndex();
  int indexNote = getNotesListCurrentIndex();
  if (indexBook < 0) return;
  if (indexNote <= 0) return;

  setNoteBookCurrentItem();
  tw->setCurrentItem(tw->currentItem()->child(indexNote));
  on_btnUp_clicked();

  clickNoteBook();
  setNotesListCurrentIndex(indexNote - 1);
}

void NotesList::on_actionMoveDown_Note_triggered() {
  int indexBook = getNoteBookCurrentIndex();
  int indexNote = getNotesListCurrentIndex();
  if (indexBook < 0) return;
  if (indexNote < 0) return;
  if (indexNote + 1 == getNotesListCount()) return;

  setNoteBookCurrentItem();
  tw->setCurrentItem(tw->currentItem()->child(indexNote));
  on_btnDown_clicked();

  clickNoteBook();
  setNotesListCurrentIndex(indexNote + 1);
}

void NotesList::on_actionImport_Note_triggered() {
  int indexBook = getNoteBookCurrentIndex();
  if (indexBook < 0) return;

  setNoteBookCurrentItem();
  int fileCount = on_btnImport_clicked();

  while (!isImportFilesEnd)
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

  mw_one->closeProgress();

  if (fileCount > 0) {
    clickNoteBook();

    clickNoteList();
    saveNotesList();

    updateAllNoteIndexManager();
  }
}

void NotesList::on_actionExport_Note_triggered() {
  int indexBook = getNoteBookCurrentIndex();
  int indexNote = getNotesListCurrentIndex();

  if (indexBook < 0) return;
  if (indexNote < 0) return;

  setNoteBookCurrentItem();
  tw->setCurrentItem(tw->currentItem()->child(indexNote));

  on_btnExport_clicked();
}

void NotesList::init_NotesListMenu(QMenu* mainMenu) {
  QAction* actNew = new QAction(tr("New Note"));
  QAction* actDel = new QAction(tr("Del Note"));
  QAction* actRename = new QAction(tr("Rename Note"));
  QAction* actMoveUp = new QAction(tr("Move Up"));
  QAction* actMoveDown = new QAction(tr("Move Down"));
  QAction* actImport = new QAction(tr("Import"));
  QAction* actExport = new QAction(tr("Export"));
  QAction* actShare = new QAction(tr("Share"));
  QAction* actCopyLink = new QAction(tr("Copy Note Link"));
  QAction* actRelationshipGraph = new QAction(tr("Relationship Graph"));
  QAction* actModificationHistory = new QAction(tr("Modification History"));

  connect(actNew, &QAction::triggered, this,
          &NotesList::on_actionAdd_Note_triggered);
  connect(actDel, &QAction::triggered, this,
          &NotesList::on_actionDel_Note_triggered);
  connect(actRename, &QAction::triggered, this,
          &NotesList::on_actionRename_Note_triggered);

  connect(actMoveUp, &QAction::triggered, this,
          &NotesList::on_actionMoveUp_Note_triggered);
  connect(actMoveDown, &QAction::triggered, this,
          &NotesList::on_actionMoveDown_Note_triggered);

  connect(actImport, &QAction::triggered, this,
          &NotesList::on_actionImport_Note_triggered);
  connect(actExport, &QAction::triggered, this,
          &NotesList::on_actionExport_Note_triggered);
  connect(actShare, &QAction::triggered, this,
          &NotesList::on_actionShareNoteFile);
  connect(actCopyLink, &QAction::triggered, this,
          &NotesList::on_actionCopyNoteLink);
  connect(actRelationshipGraph, &QAction::triggered, this,
          &NotesList::on_actionRelationshipGraph);
  connect(actModificationHistory, &QAction::triggered, this,
          &NotesList::on_actionModificationHistory);

  mainMenu->addAction(actNew);
  mainMenu->addAction(actRename);
  mainMenu->addAction(actDel);

  mainMenu->addAction(actImport);
  mainMenu->addAction(actExport);
  mainMenu->addAction(actShare);

  mainMenu->addAction(actMoveUp);
  mainMenu->addAction(actMoveDown);

  mainMenu->addAction(actCopyLink);
  mainMenu->addAction(actRelationshipGraph);
  mainMenu->addAction(actModificationHistory);

  actRename->setVisible(false);
  actDel->setVisible(false);
  actMoveUp->setVisible(false);
  actMoveDown->setVisible(false);

#ifdef Q_OS_ANDROID
  actShare->setVisible(true);
#else
  actShare->setVisible(false);
#endif

  mainMenu->setStyleSheet(m_Method->qssMenu);
}

void NotesList::on_actionModificationHistory() {
  // 读取差异记录
  QList<QJsonObject> allDiffs =
      m_Notes->loadAllDiffs(m_Notes->getCurrentJSON(currentMDFile));

  // 按修改时间排序（最新的在前面）
  std::sort(allDiffs.begin(), allDiffs.end(),
            [](const QJsonObject& a, const QJsonObject& b) {
              return a["modifyTime"].toString() > b["modifyTime"].toString();
            });

  if (mui->qwNoteVersion->source().isEmpty()) {
    mui->qwNoteVersion->rootContext()->setContextProperty("m_NotesList",
                                                          m_NotesList);
    mui->qwNoteVersion->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/NoteVersionList.qml")));
  }

  // 获取 QML 根对象（即 NoteVersionList.qml 中的 Rectangle）
  QQuickItem* rootItem = mui->qwNoteVersion->rootObject();
  if (!rootItem) {
    qWarning() << "获取 QML 根对象失败";
    return;
  }

  mui->frameNoteList->hide();
  mui->frameDiff->show();

  // 获取 QML 中的 ListModel（id: noteVersionModel）
  QObject* versionModel = rootItem->findChild<QObject*>("noteVersionModel");
  if (!versionModel) {
    qWarning() << "获取 noteVersionModel 失败";
    return;
  }

  // 清空旧数据（调用 QML 中的 clearModel() 方法）
  QMetaObject::invokeMethod(versionModel, "clearModel");

  noteDiffTime.clear();
  noteDiffHtml.clear();
  noteDiffPatch.clear();

  // 遍历展示（例如在QListView中显示版本列表）
  foreach (const QJsonObject& diff, allDiffs) {
    QString version = diff["version"].toString();  // 版本（笔记修改时间）
    QString time = diff["modifyTime"].toString();  // 修改时间
    QString html = diff["htmlDiff"].toString();    // 可视化内容
    QString patch = diff["patch"].toString();

    noteDiffTime.append(time);
    noteDiffHtml.append(html);
    noteDiffPatch.append(patch);

    QMetaObject::invokeMethod(
        versionModel, "addRecord",
        Q_ARG(QVariant, version)  // 传入修改时间（QVariant 适配 QML 类型）
    );
  }

  setNoteDiffHtmlToQML("");
}

void NotesList::getNoteDiffHtml() {
  int index = getSelectedVersionIndex();
  if (index < 0) return;

  QString html = noteDiffHtml.at(index);
  setNoteDiffHtmlToQML(html);
}

void NotesList::setNoteDiffHtmlToQML(const QString& html) {
  if (mui->qwNoteDiff->source().isEmpty()) {
    mui->qwNoteDiff->rootContext()->setContextProperty("m_NotesList",
                                                       m_NotesList);
    mui->qwNoteDiff->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/NoteDiffHtmlViewer.qml")));
    mui->qwNoteDiff->setResizeMode(
        QQuickWidget::SizeRootObjectToView);  // 自适应大小
  }
  QQuickItem* rootItem = mui->qwNoteDiff->rootObject();
  if (!rootItem) {
    qWarning() << "获取 QML 根对象失败，无法更新 HTML 内容";
    return;
  }

  // 设置 QML 中的 htmlContent 属性（名称必须与 QML 中完全一致）
  rootItem->setProperty("htmlContent", html);
}

int NotesList::getSelectedVersionIndex() {
  QQuickItem* rootItem = mui->qwNoteVersion->rootObject();
  if (!rootItem) {
    qWarning() << "获取 QML 根对象失败，无法读取选中索引";
    return -1;
  }

  // 读取根对象的 currentSelectedIndex 属性
  // 注意：属性名必须和 QML 中定义的完全一致（currentSelectedIndex）
  QVariant selectedIndexVar = rootItem->property("currentSelectedIndex");
  if (!selectedIndexVar.isValid()) {
    qWarning() << "QML 中未找到 currentSelectedIndex 属性，可能 QML 未正确修改";
    return -1;
  }

  int selectedIndex = selectedIndexVar.toInt();
  qDebug() << "当前选中的列表索引：" << selectedIndex;

  if (selectedIndex == -1) {
    qDebug() << "当前无选中项";
  }

  return selectedIndex;
}

void NotesList::on_actionRelationshipGraph() {
  QFileInfo fi(currentMDFile);
  if (!fi.exists()) return;

  mw_one->showProgress();

  if (m_graphController) {
    m_graphController->setCurrentNotePath(currentMDFile);

  } else {
    mw_one->closeProgress();
  }
}

void NotesList::on_actionCopyNoteLink() {
  int index = m_Method->getCurrentIndexFromQW(mui->qwNoteList);
  QString file = noteModel->getText3(index);
  QString name = noteModel->getText0(index);
  QString strlink = "[" + name + "](" + file + ")";
  QClipboard* clipboard = QApplication::clipboard();
  clipboard->setText(strlink);
  auto msg = std::make_unique<ShowMessage>(this);
  msg->showMsg(appName, strlink, 1);
}

void NotesList::on_actionShareNoteFile() {
  if (QFile::exists(currentMDFile)) {
    mw_one->m_ReceiveShare->shareImage(tr("Share to"), currentMDFile, "*/*");
  }
}

void NotesList::setNoteLabel() {
  QString notesSum = QString::number(getNotesListCount());
  int index_note = getNotesListCurrentIndex();
  QString notesSn = "";
  if (index_note >= 0) notesSn = QString::number(index_note + 1) + "/";
  mui->lblNoteList->setText(notesSn + notesSum);
  int index = getNoteBookCurrentIndex();
  m_Method->modifyItemText3(mui->qwNoteBook, index, notesSum);
}

void NotesList::on_btnMoveTo_clicked() {
  QTreeWidgetItem* item = tw->currentItem();
  if (item == NULL) return;

  int count = item->childCount();
  for (int i = 0; i < count; i++) {
    if (item->child(i)->text(1).isEmpty()) return;
  }

  if (moveItem(tw)) {
    saveNotesList();
    updateAllNoteIndexManager();

    QTimer::singleShot(10, this,
                       [this]() { setCurrentItemFromMDFile(currentMDFile); });
  }
}

bool NotesList::moveItem(QTreeWidget* twMain) {
  QTreeWidgetItem* item = twMain->currentItem();
  if (item == NULL) return false;

  m_MoveTo = new MoveTo(this);
  m_MoveTo->showDialog();
  if (!m_MoveTo->isOk) return false;

  // NoteBook
  if (item->text(1).isEmpty()) {
    QTreeWidgetItem* new_item = item;

    if (m_MoveTo->strCurrentItem == tr("Main Root")) {
      if (item->parent() == NULL) return false;

      item->parent()->removeChild(item);
      tw->addTopLevelItem(new_item);
      tw->setCurrentItem(new_item);
    }

    if (m_MoveTo->strCurrentItem != tr("Main Root")) {
      if (m_MoveTo->currentItem != item) {
        if (item->parent() == NULL)
          tw->takeTopLevelItem(tw->currentIndex().row());
        else
          item->parent()->removeChild(item);

        m_MoveTo->currentItem->addChild(new_item);
        tw->setCurrentItem(new_item);
      }
    }
  }

  // Notes
  if (!item->text(1).isEmpty()) {
    if (m_MoveTo->strCurrentItem == tr("Main Root")) return false;

    QTreeWidgetItem* new_item = item;
    item->parent()->removeChild(item);
    m_MoveTo->currentItem->addChild(new_item);
    tw->setCurrentItem(new_item);
  }

  return true;
}

void NotesList::loadAllRecycle() {
  m_Method->clearAllBakList(mui->qwNoteRecycle);
  int childCount = twrb->topLevelItem(0)->childCount();
  for (int i = 0; i < childCount; i++) {
    QTreeWidgetItem* childItem = twrb->topLevelItem(0)->child(i);
    QString text0 = childItem->text(0);
    QString text3 = iniDir + childItem->text(1);

    m_Method->addItemToQW(mui->qwNoteRecycle, text0, "", "", text3, 0);
  }
}

QVariant NotesList::addQmlTreeTopItem(QString strItem) {
  QQuickItem* root = mui->qwNotesTree->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject*)root, "addTopItem",
                            Q_RETURN_ARG(QVariant, item),
                            Q_ARG(QVariant, strItem));
  return item;
}

QVariant NotesList::addQmlTreeChildItem(QVariant parentItem,
                                        QString strChildItem,
                                        QString iconFile) {
  QQuickItem* root = mui->qwNotesTree->rootObject();
  QVariant item;
  QMetaObject::invokeMethod(
      (QObject*)root, "addChildItem", Q_RETURN_ARG(QVariant, item),
      Q_ARG(QVariant, parentItem), Q_ARG(QVariant, strChildItem),
      Q_ARG(QVariant, iconFile));
  return item;
}

void NotesList::clearQmlTree() {
  QQuickItem* root = mui->qwNotesTree->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "clearAll");
}

void NotesList::initQmlTree() {
  clearQmlTree();

  QString strItem, strChildItem;
  int topcount = tw->topLevelItemCount();
  for (int i = 0; i < topcount; i++) {
    QTreeWidgetItem* topItem = tw->topLevelItem(i);
    strItem = topItem->text(0);
    auto parentItem = addQmlTreeTopItem(strItem);
    int childcount = topItem->childCount();
    for (int j = 0; j < childcount; j++) {
      QTreeWidgetItem* childItem = topItem->child(j);
      strChildItem = childItem->text(0);
      if (childItem->text(1).isEmpty()) {
        auto parentItem2 =
            addQmlTreeChildItem(parentItem, strChildItem, "/res/nb.png");
        int count2 = childItem->childCount();
        for (int n = 0; n < count2; n++) {
          QString str = childItem->child(n)->text(0);
          addQmlTreeChildItem(parentItem2, str, "/res/n.png");
        }
      } else {
        addQmlTreeChildItem(parentItem, strChildItem, "/res/n.png");
      }
    }
  }
}

void NotesList::readyNotesData(QTreeWidgetItem* topItem) {
  // 主线程预收集UI数据（按值捕获到后台线程，安全）
  QVector<QPair<QString, QString>> uiDataList;
  QVector<QTreeWidgetItem*> childItems;
  int child_count = topItem->childCount();
  for (int i = 0; i < child_count; ++i) {
    QTreeWidgetItem* child = topItem->child(i);
    uiDataList.append({child->text(0), child->text(1)});
    childItems.append(child);
  }
  QString iniDirCopy = iniDir;  // 拷贝一份，避免捕获this的成员变量（更安全）

  // 定义后台线程返回的结果类型
  struct RawResult {
    QString text0;
    QString text1;
    QString text2;
    QString text3;
  };

  // 关键：后台线程通过return返回结果，不引用捕获局部变量
  QFuture<QVector<RawResult>> future = QtConcurrent::run([=]() {
    qDebug() << "后台处理开始，共" << child_count << "项";
    QVector<RawResult> rawResults;  // 后台线程内部创建，生命周期由线程管理
    rawResults.reserve(child_count);

    for (int i = 0; i < child_count; ++i) {
      const auto& uiData = uiDataList[i];
      QString text0 = uiData.first;
      QString text3 = uiData.second;

      if (!text3.isEmpty()) {
        QString file =
            iniDirCopy + text3;  // 使用拷贝的iniDir，避免访问this成员
        QString item1 = m_Method->getLastModified(file);
        QString strSize = m_Method->getFileSize(QFile(file).size(), 2);
        QString text2 = "";
        if (!QFile::exists(file)) text2 = tr("File does not exist");

        rawResults.push_back({text0, item1 + " " + strSize, text2, text3});
      }
    }
    qDebug() << "后台处理完成，有效数据" << rawResults.size() << "项";
    return rawResults;  // 返回结果，由QFuture管理
  });

  // 主线程接收后台返回的结果
  QFutureWatcher<QVector<RawResult>>* watcher =
      new QFutureWatcher<QVector<RawResult>>(this);
  connect(watcher, &QFutureWatcher<QVector<RawResult>>::finished, this, [=]() {
    qDebug() << "主线程开始处理结果";

    // 获取后台线程返回的结果（安全，由QFuture保证生命周期）
    QVector<RawResult> rawResults = watcher->result();

    // 构造NoteItem并更新模型
    QList<NoteItem> batchItems;
    batchItems.reserve(rawResults.size());

    // 迭代器循环
    for (auto it = rawResults.constBegin(); it != rawResults.constEnd(); ++it) {
      const RawResult& result = *it;
      NoteItem item;
      item.text0 = result.text0;
      item.text1 = result.text1;
      item.text2 = result.text2;
      item.text3 = result.text3;
      item.myh = 0;
      batchItems.append(item);
    }

    noteModel->addBatchItems(batchItems);
    pNoteItems.append(childItems.begin(), childItems.end());

    // 其他UI操作

    int index = m_Method->getCurrentIndexFromQW(mui->qwNoteBook);
    int noteslistIndex = getSavedNotesListIndex(index);
    setNotesListCurrentIndex(noteslistIndex);

    if (isImportNotes) {
      int noteCount = getNotesListCount();
      setNotesListCurrentIndex(noteCount - 1);
      isImportNotes = false;
    }

    setNoteLabel();
    clickNoteList();
    if (isMouseClick) {
      setNotesListCurrentIndex(-1);
      isMouseClick = false;
    }

    isReadyNoteDataEnd = true;

    watcher->deleteLater();

    qDebug() << "主线程处理结果完成！";
  });
  watcher->setFuture(future);
}

void NotesList::clickNoteBook() {
  int count = m_Method->getCountFromQW(mui->qwNoteBook);
  if (count <= 0) return;

  int index = m_Method->getCurrentIndexFromQW(mui->qwNoteBook);
  if (index < 0) return;

  mui->lblNoteBook->setText(QString::number(index + 1) + "/" +
                            QString::number(count));

  pNoteItems.clear();
  isActColorFlagStatus = true;

  m_Method->clearAllBakList(mui->qwNoteList);

  QString text1 = m_Method->getText1(mui->qwNoteBook, index);
  QString text2 = m_Method->getText2(mui->qwNoteBook, index);
  if (text2.isEmpty()) {
    int index_top = text1.toInt();
    QTreeWidgetItem* topItem = tw->topLevelItem(index_top);

    readyNotesData(topItem);
  }
}

void NotesList::clickNoteList() {
  int indexTop = m_Method->getCurrentIndexFromQW(mui->qwNoteBook);
  int index = m_Method->getCurrentIndexFromQW(mui->qwNoteList);

  if (index < 0) {
    currentMDFile = "";
    return;
  }

  if (indexTop < 0) return;

  QString strMD = m_Method->getText3(mui->qwNoteList, index);
  currentMDFile = iniDir + strMD;

  setNoteLabel();

  if (!QFile::exists(currentMDFile)) {
    /*ShowMessage *msg = new ShowMessage(mw_one);
    msg->showMsg(appName,
                 tr("The current note does not exist. Please select another "
                    "note or create a new note."),
                 0);*/

    return;
  }

  QString noteName = m_Method->getText0(mui->qwNoteList, index);
  noteTitle = noteName;

  QTreeWidgetItem* itemTop = tw->topLevelItem(indexTop);
  QTreeWidgetItem* item = itemTop->child(index);
  tw->setCurrentItem(item);

  int indexNoteBook = m_Method->getCurrentIndexFromQW(mui->qwNoteBook);
  if (indexNoteBook < 0) return;

  QString s_tr = QString::number(indexNoteBook) + "=" + QString::number(index);
  int count2 = mIndexList.count();
  int i = 0;
  for (i = 0; i < count2; i++) {
    QString str = mIndexList.at(i);
    if (str.split("=").at(0).toInt() == indexNoteBook) {
      mIndexList.removeOne(str);
      break;
    }
  }
  mIndexList.append(s_tr);
}

void NotesList::mouseClickNoteBook() {
  isMouseClick = true;
  clickNoteBook();
}

void NotesList::saveNotesListIndex() {
  QJsonObject root;
  QJsonArray array;

  for (const QString& item : std::as_const(mIndexList)) {
    array.append(item);
  }

  root["list"] = array;

  QFile file(privateDir + QDir::separator() + "noteslistindex.json");
  if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QJsonDocument doc(root);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
  } else {
    qWarning() << "无法打开文件进行写入:" << file.errorString();
  }
}

void NotesList::loadNotesListIndex() {
  QFile file(privateDir + QDir::separator() + "noteslistindex.json");
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning() << "无法打开文件进行读取:" << file.errorString();
    return;
  }

  QByteArray data = file.readAll();
  file.close();

  QJsonDocument doc = QJsonDocument::fromJson(data);
  if (doc.isNull()) {
    qWarning() << "JSON 解析失败";
    return;
  }

  QJsonObject root = doc.object();
  QJsonArray array = root["list"].toArray();

  mIndexList.clear();
  for (const QJsonValue& value : std::as_const(array)) {
    if (value.isString()) {
      mIndexList.append(value.toString());
    }
  }
}

int NotesList::getSavedNotesListIndex(int notebookIndex) {
  int index = 0;
  int in0, in1;
  for (int i = 0; i < mIndexList.count(); i++) {
    QString str = mIndexList.at(i);
    in0 = str.split("=").at(0).toInt();
    in1 = str.split("=").at(1).toInt();

    if (notebookIndex == in0) {
      index = in1;
      break;
    }
  }

  int count = m_Method->getCountFromQW(mui->qwNoteList);
  if (count > 0) {
    if (index < 0) index = 0;
    if (index > count - 1) index = count - 1;
  } else {
    index = -1;
  }

  return index;
}

void NotesList::genRecentOpenMenu() {
  menuRecentOpen = new QMenu(this);
  menuRecentOpen->setMaximumWidth(mw_one->width());
  int count = listRecentOpen.count();
  for (int i = 0; i < count; i++) {
    QString name, file, item;
    item = listRecentOpen.at(i);
    QStringList list = item.split("===");
    name = list.at(0);
    file = iniDir + list.at(1);

    if (QFile::exists(file)) {
      QFontMetrics fm(mw_one->font());
      QString txt = QString::number(i + 1) + " . " + name;
      QString menuTitle =
          fm.elidedText(txt, Qt::ElideRight, mw_one->width() - 30);
      QAction* act = new QAction(menuTitle, menuRecentOpen);
      menuRecentOpen->addAction(act);

      connect(act, &QAction::triggered, this, [=]() {
        currentMDFile = file;
        noteTitle = name;

#ifdef Q_OS_ANDROID
        mw_one->on_btnOpenNote_clicked();
#else
        mw_one->on_btnEditNote_clicked();
#endif

        saveCurrentNoteInfo();

        setCurrentItemFromMDFile(currentMDFile);
      });
    }
  }

  int x = 0;
  x = mw_one->geometry().x() + 2;
  int y = mw_one->geometry().y() + mui->btnRecentOpen->height() + 4;
  QPoint pos(x, y);
  menuRecentOpen->exec(pos);
}

void NotesList::updateNoteIndexManager(QString mdFile, int notebookIndex,
                                       int noteIndex) {
  m_Notes->m_NoteIndexManager->setNotebookIndex(mdFile, notebookIndex);
  m_Notes->m_NoteIndexManager->setNoteIndex(mdFile, noteIndex);
}

void NotesList::updateAllNoteIndexManager() {
  int topCount = tw->topLevelItemCount();
  for (int i = 0; i < topCount; i++) {
    QTreeWidgetItem* topItem = tw->topLevelItem(i);
    int childCount = topItem->childCount();
    for (int j = 0; j < childCount; j++) {
      QTreeWidgetItem* childItem = topItem->child(j);
      QString mdFile = iniDir + childItem->text(1);
      QString title = childItem->text(0);
      m_Notes->m_NoteIndexManager->setNoteTitle(mdFile, title);
      updateNoteIndexManager(mdFile, i, j);
    }
  }
}

bool NotesList::setCurrentItemFromMDFile(QString mdFile) {
  if (!QFile::exists(mdFile)) return false;

  int indexNoteBook, indexNote, countNoteBook, countNotes;
  indexNoteBook = m_Notes->m_NoteIndexManager->getNotebookIndex(mdFile);
  indexNote = m_Notes->m_NoteIndexManager->getNoteIndex(mdFile);
  countNoteBook = m_Method->getCountFromQW(mui->qwNoteBook);

  if (indexNoteBook < 0) return false;
  if (indexNoteBook >= countNoteBook) return false;

  setNoteBookCurrentIndex(indexNoteBook);

  isReadyNoteDataEnd = false;
  clickNoteBook();

  while (!isReadyNoteDataEnd)
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

  countNotes = m_Method->getCountFromQW(mui->qwNoteList);
  if (indexNote < 0 || indexNote >= countNotes) return false;

  setNotesListCurrentIndex(indexNote);
  clickNoteList();

  qDebug() << indexNoteBook << indexNote
           << m_Notes->m_NoteIndexManager->getNoteTitle(mdFile) << mdFile;

  return true;
}

QString NotesList::getCurrentNoteNameFromMDFile(QString mdFile) {
  return m_Notes->m_NoteIndexManager->getNoteTitle(mdFile);
}

void NotesList::genCursorText() {
  // 直接从文件读取内容（避免创建临时UI组件）
  QString strBuffer = loadText(currentMDFile);
  if (strBuffer.isEmpty()) {
    qWarning() << "Loaded text is empty for file:" << currentMDFile;
    return;
  }

  int curPos = mw_one->m_ReceiveShare->getCursorPos();
  curPos = qBound(0, curPos, strBuffer.length());

  // 使用QString方法替代QTextCursor操作
  QString str0 = strBuffer.mid(qMax(0, curPos - 5), 5);
  QString str1 = strBuffer.mid(curPos, 5);

  QString curText =
      QString::number(curPos) + "  (\"" + str0 + "|" + str1 + "\"" + ")";
  QString filePath = privateDir + "cursor_text.txt";

  // 使用Qt的原子写入函数
  if (!safeWriteFile(filePath, curText)) {
    qCritical() << "Failed to write cursor_text file:" << filePath;
  } else {
    qDebug() << "cursor_text saved:" << curText;
  }
}

// 安全的文件写入函数
bool NotesList::safeWriteFile(const QString& filePath, const QString& content) {
  QTemporaryFile tempFile;
  if (tempFile.open()) {
    QTextStream stream(&tempFile);
    stream << content;
    tempFile.close();

    // 原子操作：替换原文件
    return QFile::rename(tempFile.fileName(), filePath);
  }
  return false;
}

QStringList NotesList::extractLocalImagesFromMarkdown(const QString& filePath) {
  QStringList images;
  QFile file(filePath);

  // 打开文件
  if (!file.open(QIODevice::ReadOnly)) {
    return images;  // 返回空列表如果打开失败
  }

  QTextStream in(&file);
  QString content = in.readAll();
  file.close();

  // 正则表达式匹配所有图片路径
  QRegularExpression re("!\\[.*?\\]\\((.*?)\\)");
  QRegularExpressionMatchIterator matchIterator = re.globalMatch(content);

  while (matchIterator.hasNext()) {
    QRegularExpressionMatch match = matchIterator.next();
    QString captured = match.captured(1).trimmed();  // 去除首尾空格

    // 提取第一个空格前的部分作为路径（处理可能存在的标题）
    QString path = captured.section(' ', 0, 0);

    // 过滤网络路径并检查非空
    if (!path.startsWith("http://") && !path.startsWith("https://") &&
        !path.isEmpty()) {
      images.append(path);
    }
  }

  // 可选：去重
  images.removeDuplicates();

  qDebug() << "images=" << images;

  return images;
}

void NotesList::onSearchTextChanged(const QString& text) {
  QTimer::singleShot(300, this, [this, text]() {  // 防抖处理
    auto results =
        m_dbManager.searchDocuments(text, m_Notes->m_NoteIndexManager);
    m_searchModel.setResults(results);
    mui->lblNoteSearchResult->setText(tr("Note Search Results:") +
                                      QString::number(results.count()));
  });
}

void NotesList::openSearch() { return; }

QString NotesList::getSearchResultQmlFile() {
  QQuickItem* root = mui->qwNotesSearchResult->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject*)root, "getQmlCurrentMDFile",
                            Q_RETURN_ARG(QVariant, item));
  return item.toString();
}

QStringList NotesList::getValidMDFiles() { return validMDFiles; }
template class QFutureWatcher<ResultsMap>;

void NotesList::showFindNotes() {
  recycleNotesList.clear();
  int count = ui->treeWidgetRecycle->topLevelItem(0)->childCount();
  for (int i = 0; i < count; i++) {
    QString file =
        iniDir + ui->treeWidgetRecycle->topLevelItem(0)->child(i)->text(1);
    recycleNotesList.append(file);
  }
  qDebug() << "recycle notes = " << recycleNotesList;

  mui->frameNoteList->hide();
  mui->frameNotesSearchResult->show();
  mui->editNotesSearch->setFocus();

  openSearch();
}

void NotesList::restoreNoteFromRecycle() {
  int count = m_Method->getCountFromQW(mui->qwNoteRecycle);
  if (count == 0) return;

  int index = m_Method->getCurrentIndexFromQW(mui->qwNoteRecycle);
  if (index < 0) return;

  if (getNoteBookCount() == 0) return;

  setTWRBCurrentItem();
  on_btnRestore_clicked();
}

void NotesList::initNoteGraphView() {
  // 1. 先初始化图谱组件（注册QML类型和单例）
  registerNoteGraphTypes();  // 注册 NoteGraphModel 和 NoteRelationParser
  initializeNoteGraph();     // 注册 NoteGraphController 单例

  // 2. 确保 QQuickWidget 已初始化
  if (!mui->qwNoteGraphView) {
    qWarning() << "QQuickWidget 未初始化";
    return;
  }

  // 在加载 QML 前，先设置一个占位符属性 ***
  // 这可以避免 QML 在引擎初始化早期阶段因访问未定义属性而报错
  // 使用 nullptr 作为初始值
  mui->qwNoteGraphView->rootContext()->setContextProperty(
      "graphController", QVariant::fromValue<QObject*>(nullptr));
  mui->qwNoteGraphView->rootContext()->setContextProperty("isDark", isDark);

  // 3. 加载 QML 源文件（这一步会触发 QML 引擎初始化）
  // 此时，QML 引擎已经知道 graphController 这个属性的存在（虽然是 nullptr）
  mui->qwNoteGraphView->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/NoteGraphView.qml")));

  // 4. 获取 QML 引擎（此时引擎已初始化）
  QQmlEngine* engine = mui->qwNoteGraphView->engine();
  if (!engine) {
    qWarning() << "无法获取 QML 引擎";
    // 即使引擎获取失败，我们也已经设置了占位符，避免 QML 报错
    return;
  }

  // 5. 尝试获取 NoteGraphController 单例（Qt 6 正确写法）
  NoteGraphController* controller =
      engine->singletonInstance<NoteGraphController*>("NoteGraph",
                                                      "NoteGraphController");
  if (!controller) {
    qWarning() << "无法从引擎获取 NoteGraphController 单例";
    // 注意：这里不再 return，而是继续将 nullptr 设置给属性
    // 这样 QML 中的检查 (graphController && ...) 可以处理控制器不可用的情况
  } else {
    qDebug() << "成功获取 NoteGraphController 单例指针:" << controller;
  }

  // 6. 无论 controller 是否为空，都更新上下文属性 ***
  // 如果 controller 有效，QML 现在可以访问到真正的控制器实例。
  // 如果 controller 是 nullptr，QML 中的空值检查可以防止崩溃。
  m_graphController = controller;  // 更新成员变量
  mui->qwNoteGraphView->rootContext()->setContextProperty("graphController",
                                                          m_graphController);
  qDebug() << "已将 graphController 上下文属性设置为:" << m_graphController;

  // 7. 如果 controller 有效，则连接节点双击信号（打开对应笔记）
  if (m_graphController) {
    connect(m_graphController, &NoteGraphController::nodeDoubleClicked, this,
            &NotesList::onNoteNodeDoubleClicked);
    qDebug() << "已连接 NoteGraphController 的 nodeDoubleClicked 信号";
  } else {
    qWarning() << "NoteGraphController 为空，未连接 nodeDoubleClicked 信号";
  }
}

// 节点双击事件处理（打开对应的笔记）
void NotesList::onNoteNodeDoubleClicked(const QString& filePath) {
  qDebug() << "打开笔记：" << filePath;
  QFileInfo fi(filePath);
  if (fi.exists()) {
    currentMDFile = filePath;
  } else {
    currentMDFile = iniDir + filePath;
  }

  m_Notes->previewNote();
}

void NotesList::moveToFirst() {
  return;

  ////////////////////////////////////////////////////////

  int indexNote = m_Method->getCurrentIndexFromQW(mui->qwNoteList);
  if (indexNote <= 0) return;
  int countNote = m_Method->getCountFromQW(mui->qwNoteList);
  if (countNote == 1) return;

  QTreeWidgetItem* item = tw->currentItem();
  if (item == NULL) return;

  if (item->parent() != NULL) {
    QTreeWidgetItem* parentItem = item->parent();
    parentItem->removeChild(item);
    parentItem->insertChild(0, item);
    tw->setCurrentItem(item);

    resetQML_List();
    saveNotesList();

    updateAllNoteIndexManager();

    setNotesListCurrentIndex(0);
  }
}

void NotesList::qmlOpenEdit() { mui->btnEditNote->click(); }

QStringList NotesList::getRecycleNoteFiles() {
  QStringList cycleFiles;
  QTreeWidgetItem* cycleTopItem = twrb->topLevelItem(0);
  int count = cycleTopItem->childCount();
  for (int i = 0; i < count; i++) {
    QString filePath = iniDir + cycleTopItem->child(i)->text(1);
    cycleFiles.append(filePath);
  }
  return cycleFiles;
}

void NotesList::newtextToOldtextFromDiffStr() {
  int targetIndex = getSelectedVersionIndex();
  if (targetIndex < 0) {
    qDebug() << "无效的版本索引";
    return;
  }

  // 检查补丁列表是否足够（需要 targetIndex 个补丁才能反推到该版本）
  if (targetIndex >= noteDiffPatch.size()) {
    qDebug() << "补丁数量不足，无法反推到指定版本";
    return;
  }

  // 从最新版本（index 0）开始
  QString currentText = loadText(currentMDFile);
  bool allSuccess = true;

  // 依次应用补丁反推：从 index 0 → 1 → ... → targetIndex
  for (int i = 0; i <= targetIndex; ++i) {
    QString patchStr = noteDiffPatch.at(i);
    QVector<bool> revertSuccess;

    // 调用反推函数，用当前文本和补丁得到上一版本文本
    currentText = m_Notes->m_NoteDiffManager.revertPatchToOldText(
        currentText, patchStr, revertSuccess);

    // 检查当前步骤是否成功
    if (revertSuccess.contains(false)) {
      allSuccess = false;
      qWarning() << "反推版本" << i << "时部分补丁应用失败";
      // 可以选择继续或中断，这里选择继续尝试
    }
  }

  // 处理最终结果
  QString html = markdownToHtmlWithMath(currentText);
  html = html.replace("images/", "file:///" + iniDir + "memo/images/");

  if (allSuccess) {
    qDebug() << "成功反推到版本" << targetIndex;
    setNoteDiffHtmlToQML(html);
  } else {
    qDebug() << "反推完成，但部分步骤存在问题，结果可能不准确";
    setNoteDiffHtmlToQML(html);  // 仍显示结果，但可能有误差
  }
}

void NotesList::closeNoteDiff() {
  noteDiffHtml.clear();
  noteDiffPatch.clear();
  noteDiffTime.clear();
  mui->frameDiff->hide();
  mui->frameNoteList->show();
}

void NotesList::showNoteBookMenu(int x, int y) {
  menuNoteBook = new QMenu(this);
  init_NoteBookMenu(menuNoteBook);

  QPoint pos(mw_one->geometry().x() + x, mw_one->geometry().y() + y);
  menuNoteBook->exec(pos);
}

void NotesList::showNotsListMenu(int x, int y) {
  menuNoteList = new QMenu(this);
  init_NotesListMenu(menuNoteList);

  QPoint pos(mw_one->geometry().x() + x, mw_one->geometry().y() + y);
  menuNoteList->exec(pos);
}
