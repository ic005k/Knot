#ifndef TODO_H
#define TODO_H

#include <QCheckBox>
#include <QDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidgetItem>
#include <QSettings>
#include <QTextEdit>
#include <QToolButton>

namespace Ui {
class Todo;
}

class Todo : public QDialog {
  Q_OBJECT

 public:
  explicit Todo(QWidget *parent = nullptr);
  Ui::Todo *ui;
  ~Todo();

  bool isNeedSave = false;
  bool isToday = false;
  QStringList tableLists;
  void init_Todo();
  QString alarmStyle =
      "QLabel{background:rgb(112,128,105);color:rgb(255,255,255);border:2px;"
      "border-radius:4px;padding:2px 4px;}";
  QString alarmStyleToday =
      "QLabel{background:rgb(255,11,25);color:white;border:2px;"
      "border-radius:4px;padding:2px 4px;}";
  QString alarmStyleTomorrow =
      "QLabel{background:rgb(245,245,25);color:black;border:2px;"
      "border-radius:4px;padding:2px 4px;}";

  QString styleDark =
      "#listWidget::item {background-color: #393d49;color: #ffffff;border: "
      "transparent;padding: 8px; height: 65;}"
      "#listWidget::item:hover {background-color: #4e5465;}"
      "#listWidget::item:selected {border-left: 5px solid #009688;}";

  void startTimerAlarm(QString text);
  void stopTimerAlarm();

  void sendMsgAlarm(QString text);

  qlonglong getSecond(QString strDateTime);

  int getEditTextHeight(QTextEdit *edit);

  void insertItem(QString strTime, int type, QString strText, int curIndex);
  int getCurrentIndex();
  QString getItemTime(int index);
  QString getItemTodoText(int index);
  void delItem(int index);
  void setCurrentIndex(int index);
  void setHighPriority(bool isBool);

  int getCount();

  void modifyTime(int index, QString strTime);
  void modifyTodoText(int index, QString strTodoText);
  void clearAll();
  void addItem(QString strTime, int type, QString strText);
  int getItemType(int index);
  void modifyType(int index, int type);

  int setItemHeight(QString strTodoText);

 public slots:
  void saveTodo();
  void refreshTableLists();
  void refreshAlarm();
  void reeditText();
  void addToRecycle();
  void isAlarm(int index);
  void insertRecycle(QString strTime, int type, QString strText, int curIndex);

 protected:
  void keyReleaseEvent(QKeyEvent *event) override;
  void closeEvent(QCloseEvent *event) override;
  bool eventFilter(QObject *watch, QEvent *evn) override;

 public:
  void on_SetAlarm();
  void on_DelAlarm();
  void on_btnAdd_clicked();

  void on_btnHigh_clicked();

  void on_btnLow_clicked();

  void on_btnSetTime_clicked();

  void on_btnRecycle_clicked();

  void on_btnReturn_clicked();

  void on_btnClear_clicked();

  void on_btnRestore_clicked();

  void on_btnDel_clicked();

  void on_editTodo_textChanged();

  void delItemRecycle(int index);
  int getCountRecycle();
  void clearAllRecycle();
  int getCurrentIndexRecycle();
  QString getItemTodoTextRecycle(int index);
  QString getItemTimeRecycle(int index);
  void addItemRecycle(QString strTime, int type, QString strText);

  void refreshTableListsFromIni();

  QString getTimeStr(QString str);

 private:
  QListWidgetItem *editItem;

  QString todotxt;
  QLabel *lblModi;
  QTextEdit *editModi;
  bool isModi = false;
  bool isWeekValid(QString lblDateTime, QString strDate);
  bool isTomorrow = false;
  void changeTodoIcon(bool isToday);
  QSettings *iniTodo;
};
#endif  // TODO_H
