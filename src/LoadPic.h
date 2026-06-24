#ifndef LOADPIC_H
#define LOADPIC_H

#include <QByteArray>
#include <QDebug>
#include <QDialog>
#include <QFile>
#include <QImage>
#include <QKeyEvent>
#include <QString>

namespace Ui {
class LoadPic;
}

class LoadPic : public QDialog {
  Q_OBJECT

 public:
  explicit LoadPic(QWidget* parent = nullptr);
  ~LoadPic();
  Ui::LoadPic* ui;

  void initMain(QString imgFile);

  bool saveBase64ToPng(const QString &base64Str, const QString &savePath);
  protected:
  bool eventFilter(QObject* watch, QEvent* evn) override;

 private slots:

 private:
};

#endif  // LOADPIC_H
