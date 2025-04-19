#ifndef TEXTSELECTOR_H
#define TEXTSELECTOR_H

#include <QClipboard>
#include <QDialog>
#include <QKeyEvent>

namespace Ui {
class TextSelector;
}

class TextSelector : public QDialog {
  Q_OBJECT

 public:
  explicit TextSelector(QWidget *parent = nullptr);
  ~TextSelector();
  Ui::TextSelector *ui;

  void init(int y);
  int oriHeight;

 protected:
  bool eventFilter(QObject *watch, QEvent *evn) override;

 public slots:
  void on_btnLeft1_clicked();

  void on_btnLeft0_clicked();

  void on_btnRight1_clicked();

  void on_btnRight0_clicked();

  void on_btnClose_clicked();

 private slots:

  void on_btnCopy_clicked();

  void on_btnCut_clicked();

  void on_btnPaste_clicked();

  void on_btnSetAll_clicked();

  void on_btnBing_clicked();

  void on_btnDel_clicked();

  void on_btnShareTxt_clicked();

 private:
  bool isMouseRelease = false;
  bool isMousePress = false;
  bool isMouseMove = false;
  QString strByCopyText;
};

#endif  // TEXTSELECTOR_H
