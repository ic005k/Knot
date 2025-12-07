#ifndef SPEEDOMETER_H
#define SPEEDOMETER_H

#include <QColor>
#include <QLinearGradient>
#include <QPropertyAnimation>
#include <QWidget>
// 引入明暗模式全局变量头文件
#include "src/defines.h"

class Speedometer : public QWidget {
  Q_OBJECT
  Q_PROPERTY(double currentSpeed READ currentSpeed WRITE setCurrentSpeed NOTIFY
                 currentSpeedChanged)

 public:
  explicit Speedometer(QWidget* parent = nullptr);

  // 尺寸策略设置
  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;

  // 属性方法
  double maxSpeed() const;
  void setMaxSpeed(double maxSpeed);
  double currentSpeed() const;
  void setCurrentSpeed(double speed);
  double minSpeed() const;
  void setMinSpeed(double minSpeed);
  int decimalPlaces() const;
  void setDecimalPlaces(int places);
  double aspectRatio() const;
  void setAspectRatio(double ratio);
  QColor backgroundColor() const;
  void setBackgroundColor(const QColor& color);
  QColor foregroundColor() const;
  void setForegroundColor(const QColor& color);
  QColor accentColor() const;
  void setAccentColor(const QColor& color);
  QColor needleColor() const;
  void setNeedleColor(const QColor& color);

  void updateThemeColors(bool autoAdaptForeground = true,
                         bool forceReset = false);

 signals:
  void currentSpeedChanged(double speed);

 protected:
  void paintEvent(QPaintEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;

 private:
  void calculateMetrics();
  void createSpeedTextAnimation();
  void updateGradient();

  // 新增：计算颜色亮度（0-255，0=纯黑，255=纯白）
  static int colorLuminance(const QColor& color);
  // 新增：根据背景色自动计算对比色（前景色）
  QColor getContrastColor(const QColor& bgColor) const;

  double m_maxSpeed = 100.0;
  double m_currentSpeed = 0.0;
  double m_minSpeed = 0.0;
  double m_aspectRatio = 2.0;  // 宽度/高度比例
  int m_decimalPlaces = 1;

  // 颜色成员改为：默认值由主题决定，用户自定义时优先使用自定义值
  QColor m_backgroundColor;  // 移除固定初始值
  QColor m_foregroundColor;
  QColor m_accentColor;
  QColor m_needleColor;

  // 渐变色
  QLinearGradient m_gaugeGradient;

  // 尺寸计算缓存
  QRect m_speedRect;
  QRect m_gaugeRect;
  QRect m_scaleRect;
  QRect m_tickTextRect;
  double m_needleHeight = 0.0;
  QFont m_speedFont;
  QFont m_scaleFont;
  QPropertyAnimation* m_speedAnimation = nullptr;
};

#endif  // SPEEDOMETER_H
