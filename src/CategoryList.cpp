#include "CategoryList.h"

#include "MainWindow.h"
#include "src/defines.h"
#include "ui_CategoryList.h"
#include "ui_MainWindow.h"

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
  int count = m_Method->getCountFromQW(mui->qwCategory);
  if (count == 0) return;

  int row = m_Method->getCurrentIndexFromQW(mui->qwCategory);

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

    m_Method->delItemFromQW(mui->qwCategory, row);
  }
  mw_one->m_EditRecord->saveMyClassification();
  if (ui->listWidget->count() > 0)
    on_listWidget_itemClicked(ui->listWidget->currentItem());
  else
    ui->editRename->clear();

  count = m_Method->getCountFromQW(mui->qwCategory);
  mui->lblTypeInfo->setText(tr("Total") + " : " + QString::number(count));
}

void CategoryList::on_btnOk_clicked() {
  int index = m_Method->getCurrentIndexFromQW(mui->qwCategory);
  ui->listWidget->setCurrentRow(index);

  setCategoryText();

  on_btnCancel_clicked();
}

void CategoryList::setCategoryText() {
  int row = ui->listWidget->currentRow();
  if (row >= 0) {
    mui->editCategory->setText(ui->listWidget->currentItem()->text());
  }

  close();
}

void CategoryList::on_listWidget_itemDoubleClicked(QListWidgetItem* item) {
  Q_UNUSED(item);
  setCategoryText();
}

void CategoryList::on_btnRename_clicked() {
  if (ui->listWidget->count() == 0) return;

  int row = m_Method->getCurrentIndexFromQW(mui->qwCategory);
  ui->listWidget->setCurrentRow(row);

  oldName = ui->listWidget->currentItem()->text().trimmed();

  QString text = ui->editRename->text().trimmed();
  QString str = ui->listWidget->currentItem()->text();
  if (!text.isEmpty() && text != str) {
    int index = ui->listWidget->currentRow();
    ui->listWidget->takeItem(index);
    QListWidgetItem* item = new QListWidgetItem(text);

    ui->listWidget->insertItem(index, item);

    m_Method->modifyItemText0(mui->qwCategory, row, text);

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

    renameAll();

    mui->editCategory->setText(ui->editRename->text().trimmed());

    mw_one->reloadMain();
  }
}

void CategoryList::renameAll() {
  QDir dir(iniDir);

  // 设置过滤器：只要.ini文件且排除目录
  QStringList filters;
  filters << "*.ini";
  dir.setNameFilters(filters);
  dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);  // 只获取文件，排除.和..

  // 获取文件信息列表
  QFileInfoList fileList = dir.entryInfoList();

  for (int i = 0; i < fileList.count(); i++) {
    QString file = fileList.at(i).absoluteFilePath();
    QString str = loadText(file);

    QString strNew = str;

    QString newName = ui->editRename->text().trimmed();
    if (strNew.contains("childDesc")) strNew = strNew.replace(oldName, newName);

    if (str != strNew) {
      StringToFile(strNew, file);
      qDebug() << oldName << " --> " << newName;
      qDebug() << "Rename: " + file;
    }
  }
}

void CategoryList::on_btnCancel_clicked() {
  mw_one->clearWidgetFocus();

  mui->frameCategory->hide();
  mui->frameEditRecord->show();
}
