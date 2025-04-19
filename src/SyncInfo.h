#ifndef SYNCINFO_H
#define SYNCINFO_H

#include <QDialog>
#include <QKeyEvent>

namespace Ui {
class SyncInfo;
}

class SyncInfo : public QDialog {
  Q_OBJECT

 public:
  explicit SyncInfo(QWidget *parent = nullptr);
  ~SyncInfo();
  Ui::SyncInfo *ui;

  void init();
  void runSync(QString path);

 protected:
  bool eventFilter(QObject *watch, QEvent *evn) override;

 private slots:
  void on_btnClose_clicked();

 private:
  QStringList infoList;
};

#endif  // SYNCINFO_H
