#pragma once
#include <QPaintEvent>
#include <QPainter>
#include <QRect>
#include <QWidget>
#include <cmath>

#define _USE_MATH_DEFINES
#include <cmath>

// 前向声明（若需关联Steps类）
class Steps;

// 方形指南针核心类（适配任意长宽控件）
class CompassWidget : public QWidget {
  Q_OBJECT
 public:
  explicit CompassWidget(QWidget* parent = nullptr);

  // 设置方位角（正北0°，顺时针：东90°、南180°、西270°）
  void setBearing(double bearing);

 protected:
  void paintEvent(QPaintEvent* event) override;

 private:
  // 辅助：绘制方位文字（N/E/S/W）
  void drawDirText(QPainter* painter, const QString& text, const QRect& rect);

  double m_bearing = 0.0;   // 当前方位角
  const int m_padding = 5;  // 控件内边距（避免贴边）
};
