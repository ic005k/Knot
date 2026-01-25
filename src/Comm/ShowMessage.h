#ifndef SHOWMESSAGE_H
#define SHOWMESSAGE_H

#include <QClipboard>
#include <QDialog>
#include <QGuiApplication>  // Qt6替代QDesktopWidget的核心头文件
#include <QScreen>          // Qt6屏幕信息类
#include <QScroller>

namespace Ui {
class ShowMessage;
}

class MainWindow;
class Notes;

class ShowMessage : public QDialog {
  Q_OBJECT

 public:
  explicit ShowMessage(QWidget* parent = nullptr);
  ~ShowMessage() override;
  Ui::ShowMessage* ui;
  bool showMsg(QString title, QString msgtxt, int btnCount);
  QString AutoFeed(QString text, int nCharCount);

 protected:
  bool eventFilter(QObject* watch, QEvent* evn) override;

 private slots:
  void on_btnCancel_clicked();
  void on_btnOk_clicked();
  void on_btnCopy_clicked();
  void on_btnDel_clicked();
  void on_editMsg_textChanged();

 private:
  MainWindow* mw_one = nullptr;
  Notes* m_Notes = nullptr;
  bool isValue = false;
  int btn_count = 0;
  QString copyText;

  void init(int btnCount);
};

#endif  // SHOWMESSAGE_H
