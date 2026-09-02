#ifndef EDITRECORD_H
#define EDITRECORD_H

#include <QCompleter>
#include <QDialog>
#include <QList>
#include <QListWidget>
#include <QSet>
#include <QStringListModel>
#include <QTabWidget>
#include <QTextEdit>
#include <QWidgetAction>
#include <algorithm>

#include "DataManager.h"
#include "src/CategoryList.h"

class EditRecord : public QDialog {
  Q_OBJECT

 public:
  explicit EditRecord(QWidget* parent = nullptr);
  ~EditRecord() override;

  static void saveMyClassification();
  QString lblStyleHighLight =
      "QLabel{background: rgb(45,182,116); color:white;}";
  static int removeDuplicates(QStringList* that);

  void init_MyCategory();

  void getTime(int h, int m);

  static void saveCurrentYearData();

  QCompleter* completer = nullptr;

 protected:
  bool eventFilter(QObject* watched, QEvent* event) override;

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

  void on_btnType_clicked();

  void on_btnClearAmount_clicked();

  void on_btnClearDesc_clicked();

  void on_editAmount_textChanged(const QString& arg1);

  void on_hsH_valueChanged(int value);

  void on_hsM_valueChanged(int value);

  void on_btnClearDetails_clicked();

  void on_editCategory_textChanged(const QString& arg1);

  void on_editDetails_textChanged();

  void saveCurrentValue();
  void setCurrentValue();

  void monthSum();

  void updateCategoryCompleterList();

  void on_AddRecord();

  void openAddEventRecord(const QString& titleText, const QString& categoryText,
                          const QString& noteText, const QString& amountText,
                          const QString& timeTagText);
  void setDataToUI();
  void modify_Data();
  void reeditMainEventData(int index0, int index1);
  int idxMainDate, idxMainDateDetail;

  void showCategorySelectDialog();
  private:
  void set_Amount(QString Number);

  QString lblStyle;
  int nH;

  static QList<int> getExistingYears(QTreeWidget* tw);

  void initCategoryCompleter();

  QStringListModel* m_categoryModel = nullptr;
};

#endif  // EDITRECORD_H
