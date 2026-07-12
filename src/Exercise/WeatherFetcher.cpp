#include "WeatherFetcher.h"

#include <QMetaObject>
#include <QMutexLocker>
#include <QString>

WeatherFetcher* WeatherFetcher::instance() {
  static QMutex mutex;
  QMutexLocker locker(&mutex);
  static WeatherFetcher* s_instance = nullptr;

  /*if (!s_instance) {
    // 专用工作线程静态全局
    static QThread* s_workerThread = new QThread();
    s_workerThread->setObjectName("WeatherFetcherThread");

    // 创建无父实例
    s_instance = new WeatherFetcher();
    s_instance->moveToThread(s_workerThread);

    s_workerThread->start();

    // 线程自动释放连接
    QObject::connect(s_workerThread, &QThread::finished, s_workerThread,
                     &QThread::deleteLater);
    QObject::connect(s_instance, &QObject::destroyed, s_workerThread,
                     &QThread::quit);
  }*/

  if (!s_instance) {
    s_instance = new WeatherFetcher();
    s_instance->m_workerThread = new QThread();
    s_instance->m_workerThread->setObjectName("WeatherFetcherThread");
    s_instance->moveToThread(s_instance->m_workerThread);
    s_instance->m_workerThread->start();
  }

  return s_instance;
}

void WeatherFetcher::fetchWeather(double latitude, double longitude) {
  // 跨线程异步转发
  if (QThread::currentThread() != thread()) {
    QMetaObject::invokeMethod(this, "fetchWeather", Qt::QueuedConnection,
                              Q_ARG(double, latitude),
                              Q_ARG(double, longitude));
    return;
  }

  if (!m_networkManager) initNetworkManager();

  // 拼接API地址
  QString urlString = QString(
                          "https://api.open-meteo.com/v1/forecast?"
                          "latitude=%1&longitude=%2&"
                          "current=apparent_temperature,weather_code&"
                          "timezone=auto&"
                          "units=metric")
                          .arg(latitude, 0, 'f', 6)
                          .arg(longitude, 0, 'f', 6);

  m_networkManager->get(QNetworkRequest(QUrl(urlString)));
}

WeatherFetcher::WeatherCondition WeatherFetcher::weatherCodeToCondition(
    int code) {
  if (code >= 0 && code <= 2) return Clear;
  if (code == 3) return PartlyCloudy;
  if (code >= 45 && code <= 48) return Fog;
  if ((code >= 51 && code <= 57) || (code >= 61 && code <= 67)) return Rain;
  if (code >= 71 && code <= 77) return Snow;
  if (code >= 80 && code <= 86) return Shower;
  if (code >= 95 && code <= 99) return Thunderstorm;
  return Unknown;
}

QString WeatherFetcher::conditionToChinese(
    WeatherFetcher::WeatherCondition condition) {
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

QString WeatherFetcher::conditionToEnglish(
    WeatherFetcher::WeatherCondition condition) {
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

QString WeatherFetcher::conditionToUnicode(
    WeatherFetcher::WeatherCondition condition) {
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

QString WeatherFetcher::conditionToUnicode_test(
    WeatherFetcher::WeatherCondition condition) {
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

void WeatherFetcher::onReplyFinished(QNetworkReply* reply) {
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

    double apparentTemperature = currentObj["apparent_temperature"].toDouble();
    int weatherCode = currentObj["weather_code"].toInt();
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

WeatherFetcher::WeatherFetcher(QObject* parent)
    : QObject(parent), m_networkManager(nullptr) {}

void WeatherFetcher::initNetworkManager() {
  if (!m_networkManager) {
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished, this,
            &WeatherFetcher::onReplyFinished);
  }
}

void WeatherFetcher::shutdown() {
  qDebug() << "[WeatherFetcher] shutdown() called, thread:"
           << QThread::currentThread();

  // ✅ 只允许在 WeatherFetcher 所在线程执行网络清理
  if (QThread::currentThread() != thread()) {
    QMetaObject::invokeMethod(this, "shutdown", Qt::QueuedConnection);
    return;
  }

  if (m_networkManager) {
    // 中止所有未完成的请求
    for (QNetworkReply* reply :
         m_networkManager->findChildren<QNetworkReply*>()) {
      reply->abort();
    }
    m_networkManager->deleteLater();
    m_networkManager = nullptr;
  }

  // ✅ 只做资源清理，不做线程管理
}