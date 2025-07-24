#ifndef MAINHELPER_H
#define MAINHELPER_H

#include <QDialog>
#include <QObject>
#include <QWidget>

class MainWindow;  // 前向声明，避免头文件循环包含

class MainHelper : public QDialog {
  Q_OBJECT
 public:
  explicit MainHelper(QWidget *parent = nullptr);

  void clickBtnChart();
  void clickBtnRestoreTab();
  bool mainEventFilter(QObject *watch, QEvent *evn);
 signals:
};

#endif  // MAINHELPER_H
