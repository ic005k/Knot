#ifndef RECEIVESHARE_H
#define RECEIVESHARE_H

#include <QClipboard>
#include <QDialog>
#include <QKeyEvent>

#include "src/Comm/Method.h"

namespace Ui {
class ReceiveShare;
}

class ReceiveShare : public QDialog {
  Q_OBJECT

 public:
  explicit ReceiveShare(QWidget *parent = nullptr);
  ~ReceiveShare();
  Ui::ReceiveShare *ui;

  QString shareType;
  QString strReceiveShareData = "test data...";

  void closeAllActiveWindows();
  QObjectList getAllFrame(QObjectList lstUIControls);

  void shareImages(const QString &title, const QStringList &imagesPathList);
  void shareString(const QString &title, const QString &content);

  void shareImage(const QString &title, const QString &path,
                  const QString &fileType);

  QString getShareType();
  QString getShareString();
  QString getShareDone();
  void setShareDone(QString strDone);

  void moveTaskToFront();

  int getCursorPos();
  void setCursorPos(int pos);
  QString getShareMethod();
  void goReceiveShare();

  void closeAllActiveWindowsKeep(QString frameName);

  void closeAllChildWindows();

 protected:
  bool eventFilter(QObject *watch, QEvent *evn) override;

  void closeEvent(QCloseEvent *event) override;

 public slots:

  void on_btnInsertToNote_clicked();
  void on_btnAppendToNote_clicked();

 private slots:

 private:
  int nMethod = 2;
  void Close();
  void addToNote(bool isInsert);

  bool isInsertToNote;
  QString addToNote_Java();
  int getImgCount();
};

#endif  // RECEIVESHARE_H
