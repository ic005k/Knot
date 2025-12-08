#pragma once
#include <QPaintEvent>
#include <QPainter>
#include <QRect>
#include <QString>
#include <QWidget>
#include <cmath>

class CompassWidget : public QWidget {
  Q_OBJECT
 public:
  explicit CompassWidget(QWidget* parent = nullptr);

  // 设置方位角（正北0°，顺时针：东90°、南180°、西270°）
  void setBearing(double bearing);

  // 设置速度和单位
  void setSpeed(double speed);
  void setSpeedUnit(const QString& unit);

 protected:
  void paintEvent(QPaintEvent* event) override;

 private:
  double m_bearing = 0.0;        // 当前方位角
  double m_speed = 0.0;          // 当前速度
  QString m_speedUnit = "km/h";  // 速度单位
  const int m_padding = 5;       // 控件内边距
};
