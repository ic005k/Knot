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

class WeatherFetcher : public QObject {
  Q_OBJECT

 public:
  // 构造函数，需要传入父对象
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
    // 仅请求当前温度数据，最小化数据传输
    QString urlString = QString(
                            "https://api.open-meteo.com/v1/forecast?"
                            "latitude=%1&longitude=%2&"
                            "current=temperature_2m&"
                            "timezone=auto&"
                            "units=metric")            // 确保返回摄氏度
                            .arg(latitude, 0, 'f', 6)  // 保留6位小数的精度
                            .arg(longitude, 0, 'f', 6);

    // 发送GET请求
    m_networkManager->get(QNetworkRequest(QUrl(urlString)));
  }

 signals:
  // 天气数据获取成功信号，返回温度值(摄氏度)
  void weatherUpdated(double temperature);

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

    // 提取温度数据
    if (rootObj.contains("current") && rootObj["current"].isObject()) {
      QJsonObject currentObj = rootObj["current"].toObject();
      if (currentObj.contains("temperature_2m") &&
          currentObj["temperature_2m"].isDouble()) {
        double temperature = currentObj["temperature_2m"].toDouble();
        emit weatherUpdated(temperature);
        reply->deleteLater();
        return;
      }
    }

    // 如果到这里，说明数据格式不符合预期
    emit errorOccurred(tr("天气数据格式错误"));
    reply->deleteLater();
  }

 private:
  // 网络访问管理器
  QNetworkAccessManager *m_networkManager;
};

#endif  // WEATHERFETCHER_H
