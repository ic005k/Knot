#ifndef MOVETO_H
#define MOVETO_H

#include <QDialog>
#include <QTreeWidgetItem>

namespace Ui {
class MoveTo;
}

class MoveTo : public QDialog {
  Q_OBJECT

 public:
  explicit MoveTo(QWidget *parent = nullptr);
  ~MoveTo();
  Ui::MoveTo *ui;
  QString strCurrentItem;
  QTreeWidgetItem *currentItem;
  bool isOk = false;

  void initTopNoteBook();
  void initAllNoteBook();

 protected:
  void closeEvent(QCloseEvent *event) override;
  bool eventFilter(QObject *watch, QEvent *evn) override;
 private slots:
  void on_btnCancel_clicked();

  void on_btnOk_clicked();

 private:
  QList<QTreeWidgetItem *> listItems;
  void showDialog();
  QWidget *m_widget;
  bool isNoteBook = false;
  bool isNote = false;
};

#endif  // MOVETO_H
