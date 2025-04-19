#ifndef SETREADERTEXT_H
#define SETREADERTEXT_H

#include <QDialog>
#include <QKeyEvent>

namespace Ui {
class dlgSetText;
}

class dlgSetText : public QDialog {
  Q_OBJECT

 public:
  explicit dlgSetText(QWidget *parent = nullptr);
  ~dlgSetText();
  Ui::dlgSetText *ui;

  void init(int x, int y, int w, int h);

 protected:
  bool eventFilter(QObject *watch, QEvent *evn) override;

 private slots:
  void on_btnBack_clicked();

  void on_btnCopy_clicked();

  void on_btnSearch_clicked();

  void on_lineEdit_textChanged(const QString &arg1);

  void on_btnShare_clicked();

 private:
};

#endif  // SETREADERTEXT_H
