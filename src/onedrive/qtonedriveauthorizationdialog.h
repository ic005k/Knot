#ifndef ONEDRIVEAUTHORIZATIONDIALOG_H
#define ONEDRIVEAUTHORIZATIONDIALOG_H

#include <QDialog>
#include <QQuickWidget>
#include <QUrl>

#include "authserver.h"

class QWebEngineView;

class QtOneDriveAuthorizationDialog : public QDialog {
  Q_OBJECT

 public:
  explicit QtOneDriveAuthorizationDialog(const QUrl &url, QWidget *parent = 0);
  ~QtOneDriveAuthorizationDialog();

  QWebEngineView *webView_ = nullptr;
  static bool isExists() { return isExists_; }
  void setNeedToClose(bool close) { isNeedToClose_ = close; }
  void sendMsg(QString strUri);

 private slots:
  void on_timer();
  void on_webView_loadFinished(bool arg1);

  void handleAuthCode(const QString &code, const QString &codeVerifier,
                      const QString &url);

 signals:
  void success(const QUrl &url);
  void error(const QUrl &url);

 private:
  static bool isExists_;
  bool isNeedToClose_ = false;
  QUrl url_;
  QTimer *timer;

  AuthServer *m_authServer;
  QString m_codeVerifier;  // 保存 PKCE 的 code_verifier
};

#endif  // ONEDRIVEAUTHORIZATIONDIALOG_H
