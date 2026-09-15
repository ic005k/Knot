#include "RecentOpen.h"

#include "Comm/Method.h"
#include "MainWindow.h"
#include "Notes/NotesList.h"
#include "Notes/RecentNoteDelegate.h"
#include "defines.h"
#include "ui_RecentOpen.h"

RecentOpen::RecentOpen(QWidget* parent)
    : QDialog(parent), ui(new Ui::RecentOpen) {
  ui->setupUi(this);
  setModal(true);

  // 隐藏水平滚动条，确保 Delegate 能正确获取 viewport 宽度来计算省略号
  ui->listRecentNotes->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  ui->listRecentNotes->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  // 设置自定义代理
  ui->listRecentNotes->setItemDelegate(new RecentNoteDelegate(this));
}

RecentOpen::~RecentOpen() { delete ui; }

void RecentOpen::on_btnView_clicked() {
  int idx = ui->listRecentNotes->currentRow();
  if (idx == -1) return;

  QString str = ui->listRecentNotes->currentItem()->text();
  currentMDFile = str.split("===").at(1);
  m_Notes->previewNote();
}

void RecentOpen::on_btnEdit_clicked() {
  int idx = ui->listRecentNotes->currentRow();
  if (idx == -1) return;

  QString str = ui->listRecentNotes->currentItem()->text();
  currentMDFile = str.split("===").at(1);
  m_Notes->openEditUI();
}

void RecentOpen::showRecentOpen(QStringList list) {
  setDataToRecentList(list);
  m_Method->setWinPos(this, 350, 3);
  show();
}

void RecentOpen::setDataToRecentList(QStringList list) {
  ui->listRecentNotes->clear();
  // list 中的元素已经是 "名称===路径" 格式
  for (const QString& item : list) {
    QListWidgetItem* listItem = new QListWidgetItem(item, ui->listRecentNotes);
    ui->listRecentNotes->addItem(listItem);
  }
}

void RecentOpen::on_editKeyword_textChanged(const QString& arg1) {
  QStringList filteredList;
  QStringList m_recentList = m_NotesList->listRecentOpen;

  const QString keyword = arg1.trimmed();

  if (keyword.isEmpty()) {
    // 关键词为空时，恢复显示全部列表
    filteredList = m_recentList;
  } else {
    // 遍历原始列表，仅对 "===" 前面的名称进行大小写不敏感匹配
    for (const QString& item : m_recentList) {
      int sepIndex = item.indexOf("===");
      // 兼容可能没有分隔符的脏数据
      QString name = (sepIndex >= 0) ? item.left(sepIndex) : item;

      if (name.contains(keyword, Qt::CaseInsensitive)) {
        filteredList.append(item);
      }
    }
  }

  setDataToRecentList(filteredList);
}
