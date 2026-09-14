#include "NoteSearch.h"

#include "Comm/Method.h"
#include "MainWindow.h"
#include "NoteSearchDelegate.h"
#include "Notes/NotesList.h"
#include "defines.h"
#include "ui_NoteSearch.h"

NoteSearch::NoteSearch(QWidget* parent)
    : QDialog(parent), ui(new Ui::NoteSearch) {
  ui->setupUi(this);
  setModal(true);

  // 隐藏水平滚动条，防止出现不必要的横向滚动
  ui->listSearch->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  NoteSearchDelegate* delegate = new NoteSearchDelegate(ui->listSearch);
  ui->listSearch->setItemDelegate(delegate);
}

NoteSearch::~NoteSearch() { delete ui; }

void NoteSearch::showNoteSearch() {
  m_Method->setWinPos(this, 350);
  show();
}

void NoteSearch::on_btnSearch_clicked() {
  QString kw = ui->editSearch->text().trimmed();
  if (kw.isEmpty()) return;

  ui->lblResult->setText(tr("Note Search Results: ") + QString::number(0));
  m_NotesList->startFind(kw);
}

void NoteSearch::on_btnView_clicked() {
  int idx = ui->listSearch->currentRow();
  if (idx == -1) return;

  QString str = ui->listSearch->currentItem()->text();
  currentMDFile = str.split("===").at(2);
  m_Notes->previewNote();
}

void NoteSearch::on_btnEdit_clicked() {
  int idx = ui->listSearch->currentRow();
  if (idx == -1) return;

  QString str = ui->listSearch->currentItem()->text();
  currentMDFile = str.split("===").at(2);
  m_Notes->openEditUI();
}

void NoteSearch::on_editSearch_textChanged(const QString& arg1) {
  QString kw = arg1.trimmed();
  if (kw.isEmpty()) return;

  ui->lblResult->setText(tr("Note Search Results: ") + QString::number(0));
  m_NotesList->onSearchTextChanged(kw);
}

void NoteSearch::setDataToList(QStringList list) {
  ui->listSearch->clear();

  for (const QString& item : list) {
    QListWidgetItem* listItem = new QListWidgetItem(item, ui->listSearch);
    listItem->setData(Qt::FontRole, QFont());
    ui->listSearch->addItem(listItem);
  }

  ui->listSearch->doItemsLayout();

  ui->lblResult->setText(tr("Note Search Results: ") +
                         QString::number(list.count()));
}
