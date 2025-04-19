#include "CategoryList.h"

#include "MainWindow.h"
#include "ui_CategoryList.h"
#include "ui_MainWindow.h"
extern MainWindow* mw_one;
extern Method* m_Method;
extern int fontSize, red;
extern QTabWidget *tabData, *tabChart;

CategoryList::CategoryList(QWidget* parent)
    : QDialog(parent), ui(new Ui::CategoryList) {
  ui->setupUi(this);

  setWindowFlag(Qt::FramelessWindowHint);
  // setAttribute(Qt::WA_TranslucentBackground);
  this->layout()->setContentsMargins(5, 5, 5, 5);

  ui->frame->setStyleSheet(
      "#frame{background-color: rgb(255, 255, 255);border-radius:10px; "
      "border:1px solid gray;}");

  setModal(true);
  this->installEventFilter(this);

  ui->listWidget->setStyleSheet("#listWidget{ border:None;}");
  ui->listWidget->verticalScrollBar()->setStyleSheet(m_Method->vsbarStyleSmall);
  ui->listWidget->setVerticalScrollMode(QListWidget::ScrollPerPixel);
  QScroller::grabGesture(ui->listWidget, QScroller::LeftMouseButtonGesture);
  ui->listWidget->horizontalScrollBar()->setHidden(true);
  ui->listWidget->setViewMode(QListView::IconMode);
  ui->listWidget->setMovement(QListView::Static);
  ui->listWidget->setStyleSheet(m_Method->listStyleMain);
  ui->listWidget->setSpacing(12);
  m_Method->setSCrollPro(ui->listWidget);
  QFont font;
  font.setPointSize(fontSize + 3);
  ui->listWidget->setFont(font);
  mw_one->setLineEditQss(ui->editRename, 0, 1, "#4169E1", "#4169E1");
  ui->btnRename->setFixedHeight(ui->editRename->height() + 2);
}

CategoryList::~CategoryList() { delete ui; }

void CategoryList::keyReleaseEvent(QKeyEvent* event) { Q_UNUSED(event) }

void CategoryList::closeEvent(QCloseEvent* event) { Q_UNUSED(event); }

bool CategoryList::eventFilter(QObject* watch, QEvent* evn) {
  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
      close();
      return true;
    }
  }

  return QWidget::eventFilter(watch, evn);
}

void CategoryList::on_listWidget_itemClicked(QListWidgetItem* item) {
  Q_UNUSED(item);
  QString txt = item->text();
  ui->editRename->setText(txt);
}

void CategoryList::on_btnDel_clicked() {
  int count = m_Method->getCountFromQW(mw_one->ui->qwCategory);
  if (count == 0) return;

  int row = m_Method->getCurrentIndexFromQW(mw_one->ui->qwCategory);

  if (row < 0) return;

  ui->listWidget->setCurrentRow(row);
  if (row >= 0) {
    m_Method->m_widget = new QWidget(mw_one);
    ShowMessage* m_ShowMsg = new ShowMessage(this);
    if (!m_ShowMsg->showMsg("Kont",
                            tr("Delete this category?") + "\n\n" +
                                ui->listWidget->currentItem()->text(),
                            2))
      return;

    ui->listWidget->takeItem(row);

    m_Method->delItemFromQW(mw_one->ui->qwCategory, row);
  }
  mw_one->m_EditRecord->saveMyClassification();
  if (ui->listWidget->count() > 0)
    on_listWidget_itemClicked(ui->listWidget->currentItem());
  else
    ui->editRename->clear();

  count = m_Method->getCountFromQW(mw_one->ui->qwCategory);
  mw_one->ui->lblTypeInfo->setText(tr("Total") + " : " +
                                   QString::number(count));
}

void CategoryList::on_btnOk_clicked() {
  int index = m_Method->getCurrentIndexFromQW(mw_one->ui->qwCategory);
  ui->listWidget->setCurrentRow(index);

  setCategoryText();

  on_btnCancel_clicked();
}

void CategoryList::setCategoryText() {
  int row = ui->listWidget->currentRow();
  if (row >= 0) {
    mw_one->ui->editCategory->setText(ui->listWidget->currentItem()->text());
  }

  close();
}

void CategoryList::on_listWidget_itemDoubleClicked(QListWidgetItem* item) {
  Q_UNUSED(item);
  setCategoryText();
}

void CategoryList::on_btnRename_clicked() {
  if (ui->listWidget->count() == 0) return;

  int row = m_Method->getCurrentIndexFromQW(mw_one->ui->qwCategory);
  ui->listWidget->setCurrentRow(row);

  QString text = ui->editRename->text().trimmed();
  QString str = ui->listWidget->currentItem()->text();
  if (!text.isEmpty() && text != str) {
    int index = ui->listWidget->currentRow();
    ui->listWidget->takeItem(index);
    QListWidgetItem* item = new QListWidgetItem(text);

    ui->listWidget->insertItem(index, item);

    m_Method->modifyItemText0(mw_one->ui->qwCategory, row, text);

    QStringList list;
    for (int i = 0; i < ui->listWidget->count(); i++) {
      list.append(ui->listWidget->item(i)->text().trimmed());
    }
    mw_one->m_EditRecord->removeDuplicates(&list);
    ui->listWidget->clear();
    for (int i = 0; i < list.count(); i++) {
      QListWidgetItem* item = new QListWidgetItem(list.at(i));

      ui->listWidget->addItem(item);
    }
    if (index >= 0) ui->listWidget->setCurrentRow(index);
    mw_one->m_EditRecord->saveMyClassification();

    for (int i = 0; i < tabData->tabBar()->count(); i++) {
      QTreeWidget* tw = (QTreeWidget*)tabData->widget(i);
      for (int m = 0; m < tw->topLevelItemCount(); m++) {
        QTreeWidgetItem* topItem = tw->topLevelItem(m);
        for (int n = 0; n < topItem->childCount(); n++) {
          if (str == topItem->child(n)->text(2)) {
            topItem->child(n)->setText(2, text);
          }
        }
      }
    }
    mw_one->startSave("alltab");

    mw_one->ui->editCategory->setText(ui->editRename->text().trimmed());

    mw_one->reloadMain();
  }
}

void CategoryList::on_btnCancel_clicked() {
  mw_one->ui->frameCategory->hide();
  mw_one->ui->frameEditRecord->show();
}
