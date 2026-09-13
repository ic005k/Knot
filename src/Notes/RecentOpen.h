#ifndef RECENTOPEN_H
#define RECENTOPEN_H

#include <QDialog>

namespace Ui {
class RecentOpen;
}

class RecentOpen : public QDialog {
  Q_OBJECT

 public:
  explicit RecentOpen(QWidget* parent = nullptr);
  ~RecentOpen();

  void showRecentOpen(QStringList list);
  void setDataToRecentList(QStringList list);
 private slots:
  void on_btnView_clicked();

  void on_btnEdit_clicked();

  void on_editKeyword_textChanged(const QString& arg1);

 private:
  Ui::RecentOpen* ui;
};

#endif  // RECENTOPEN_H
