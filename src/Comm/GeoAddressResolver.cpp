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

QString GeoAddressResolver::parseTencentResponse(const QJsonObject &root) {
  int status = root["status"].toInt();
  if (status != 0) {
    qWarning() << "腾讯云API错误：" << root["message"].toString();
    return "";
  }

  QJsonObject result = root["result"].toObject();
  QJsonObject addressObj = result["address_component"].toObject();

  // 提取各层级地址字段
  QString country = addressObj["nation"].toString();
  QString province = addressObj["province"].toString();
  QString city = addressObj["city"].toString();
  QString district = addressObj["district"].toString();
  QString street = addressObj["street"].toString();
  QString streetNumber = addressObj["street_number"].toString();

  QString baseAddress;
  if (country == "中国") {
    // 第一步：预处理街道和门牌号，去除重复前缀
    QString processedStreet = street;
    QString processedStreetNumber = streetNumber;

    // 仅当街道和门牌号都非空时，检查并去除重复前缀
    if (!processedStreet.isEmpty() && !processedStreetNumber.isEmpty()) {
      // 检查门牌号是否以街道名为完整前缀（完全匹配）
      if (processedStreetNumber.startsWith(processedStreet)) {
        // 提取前缀后的剩余部分（门牌号主体）
        QString suffix = processedStreetNumber.mid(processedStreet.length());
        // 确保剩余部分非空（避免门牌号丢失）
        if (!suffix.isEmpty()) {
          processedStreetNumber = suffix;
        }
      }
    }

    // 第二步：按层级拼接并去重连续重复项（整合省/市/区/街道/门牌号）
    QStringList addressParts;

    // 添加省份（非空则保留）
    if (!province.isEmpty()) {
      addressParts << province;
    }

    // 添加城市（非空且与上一层级不同）
    if (!city.isEmpty() &&
        (addressParts.isEmpty() || city != addressParts.last())) {
      addressParts << city;
    }

    // 添加区（非空且与上一层级不同）
    if (!district.isEmpty() &&
        (addressParts.isEmpty() || district != addressParts.last())) {
      addressParts << district;
    }

    // 添加处理后的街道（非空且与上一层级不同）
    if (!processedStreet.isEmpty() &&
        (addressParts.isEmpty() || processedStreet != addressParts.last())) {
      addressParts << processedStreet;
    }

    // 添加处理后的门牌号（非空且与上一层级不同）
    if (!processedStreetNumber.isEmpty() &&
        (addressParts.isEmpty() ||
         processedStreetNumber != addressParts.last())) {
      addressParts << processedStreetNumber;
    }

    // 拼接所有非重复部分
    baseAddress = addressParts.join("");
  } else {
    // 国外地址逻辑（按需可复用国内的去重逻辑）
    baseAddress = QString("%1%2%3").arg(country, province, city);
  }

  // 提取POI信息（逻辑不变）
  QString poiName;
  QJsonArray pois = result["pois"].toArray();
  if (!pois.isEmpty()) {
    QJsonObject firstPoi = pois[0].toObject();
    poiName = firstPoi["title"].toString();
  }

  // 组合最终地址
  if (!poiName.isEmpty()) {
    return QString("%1（%2）").arg(poiName, baseAddress);
  } else {
    return baseAddress;
  }
}
