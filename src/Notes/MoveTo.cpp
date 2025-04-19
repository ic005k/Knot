#include "MoveTo.h"

#include "src/Comm/Method.h"
#include "src/MainWindow.h"
#include "ui_MainWindow.h"
#include "ui_MoveTo.h"

extern MainWindow* mw_one;
extern Method* m_Method;
extern bool isDark, isAndroid;

MoveTo::MoveTo(QWidget* parent) : QDialog(parent), ui(new Ui::MoveTo) {
  ui->setupUi(this);
  setModal(true);
  this->installEventFilter(this);
  ui->listWidget->installEventFilter(this);

  setWindowFlag(Qt::FramelessWindowHint);
  QString style = "QDialog{border-radius:0px;border:0px solid darkred;}";
  this->setStyleSheet(style);

  ui->listWidget->verticalScrollBar()->setStyleSheet(m_Method->vsbarStyleSmall);
  ui->listWidget->setVerticalScrollMode(QListWidget::ScrollPerPixel);
  QScroller::grabGesture(ui->listWidget, QScroller::LeftMouseButtonGesture);
  if (!isDark) {
#ifdef Q_OS_ANDROID
    ui->listWidget->setStyleSheet("selection-background-color: lightblue");
#else

#endif
  }

  mw_one->set_ToolButtonStyle(this);

  QTreeWidgetItem* item = NULL;
  if (!mw_one->m_NotesList->ui->frame0->isHidden() ||
      !mw_one->ui->frameNoteList->isHidden())
    item = mw_one->m_NotesList->tw->currentItem();

  if (!mw_one->m_NotesList->ui->frame1->isHidden() ||
      !mw_one->ui->frameNoteRecycle->isHidden())
    item = mw_one->m_NotesList->twrb->currentItem();

  if (item == NULL) close();

  if (item->text(1).isEmpty()) {
    isNoteBook = true;
    initTopNoteBook();
  } else {
    isNote = true;
    initAllNoteBook();
  }

  ui->lblItem->setText(item->text(0));
  ui->lblItem->adjustSize();
  ui->lblItem->setWordWrap(true);
  showDialog();
}

MoveTo::~MoveTo() { delete ui; }

bool MoveTo::eventFilter(QObject* watch, QEvent* evn) {
  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evn);

    if (keyEvent->key() == Qt::Key_Back) {
      on_btnCancel_clicked();
    }
    return true;
  }

  return QWidget::eventFilter(watch, evn);
}

void MoveTo::showDialog() {
  int x, y, w, h;
  h = mw_one->height() - 40;
  if (isAndroid) {
    w = mw_one->width();
    x = mw_one->geometry().x();
    y = mw_one->geometry().y();
  }

  else {
    w = width();
    if (w > 500) w = 500;
    x = mw_one->geometry().x() + (mw_one->width() - w) / 2;
    y = mw_one->geometry().y() + (mw_one->height() - h) / 2;
  }

  setGeometry(x, y, w, h);

  m_widget = new QWidget(mw_one);
  m_widget->resize(mw_one->width(), mw_one->height());
  m_widget->move(0, 0);
  m_widget->setStyleSheet("background-color:rgba(0, 0, 0,35%);");
  m_widget->show();

  ui->listWidget->setFocus();
  if (ui->listWidget->count() > 0) ui->listWidget->setCurrentRow(0);

  show();

  while (!isHidden()) QCoreApplication::processEvents();
}

void MoveTo::closeEvent(QCloseEvent* event) {
  Q_UNUSED(event)
  m_widget->close();
}

void MoveTo::on_btnCancel_clicked() {
  isOk = false;
  strCurrentItem = "";
  currentItem = NULL;
  close();
}

void MoveTo::on_btnOk_clicked() {
  isOk = true;
  strCurrentItem = ui->listWidget->currentItem()->text();
  if (isNoteBook) currentItem = listItems.at(ui->listWidget->currentRow() - 1);
  if (isNote) currentItem = listItems.at(ui->listWidget->currentRow());

  close();
}

void MoveTo::initTopNoteBook() {
  ui->listWidget->clear();
  listItems.clear();
  QStringList itemList;
  itemList.append(tr("Main Root"));
  int count = mw_one->m_NotesList->tw->topLevelItemCount();
  for (int i = 0; i < count; i++) {
    QTreeWidgetItem* topItem = mw_one->m_NotesList->tw->topLevelItem(i);
    QString strTop = topItem->text(0);
    itemList.append(strTop);
    listItems.append(topItem);
  }

  ui->listWidget->addItems(itemList);
}

void MoveTo::initAllNoteBook() {
  ui->listWidget->clear();
  listItems.clear();
  QStringList itemList;

  int count = mw_one->m_NotesList->tw->topLevelItemCount();
  for (int i = 0; i < count; i++) {
    QTreeWidgetItem* topItem = mw_one->m_NotesList->tw->topLevelItem(i);
    QString strTop = topItem->text(0);
    itemList.append(strTop);
    listItems.append(topItem);
    int childCount = topItem->childCount();
    for (int j = 0; j < childCount; j++) {
      QTreeWidgetItem* childItem = topItem->child(j);
      if (childItem->text(1).isEmpty()) {
        itemList.append(childItem->text(0));
        listItems.append(childItem);
      }
    }
  }

  ui->listWidget->addItems(itemList);
}
