#ifndef WEATHERFETCHER_H
#define WEATHERFETCHER_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QString>
#include <QThread>
#include <QUrl>

// #include "src/defines.h" // 根据实际路径保留

// 前置声明，避免循环依赖
class WeatherFetcherPrivate;

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

  // 简化的单例模式（无lambda，彻底兼容VS）
  static WeatherFetcher* instance() {
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    static WeatherFetcher* s_instance = nullptr;

    if (!s_instance) {
      // 1. 创建专用工作线程（全局静态，避免栈变量生命周期问题）
      static QThread* s_workerThread = new QThread();
      s_workerThread->setObjectName("WeatherFetcherThread");

      // 2. 创建实例（无父对象）
      s_instance = new WeatherFetcher();

      // 3. 移动实例到工作线程
      s_instance->moveToThread(s_workerThread);

      // 4. 启动线程（线程启动后自动初始化网络管理器）
      s_workerThread->start();

      // 5. 线程清理逻辑（无需lambda，直接连接）
      QObject::connect(s_workerThread, &QThread::finished, s_workerThread,
                       &QThread::deleteLater);
      QObject::connect(s_instance, &QObject::destroyed, s_workerThread,
                       &QThread::quit);
    }
    return s_instance;
  }

  // 核心方法：使用经纬度获取天气信息（线程安全）
  Q_INVOKABLE void fetchWeather(double latitude, double longitude) {
    // 检查当前调用线程是否是对象归属线程
    if (QThread::currentThread() != thread()) {
      // 跨线程调用：通过 Qt 元对象系统异步调用
      QMetaObject::invokeMethod(this, "fetchWeather", Qt::QueuedConnection,
                                Q_ARG(double, latitude),
                                Q_ARG(double, longitude));
      return;
    }

    // 确保网络管理器已初始化（延迟初始化，第一次调用时初始化）
    if (!m_networkManager) {
      initNetworkManager();
    }

    // 构建Open-Meteo API请求URL
    QString urlString = QString(
                            "https://api.open-meteo.com/v1/forecast?"
                            "latitude=%1&longitude=%2&"
                            "current=apparent_temperature,weather_code&"
                            "timezone=auto&"
                            "units=metric")
                            .arg(latitude, 0, 'f', 6)
                            .arg(longitude, 0, 'f', 6);

    // 发送GET请求（在对象归属线程中执行）
    m_networkManager->get(QNetworkRequest(QUrl(urlString)));
  }

  // 静态方法：将天气代码转换为枚举（保持不变）
  static WeatherCondition weatherCodeToCondition(int code) {
    if (code >= 0 && code <= 2) return Clear;
    if (code == 3) return PartlyCloudy;
    if (code >= 45 && code <= 48) return Fog;
    if ((code >= 51 && code <= 57) || (code >= 61 && code <= 67)) return Rain;
    if (code >= 71 && code <= 77) return Snow;
    if (code >= 80 && code <= 86) return Shower;
    if (code >= 95 && code <= 99) return Thunderstorm;
    return Unknown;
  }

  // 其他静态方法（conditionToChinese/English/Unicode等）保持不变
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

  static QString conditionToUnicode(WeatherCondition condition) {
    switch (condition) {
      case Clear:
        return "/res/weather/clear.svg";
      case PartlyCloudy:
        return "/res/weather/partly_cloudy.svg";
      case Cloudy:
        return "/res/weather/cloudy.svg";
      case Fog:
        return "/res/weather/fog.svg";
      case Rain:
        return "/res/weather/rain.svg";
      case Snow:
        return "/res/weather/snow.svg";
      case Shower:
        return "/res/weather/shower.svg";
      case Thunderstorm:
        return "/res/weather/thunderstorm.svg";
      default:
        return "";
    }
  }

  static QString conditionToUnicode_test(WeatherCondition condition) {
    switch (condition) {
      case Clear:
        return "☀️";
      case PartlyCloudy:
        return "🌤️";
      case Cloudy:
        return "☁️";
      case Fog:
        return "🌫️";
      case Rain:
        return "🌧️";
      case Snow:
        return "❄️";
      case Shower:
        return "🌦️";
      case Thunderstorm:
        return "🌩️";
      default:
        return "";
    }
  }

 signals:
  void weatherUpdated(double apparentTemperature, int weatherCode);
  void weatherUpdated(double apparentTemperature, int weatherCode,
                      const QString& weatherDesc);
  void errorOccurred(const QString& errorMessage);

 private slots:
  // 网络请求完成处理（保持不变）
  void onReplyFinished(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(tr("网络错误: %1").arg(reply->errorString()));
      reply->deleteLater();
      return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

    if (jsonDoc.isNull()) {
      emit errorOccurred(tr("无法解析天气数据"));
      reply->deleteLater();
      return;
    }

    QJsonObject rootObj = jsonDoc.object();
    if (rootObj.contains("current") && rootObj["current"].isObject()) {
      QJsonObject currentObj = rootObj["current"].toObject();

      if (!currentObj.contains("apparent_temperature") ||
          !currentObj["apparent_temperature"].isDouble()) {
        emit errorOccurred(tr("未获取到体感温度数据"));
        reply->deleteLater();
        return;
      }

      if (!currentObj.contains("weather_code") ||
          !currentObj["weather_code"].isDouble()) {
        emit errorOccurred(tr("未获取到天气代码数据"));
        reply->deleteLater();
        return;
      }

      double apparentTemperature =
          currentObj["apparent_temperature"].toDouble();
      int weatherCode = currentObj["weather_code"].toInt();
      // 替换 isZH_CN 为实际判断逻辑（如果没有定义，可先默认中文）
      bool isZH_CN = true;
      QString weatherDesc =
          isZH_CN ? conditionToChinese(weatherCodeToCondition(weatherCode))
                  : conditionToEnglish(weatherCodeToCondition(weatherCode));

      emit weatherUpdated(apparentTemperature, weatherCode);
      emit weatherUpdated(apparentTemperature, weatherCode, weatherDesc);
    } else {
      emit errorOccurred(tr("天气数据格式错误"));
    }

    reply->deleteLater();
  }

 private:
  // 私有化构造函数（单例模式）
  explicit WeatherFetcher(QObject* parent = nullptr)
      : QObject(parent), m_networkManager(nullptr) {}

  // 禁止拷贝
  WeatherFetcher(const WeatherFetcher&) = delete;
  WeatherFetcher& operator=(const WeatherFetcher&) = delete;

  // 初始化网络管理器（第一次调用fetchWeather时初始化）
  void initNetworkManager() {
    if (!m_networkManager) {
      m_networkManager = new QNetworkAccessManager(this);
      connect(m_networkManager, &QNetworkAccessManager::finished, this,
              &WeatherFetcher::onReplyFinished);
    }
  }

  QNetworkAccessManager* m_networkManager;
};

#endif  // WEATHERFETCHER_H