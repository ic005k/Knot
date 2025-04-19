#ifndef EDITRECORD_H
#define EDITRECORD_H

#include <QCompleter>
#include <QDialog>
#include <QListWidget>
#include <QSet>
#include <QTextEdit>
#include <QWidgetAction>

#include "src/CategoryList.h"

namespace Ui {
class EditRecord;
}

class EditRecord : public QDialog {
  Q_OBJECT

 public:
  explicit EditRecord(QWidget *parent = nullptr);
  ~EditRecord();
  Ui::EditRecord *ui;

  static void saveMyClassification();
  QString lblStyleHighLight =
      "QLabel{background: rgb(45,182,116); color:white;}";
  static int removeDuplicates(QStringList *that);

  void init_MyCategory();

  void getTime(int h, int m);

  void init();

  static void saveAdded();
  static void saveModified();

 protected:
  void keyReleaseEvent(QKeyEvent *event) override;
  bool eventFilter(QObject *watch, QEvent *evn) override;

 public:
  void on_btnOk_clicked();

  void on_btn7_clicked();
  void on_btn8_clicked();
  void on_btn9_clicked();
  void on_btn4_clicked();
  void on_btn5_clicked();
  void on_btn6_clicked();
  void on_btn1_clicked();
  void on_btn2_clicked();
  void on_btn3_clicked();
  void on_btn0_clicked();
  void on_btnDot_clicked();
  void on_btnDel_clicked();

  void on_btnCustom_clicked();

  void on_btnClearAmount_clicked();

  void on_btnClearDesc_clicked();

  void on_editAmount_textChanged(const QString &arg1);

  void on_hsH_valueChanged(int value);

  void on_hsM_valueChanged(int value);

  void on_btnClearDetails_clicked();

  void on_editCategory_textChanged(const QString &arg1);

  void on_editDetails_textChanged();

  void saveCurrentValue();
  void setCurrentValue();

  void monthSum();

  void writeToLog(QString str);

 private:
  void set_Amount(QString Number);

  QString lblStyle;
  int nH;
};

#endif  // EDITRECORD_H
