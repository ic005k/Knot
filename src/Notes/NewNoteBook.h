#ifndef NEWNOTEBOOK_H
#define NEWNOTEBOOK_H

#include <QDialog>
#include <QKeyEvent>

#include "src/defines.h"
#include "ui_NewNoteBook.h"

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

  void showDialog();

 protected:
  bool eventFilter(QObject *watch, QEvent *evn) override;

  void closeEvent(QCloseEvent *event) override;
 private slots:
  void on_btnCancel_clicked();

  void on_btnOk_clicked();

 private:
  EditEventFilter *editFilter = nullptr;
};

#endif  // NEWNOTEBOOK_H
