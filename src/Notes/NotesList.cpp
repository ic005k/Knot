#include "NotesList.h"

#include "src/MainWindow.h"
#include "ui_MainWindow.h"
#include "ui_NewNoteBook.h"
#include "ui_NotesList.h"

extern MainWindow *mw_one;
extern Method *m_Method;
extern QString iniDir, privateDir, currentMDFile, appName, errorInfo;
extern bool isAndroid, isDark;
extern int fontSize;

extern QString loadText(QString textFile);
extern QString getTextEditLineText(QTextEdit *txtEdit, int i);
extern void TextEditToFile(QTextEdit *txtEdit, QString fileName);
extern void StringToFile(QString buffers, QString fileName);

QString strNoteNameIndexFile = "";

NotesList::NotesList(QWidget *parent) : QDialog(parent), ui(new Ui::NotesList) {
  ui->setupUi(this);
  this->installEventFilter(this);

  strNoteNameIndexFile = privateDir + "MyNoteNameIndex";

  // 注册模型到 QML
  qmlRegisterType<SearchResultModel>("com.example", 1, 0, "SearchResultModel");
  qRegisterMetaType<KeywordPosition>("KeywordPosition");
  qRegisterMetaType<QList<KeywordPosition>>("QList<KeywordPosition>");

  connect(pAndroidKeyboard, &QInputMethod::visibleChanged, this,
          &NotesList::on_KVChanged);

  tw = ui->treeWidget;
  twrb = ui->treeWidgetRecycle;

  mw_one->ui->f_FindNotes->setStyleSheet(
      "QFrame{background-color: #455364;color: #FFFFFF;border-radius:10px; "
      "border:0px solid gray;}");
  mw_one->ui->f_Tools->setStyleSheet(mw_one->ui->f_FindNotes->styleSheet());

  setModal(true);
  this->layout()->setSpacing(5);
  this->layout()->setContentsMargins(2, 2, 2, 2);
  ui->frame1->hide();

  tw->setVerticalScrollMode(QTreeWidget::ScrollPerPixel);
  QScroller::grabGesture(tw, QScroller::LeftMouseButtonGesture);
  m_Method->setSCrollPro(tw);

  twrb->setVerticalScrollMode(QTreeWidget::ScrollPerPixel);
  QScroller::grabGesture(twrb, QScroller::LeftMouseButtonGesture);
  m_Method->setSCrollPro(twrb);

  ui->treeWidget->headerItem()->setText(0, tr("Notebook"));
  ui->treeWidget->setColumnHidden(1, true);
  twrb->header()->hide();
  twrb->setColumnHidden(1, true);
  twrb->setColumnWidth(0, 180);

  set_memo_dir();

  QFont font = this->font();
  font.setPointSize(fontSize - 1);
  font.setBold(true);
  ui->lblCount->setFont(font);

  ui->btnFind->hide();
  ui->editFind->hide();
  ui->lblCount->hide();
  ui->btnNewNote->hide();
  ui->btnNewNoteBook->hide();
  ui->editBook->hide();
  ui->editNote->hide();
  ui->btnImport->hide();
  ui->btnExport->hide();
  ui->editName->hide();

  mw_one->ui->btnManagement->setHidden(true);

  QScroller::grabGesture(ui->editName, QScroller::LeftMouseButtonGesture);
  m_Method->setSCrollPro(ui->editName);

  initNotesList();
  initRecycle();

  m_searchResultModel = new SearchResultModel(this);
  mw_one->ui->qwNotesSearchResult->rootContext()->setContextProperty(
      "searchResultModel", m_searchResultModel);
  m_searchEngine = new NotesSearchEngine(this);
  connect(m_searchEngine, &NotesSearchEngine::indexBuildFinished, this,
          &NotesList::onSearchIndexReady);

  // 连接搜索框
  connect(mw_one->ui->editNotesSearch, &QLineEdit::textChanged, this,
          &NotesList::onSearchTextChanged);

  loadIndexTimestamp();
  m_searchEngine->loadIndex(privateDir + "MyNotesIndex");

  m_dbManager.initDatabase(privateDir + "md_database_v3.db");
  mw_one->ui->qwNotesSearchResult->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/SearchResults.qml")));
  mw_one->ui->qwNotesSearchResult->rootContext()->setContextProperty(
      "searchModel", &m_searchModel);
  m_dbManager.updateFilesIndex(iniDir + "memo");
}

NotesList::~NotesList() { delete ui; }

void NotesList::set_memo_dir() {
  QString path = iniDir + "memo/";
  QDir dir(path);
  if (!dir.exists()) dir.mkdir(path);
}

bool NotesList::eventFilter(QObject *watch, QEvent *evn) {
  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(evn);
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
      QTreeWidgetItem *item = tw->currentItem();
      on_treeWidget_itemClicked(item, 0);
      return true;
    }
  }

  return QWidget::eventFilter(watch, evn);
}

void NotesList::on_btnClose_clicked() { this->close(); }

void NotesList::on_btnNewNoteBook_clicked() {
  QTreeWidgetItem *item = new QTreeWidgetItem();
  item->setText(0, ui->editBook->text().trimmed());
  item->setForeground(0, Qt::red);
  item->setIcon(0, QIcon(":/res/nb.png"));

  if (rootIndex == 0) {
    ui->treeWidget->addTopLevelItem(item);
    ui->treeWidget->setCurrentItem(item);
  } else {
    QTreeWidgetItem *topItem = tw->topLevelItem(rootIndex - 1);
    tw->setCurrentItem(topItem);
    topItem->addChild(item);
    tw->setCurrentItem(item);
  }
}

void NotesList::on_btnNewNote_clicked() {
  if (ui->treeWidget->topLevelItemCount() == 0) return;

  int rand = QRandomGenerator::global()->generate();
  if (rand < 0) rand = 0 - rand;

  QString noteFile = "memo/" + mw_one->m_Notes->getDateTimeStr() + "_" +
                     QString::number(rand) + ".md";
  QTreeWidgetItem *parentitem = ui->treeWidget->currentItem();

  QTreeWidgetItem *item1 = new QTreeWidgetItem(parentitem);
  item1->setText(0, ui->editNote->text().trimmed());
  item1->setText(1, noteFile);
  item1->setIcon(0, QIcon(":/res/n.png"));

  QTextEdit *edit = new QTextEdit();
  edit->append("");
  TextEditToFile(edit, iniDir + noteFile);

  ui->treeWidget->setCurrentItem(item1);
  on_treeWidget_itemClicked(item1, 0);

  pNoteItems.clear();
  int count = parentitem->childCount();
  for (int i = 0; i < count; i++) {
    pNoteItems.append(parentitem->child(i));
  }
}

