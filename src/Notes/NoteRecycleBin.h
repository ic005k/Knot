#ifndef NOTERECYCLEBIN_H
#define NOTERECYCLEBIN_H

#include <QDialog>

namespace Ui {
class NoteRecycleBin;
}

class NoteRecycleBin : public QDialog {
  Q_OBJECT

 public:
  explicit NoteRecycleBin(QWidget* parent = nullptr);
  ~NoteRecycleBin();

  void showNoteRecycleBin(QStringList list);
  void setDataToList(QStringList list);
  QStringList getCheckedPaths() const;
  private slots:
  void on_btnSelAll_clicked();

  void on_btnDesAll_clicked();

  void on_btnRestore_clicked();

  void on_btnDel_clicked();

 private:
  Ui::NoteRecycleBin* ui;
};

#endif  // NOTERECYCLEBIN_H
