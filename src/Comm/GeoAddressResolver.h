#ifndef GEOADDRESSRESOLVER_H
#define GEOADDRESSRESOLVER_H

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

class GeoAddressResolver : public QObject {
  Q_OBJECT
 public:
  explicit GeoAddressResolver(QObject *parent = nullptr);

  // 设置腾讯云官方API密钥（仅需这一个key）
  void setTencentApiKey(const QString &apiKey);

  // 核心接口：输入WGS84经纬度，获取位置名称（异步）
  void getAddressFromCoord(double latitude, double longitude);

 signals:
  void addressResolved(const QString &address);  // 成功返回地址
  void resolveFailed(const QString &errorMsg);   // 失败返回错误

 private slots:
  void onReplyFinished(QNetworkReply *reply);  // 网络响应处理

 private:
  // 解析腾讯云官方API响应
  QString parseTencentResponse(const QJsonObject &root);

 private:
  QNetworkAccessManager *m_netMgr;
  QString m_tencentApiKey;  // 腾讯云官方WebService API Key
};

#endif  // GEOADDRESSRESOLVER_H