void NotesList::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column) {
  Q_UNUSED(column);

  if (ui->treeWidget->topLevelItemCount() == 0) return;

  ui->editName->setText(item->text(0));

  return;

  if (item->parent() != NULL) {
    if (tw->currentIndex().row() == 0) {
      if (tw->currentIndex().parent().row() == 0) {
      }
    }

    QSettings Reg(iniDir + "curmd.ini", QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    Reg.setIniCodec("utf-8");
#endif

    QString curmd = item->text(1);
    Reg.setValue("/MainNotes/currentItem", curmd);

    currentMDFile = iniDir + curmd;

    setNoteName(item->text(0));
  }
}

QString NotesList::getCurrentMDFile() {
  QSettings Reg(iniDir + "curmd.ini", QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  Reg.setIniCodec("utf-8");
#endif

  QString curmd = Reg.value("/MainNotes/currentItem", "memo/xxx.md").toString();
  mw_one->ui->lblNoteName->setText(
      Reg.value("/MainNotes/NoteName", tr("Note Name")).toString());

  return iniDir + curmd;
}

void NotesList::on_btnRename_clicked() {
  QTreeWidgetItem *item = ui->treeWidget->currentItem();
  if (item == NULL) return;

  QDialog *dlg = new QDialog(this);
  QVBoxLayout *vbox0 = new QVBoxLayout;
  dlg->setLayout(vbox0);
  vbox0->setContentsMargins(5, 5, 5, 5);
  dlg->setModal(true);
  dlg->setWindowFlag(Qt::FramelessWindowHint);

  QFrame *frame = new QFrame(this);
  vbox0->addWidget(frame);

  QVBoxLayout *vbox = new QVBoxLayout;

  frame->setLayout(vbox);
  vbox->setContentsMargins(6, 6, 6, 10);
  vbox->setSpacing(10);

  QLabel *lblTitle = new QLabel(this);
  lblTitle->adjustSize();
  lblTitle->setWordWrap(true);
  lblTitle->setText(tr("Rename"));
  vbox->addWidget(lblTitle);

  QFrame *hframe = new QFrame(this);
  hframe->setFrameShape(QFrame::HLine);
  hframe->setStyleSheet("QFrame{background:red;min-height:2px}");
  vbox->addWidget(hframe);
  hframe->hide();

  QTextEdit *edit = new QTextEdit(this);
  vbox->addWidget(edit);
  edit->setPlainText(item->text(0));
  QScroller::grabGesture(edit, QScroller::LeftMouseButtonGesture);
  edit->horizontalScrollBar()->setHidden(true);
  edit->verticalScrollBar()->setStyleSheet(
      mw_one->ui->editDetails->verticalScrollBar()->styleSheet());
  m_Method->setSCrollPro(edit);

  if (edit->toPlainText().trimmed().length() == 0) {
    edit->setPlainText(mw_one->m_Notes->new_title);
  }

  QToolButton *btnCancel = new QToolButton(this);
  QToolButton *btnPaste = new QToolButton(this);
  QToolButton *btnCopy = new QToolButton(this);
  QToolButton *btnOk = new QToolButton(this);
  btnCancel->setText(tr("Cancel"));
  btnPaste->setText(tr("Paste"));
  btnCopy->setText(tr("Copy"));
  btnOk->setText(tr("OK"));

  btnOk->setFixedHeight(35);
  btnCancel->setFixedHeight(35);
  btnCopy->setFixedHeight(35);
  btnPaste->setFixedHeight(35);

  QHBoxLayout *hbox = new QHBoxLayout;
  hbox->addWidget(btnCancel);
  hbox->addWidget(btnPaste);
  hbox->addWidget(btnCopy);
  hbox->addWidget(btnOk);
  btnCancel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  btnPaste->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  btnCopy->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  btnOk->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

  QSpacerItem *sparcer_item =
      new QSpacerItem(0, 60, QSizePolicy::Fixed, QSizePolicy::Expanding);
  vbox->addItem(sparcer_item);

  vbox->addLayout(hbox, 0);

  connect(btnCancel, &QToolButton::clicked, [=]() mutable { dlg->close(); });
  connect(dlg, &QDialog::rejected,
          [=]() mutable { m_Method->closeGrayWindows(); });
  connect(dlg, &QDialog::accepted,
          [=]() mutable { m_Method->closeGrayWindows(); });
  connect(btnPaste, &QToolButton::clicked, [=]() mutable { edit->paste(); });
  connect(btnCopy, &QToolButton::clicked, [=]() mutable {
    edit->selectAll();
    edit->copy();
    dlg->close();
  });
  connect(btnOk, &QToolButton::clicked, [=]() mutable {
    renameCurrentItem(edit->toPlainText().trimmed());

    saveNotesList();
    dlg->close();
  });

  int x, y, w, h;
  h = mw_one->height() / 3;
  if (isAndroid) {
    w = mw_one->width() - 2;
    y = mw_one->geometry().y();
  } else {
    w = mw_one->width();
    if (w > 300)
      w = 300;
    else
      w = mw_one->width() - 20;
    y = mw_one->geometry().y() + (mw_one->height() - h) / 2;
  }

  x = mw_one->geometry().x() + (mw_one->width() - w) / 2;
  dlg->setGeometry(x, y, w, h);
  dlg->setModal(true);
  mw_one->set_ToolButtonStyle(dlg);

  m_Method->m_widget = new QWidget(mw_one);
  m_Method->showGrayWindows();

  dlg->show();
}

void NotesList::renameCurrentItem(QString title) {
  QTreeWidgetItem *item = ui->treeWidget->currentItem();
  if (item == NULL) return;

  item->setText(0, title.trimmed());
  if (item->parent() != NULL && !item->text(1).isEmpty()) {
    setNoteName(item->text(0));

    mw_one->m_Notes->m_NoteIndexManager->setNoteTitle(iniDir + item->text(1),
                                                      item->text(0));
    mw_one->m_Notes->m_NoteIndexManager->saveIndex(strNoteNameIndexFile);

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

void NotesList::setNoteName(QString name) {
  mw_one->ui->lblNoteName->adjustSize();
  mw_one->ui->lblNoteName->setWordWrap(true);
  mw_one->ui->lblNoteName->setText(name);
  mw_one->ui->lblNoteName->setToolTip(name);
}

void NotesList::on_btnDel_clicked() {
  if (tw->topLevelItemCount() == 0) return;

  QTreeWidgetItem *item = tw->currentItem();

  if (item == NULL) return;

  QString strFlag;
  if (item->text(1).isEmpty())
    strFlag = tr("NoteBook");
  else
    strFlag = tr("Note");

  m_Method->m_widget = new QWidget(this);
  ShowMessage *m_ShowMsg = new ShowMessage(this);
  if (!m_ShowMsg->showMsg("Knot",
                          tr("Move to the recycle bin?") + "\n\n" + strFlag +
                              " : " + item->text(0),
                          2)) {
    return;
  }

  QString str0, str1;
  // Top Item
  if (item->parent() == NULL) {
    int count = item->childCount();
    int index = tw->currentIndex().row();

    for (int i = 0; i < count; i++) {
      QTreeWidgetItem *childItem = new QTreeWidgetItem;

      str0 = item->child(i)->text(0);
      str1 = item->child(i)->text(1);

      // Child Notes
      if (!str1.isEmpty()) {
        childItem->setText(0, str0);
        childItem->setText(1, str1);
        addItem(twrb, childItem);

        // Child NoteBook
      } else {
        int count1 = item->child(i)->childCount();
        for (int j = 0; j < count1; j++) {
          str0 = item->child(i)->child(j)->text(0);
          str1 = item->child(i)->child(j)->text(1);
          QTreeWidgetItem *childItem = new QTreeWidgetItem;
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
      QTreeWidgetItem *childItem = new QTreeWidgetItem;
      childItem->setText(0, str0);
      childItem->setText(1, str1);
      addItem(twrb, childItem);

      // Child NoteBook
    } else {
      int count = item->childCount();
      for (int n = 0; n < count; n++) {
        str0 = item->child(n)->text(0);
        str1 = item->child(n)->text(1);
        QTreeWidgetItem *childItem = new QTreeWidgetItem;
        childItem->setText(0, str0);
        childItem->setText(1, str1);
        addItem(twrb, childItem);
      }
    }

    item->parent()->removeChild(item);
  }

  if (tw->topLevelItemCount() > 0) {
    if (tw->currentItem()->childCount() == 0) {
      mw_one->m_Notes->loadEmptyNote();
    }
  } else {
    mw_one->m_Notes->loadEmptyNote();
    mw_one->ui->lblNoteBook->setText(tr("Note Book"));
    mw_one->ui->lblNoteList->setText(tr("Note List"));
  }

  tw->setFocus();

  saveNotesList();

  saveRecycle();

  resetQML_List();
}

void NotesList::addItem(QTreeWidget *tw, QTreeWidgetItem *item) {
  item->setIcon(0, QIcon(":/res/n.png"));
  tw->setFocus();
  if (tw == twrb) {
    tw->setCurrentItem(tw->topLevelItem(0));
    QTreeWidgetItem *curItem = tw->currentItem();
    curItem->addChild(item);
  } else {
    QTreeWidgetItem *curItem = tw->currentItem();
    curItem->addChild(item);
  }

  tw->expandAll();
}

void NotesList::delFile(QString file) {
  QFile _file(file);
  if (_file.exists()) _file.remove();
  _file.close();
}

bool NotesList::on_btnImport_clicked() {
  if (ui->treeWidget->topLevelItemCount() == 0) return false;

  QStringList fileNames;
  fileNames =
      QFileDialog::getOpenFileNames(this, tr("Knot"), "", tr("MD File (*.*)"));

  if (fileNames.at(0).isNull()) return false;

  QTreeWidgetItem *item = ui->treeWidget->currentItem();
  for (int i = 0; i < fileNames.count(); i++) {
    QString fileName = fileNames.at(i);

    bool isMD = false;
    QString strInfo;
    isMD = fileName.contains(".md");
    strInfo = fileName;
#ifdef Q_OS_ANDROID
    QString fileAndroid = mw_one->m_Reader->getUriRealPath(fileName);
    isMD = fileAndroid.contains(".md");

    QStringList list = fileAndroid.split("/");
    QString str = list.at(list.count() - 1);
    if (str.toInt() > 0) isMD = true;
    strInfo = fileAndroid;

#endif

    if (!isMD) {
      m_Method->m_widget = new QWidget(this);
      ShowMessage *m_ShowMsg = new ShowMessage(this);
      m_ShowMsg->showMsg("Knot",
                         tr("Invalid Markdown file.") + "\n\n" + strInfo, 1);
    }

    if (QFile(fileName).exists() && isMD) {
      QTreeWidgetItem *item1;
      QFileInfo fi(strInfo);
      QString name = fi.fileName();
      QString suffix = fi.suffix();
      name.replace("." + suffix, "");

      item1 = new QTreeWidgetItem(item);
      item1->setText(0, name);

      tw->setCurrentItem(item1);

      QString a = "memo/" + mw_one->m_Notes->getDateTimeStr() + "_" +
                  QString::number(i) + ".md";
      currentMDFile = iniDir + a;
      QString str = loadText(fileName);
      QTextEdit *edit = new QTextEdit();
      edit->setAcceptRichText(false);
      edit->setPlainText(str);
      TextEditToFile(edit, currentMDFile);

      item1->setText(1, a);

      qDebug() << fileName << a;
    }
  }

  return true;
}

void NotesList::on_btnExport_clicked() {
  if (ui->treeWidget->topLevelItemCount() == 0) return;

  QTreeWidgetItem *item = tw->currentItem();
  if (item->parent() == NULL) return;

  QString name = item->text(0);
  name = name + ".md";
  QString fileName;
  QFileDialog fd;
  fileName = fd.getSaveFileName(this, name, name, tr("MD File(*.*)"));

  if (fileName == "") return;

  QString mdfile = iniDir + item->text(1);

  QString str = loadText(mdfile);
  QTextEdit *edit = new QTextEdit();
  edit->setAcceptRichText(false);
  edit->setPlainText(str);

  TextEditToFile(edit, fileName);
}

void NotesList::closeEvent(QCloseEvent *event) { Q_UNUSED(event); }

void NotesList::resetQML_List() {
  if (tw->topLevelItemCount() == 0) {
    m_Method->clearAllBakList(mw_one->ui->qwNoteBook);
    m_Method->clearAllBakList(mw_one->ui->qwNoteList);
    return;
  }

  loadAllNoteBook();

  int index = 0;
  QTreeWidgetItem *item = tw->currentItem();
  QTreeWidgetItem *pNoteBook = NULL;
  QTreeWidgetItem *pNote = NULL;
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

  int count = m_Method->getCountFromQW(mw_one->ui->qwNoteBook);

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
  QTreeWidgetItem *item = twrb->currentItem();
  if (item->parent() == NULL) {
    index = -1;
  } else {
    index = twrb->currentIndex().row();
  }

  m_Method->setCurrentIndexFromQW(mw_one->ui->qwNoteRecycle, index);
}

void NotesList::initRecentOpen() {
  QSettings iniFile(iniDir + "recentopen.ini", QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  iniFile.setIniCodec("utf-8");
#endif
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
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  iniFile.setIniCodec("utf-8");
#endif

  iniFile.setValue("/RecentOpen/count", listRecentOpen.count());
  for (int i = 0; i < listRecentOpen.count(); i++) {
    iniFile.setValue("/RecentOpen/list" + QString::number(i),
                     listRecentOpen.at(i));
  }
}

void NotesList::saveCurrentNoteInfo() {
  QSettings Reg(iniDir + "curmd.ini", QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  Reg.setIniCodec("utf-8");
#endif

  QString str = currentMDFile;
  QString iniName = str.replace(iniDir, "");

  Reg.setValue("/MainNotes/currentItem", iniName);
  Reg.setValue("/MainNotes/NoteName", mw_one->ui->lblNoteName->text());
}

void NotesList::saveNoteBookVPos() {
  QSettings iniFile(privateDir + "notes.ini", QSettings::IniFormat);
  iniFile.setValue("/NoteBook/VPos",
                   m_Method->getVPosForQW(mw_one->ui->qwNoteBook));
}

void NotesList::setNoteBookVPos() {
  QSettings iniFile(privateDir + "notes.ini", QSettings::IniFormat);
  qreal txtPos = iniFile.value("/NoteBook/VPos", 0).toReal();
  m_Method->setVPosForQW(mw_one->ui->qwNoteBook, txtPos);
}

void NotesList::saveNotesList() {
  QSettings iniNotes(iniDir + "mainnotes.ini", QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  iniNotes.setIniCodec("utf-8");
#endif

  mw_one->isNeedAutoBackup = true;
  mw_one->strLatestModify = tr("Modi Notes List");

  int count = tw->topLevelItemCount();
  iniNotes.setValue("/MainNotes/topItemCount", count);
  for (int i = 0; i < count; i++) {
    QTreeWidgetItem *topItem = tw->topLevelItem(i);
    QString strtop = topItem->text(0);
    iniNotes.setValue("/MainNotes/strTopItem" + QString::number(i), strtop);

    int childCount = topItem->childCount();
    iniNotes.setValue("/MainNotes/childCount" + QString::number(i), childCount);

    for (int j = 0; j < childCount; j++) {
      QTreeWidgetItem *childItem = topItem->child(j);
      QString strChild0 = childItem->text(0);
      QString strChild1 = childItem->text(1);

      if (!strChild1.isEmpty()) {
        iniNotes.setValue(
            "/MainNotes/childItem0" + QString::number(i) + QString::number(j),
            strChild0);
        iniNotes.setValue(
            "/MainNotes/childItem1" + QString::number(i) + QString::number(j),
            strChild1);
      } else {
        int count = childItem->childCount();
        iniNotes.setValue(
            "/MainNotes/childCount" + QString::number(i) + QString::number(j),
            count);
        iniNotes.setValue(
            "/MainNotes/childItem0" + QString::number(i) + QString::number(j),
            strChild0);
        iniNotes.setValue(
            "/MainNotes/childItem1" + QString::number(i) + QString::number(j),
            "");
        for (int n = 0; n < count; n++) {
          QString strChild00 = childItem->child(n)->text(0);
          QString strChild11 = childItem->child(n)->text(1);

          iniNotes.setValue("/MainNotes/childItem0" + QString::number(i) +
                                QString::number(j) + QString::number(n),
                            strChild00);
          iniNotes.setValue("/MainNotes/childItem1" + QString::number(i) +
                                QString::number(j) + QString::number(n),
                            strChild11);
        }
      }
    }
  }

  // Save Note Name
  QSettings Reg1(iniDir + "curmd.ini", QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  Reg1.setIniCodec("utf-8");
#endif

  Reg1.setValue("/MainNotes/NoteName", mw_one->ui->lblNoteName->text());
}

void NotesList::saveRecycle() {
  QSettings iniNotes(iniDir + "mainnotes.ini", QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  iniNotes.setIniCodec("utf-8");
#endif

  mw_one->isNeedAutoBackup = true;
  mw_one->strLatestModify = tr("Modi Notes Recycle");

  int i = 0;
  QTreeWidgetItem *topItem = twrb->topLevelItem(i);
  int childCount = topItem->childCount();
  iniNotes.setValue("/MainNotes/rbchildCount" + QString::number(i), childCount);

  for (int j = 0; j < childCount; j++) {
    QTreeWidgetItem *childItem = twrb->topLevelItem(i)->child(j);
    QString strChild0 = childItem->text(0);
    QString strChild1 = childItem->text(1);

    iniNotes.setValue(
        "/MainNotes/rbchildItem0" + QString::number(i) + QString::number(j),
        strChild0);
    iniNotes.setValue(
        "/MainNotes/rbchildItem1" + QString::number(i) + QString::number(j),
        strChild1);
  }
}

void NotesList::initNotesList() {
  tw->clear();

  QSettings *iniNotes =
      new QSettings(iniDir + "mainnotes.ini", QSettings::IniFormat, NULL);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  iniNotes->setIniCodec("utf-8");
#endif

  int topCount = iniNotes->value("/MainNotes/topItemCount").toInt();
  int nNoteBook = topCount;

  int notesTotal = 0;
  QString str0, str1;
  for (int i = 0; i < topCount; i++) {
    QString strTop =
        iniNotes->value("/MainNotes/strTopItem" + QString::number(i))
            .toString();
    int childCount =
        iniNotes->value("/MainNotes/childCount" + QString::number(i)).toInt();
    notesTotal = notesTotal + childCount;

    QTreeWidgetItem *topItem = new QTreeWidgetItem;
    topItem->setText(0, strTop);
    topItem->setForeground(0, Qt::red);
    QFont font = this->font();
    font.setBold(true);
    topItem->setFont(0, font);
    topItem->setIcon(0, QIcon(":/res/nb.png"));

    for (int j = 0; j < childCount; j++) {
      str0 = iniNotes
                 ->value("/MainNotes/childItem0" + QString::number(i) +
                         QString::number(j))
                 .toString();
      str1 = iniNotes
                 ->value("/MainNotes/childItem1" + QString::number(i) +
                         QString::number(j))
                 .toString();

      if (!str1.isEmpty()) {
        QTreeWidgetItem *childItem = new QTreeWidgetItem(topItem);
        childItem->setText(0, str0);
        childItem->setText(1, str1);
        childItem->setIcon(0, QIcon(":/res/n.png"));

        mw_one->m_Notes->m_NoteIndexManager->setNoteTitle(iniDir + str1, str0);

      } else {
        QTreeWidgetItem *childItem = new QTreeWidgetItem(topItem);
        childItem->setText(0, str0);
        childItem->setText(1, "");
        childItem->setForeground(0, Qt::red);
        childItem->setFont(0, font);
        childItem->setIcon(0, QIcon(":/res/nb.png"));

        nNoteBook++;
        notesTotal--;

        int count = iniNotes
                        ->value("/MainNotes/childCount" + QString::number(i) +
                                QString::number(j))
                        .toInt();
        for (int n = 0; n < count; n++) {
          QString str00, str11;
          str00 = iniNotes
                      ->value("/MainNotes/childItem0" + QString::number(i) +
                              QString::number(j) + QString::number(n))
                      .toString();
          str11 = iniNotes
                      ->value("/MainNotes/childItem1" + QString::number(i) +
                              QString::number(j) + QString::number(n))
                      .toString();
          QTreeWidgetItem *item = new QTreeWidgetItem(childItem);
          item->setText(0, str00);
          item->setText(1, str11);
          item->setIcon(0, QIcon(":/res/n.png"));

          mw_one->m_Notes->m_NoteIndexManager->setNoteTitle(iniDir + str11,
                                                            str00);

          notesTotal++;
        }
      }
    }
    tw->addTopLevelItem(topItem);
  }

  tw->headerItem()->setText(
      0, tr("Notebook") + " : " + QString::number(nNoteBook) + "  " +
             tr("Notes") + " : " + QString::number(notesTotal));
  tw->expandAll();

  mw_one->m_Notes->m_NoteIndexManager->saveIndex(strNoteNameIndexFile);

  initRecentOpen();
}

void NotesList::initRecycle() {
  twrb->clear();

  QSettings iniNotes(iniDir + "mainnotes.ini", QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  iniNotes.setIniCodec("utf-8");
#endif

  int i = 0;
  QTreeWidgetItem *topItem = new QTreeWidgetItem;
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

    QTreeWidgetItem *childItem = new QTreeWidgetItem(topItem);
    childItem->setText(0, str0);
    childItem->setText(1, str1);
    childItem->setIcon(0, QIcon(":/res/n.png"));
  }
  twrb->addTopLevelItem(topItem);

  twrb->expandAll();
}

void NotesList::on_btnRecycle_clicked() {
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
  QTreeWidgetItem *curItem = twrb->currentItem();
  if (curItem->parent() == NULL) return;

  if (moveItem(twrb)) {
    resetQML_List();
    clickNoteList();
    if (!ui->frame1->isHidden()) {
      on_btnBack_clicked();
    }
    if (!mw_one->ui->frameNoteRecycle->isHidden()) {
      mw_one->on_btnBackNoteRecycle_clicked();
    }
  } else
    return;

  saveRecycle();

  saveNotesList();
}

void NotesList::on_btnDel_Recycle_clicked() {
  QTreeWidgetItem *curItem = twrb->currentItem();
  if (curItem->parent() == NULL) {
    return;
  } else {
    m_Method->m_widget = new QWidget(this);
    ShowMessage *m_ShowMsg = new ShowMessage(this);
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

    m_dbManager.deleteFileIndex(md);

    delFile(md);

    curItem->parent()->removeChild(curItem);
  }

  saveRecycle();

  resetQML_Recycle();
}

void NotesList::setWinPos() {
  int w = mw_one->width();
  int x = mw_one->geometry().x();
  this->setGeometry(x, mw_one->geometry().y(), w, mw_one->height());
  mw_one->ui->btnBackNotes->hide();
  mw_one->ui->btnEdit->hide();
  mw_one->ui->btnNotesList->hide();
  mw_one->ui->btnSetKey->hide();
  mw_one->ui->btnPDF->hide();
}

void NotesList::clearFiles() {
  QFile::remove(iniDir + "memo.zip");
  QString tempDir = iniDir;
  knot_all_files.clear();
  QStringList fmt = QString("zip;md;html;jpg;bmp;png;ini").split(';');
  getAllFiles(tempDir, knot_all_files, fmt);

  clearMD_Pic(tw);
  clearMD_Pic(twrb);

  int count = knot_all_files.count();
  for (int i = 0; i < count; i++) {
    QString filePath = knot_all_files.at(i);

    QFile file(filePath);
    if (filePath.contains(".sync-conflict-") || filePath.contains(".png")) {
      file.remove();
    }
  }
}

void NotesList::clearInvalidMDFile() {
  QStringList tempList = knot_all_files;
  for (const QString &item : validMDFiles) {
    tempList.removeAll(item);
  }

  QDir dir;
  QString path = privateDir + "invalid_md/";
  dir.mkpath(path);
  for (int i = 0; i < tempList.count(); i++) {
    QString m_file = tempList.at(i);
    QFileInfo fi(m_file);
    if (fi.suffix() == "md") {
      QString new_file = path + fi.fileName();
      QFile::remove(new_file);
      QFile::copy(m_file, new_file);
      QFile::remove(m_file);
    }
  }
}

void NotesList::clearMD_Pic(QTreeWidget *tw) {
  for (int i = 0; i < tw->topLevelItemCount(); i++) {
    QTreeWidgetItem *topItem = tw->topLevelItem(i);
    int childCount = topItem->childCount();
    for (int j = 0; j < childCount; j++) {
      QTreeWidgetItem *childItem = topItem->child(j);
      if (!childItem->text(1).isEmpty()) {
        QString str = childItem->text(1);
        removePicFromMD(iniDir + str);
      } else {
        int count1 = childItem->childCount();
        for (int n = 0; n < count1; n++) {
          QString str = childItem->child(n)->text(1);
          removePicFromMD(iniDir + str);
        }
      }
    }
  }
}

void NotesList::removePicFromMD(QString mdfile) {
  QString txt = loadText(mdfile);

  for (int i = 0; i < knot_all_files.count(); i++) {
    QString str0 = knot_all_files.at(i);
    str0.replace(iniDir + "memo/", "");

    if (txt.contains(str0)) {
      knot_all_files.removeAt(i);
      i = 0;
    }
  }
}

void NotesList::getAllFiles(const QString &foldPath, QStringList &folds,
                            const QStringList &formats) {
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
QStringList findMarkdownFiles(const QString &dirPath) {
  Q_UNUSED(dirPath);

  QList<QString> paths;
  QDir dir(iniDir + "memo/");
  QStringList files = dir.entryList(QStringList() << "*.md", QDir::Files);
  for (const QString &file : files) {
    QFileInfo info(dir.absoluteFilePath(file));
    QString canonicalPath = info.canonicalFilePath();  // 规范化路径
    if (!canonicalPath.isEmpty() && info.exists()) {
      paths.append(canonicalPath);
    }
  }
  // 使用 QSet 去重
  return QSet<QString>(paths.begin(), paths.end()).values();
}

MySearchResult searchInFile(const QString &filePath,
                            const QRegularExpression &regex) {
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
void reduceResults(ResultsMap &result, const MySearchResult &partial) {
  if (!partial.filePath.isEmpty()) {
    result[partial.filePath] = partial;
  }
}

// 主搜索函数
QFuture<ResultsMap> performSearchAsync(const QString &dirPath,
                                       const QString &keyword) {
  QStringList files = findMarkdownFiles(dirPath);
  QRegularExpression regex(keyword,
                           QRegularExpression::CaseInsensitiveOption |
                               QRegularExpression::UseUnicodePropertiesOption);

  return QtConcurrent::mappedReduced(  // 注意改用 mappedReduced 非阻塞版本
      files, SearchMapper(regex), reduceResults,
      QtConcurrent::ReduceOption::UnorderedReduce);
}

void displayResults(const ResultsMap &results) {
  for (auto it = results.begin(); it != results.end(); ++it) {
    qDebug() << "文件：" << it.key();
    qDebug() << "匹配行号：" << it.value().lineNumbers;
    QString file = it.key();
    QList<int> lineNumbersList = it.value().lineNumbers;
    QString strLine;
    for (int i = 0; i < lineNumbersList.count(); i++) {
      strLine = strLine + " " + QString::number(lineNumbersList.at(i));
    }
    mw_one->m_NotesList->searchResultList.append(file + "-==-" +
                                                 strLine.trimmed());
  }
  qDebug() << mw_one->m_NotesList->searchResultList;

  mw_one->m_NotesList->showNotesSearchResult();
}

void NotesList::showNotesSearchResult() {
  if (searchResultList.count() == 0) {
    mw_one->closeProgress();
    return;
  }

  mw_one->ui->frameNoteList->hide();
  mw_one->ui->frameNotesSearchResult->show();

  m_Method->clearAllBakList(mw_one->ui->qwNotesSearchResult);
  for (int i = 0; i < searchResultList.count(); i++) {
    QStringList list = searchResultList.at(i).split("-==-");
    QString note_name = getCurrentNoteNameFromMDFile(list.at(0));
    if (note_name != "")
      m_Method->addItemToQW(mw_one->ui->qwNotesSearchResult, note_name,
                            list.at(0), list.at(1), "", 0);
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
    mw_one->ui->btnFindNextNote->setEnabled(false);
    mw_one->ui->btnFindPreviousNote->setEnabled(false);
    mw_one->ui->lblShowLineSn->setText("0");
    mw_one->ui->lblFindNoteCount->setText("0");
    ShowMessage *msg = new ShowMessage(this);
    msg->showMsg("Knot", tr("No match was found."), 1);

    return;
  }

  m_Method->clearAllBakList(mw_one->ui->qwNotesSearchResult);

  // ▶️ 处理搜索结果

  for (auto it = results.constBegin(); it != results.constEnd(); ++it) {
    const QString &filePath = it.key();
    const QList<int> lines = it.value().lineNumbers;

    qDebug() << "文件：" << it.key();
    qDebug() << "匹配行号：" << it.value().lineNumbers;

    QString strLineSn;
    for (int i = 0; i < lines.count(); i++) {
      strLineSn = strLineSn + " " + QString::number(lines.at(i));
    }
    strLineSn = strLineSn.trimmed();

    // QString note_name = getCurrentNoteNameFromMDFile(filePath);
    // if (note_name != "")
    //   m_Method->addItemToQW(mw_one->ui->qwNotesSearchResult, note_name,
    //                         filePath, strLineSn, "", 0);

    if (!recycleNotesList.contains(filePath))
      searchResultList.append(filePath + "-==-" + strLineSn);
  }

  // ▶️ 释放资源
  watcher->deleteLater();

  // mw_one->ui->frameNoteList->hide();
  // mw_one->ui->frameNotesSearchResult->show();

  if (searchResultList.count() > 0) {
    mw_one->ui->btnFindNextNote->setEnabled(true);
    mw_one->ui->btnFindPreviousNote->setEnabled(true);
    mw_one->ui->lblFindNoteCount->setText(
        QString::number(searchResultList.count()));

    goNext();
  }

  mw_one->closeProgress();
}

void NotesList::startFind(QString strFind) {
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

void NotesList::goPrevious() {
  findCount = findCount - 1;
  if (findCount < 0) findCount = searchResultList.count() - 1;

  QStringList list = searchResultList.at(findCount).split("-==-");
  QString md_file = list.at(0);
  mw_one->ui->lblShowLineSn->setText(list.at(1));
  setCurrentItemFromMDFile(md_file);
  clickNoteList();

  mw_one->ui->lblFindNoteCount->setText(
      QString::number(findCount + 1) + " -> " +
      QString::number(searchResultList.count()));

  if (pAndroidKeyboard->isVisible()) pAndroidKeyboard->hide();
}

void NotesList::goNext() {
  findCount = findCount + 1;

  if (findCount > searchResultList.count() - 1) findCount = 0;

  QStringList list = searchResultList.at(findCount).split("-==-");
  QString md_file = list.at(0);
  mw_one->ui->lblShowLineSn->setText(list.at(1));
  setCurrentItemFromMDFile(md_file);
  clickNoteList();

  mw_one->ui->lblFindNoteCount->setText(
      QString::number(findCount + 1) + " -> " +
      QString::number(searchResultList.count()));

  if (pAndroidKeyboard->isVisible()) pAndroidKeyboard->hide();
}

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

  mw_one->ui->lblFindNoteCount->setText(QString::number(findCount + 1) +
                                        " -> " +
                                        QString::number(findResult.count()));
}

void NotesList::on_btnFind_clicked() {
  QString strFind = ui->editFind->text().trimmed().toLower();
  if (strFind == "") {
    mw_one->ui->btnFindNextNote->setEnabled(false);
    mw_one->ui->btnFindPreviousNote->setEnabled(false);
    return;
  }
  findResultList.clear();
  int count = tw->topLevelItemCount();
  for (int i = 0; i < count; i++) {
    QTreeWidgetItem *topItem = tw->topLevelItem(i);
    if (topItem->text(0).toLower().contains(strFind)) {
      findResultList.append(topItem);
      qDebug() << topItem->text(0);
    }
    int childCount = topItem->childCount();
    for (int j = 0; j < childCount; j++) {
      QTreeWidgetItem *childItem = topItem->child(j);
      if (childItem->text(0).toLower().contains(strFind)) {
        findResultList.append(childItem);
        qDebug() << childItem->text(0);
      }

      QString str1 = childItem->text(1);
      if (str1.isEmpty()) {
        int count = childItem->childCount();
        for (int n = 0; n < count; n++) {
          QTreeWidgetItem *item = childItem->child(n);
          if (item->text(0).toLower().contains(strFind)) {
            findResultList.append(item);
            qDebug() << item->text(0);
          }
        }
      }
    }
  }

  if (findResultList.count() > 0) {
    tw->setCurrentItem(findResultList.at(0));
    tw->scrollToItem(tw->currentItem());

    localItem();

    findCount = 0;
    ui->lblCount->setText(QString::number(findCount + 1) + "->" +
                          QString::number(findResultList.count()));

    mw_one->ui->lblFindNoteCount->setText(ui->lblCount->text());

    mw_one->ui->btnFindNextNote->setEnabled(true);
    mw_one->ui->btnFindPreviousNote->setEnabled(true);
  } else {
    mw_one->ui->btnFindNextNote->setEnabled(false);
    mw_one->ui->btnFindPreviousNote->setEnabled(false);

    ui->lblCount->setText("0");

    mw_one->ui->lblFindNoteCount->setText(ui->lblCount->text());

    setNoteBookCurrentIndex(-1);
    setNotesListCurrentIndex(-1);
  }
}

void NotesList::localItem() {
  QTreeWidgetItem *item = tw->currentItem();
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
        QTreeWidgetItem *item1 = item->parent()->child(i);
        if (item1 == item) break;

        if (item1->text(1).isEmpty()) newRow++;
      }

      setNoteBookCurrentIndex(index + newRow + 1);
    }

    clickNoteBook();
    setNotesListCurrentIndex(-1);

    // Notes
  } else {
    QTreeWidgetItem *top_item = item->parent()->parent();
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
        QTreeWidgetItem *item1 = top_item->child(i);
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
  return m_Method->getText0(mw_one->ui->qwNoteBook, index);
}

QString NotesList::getNotesListText0(int index) {
  return m_Method->getText0(mw_one->ui->qwNoteList, index);
}

void NotesList::modifyNoteBookText0(QString text0, int index) {
  m_Method->modifyItemText0(mw_one->ui->qwNoteBook, index, text0);
}

void NotesList::modifyNotesListText0(QString text0, int index) {
  m_Method->modifyItemText0(mw_one->ui->qwNoteList, index, text0);
}

void NotesList::on_editFind_textChanged(const QString &arg1) {
  if (arg1.trimmed() == "") {
    ui->lblCount->setText("0");
    mw_one->ui->lblFindNoteCount->setText("0");
  }
  on_btnFind_clicked();
}

void NotesList::on_editFind_returnPressed() {}

void NotesList::on_KVChanged() {
  if (!pAndroidKeyboard->isVisible()) {
  } else {
  }
}

void NotesList::moveBy(int ud) {
  QTreeWidgetItem *item = tw->currentItem();

  if (item == NULL) return;

  if (item->parent() != NULL) {
    QTreeWidgetItem *parentItem = item->parent();
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
}

void NotesList::on_btnUp_clicked() { moveBy(-1); }

void NotesList::on_btnDown_clicked() { moveBy(1); }

void NotesList::setTWRBCurrentItem() {
  int index = m_Method->getCurrentIndexFromQW(mw_one->ui->qwNoteRecycle);
  QTreeWidgetItem *topItem = twrb->topLevelItem(0);
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

  QString text1 = m_Method->getText1(mw_one->ui->qwNoteBook, index);
  QString text2 = m_Method->getText2(mw_one->ui->qwNoteBook, index);
  if (text2.isEmpty()) {
    tw->setCurrentItem(tw->topLevelItem(text1.toInt()));

  } else {
    QStringList list = text1.split("===");
    int indexMain, indexChild;
    if (list.count() == 2) {
      indexMain = list.at(0).toInt();
      indexChild = list.at(1).toInt();

      QTreeWidgetItem *topItem = tw->topLevelItem(indexMain);
      QTreeWidgetItem *childItem = topItem->child(indexChild);
      tw->setCurrentItem(childItem);
    }
  }
}

void NotesList::on_actionAdd_NoteBook_triggered() {
  QString text;

  NewNoteBook *m_new = new NewNoteBook(mw_one);
  text = m_new->notebookName;
  if (m_new->isOk && !text.isEmpty()) {
    rootIndex = m_new->rootIndex;
    ui->editBook->setText(text);
    on_btnNewNoteBook_clicked();

    loadAllNoteBook();

    int count = m_Method->getCountFromQW(mw_one->ui->qwNoteBook);
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

  saveRecycle();

  saveNotesList();

  if (count == 0) {
    mw_one->m_Notes->loadEmptyNote();
  }
}

void NotesList::on_actionRename_NoteBook_triggered() {
  int index = getNoteBookCurrentIndex();
  if (index < 0) return;

  on_btnRename_clicked();
}

int NotesList::getNoteBookCount() {
  int count = m_Method->getCountFromQW(mw_one->ui->qwNoteBook);
  return count;
}

int NotesList::getNotesListCount() {
  int count = m_Method->getCountFromQW(mw_one->ui->qwNoteList);
  return count;
}

int NotesList::getNoteBookCurrentIndex() {
  int index = m_Method->getCurrentIndexFromQW(mw_one->ui->qwNoteBook);
  return index;
}

int NotesList::getNotesListCurrentIndex() {
  int index = m_Method->getCurrentIndexFromQW(mw_one->ui->qwNoteList);
  return index;
}

void NotesList::setNoteBookCurrentIndex(int index) {
  m_Method->setCurrentIndexFromQW(mw_one->ui->qwNoteBook, index);
}

void NotesList::setNotesListCurrentIndex(int index) {
  m_Method->setCurrentIndexFromQW(mw_one->ui->qwNoteList, index);
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
  QTreeWidgetItem *item = tw->currentItem();
  int index = 0;
  if (item->parent() == NULL) {
    index = tw->indexOfTopLevelItem(item);
  } else {
    int index0 = tw->indexOfTopLevelItem(item->parent());
    int index1 = 0;
    int count = item->parent()->childCount();
    for (int i = 0; i < count; i++) {
      QTreeWidgetItem *item0 = item->parent()->child(i);
      if (item0 == item) break;

      if (item0->text(1).isEmpty()) index1++;
    }

    index = index0 + index1 + 1;
  }

  return index;
}

void NotesList::loadAllNoteBook() {
  pNoteBookItems.clear();

  m_Method->clearAllBakList(mw_one->ui->qwNoteBook);
  m_Method->clearAllBakList(mw_one->ui->qwNoteList);
  int count = tw->topLevelItemCount();
  for (int i = 0; i < count; i++) {
    QTreeWidgetItem *topItem = tw->topLevelItem(i);
    QString str = topItem->text(0);

    int sum = topItem->childCount();
    int child_count = sum;
    for (int n = 0; n < child_count; n++) {
      if (topItem->child(n)->text(1).isEmpty()) sum--;
    }

    QString strSum = QString::number(sum);
    m_Method->addItemToQW(mw_one->ui->qwNoteBook, str, QString::number(i), "",
                          strSum, fontSize);
    pNoteBookItems.append(topItem);

    int childCount = topItem->childCount();
    for (int j = 0; j < childCount; j++) {
      QTreeWidgetItem *childItem = topItem->child(j);
      if (childItem->text(1).isEmpty()) {
        m_Method->addItemToQW(
            mw_one->ui->qwNoteBook, childItem->text(0),
            QString::number(i) + "===" + QString::number(j), "isNoteBook",
            QString::number(childItem->childCount()), fontSize - 1);

        pNoteBookItems.append(childItem);
      }
    }
  }
}

void NotesList::init_NoteBookMenu(QMenu *mainMenu) {
  QAction *actNew = new QAction(tr("New NoteBook"));
  QAction *actDel = new QAction(tr("Del NoteBook"));
  QAction *actRename = new QAction(tr("Rename NoteBook"));
  QAction *actMoveUp = new QAction(tr("Move Up"));
  QAction *actMoveDown = new QAction(tr("Move Down"));

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

  mainMenu->addAction(actNew);
  mainMenu->addAction(actRename);
  mainMenu->addAction(actDel);

  mainMenu->addAction(actMoveUp);
  mainMenu->addAction(actMoveDown);

  actRename->setVisible(false);
  actDel->setVisible(false);
  actMoveUp->setVisible(false);
  actMoveDown->setVisible(false);

  mainMenu->setStyleSheet(m_Method->qssMenu);
}

void NotesList::on_actionAdd_Note_triggered() {
  int notebookIndex = getNoteBookCurrentIndex();
  if (notebookIndex < 0) return;

  QString text = "";

  tw->setCurrentItem(pNoteBookItems.at(notebookIndex));
  ui->editNote->setText(text);

  on_btnNewNote_clicked();

  QTreeWidgetItem *childItem = tw->currentItem();
  QString text3 = childItem->text(1);
  m_Method->addItemToQW(mw_one->ui->qwNoteList, text, "", "", text3, 0);

  int count = getNotesListCount();
  setNotesListCurrentIndex(count - 1);

  clickNoteList();
  mw_one->on_btnEditNote_clicked();

  setNoteLabel();
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

  saveRecycle();

  saveNotesList();

  if (count == 0) {
    mw_one->m_Notes->loadEmptyNote();
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
  bool isOk = on_btnImport_clicked();

  if (isOk) {
    clickNoteBook();
    setNotesListCurrentIndex(getNotesListCount() - 1);
    clickNoteList();

    mw_one->m_Notes->updateMDFileToSyncLists(currentMDFile);
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

void NotesList::init_NotesListMenu(QMenu *mainMenu) {
  QAction *actNew = new QAction(tr("New Note"));
  QAction *actDel = new QAction(tr("Del Note"));
  QAction *actRename = new QAction(tr("Rename Note"));
  QAction *actMoveUp = new QAction(tr("Move Up"));
  QAction *actMoveDown = new QAction(tr("Move Down"));
  QAction *actImport = new QAction(tr("Import"));
  QAction *actExport = new QAction(tr("Export"));
  QAction *actShare = new QAction(tr("Share"));

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

  mainMenu->addAction(actNew);
  mainMenu->addAction(actRename);
  mainMenu->addAction(actDel);

  mainMenu->addAction(actImport);
  mainMenu->addAction(actExport);
  mainMenu->addAction(actShare);

  mainMenu->addAction(actMoveUp);
  mainMenu->addAction(actMoveDown);

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

void NotesList::on_actionShareNoteFile() {
  if (QFile::exists(currentMDFile)) {
    mw_one->m_ReceiveShare->shareImage(tr("Share to"), currentMDFile, "*/*");
  }
}

void NotesList::setNoteLabel() {
  mw_one->ui->lblNoteBook->setText("" + QString::number(getNoteBookCount()));
  QString notesSum = QString::number(getNotesListCount());
  mw_one->ui->lblNoteList->setText("" + notesSum);
  int index = getNoteBookCurrentIndex();
  m_Method->modifyItemText3(mw_one->ui->qwNoteBook, index, notesSum);
}

void NotesList::on_btnMoveTo_clicked() {
  QTreeWidgetItem *item = tw->currentItem();
  if (item == NULL) return;

  int count = item->childCount();
  for (int i = 0; i < count; i++) {
    if (item->child(i)->text(1).isEmpty()) return;
  }

  if (moveItem(tw)) {
    resetQML_List();
    saveNotesList();
  }
}

bool NotesList::moveItem(QTreeWidget *twMain) {
  QTreeWidgetItem *item = twMain->currentItem();
  if (item == NULL) return false;

  MoveTo *m_MoveTo = new MoveTo(this);
  if (!m_MoveTo->isOk) return false;

  // NoteBook
  if (item->text(1).isEmpty()) {
    QTreeWidgetItem *new_item = item;

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

    QTreeWidgetItem *new_item = item;
    item->parent()->removeChild(item);
    m_MoveTo->currentItem->addChild(new_item);
    tw->setCurrentItem(new_item);
  }

  return true;
}

void NotesList::loadAllRecycle() {
  m_Method->clearAllBakList(mw_one->ui->qwNoteRecycle);
  int childCount = twrb->topLevelItem(0)->childCount();
  for (int i = 0; i < childCount; i++) {
    QTreeWidgetItem *childItem = twrb->topLevelItem(0)->child(i);
    QString text0 = childItem->text(0);
    QString text3 = childItem->text(1);

    m_Method->addItemToQW(mw_one->ui->qwNoteRecycle, text0, "", "", text3, 0);
  }
}

void NotesList::localNotesItem() {
  int topcount = tw->topLevelItemCount();
  QString mdfile;
  QTreeWidgetItem *curChildItem = NULL;
  QTreeWidgetItem *curParentItem = NULL;
  for (int i = 0; i < topcount; i++) {
    QTreeWidgetItem *topItem = tw->topLevelItem(i);
    int count = topItem->childCount();
    for (int j = 0; j < count; j++) {
      QTreeWidgetItem *childItem = topItem->child(j);
      mdfile = childItem->text(1);
      if (mdfile.isEmpty()) {
        int count1 = childItem->childCount();
        for (int n = 0; n < count1; n++) {
          mdfile = childItem->child(n)->text(1);
          if (iniDir + mdfile == currentMDFile) {
            curChildItem = childItem->child(n);
            curParentItem = childItem;
            break;
          }
        }
      } else {
        mdfile = childItem->text(1);
        if (iniDir + mdfile == currentMDFile) {
          curChildItem = childItem;
          curParentItem = topItem;
          break;
        }
      }
    }
  }

  int count0 = pNoteBookItems.count();
  for (int i = 0; i < count0; i++) {
    if (pNoteBookItems.at(i) == curParentItem) {
      setNoteBookCurrentIndex(i);
      break;
    }
  }

  clickNoteBook();

  int count1 = pNoteItems.count();
  for (int i = 0; i < count1; i++) {
    if (pNoteItems.at(i) == curChildItem) {
      setNotesListCurrentIndex(i);
      break;
    }
  }
}

QVariant NotesList::addQmlTreeTopItem(QString strItem) {
  QQuickItem *root = mw_one->ui->qwNotesTree->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject *)root, "addTopItem",
                            Q_RETURN_ARG(QVariant, item),
                            Q_ARG(QVariant, strItem));
  return item;
}

QVariant NotesList::addQmlTreeChildItem(QVariant parentItem,
                                        QString strChildItem,
                                        QString iconFile) {
  QQuickItem *root = mw_one->ui->qwNotesTree->rootObject();
  QVariant item;
  QMetaObject::invokeMethod(
      (QObject *)root, "addChildItem", Q_RETURN_ARG(QVariant, item),
      Q_ARG(QVariant, parentItem), Q_ARG(QVariant, strChildItem),
      Q_ARG(QVariant, iconFile));
  return item;
}

void NotesList::clearQmlTree() {
  QQuickItem *root = mw_one->ui->qwNotesTree->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "clearAll");
}

void NotesList::initQmlTree() {
  clearQmlTree();

  QString strItem, strChildItem;
  int topcount = tw->topLevelItemCount();
  for (int i = 0; i < topcount; i++) {
    QTreeWidgetItem *topItem = tw->topLevelItem(i);
    strItem = topItem->text(0);
    auto parentItem = addQmlTreeTopItem(strItem);
    int childcount = topItem->childCount();
    for (int j = 0; j < childcount; j++) {
      QTreeWidgetItem *childItem = topItem->child(j);
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

void NotesList::clickNoteBook() {
  pNoteItems.clear();

  m_Method->clearAllBakList(mw_one->ui->qwNoteList);
  int index = m_Method->getCurrentIndexFromQW(mw_one->ui->qwNoteBook);
  QString text1 = m_Method->getText1(mw_one->ui->qwNoteBook, index);
  QString text2 = m_Method->getText2(mw_one->ui->qwNoteBook, index);
  if (text2.isEmpty()) {
    int index_top = text1.toInt();
    QTreeWidgetItem *topItem = mw_one->m_NotesList->tw->topLevelItem(index_top);
    int child_count = topItem->childCount();
    for (int i = 0; i < child_count; i++) {
      QString text0 = topItem->child(i)->text(0);
      QString text3 = topItem->child(i)->text(1);
      if (!text3.isEmpty()) {
        QString file = iniDir + text3;
        QString item1 = m_Method->getLastModified(file);
        m_Method->addItemToQW(mw_one->ui->qwNoteList, text0, item1, "", text3,
                              0);

        mw_one->m_NotesList->pNoteItems.append(topItem->child(i));
      }
    }
  } else {
    QStringList list = text1.split("===");
    int indexMain, indexChild;
    if (list.count() == 2) {
      indexMain = list.at(0).toInt();
      indexChild = list.at(1).toInt();

      QTreeWidgetItem *topItem =
          mw_one->m_NotesList->tw->topLevelItem(indexMain);
      QTreeWidgetItem *childItem = topItem->child(indexChild);
      int count = childItem->childCount();
      for (int n = 0; n < count; n++) {
        QString text0 = childItem->child(n)->text(0);
        QString text3 = childItem->child(n)->text(1);
        QString file = iniDir + text3;
        QString item1 = m_Method->getLastModified(file);
        m_Method->addItemToQW(mw_one->ui->qwNoteList, text0, item1, "", text3,
                              0);

        mw_one->m_NotesList->pNoteItems.append(childItem->child(n));
      }
    }
  }

  mw_one->m_NotesList->setNotesListCurrentIndex(-1);
  mw_one->m_NotesList->setNoteLabel();
}

void NotesList::clickNoteList() {
  mw_one->m_Notes->saveQMLVPos();

  int index = m_Method->getCurrentIndexFromQW(mw_one->ui->qwNoteList);
  QString strMD = m_Method->getText3(mw_one->ui->qwNoteList, index);
  currentMDFile = iniDir + strMD;

  if (!QFile::exists(currentMDFile)) {
    ShowMessage *msg = new ShowMessage(mw_one);
    msg->showMsg(appName,
                 tr("The current note does not exist. Please select another "
                    "note or create a new note."),
                 0);

    return;
  }

  QString noteName = m_Method->getText0(mw_one->ui->qwNoteList, index);

  mw_one->ui->lblNoteName->setText(noteName);

  tw->setCurrentItem(pNoteItems.at(index));

  saveCurrentNoteInfo();

  for (int i = 0; i < listRecentOpen.count(); i++) {
    QString item = listRecentOpen.at(i);
    if (item.contains(strMD)) {
      listRecentOpen.removeAt(i);
      break;
    }
  }
  listRecentOpen.insert(0, noteName + "===" + strMD);

  int count = listRecentOpen.count();
  if (count > 15) {
    listRecentOpen.removeAt(count - 1);
  }
  saveRecentOpen();
  genCursorText();
}

void NotesList::genRecentOpenMenu() {
  QMenu *menuRecentOpen = new QMenu(this);
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
      QString qsLine = fm.elidedText(txt, Qt::ElideRight, mw_one->width() - 30);
      QAction *act = new QAction(qsLine);
      menuRecentOpen->addAction(act);

      connect(act, &QAction::triggered, this, [=]() {
        currentMDFile = file;

        mw_one->ui->lblNoteName->setText(name);

        mw_one->on_btnOpenNote_clicked();

        listRecentOpen.removeAt(i);
        listRecentOpen.insert(0, item);
        saveRecentOpen();
        saveCurrentNoteInfo();

        setCurrentItemFromMDFile(currentMDFile);
      });
    }
  }

  int x = 0;
  x = mw_one->geometry().x() + 2;
  int y = mw_one->geometry().y() + mw_one->ui->btnRecentOpen0->height() + 4;
  QPoint pos(x, y);
  menuRecentOpen->exec(pos);
}

void NotesList::setCurrentItemFromMDFile(QString mdFile) {
  int count = getNoteBookCount();
  bool isBreak = false;
  for (int i = 0; i < count; i++) {
    setNoteBookCurrentIndex(i);
    clickNoteBook();
    int count1 = getNotesListCount();
    for (int j = 0; j < count1; j++) {
      setNotesListCurrentIndex(j);
      QString strMD = iniDir + m_Method->getText3(mw_one->ui->qwNoteList, j);
      if (mdFile == strMD) {
        isBreak = true;
        break;
      }
    }

    if (isBreak) break;
  }
}

QString NotesList::getCurrentNoteNameFromMDFile(QString mdFile) {
  return mw_one->m_Notes->m_NoteIndexManager->getNoteTitle(mdFile);
}

void NotesList::genCursorText() {
  QTextEdit *edit = new QTextEdit;
  QString strBuffer = loadText(currentMDFile);
  edit->setPlainText(strBuffer);
  int curPos = mw_one->m_ReceiveShare->getCursorPos();
  if (curPos < 0) curPos = 0;
  if (curPos > strBuffer.length()) curPos = strBuffer.length();
  QTextCursor tmpCursor = edit->textCursor();

  int start = curPos - 5;
  int end = curPos;
  if (start < 0) start = 0;
  tmpCursor.setPosition(start, QTextCursor::MoveAnchor);
  tmpCursor.setPosition(end, QTextCursor::KeepAnchor);
  QString str0 = tmpCursor.selectedText();

  start = curPos;
  end = curPos + 5;
  int nLength = strBuffer.length();
  if (end > nLength) end = nLength;
  tmpCursor.setPosition(start, QTextCursor::MoveAnchor);
  tmpCursor.setPosition(end, QTextCursor::KeepAnchor);
  QString str1 = tmpCursor.selectedText();

  QString curText =
      QString::number(curPos) + "  (\"" + str0 + "|" + str1 + "\"" + ")";
  StringToFile(curText, privateDir + "cursor_text.txt");
  qDebug() << "cursor_text=" << curText;
}

QStringList NotesList::extractLocalImagesFromMarkdown(const QString &filePath) {
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

void NotesList::onSearchIndexReady() {
  qDebug() << "索引构建完成，共索引文档：" << m_searchEngine->documentCount();

  // 更新最后索引时间,如果 m_lastIndexTime 在多个线程中被访问，需添加互斥锁
  QMutexLocker locker(&m_indexTimeMutex);
  m_lastIndexTime = QDateTime::currentDateTime();
  saveIndexTimestamp();

  m_searchEngine->saveIndex(privateDir + "MyNotesIndex");

  m_isIndexing = false;
}

void NotesList::onSearchTextChanged(const QString &text) {
  QTimer::singleShot(300, [this, text]() {  // 防抖处理
    auto results =
        m_dbManager.searchDocuments(text, mw_one->m_Notes->m_NoteIndexManager);
    m_searchModel.setResults(results);
  });

  ////////
  return;
  ///////

  QList<SearchResult> results = m_searchEngine->search(text);

  // 打印调试信息
  qDebug() << "===== 搜索开始 =====";
  qDebug() << "查询内容:" << text;
  qDebug() << "共找到" << results.size() << "个匹配文件";

  for (const SearchResult &result : results) {
    qDebug() << "----------------------------------------";
    qDebug() << result;  // 直接调用定义好的 operator<<
  }

  qDebug() << "===== 搜索结束 =====";

  // 生成模型数据
  QList<SearchResult> modelData;
  for (const SearchResult &result : results) {
    SearchResult item;
    QString file = result.filePath;
    item.filePath = file;
    item.previewText = generatePreviewText(result);
    item.highlightPos = result.highlightPos;  // 直接使用已处理的位置

    QString title = mw_one->m_Notes->m_NoteIndexManager->getNoteTitle(file);
    if (title.length() > 0)
      item.fileTitle = title;
    else {
      QFileInfo fi(file);
      item.fileTitle = fi.baseName();
    }
    modelData.append(item);
  }

  // 更新成员变量模型
  m_searchResultModel->updateResults(modelData);

  // 调试输出
  qDebug() << "搜索结果已更新，数量：" << modelData.size();
  mw_one->ui->lblNoteTitle->setText("");
  mw_one->ui->lblNoteSearchResult->setText(tr("Note Search Results:") + " " +
                                           QString::number(modelData.size()));
}

void NotesList::openSearch() {
  return;

  if (m_isIndexing) return;
  m_isIndexing = true;

  // 启动异步索引构建
  QList<QString> notePaths =
      findMarkdownFiles(iniDir + "memo/");  // 需要实现文件遍历逻辑

  QSet<QString> set2(recycleNotesList.begin(), recycleNotesList.end());
  QStringList result;
  for (const QString &str : notePaths) {
    if (!set2.contains(str)) {
      result.append(str);
    }
  }

  notePaths = result;

  QStringList newPaths = findUnindexedFiles(notePaths);
  if (!QFile::exists(privateDir + "MyNotesIndex"))
    m_searchEngine->buildIndexAsync(notePaths, true);
  else
    m_searchEngine->buildIndexAsync(newPaths, false);
}

// 增量索引更新
QList<QString> NotesList::findUnindexedFiles(const QList<QString> &allPaths) {
  QList<QString> newPaths;
  for (const QString &path : allPaths) {
    QFileInfo fileInfo(path);

    // 检查文件最后修改时间是否晚于索引保存时间
    if (!m_searchEngine->hasDocument(path) &&
        fileInfo.lastModified() > m_lastIndexTime) {
      newPaths.append(path);
    }
  }

  qDebug() << "newPaths=" << newPaths;
  return newPaths;
}

// 保存到 QSettings
void NotesList::saveIndexTimestamp() {
  QSettings settings;
  settings.setValue("KnotNotes_LastIndexTime", m_lastIndexTime);
}

// 从 QSettings 加载
void NotesList::loadIndexTimestamp() {
  QSettings settings;
  m_lastIndexTime = settings.value("KnotNotes_LastIndexTime", 0).toDateTime();
}

QString NotesList::generatePreviewText(const SearchResult &result) {
  // 1. 获取文档原始内容（假设已存储）
  QString content = m_searchEngine->getDocumentContent(result.filePath);

  // 2. 如果没有原始内容，尝试从文件读取
  if (content.isEmpty()) {
    QFile file(result.filePath);
    if (file.open(QIODevice::ReadOnly)) {
      content = QString::fromUtf8(file.readAll());
      file.close();
    }
  }

  // 3. 生成带高亮的预览文本
  QString preview;
  preview = content.left(200);  // 截取前200字符

  // QList<KeywordPosition> sortedPositions = result.highlightPos;

  // 按起始位置反向排序（避免插入标签导致偏移错乱）
  // std::sort(sortedPositions.begin(), sortedPositions.end(),
  //          [](const KeywordPosition &a, const KeywordPosition &b) {
  //           return a.charStart > b.charStart;
  //         });

  // 插入高亮标签
  // for (const KeywordPosition &pos : sortedPositions) {
  // int start = pos.charStart;
  // int end = pos.charEnd;
  // if (start < preview.length() && end <= preview.length()) {
  //     preview.insert(end, "</span>");
  //    preview.insert(start, "<span style='color: #e74c3c; font-weight:
  //    500;'>");
  // }
  //}

  return preview;
}

QString NotesList::getSearchResultQmlFile() {
  QQuickItem *root = mw_one->ui->qwNotesSearchResult->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject *)root, "getQmlCurrentMDFile",
                            Q_RETURN_ARG(QVariant, item));
  return item.toString();
}

QStringList NotesList::getValidMDFiles() { return validMDFiles; }

template class QFutureWatcher<ResultsMap>;
