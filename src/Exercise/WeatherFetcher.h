#ifndef WEATHERFETCHER_H
#define WEATHERFETCHER_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QString>
#include <QUrl>

#include "src/defines.h"

class WeatherFetcher : public QObject {
  Q_OBJECT

 public:
  // 天气状况枚举
  enum WeatherCondition {
    Clear,         // 晴
    PartlyCloudy,  // 多云
    Cloudy,        // 阴
    Fog,           // 雾
    Rain,          // 雨
    Snow,          // 雪
    Shower,        // 阵雨
    Thunderstorm,  // 雷暴
    Unknown        // 未知
  };
  Q_ENUM(WeatherCondition)

  // 构造函数
  explicit WeatherFetcher(QObject *parent = nullptr) : QObject(parent) {
    // 初始化网络访问管理器
    m_networkManager = new QNetworkAccessManager(this);

    // 连接网络请求完成信号
    connect(m_networkManager, &QNetworkAccessManager::finished, this,
            &WeatherFetcher::onReplyFinished);
  }

  // 核心方法：使用经纬度获取天气信息
  void fetchWeather(double latitude, double longitude) {
    // 构建Open-Meteo API请求URL
    QString urlString =
        QString(
            "https://api.open-meteo.com/v1/forecast?"
            "latitude=%1&longitude=%2&"
            "current=apparent_temperature,weather_code&"  // 请求体感温度和天气代码
            "timezone=auto&"
            "units=metric")            // 确保返回摄氏度
            .arg(latitude, 0, 'f', 6)  // 保留6位小数的精度
            .arg(longitude, 0, 'f', 6);

    // 发送GET请求
    m_networkManager->get(QNetworkRequest(QUrl(urlString)));
  }

  // 静态方法：将天气代码转换为枚举
  static WeatherCondition weatherCodeToCondition(int code) {
    if (code >= 0 && code <= 2) return Clear;  // 晴天/少云
    if (code == 3) return PartlyCloudy;        // 多云
    if (code >= 45 && code <= 48) return Fog;  // 雾
    if ((code >= 51 && code <= 57) || (code >= 61 && code <= 67))
      return Rain;                                      // 雨
    if (code >= 71 && code <= 77) return Snow;          // 雪
    if (code >= 80 && code <= 86) return Shower;        // 阵雨
    if (code >= 95 && code <= 99) return Thunderstorm;  // 雷暴
    return Unknown;                                     // 未知
  }

  // 静态方法：将天气枚举转换为中文描述
  static QString conditionToChinese(WeatherCondition condition) {
    switch (condition) {
      case Clear:
        return "晴";
      case PartlyCloudy:
        return "多云";
      case Cloudy:
        return "阴";
      case Fog:
        return "雾";
      case Rain:
        return "雨";
      case Snow:
        return "雪";
      case Shower:
        return "阵雨";
      case Thunderstorm:
        return "雷暴";
      default:
        return "";
    }
  }

  static QString conditionToEnglish(WeatherCondition condition) {
    switch (condition) {
      case Clear:
        return "Clear";
      case PartlyCloudy:
        return "PartlyCloudy";
      case Cloudy:
        return "Cloudy";
      case Fog:
        return "Fog";
      case Rain:
        return "Rain";
      case Snow:
        return "Snow";
      case Shower:
        return "Shower";
      case Thunderstorm:
        return "Thunderstorm";
      default:
        return "";
    }
  }

  // 将天气枚举转换为Unicode符号
  static QString conditionToUnicode(WeatherCondition condition) {
    switch (condition) {
      case Clear:
        return "/res/weather/clear.svg";  // 晴天
      case PartlyCloudy:
        return "/res/weather/partly_cloudy.svg";  // 多云
      case Cloudy:
        return "/res/weather/cloudy.svg";  // 阴天
      case Fog:
        return "/res/weather/fog.svg";  // 雾
      case Rain:
        return "/res/weather/rain.svg";  // 雨
      case Snow:
        return "/res/weather/snow.svg";  // 雪
      case Shower:
        return "/res/weather/shower.svg";  // 阵雨
      case Thunderstorm:
        return "/res/weather/thunderstorm.svg";  // 雷暴
      default:
        return "";  // 未知
    }
  }

  static QString conditionToUnicode_test(WeatherCondition condition) {
    switch (condition) {
      case Clear:
        return "☀️";  // 晴天
      case PartlyCloudy:
        return "🌤️";  // 替代多云符号，安卓支持更好
      case Cloudy:
        return "☁️";  // 阴天
      case Fog:
        return "🌫️";  // 雾
      case Rain:
        return "🌧️";  // 雨
      case Snow:
        return "❄️";  // 雪
      case Shower:
        return "🌦️";  // 阵雨
      case Thunderstorm:
        return "🌩️";  // 雷暴
      default:
        return "";  // 未知
    }
  }

 signals:
  // 天气数据获取成功信号，返回体感温度和天气代码
  void weatherUpdated(double apparentTemperature, int weatherCode);

  // 重载信号：返回体感温度、天气代码和天气描述
  void weatherUpdated(double apparentTemperature, int weatherCode,
                      const QString &weatherDesc);

  // 错误信号，返回错误信息
  void errorOccurred(const QString &errorMessage);

 private slots:
  // 网络请求完成处理
  void onReplyFinished(QNetworkReply *reply) {
    // 检查网络错误
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(tr("网络错误: %1").arg(reply->errorString()));
      reply->deleteLater();
      return;
    }

    // 读取并解析JSON响应
    QByteArray responseData = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

    if (jsonDoc.isNull()) {
      emit errorOccurred(tr("无法解析天气数据"));
      reply->deleteLater();
      return;
    }

    QJsonObject rootObj = jsonDoc.object();

    // 提取数据
    if (rootObj.contains("current") && rootObj["current"].isObject()) {
      QJsonObject currentObj = rootObj["current"].toObject();

      // 验证体感温度
      if (!currentObj.contains("apparent_temperature") ||
          !currentObj["apparent_temperature"].isDouble()) {
        emit errorOccurred(tr("未获取到体感温度数据"));
        reply->deleteLater();
        return;
      }

      // 验证天气代码
      if (!currentObj.contains("weather_code") ||
          !currentObj["weather_code"].isDouble()) {
        emit errorOccurred(tr("未获取到天气代码数据"));
        reply->deleteLater();
        return;
      }

      // 解析数据
      double apparentTemperature =
          currentObj["apparent_temperature"].toDouble();
      int weatherCode = currentObj["weather_code"].toInt();
      QString weatherDesc;

      if (isZH_CN)
        weatherDesc = conditionToChinese(weatherCodeToCondition(weatherCode));
      else
        weatherDesc = conditionToEnglish(weatherCodeToCondition(weatherCode));

      // 发射两个信号，方便不同场景使用
      emit weatherUpdated(apparentTemperature, weatherCode);
      emit weatherUpdated(apparentTemperature, weatherCode, weatherDesc);

      reply->deleteLater();
      return;
    }

    // 数据格式不符合预期
    emit errorOccurred(tr("天气数据格式错误"));
    reply->deleteLater();
  }

 private:
  // 网络访问管理器
  QNetworkAccessManager *m_networkManager;
};

#endif  // WEATHERFETCHER_H
