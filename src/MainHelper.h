#ifndef MAINHELPER_H
#define MAINHELPER_H

#include <QObject>
#include <QWidget>

class MainWindow;  // 前向声明，避免头文件循环包含

class MainHelper : public QWidget {
  Q_OBJECT
 public:
  explicit MainHelper(QWidget *parent = nullptr);

  void clickBtnChart();
  void clickBtnRestoreTab();
 signals:
};

#endif  // MAINHELPER_H
