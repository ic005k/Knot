#ifndef BAKFILELIST_H
#define BAKFILELIST_H

#include <QDialog>

namespace Ui {
class BakFileList;
}

class BakFileList : public QDialog {
  Q_OBJECT

 public:
  explicit BakFileList(QWidget* parent = nullptr);
  ~BakFileList();

  void loadBakFile();

 protected:
  void closeEvent(QCloseEvent* event) override;

 private slots:
  void on_btnRestore_clicked();

 private:
  Ui::BakFileList* ui;
};

#endif  // BAKFILELIST_H
