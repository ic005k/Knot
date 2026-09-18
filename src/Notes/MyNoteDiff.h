#ifndef MYNOTEDIFF_H
#define MYNOTEDIFF_H

#include <QDialog>

namespace Ui {
class MyNoteDiff;
}

class MyNoteDiff : public QDialog {
  Q_OBJECT

 public:
  explicit MyNoteDiff(QWidget* parent = nullptr);
  ~MyNoteDiff();

  void showDiff();

 private:
  Ui::MyNoteDiff* ui;
};

#endif  // MYNOTEDIFF_H
