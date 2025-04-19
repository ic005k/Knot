#ifndef TODOALARM_H
#define TODOALARM_H

#include <QDial>
#include <QDialog>
#include <QHBoxLayout>
#include <QObjectList>
#include <QSpacerItem>
#include <QToolButton>

#include "src/Comm/DatePicker.h"
#include "src/Comm/Time24Picker.h"

namespace Ui {
class TodoAlarm;
}

class TodoAlarm : public QDialog {
  Q_OBJECT

 public:
  explicit TodoAlarm(QWidget *parent = nullptr);
  ~TodoAlarm();
  Ui::TodoAlarm *ui;

  Time24Picker *m_timePicker;
  DatePicker *m_datePicker;

  void initDlg();

  QString btnSelStyle =
      "QToolButton {background-color: rgb(30, 144, 255);color: "
      "white;border-radius:10px; "
      "border:1px solid gray; } QToolButton:pressed { background-color: "
      "rgb(220,220,230);color:black;}";

  void addBtn(int start, int total, int col, QString flag, bool week);

  void setDateTime();

 protected:
  bool eventFilter(QObject *obj, QEvent *evn) override;
 public slots:
  void on_btnBack_clicked();
 private slots:

  void on_btnMinute_clicked();

  void on_btnHour_clicked();

  void on_btnDay_clicked();

  void on_btnMonth_clicked();

  void on_btnYear_clicked();

  void on_btnDelDT_clicked();

  void on_btnSetDT_clicked();

  void on_btnToday_clicked();

  void on_btnTomorrow_clicked();

  void on_btnNextWeek_clicked();

  void on_chkDaily_clicked();

  void on_btnTestSpeech_clicked();

  void on_chkSpeech_clicked();

 private:
  QFont font0;
  void onBtnClick(QToolButton *btn, QString flag);
  QString y, m, d, h, mm;
  void setBtnTitle();
  void onDial(QDial *btn, QString flag);

  void addDial(int min, int max, QString flag);

  int WidgetType = 2;
  /*1=Dial  2=RollBox* 3=New */
  void getChkVoice();
  void showTimePicker();
  void showDatePicker();
};

#endif  // TODOALARM_H
