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

  // 关键1：坐标保留6位小数（避免精度丢失）
  QString latStr = QString::number(latitude, 'f', 6);  // 强制6位小数
  QString lonStr = QString::number(longitude, 'f', 6);

  // 腾讯云官方逆地理编码API地址
  QUrl url("https://apis.map.qq.com/ws/geocoder/v1/");
  QUrlQuery query;
  query.addQueryItem("location", QString("%1,%2").arg(latStr, lonStr));
  query.addQueryItem("key", m_tencentApiKey);
  query.addQueryItem("output", "json");
  // 关键2：开启POI检索，并限定范围和类型
  query.addQueryItem("get_poi", "1");  // 1=返回POI（建筑、公司等）
  query.addQueryItem(
      "poi_options",
      "radius=50;category=公司企业,楼宇;page_size=1");  // 50米内优先返回建筑
  url.setQuery(query);

  // 发起网络请求
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  m_netMgr->get(request);
}

// 解析腾讯云官方API响应数据
QString GeoAddressResolver::parseTencentResponse(const QJsonObject &root) {
  int status = root["status"].toInt();
  if (status != 0)  // status=0表示成功
  {
    qWarning() << "腾讯云API错误：" << root["message"].toString();
    return "";
  }

  QJsonObject result = root["result"].toObject();

  // 1. 提取基础地址（省/市/区/街道，保留你的原有逻辑）
  QJsonObject addressObj = result["address_component"].toObject();
  QString country = addressObj["nation"].toString();
  QString province = addressObj["province"].toString();
  QString city = addressObj["city"].toString();
  QString district = addressObj["district"].toString();
  QString street = addressObj["street"].toString();
  QString streetNumber = addressObj["street_number"].toString();

  // 基础地址字符串（如“广东省深圳市南山区高新中四道”）
  QString baseAddress;
  if (country == "中国") {
    baseAddress = QString("%1%2%3%4%5")
                      .arg(province)
                      .arg(city)
                      .arg(district)
                      .arg(street)
                      .arg(streetNumber);
  } else {
    baseAddress = QString("%1%2%3").arg(country).arg(province).arg(city);
  }

  // 2. 新增：提取POI（建筑/公司）信息（关键！）
  QString poiName;                             // 建筑名称
  QJsonArray pois = result["pois"].toArray();  // API返回的POI数组
  if (!pois.isEmpty()) {
    // 取第一个POI（距离坐标最近的建筑）
    QJsonObject firstPoi = pois[0].toObject();
    poiName = firstPoi["title"].toString();  // 建筑名称（如“高新科技园W1栋”）
  }

  // 3. 组合完整地址（建筑名 + 基础地址）
  if (!poiName.isEmpty()) {
    return QString("%1（%2）")
        .arg(poiName)
        .arg(
            baseAddress);  // 如“高新科技园W1栋（广东省深圳市南山区高新中四道）”
  } else {
    return baseAddress;  // 若无POI，返回基础地址（兼容原有逻辑）
  }
}
