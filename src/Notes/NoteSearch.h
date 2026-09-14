#ifndef NOTESEARCH_H
#define NOTESEARCH_H

#include <QDialog>

namespace Ui {
class NoteSearch;
}

class NoteSearch : public QDialog {
  Q_OBJECT

 public:
  explicit NoteSearch(QWidget* parent = nullptr);
  ~NoteSearch();

  void showNoteSearch();

  void setDataToList(QStringList list);
  private slots:
  void on_btnSearch_clicked();

  void on_btnView_clicked();

  void on_btnEdit_clicked();

  void on_editSearch_textChanged(const QString& arg1);

 private:
  Ui::NoteSearch* ui;
};

#endif  // NOTESEARCH_H
