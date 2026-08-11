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

  // 单例入口
  static WeatherFetcher* instance();

  Q_INVOKABLE void shutdown();

  QThread* workerThread() const { return m_workerThread; }

  // 核心方法：使用经纬度获取天气信息（线程安全）
  Q_INVOKABLE void fetchWeather(double latitude, double longitude);

  // 静态转换工具方法
  static WeatherCondition weatherCodeToCondition(int code);
  static QString conditionToChinese(WeatherCondition condition);
  static QString conditionToEnglish(WeatherCondition condition);
  static QString conditionToUnicode(WeatherCondition condition);
  static QString conditionToUnicode_test(WeatherCondition condition);

 signals:
  void weatherUpdated(double apparentTemperature, int weatherCode);
  void weatherUpdated(double apparentTemperature, int weatherCode,
                      const QString& weatherDesc);
  void errorOccurred(const QString& errorMessage);

 private slots:
  void onReplyFinished(QNetworkReply* reply);

 private:
  // 私有化构造，禁止拷贝赋值
  explicit WeatherFetcher(QObject* parent = nullptr);
  WeatherFetcher(const WeatherFetcher&) = delete;
  WeatherFetcher& operator=(const WeatherFetcher&) = delete;

  // 延迟初始化网络管理器
  void initNetworkManager();

  QNetworkAccessManager* m_networkManager;

  QThread* m_workerThread = nullptr;
};

#endif  // WEATHERFETCHER_H