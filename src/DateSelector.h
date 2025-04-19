#ifndef DATESELECTOR_H
#define DATESELECTOR_H

#include <QDialog>
#include <QKeyEvent>

#include "src/Comm/DatePicker.h"

namespace Ui {
class DateSelector;
}

class DateSelector : public QDialog {
  Q_OBJECT

 public:
  explicit DateSelector(QWidget *parent = nullptr);
  ~DateSelector();
  Ui::DateSelector *ui;

  DatePicker *m_datePickerYM;

  DatePicker *m_datePickerYMD;

  int dateFlag = 0; /*1=btnYeat 2=btnMonth 3=btnStartDate 4=btnEnDate*/

  void init();

  void initStartEndDate(QString flag);

 protected:
  bool eventFilter(QObject *watch, QEvent *evn) override;

  void closeEvent(QCloseEvent *event) override;

 private slots:

  void on_btnOk_clicked();

 private:
};

#endif  // DATESELECTOR_H
