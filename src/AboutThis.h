#ifndef ABOUTTHIS_H
#define ABOUTTHIS_H

#include <QDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include "src/AutoUpdate.h"

namespace Ui {
class AboutThis;
}

class AboutThis : public QDialog {
  Q_OBJECT

 public:
  explicit AboutThis(QWidget *parent = nullptr);
  ~AboutThis();
  Ui::AboutThis *ui;

  AutoUpdate *m_AutoUpdate;
  QNetworkAccessManager *manager;
  int parse_UpdateJSON(QString str);
  bool blAutoCheckUpdate = false;

  int sliderPos;

  void CheckUpdate();
  void show_download();
  int getAndroidVer();
 public slots:

  void on_btnBack_About_clicked();

 protected:
  void keyReleaseEvent(QKeyEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  bool eventFilter(QObject *obj, QEvent *evn) override;

 private slots:

  void on_btnHomePage_clicked();

  void replyFinished(QNetworkReply *reply);

  void on_btnCheckUpdate_clicked();

  void on_btnDownloadUP_clicked();

 private:
  QString getUrl(QVariantList list);
  QString s_link;
};

#endif  // ABOUTTHIS_H
