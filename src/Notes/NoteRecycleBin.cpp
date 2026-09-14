#include "NoteRecycleBin.h"

#include <QListWidgetItem>

#include "MainWindow.h"
#include "Notes/NotesList.h"
#include "RecycleBinItemDelegate.h"
#include "defines.h"
#include "ui_NoteRecycleBin.h"

NoteRecycleBin::NoteRecycleBin(QWidget* parent)
    : QDialog(parent), ui(new Ui::NoteRecycleBin) {
  ui->setupUi(this);
  setModal(true);

  // 【关键】强制使用列表模式，确保自定义Delegate垂直排列
  ui->listNoteRecycle->setViewMode(QListWidget::ListMode);

  ui->listNoteRecycle->setSelectionMode(QAbstractItemView::NoSelection);
  ui->listNoteRecycle->setFocusPolicy(Qt::NoFocus);

  // 确保间距为0，避免条目间出现巨大空白
  ui->listNoteRecycle->setSpacing(0);

  ui->listNoteRecycle->setItemDelegate(new RecycleBinItemDelegate(this));
  ui->listNoteRecycle->setFrameShape(QFrame::NoFrame);
  ui->listNoteRecycle->viewport()->setAutoFillBackground(true);
  ui->listNoteRecycle->setAttribute(Qt::WA_MacShowFocusRect, false);
}

NoteRecycleBin::~NoteRecycleBin() { delete ui; }

void NoteRecycleBin::on_btnSelAll_clicked() {}

void NoteRecycleBin::on_btnDesAll_clicked() {}

void NoteRecycleBin::on_btnRestore_clicked() {}

void NoteRecycleBin::on_btnDel_clicked() {}

void NoteRecycleBin::showNoteRecycleBin(QStringList list) {
  m_Method->setWinPos(this, 350);
  setDataToList(list);
  show();
}

void NoteRecycleBin::setDataToList(QStringList list) {
  ui->listNoteRecycle->clear();

  for (const QString& item : list) {
    // 不传 parent，手动 addItem（逻辑更清晰）
    QListWidgetItem* listItem = new QListWidgetItem();

    listItem->setData(Qt::DisplayRole, item);
    listItem->setData(Qt::UserRole, false);
    listItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

    ui->listNoteRecycle->addItem(listItem);
  }

  qInfo() << "笔记回收站=" << list;
}

// 获取所有被勾选的笔记路径（用于批量恢复或删除）
QStringList NoteRecycleBin::getCheckedPaths() const {
  QStringList checkedPaths;
  for (int i = 0; i < ui->listNoteRecycle->count(); ++i) {
    QListWidgetItem* item = ui->listNoteRecycle->item(i);
    if (item->data(Qt::UserRole).toBool()) {
      QString rawData = item->data(Qt::DisplayRole).toString();
      QStringList parts = rawData.split("===");
      if (parts.size() > 1) {
        checkedPaths.append(parts[1].trimmed());
      }
    }
  }
  return checkedPaths;
}
