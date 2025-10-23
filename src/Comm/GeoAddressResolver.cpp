#include "GeoAddressResolver.h"

#include <QDateTime>
#include <QJsonDocument>
#include <QUrlQuery>

GeoAddressResolver::GeoAddressResolver(QObject *parent) : QObject(parent) {
  m_netMgr = new QNetworkAccessManager(this);
  connect(m_netMgr, &QNetworkAccessManager::finished, this,
          &GeoAddressResolver::onReplyFinished);
}

// 设置腾讯云官方API密钥
void GeoAddressResolver::setTencentApiKey(const QString &apiKey) {
  m_tencentApiKey = apiKey;
}

// 发起逆地理编码请求（输入WGS84经纬度）
void GeoAddressResolver::getAddressFromCoord(double latitude,
                                             double longitude) {
  // 校验坐标有效性
  if (latitude < -90 || latitude > 90 || longitude < -180 || longitude > 180) {
    emit resolveFailed("无效经纬度");
    return;
  }

  // 校验API密钥
  if (m_tencentApiKey.isEmpty()) {
    emit resolveFailed("腾讯云API密钥未配置");
    return;
  }

  // 腾讯云官方逆地理编码API地址
  QUrl url("https://apis.map.qq.com/ws/geocoder/v1/");
  QUrlQuery query;
  query.addQueryItem("location", QString("%1,%2").arg(latitude).arg(
                                     longitude));  // 格式：lat,lng（WGS84）
  query.addQueryItem("key", m_tencentApiKey);
  query.addQueryItem("output", "json");
  query.addQueryItem("get_poi", "0");  // 0=不返回POI，1=返回（按需设置）
  url.setQuery(query);

  // 发起网络请求
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  m_netMgr->get(request);
}

// 解析网络响应
void GeoAddressResolver::onReplyFinished(QNetworkReply *reply) {
  if (reply->error() != QNetworkReply::NoError) {
    emit resolveFailed(QString("网络错误：%1").arg(reply->errorString()));
    reply->deleteLater();
    return;
  }

  // 解析JSON数据
  QByteArray data = reply->readAll();
  QJsonDocument doc = QJsonDocument::fromJson(data);
  if (!doc.isObject()) {
    emit resolveFailed("响应数据格式错误");
    reply->deleteLater();
    return;
  }

  QString address = parseTencentResponse(doc.object());
  if (!address.isEmpty()) {
    emit addressResolved(address);
  } else {
    emit resolveFailed("未获取到位置信息");
  }

  reply->deleteLater();
}

// 解析腾讯云官方API响应数据
QString GeoAddressResolver::parseTencentResponse(const QJsonObject &root) {
  int status = root["status"].toInt();
  if (status != 0)  // status=0表示成功
  {
    qWarning() << "腾讯云API错误：" << root["message"].toString();
    return "";
  }

  // 提取格式化地址（支持多语言，默认中文）
  QJsonObject result = root["result"].toObject();
  QJsonObject addressObj = result["address_component"].toObject();

  // 组装地址（按需调整格式，例如：国家-省份-城市-街道）
  QString country = addressObj["nation"].toString();
  QString province = addressObj["province"].toString();
  QString city = addressObj["city"].toString();
  QString district = addressObj["district"].toString();
  QString street = addressObj["street"].toString();
  QString streetNumber = addressObj["street_number"].toString();

  // 简化格式：国内地址显示到街道/门牌号，国外显示到城市
  if (country == "中国") {
    return QString("%1%2%3%4%5")
        .arg(province)
        .arg(city)
        .arg(district)
        .arg(street)
        .arg(streetNumber);
  } else {
    return QString("%1%2%3").arg(country).arg(province).arg(city);
  }
}
