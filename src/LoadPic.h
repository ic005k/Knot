#ifndef LOADPIC_H
#define LOADPIC_H

#include <QDialog>
#include <QKeyEvent>

namespace Ui {
class LoadPic;
}

class LoadPic : public QDialog {
  Q_OBJECT

 public:
  explicit LoadPic(QWidget *parent = nullptr);
  ~LoadPic();
  Ui::LoadPic *ui;

  void initMain(QString imgFile);

 protected:
  bool eventFilter(QObject *watch, QEvent *evn) override;

 private slots:

 private:
};

#endif  // LOADPIC_H
