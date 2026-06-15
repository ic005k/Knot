#include "MoveTo.h"

#include "src/Comm/Method.h"
#include "src/MainWindow.h"
#include "src/defines.h"
#include "ui_MainWindow.h"
#include "ui_MoveTo.h"

MoveTo::MoveTo(QWidget* parent) : QDialog(parent), ui(new Ui::MoveTo) {
  ui->setupUi(this);

  setModal(true);
  this->installEventFilter(this);
  ui->listWidget->installEventFilter(this);

  ui->listWidget->verticalScrollBar()->setStyleSheet(m_Method->vsbarStyleBig);
  ui->listWidget->setVerticalScrollMode(QListWidget::ScrollPerPixel);
  QScroller::grabGesture(ui->listWidget, QScroller::LeftMouseButtonGesture);

  ui->listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  ui->listWidget->setWordWrap(true);
  ui->listWidget->setUniformItemSizes(false);

  m_Method->set_ToolButtonStyle(this);

  QTreeWidgetItem* item = NULL;
  if (!m_NotesList->ui->frame0->isHidden() || !mui->frameNoteList->isHidden())
    item = tw->currentItem();

  if (!m_NotesList->ui->frame1->isHidden() ||
      !mui->frameNoteRecycle->isHidden())
    item = twrb->currentItem();

  if (item == NULL) {
    close();
    return;
  }

  if (item != NULL) {
    initAllNoteBook();

    ui->lblItem->setText(item->text(0));
  }

  ui->lblItem->adjustSize();
  ui->lblItem->setWordWrap(true);
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
    y = mw_one->geometry().y() + (mw_one->height() - h) / 2;
  } else {
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
  if (ui->listWidget->count() > 0 && nCurrentMoveTo < ui->listWidget->count() &&
      nCurrentMoveTo >= 0) {
    ui->listWidget->setCurrentRow(nCurrentMoveTo);
  }

  exec();
}

void MoveTo::closeEvent(QCloseEvent* event) {
  Q_UNUSED(event)
  nCurrentMoveTo = ui->listWidget->currentRow();
  m_widget->close();
}

void MoveTo::on_btnCancel_clicked() {
  isOk = false;
  strCurrentItem = "";
  currentItem = NULL;
  close();
  reject();
}

void MoveTo::on_btnOk_clicked() {
  isOk = true;
  strCurrentItem = ui->listWidget->currentItem()->text();

  currentItem = m_NotesList->pNoteBookItems.at(ui->listWidget->currentRow());

  close();
  accept();
}

void MoveTo::initAllNoteBook() {
  ui->listWidget->clear();
  listItems.clear();
  QStringList itemList;

  int count = m_NotesList->getNoteBookCount();
  for (int i = 0; i < count; i++) {
    QString title = m_NotesList->getNoteBookText0(i);
    itemList.append(title);
  }

  ui->listWidget->addItems(itemList);
}

void MoveTo::on_btnStopMove_clicked() {
  isStopMoveNote = true;
  close();
  accept();
}

void MoveTo::on_listWidget_currentRowChanged(int currentRow) {
  Q_UNUSED(currentRow);
}
