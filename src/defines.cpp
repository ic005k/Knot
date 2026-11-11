#include "defines.h"
#include "MainWindow.h" // ✅ 在 cpp 中包含完整定义，解决不完整类型问题
#include <QApplication>
#include <QMessageBox>

void showInfo(const QString &title, const QString &text, QWidget *parent) {
  if (!parent && mw_one) {
    parent = mw_one; // ✅ 此处编译器已知 MainWindow 继承自 QWidget
  }
  QMessageBox::information(parent, title, text);
}

bool showConfirm(const QString &title, const QString &text, QWidget *parent) {
  if (!parent && mw_one) {
    parent = mw_one;
  }
  return QMessageBox::question(parent, title, text,
                               QMessageBox::Yes | QMessageBox::No) ==
         QMessageBox::Yes;
}
