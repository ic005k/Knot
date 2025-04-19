#ifndef SHOWMESSAGE_H
#define SHOWMESSAGE_H

#include <QDialog>
#include <QEvent>
#include <QKeyEvent>
#include <QMainWindow>
#include <QMessageBox>

namespace Ui {
class ShowMessage;
}

class ShowMessage : public QDialog {
  Q_OBJECT

 public:
  explicit ShowMessage(QWidget *parent = nullptr);
  ~ShowMessage();
  Ui::ShowMessage *ui;

  bool isValue;
  bool showMsg(QString title, QString msgtxt, int btnCount);

  QString AutoFeed(QString text, int nCharCount);

 protected:
  void closeEvent(QCloseEvent *event) override;
  bool eventFilter(QObject *watch, QEvent *evn) override;

 protected slots:

 private slots:

  void on_btnCancel_clicked();

  void on_btnOk_clicked();

  void on_btnCopy_clicked();

  void on_btnDel_clicked();

  void on_editMsg_textChanged();

 private:
  void init();
  int btn_count = 0;
};

#endif  // SHOWMESSAGE_H
