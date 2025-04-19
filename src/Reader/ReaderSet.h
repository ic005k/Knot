#ifndef READERSET_H
#define READERSET_H

#include <QDialog>

namespace Ui {
class ReaderSet;
}

class ReaderSet : public QDialog {
  Q_OBJECT

 public:
  explicit ReaderSet(QWidget *parent = nullptr);
  ~ReaderSet();
  Ui::ReaderSet *ui;

  void init();

  void updateProgress();

  void saveScrollValue();

 protected:
  bool eventFilter(QObject *watch, QEvent *evn) override;

 public slots:
  void on_btnBack_clicked();

 public slots:
  void on_hSlider_sliderReleased();

  void on_btnFontPlus_clicked();

  void on_btnFontLess_clicked();

  void on_hSlider_sliderMoved(int position);

  void on_btnStyle1_clicked();

  void on_btnStyle2_clicked();

  void on_btnStyle3_clicked();

  void on_btnFont_clicked();

  void on_hSlider_valueChanged(int value);

  void on_btnGoPage_clicked();

  void on_btnBackgroundColor_clicked();

  void on_btnForegroundColor_clicked();

  void on_editBackgroundColor_textChanged(const QString &arg1);

  void on_editForegroundColor_textChanged(const QString &arg1);

  void on_btnSetBookmark_clicked();

  void on_btnLessen_clicked();

  void on_btnDefault_clicked();

  void on_btnAdd_clicked();

  void on_btnClear_clicked();

 private slots:

 private:
};

#endif  // READERSET_H
