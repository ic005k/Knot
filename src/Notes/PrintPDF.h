#ifndef PRINTPDF_H
#define PRINTPDF_H

#include <QDialog>
#include <QKeyEvent>

namespace Ui {
class PrintPDF;
}

class PrintPDF : public QDialog {
  Q_OBJECT

 public:
  explicit PrintPDF(QWidget *parent = nullptr);
  ~PrintPDF();
  Ui::PrintPDF *ui;

  QString getItem(QString title, QString lblText, QStringList valueList,
                  int valueIndex);

 protected:
  bool eventFilter(QObject *watch, QEvent *evn) override;

  void closeEvent(QCloseEvent *event) override;
 private slots:
  void on_btnCancel_clicked();

  void on_btnOk_clicked();

 private:
  QString strValue;
};

#endif  // PRINTPDF_H
