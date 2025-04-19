#ifndef NEWNOTEBOOK_H
#define NEWNOTEBOOK_H

#include <QDialog>
#include <QKeyEvent>

namespace Ui {
class NewNoteBook;
}

class NewNoteBook : public QDialog {
  Q_OBJECT

 public:
  explicit NewNoteBook(QWidget *parent = nullptr);
  ~NewNoteBook();
  Ui::NewNoteBook *ui;

  QString notebookName;
  QString notebookRoot;
  bool isOk = false;
  int rootIndex;

 protected:
  bool eventFilter(QObject *watch, QEvent *evn) override;

  void closeEvent(QCloseEvent *event) override;
 private slots:
  void on_btnCancel_clicked();

  void on_btnOk_clicked();

 private:
};

#endif  // NEWNOTEBOOK_H
