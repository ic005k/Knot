#include "src/Notes/Notes.h"

// 按钮点击
void Notes::on_btnDone_clicked() { saveMainNotes(); }
void Notes::on_btnInsertTable_clicked() {
#ifndef Q_OS_ANDROID
  QString table = "|Title1|Title2|\n|------|------|\n";
  m_EditSource->insert(table);
#endif
}

void Notes::on_btnS1_clicked() { /* 粗斜体 */ }
void Notes::on_btnS2_clicked() { /* 斜体 */ }
void Notes::on_btnS3_clicked() { /* 下划线 */ }
void Notes::on_btnS4_clicked() { /* 删除线 */ }
void Notes::on_btnS5_clicked() { /* 粗体 */ }
void Notes::on_btnColor_clicked() { /* 颜色 */ }
void Notes::on_btnPaste_clicked() { /* 粘贴图片 */ }
void Notes::on_btnPDF_clicked() { /* 导出PDF */ }
void Notes::on_btnView_clicked() {
  ui->btnDone->click();
  mui->btnOpenNote->click();
}

// 搜索
void Notes::on_btnFind_clicked() {
  if (ui->editFind->text().trimmed() == "") return;
  show_findText();
}
void Notes::on_btnPrev_clicked() {
  ui->editFind->setFocus();
  searchPrevious();
}
void Notes::on_btnNext_clicked() {
  ui->editFind->setFocus();
  searchNext();
}
void Notes::on_editFind_returnPressed() { searchNext(); }
void Notes::on_editFind_textChanged(const QString& arg1) {
  searchText(arg1.trimmed(), true);
  m_lastSearchText = arg1.trimmed();
}

// 替换
void Notes::on_btnReplace_clicked() { /* 替换选中 */ }
void Notes::on_btnFindReplace_clicked() { /* 替换并下一个 */ }
void Notes::on_btnReplaceAll_clicked() { /* 全部替换 */ }

// 链接自动补全
void Notes::on_editNoteLink_textChanged(const QString& arg1) { /* 搜索提示 */ }
void Notes::onPopupItemClicked(QListWidgetItem* item) { /* 插入链接 */ }
void Notes::insertNoteLink(const QString& title) { /* 生成链接 */ }