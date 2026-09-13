#ifndef TODORECYCLEBIN_H
#define TODORECYCLEBIN_H

#include <QDialog>

namespace Ui {
class TodoRecycleBin;
}

class TodoRecycleBin : public QDialog {
  Q_OBJECT

 public:
  explicit TodoRecycleBin(QWidget* parent = nullptr);
  ~TodoRecycleBin();

  void showTodoRecycleBin(QStringList list);
 private slots:
  void on_btnClear_clicked();

  void on_btnDel_clicked();

  void on_btnRestore_clicked();

 private:
  Ui::TodoRecycleBin* ui;
};

#endif  // TODORECYCLEBIN_H
