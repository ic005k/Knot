// authserver.cpp
#include "authserver.h"

AuthServer::AuthServer(QObject *parent) : QTcpServer(parent) {
  connect(this, &QTcpServer::newConnection, this,
          &AuthServer::handleNewConnection);
}

void AuthServer::start() {
  if (!listen(QHostAddress::LocalHost, 8080)) {
    qDebug() << "Server启动失败:" << errorString();
  } else {
    qDebug() << "Server已启动，监听端口 8080";  // 添加成功日志
  }
}

void AuthServer::setCodeVerifier(const QString &verifier) {
  m_codeVerifier = verifier;
}

void AuthServer::handleNewConnection() {
  QTcpSocket *socket = nextPendingConnection();
  connect(socket, &QTcpSocket::readyRead, [this, socket]() {
    QString request = socket->readAll();
    if (request.contains("GET /?code=")) {
      // 提取完整路径（如 "/?code=xxx"）
      QString path =
          request.split(" ")[1];  // 示例值: "/?code=xxxxx&session_state=..."

      // 构造完整 URL（如 "http://localhost:8080/?code=xxxxx"）
      QString fullUrl = "http://localhost:8080" + path;

      // 解析 code
      QUrl url(fullUrl);
      QUrlQuery query(url);
      QString code = query.queryItemValue("code");

      // 发射信号时传递完整 URL
      emit authCodeReceived(code, m_codeVerifier, fullUrl);
    }
  });
}
