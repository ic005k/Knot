#ifndef STEPSOPTIONS_H
#define STEPSOPTIONS_H

#include <QDialog>

#include "src/Comm/Method.h"

namespace Ui {
class StepsOptions;
}

class StepsOptions : public QDialog {
  Q_OBJECT

 public:
  explicit StepsOptions(QWidget *parent = nullptr);
  ~StepsOptions();
  Ui::StepsOptions *ui;

  void init();
  bool isTextChange;

 protected:
  bool eventFilter(QObject *obj, QEvent *evn) override;
  void closeEvent(QCloseEvent *event) override;
 private slots:
  void on_btnBack_clicked();

  void on_editStepsThreshold_textChanged(const QString &arg1);

  void on_editStepLength_textChanged(const QString &arg1);

 private:
};

#endif  // STEPSOPTIONS_H
