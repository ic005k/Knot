// authserver.h
#ifndef AUTHSERVER_H
#define AUTHSERVER_H

#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUrlQuery>

class AuthServer : public QTcpServer {
  Q_OBJECT
 public:
  explicit AuthServer(QObject *parent = nullptr);
  void start();
  void setCodeVerifier(
      const QString &verifier);  // 用于传递 PKCE 的 code_verifier

 signals:

  // 添加 url 参数
  void authCodeReceived(const QString &code, const QString &codeVerifier,
                        const QString &url);

 private slots:
  void handleNewConnection();

 private:
  QString m_codeVerifier;  // 保存 PKCE 的 code_verifier
};

#endif  // AUTHSERVER_H
