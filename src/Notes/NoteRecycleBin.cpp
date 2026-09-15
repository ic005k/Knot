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

void NoteRecycleBin::on_btnSelAll_clicked() {
  for (int i = 0; i < ui->listNoteRecycle->count(); ++i) {
    QListWidgetItem* item = ui->listNoteRecycle->item(i);
    if (!item->data(Qt::UserRole).toBool()) {
      item->setData(Qt::UserRole, true);
    }
  }
  // 强制刷新整个列表的视口，确保自定义Delegate重绘背景
  ui->listNoteRecycle->viewport()->update();
}

void NoteRecycleBin::on_btnDesAll_clicked() {
  for (int i = 0; i < ui->listNoteRecycle->count(); ++i) {
    QListWidgetItem* item = ui->listNoteRecycle->item(i);
    if (item->data(Qt::UserRole).toBool()) {
      item->setData(Qt::UserRole, false);
    }
  }
  // 强制刷新整个列表的视口，确保自定义Delegate重绘背景
  ui->listNoteRecycle->viewport()->update();
}

void NoteRecycleBin::on_btnRestore_clicked() {
  QStringList list1 = getCheckedItems();
  m_NotesList->restoreToNotes(list1);

  close();
}

void NoteRecycleBin::on_btnDel_clicked() {
  QStringList checkedItems = getCheckedItems();

  // 1. 执行底层数据删除
  m_NotesList->delRecycleBinNotes(checkedItems);

  // 2. 倒序遍历移除UI列表中对应的条目（防止索引错位）
  for (int i = ui->listNoteRecycle->count() - 1; i >= 0; --i) {
    QListWidgetItem* item = ui->listNoteRecycle->item(i);
    if (item->data(Qt::UserRole).toBool()) {
      // takeItem 仅移除不销毁，需手动 delete 释放内存
      delete ui->listNoteRecycle->takeItem(i);
    }
  }

  // 3. 强制刷新视口，确保自定义Delegate重绘背景
  ui->listNoteRecycle->viewport()->update();
}

void NoteRecycleBin::showNoteRecycleBin(QStringList list) {
  m_Method->setWinPos(this, 350, 3);
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

QStringList NoteRecycleBin::getCheckedItems() const {
  QStringList checkedItems;
  for (int i = 0; i < ui->listNoteRecycle->count(); ++i) {
    QListWidgetItem* item = ui->listNoteRecycle->item(i);
    if (item->data(Qt::UserRole).toBool()) {
      // 直接获取完整的 DisplayRole 数据，不做 === 分隔处理
      checkedItems.append(item->data(Qt::DisplayRole).toString());
    }
  }
  return checkedItems;
}
