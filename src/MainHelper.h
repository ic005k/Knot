#ifndef MAINHELPER_H
#define MAINHELPER_H

#include <QDialog>
#include <QMenu>
#include <QObject>
#include <QTreeWidget>
#include <QWidget>

class MainWindow;  // 前向声明，避免头文件循环包含

class MainHelper : public QDialog {
  Q_OBJECT
 public:
  explicit MainHelper(QWidget *parent = nullptr);

  void clickBtnChart();
  void clickBtnRestoreTab();
  bool mainEventFilter(QObject *watch, QEvent *evn);
  QTreeWidget *init_TreeWidget(QString name);
  void init_Menu(QMenu *mainMenu);
  void openTabRecycle();
  void initQW();
  signals:
};

#endif  // MAINHELPER_H
