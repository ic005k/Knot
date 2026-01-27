#ifndef PRINTPDF_H
#define PRINTPDF_H

#include <QCloseEvent>
#include <QDialog>
#include <QHideEvent>  // 新增：包含hideEvent所需头文件
#include <QKeyEvent>

namespace Ui {
class PrintPDF;
}

class PrintPDF : public QDialog {
  Q_OBJECT

 public:
  explicit PrintPDF(QWidget* parent = nullptr);
  ~PrintPDF();
  Ui::PrintPDF* ui;

  QString getItem(QString title, QString lblText, QStringList valueList,
                  int valueIndex);

 protected:
  bool eventFilter(QObject* watch, QEvent* evn) override;
  void closeEvent(QCloseEvent* event) override;
  void hideEvent(QHideEvent* event) override;  // 新增：声明hideEvent

 private slots:
  void on_btnCancel_clicked();
  void on_btnOk_clicked();

 private:
  QString strValue;
  static PrintPDF* m_PrintPDF;
};

#endif  // PRINTPDF_H
