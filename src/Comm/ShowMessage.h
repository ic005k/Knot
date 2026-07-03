#ifndef SHOWMESSAGE_H
#define SHOWMESSAGE_H

#include <QClipboard>
#include <QDialog>
#include <QGuiApplication>
#include <QScreen>
#include <QScroller>
#include <QTextEdit>

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

  void init(int btnCount, int adaptiveH);

  // 根据文本内容获取QTextEdit完整内容高度
  int getTextEditContentHeight(QTextEdit* edit);
  // 计算对话框所需总高度
  int calcDialogTotalHeight(int textH, int btnCount);
};

#endif  // SHOWMESSAGE_H
