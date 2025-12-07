#include "Speedometer.h"

#include <QDebug>
#include <QPainter>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <cmath>

Speedometer::Speedometer(QWidget* parent) : QWidget(parent) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setMinimumSize(300, 120);

  // 初始化主题适配颜色
  updateThemeColors();

  createSpeedTextAnimation();
  updateGradient();
}

// 新增：计算颜色亮度（ITU-R BT.709 标准）
int Speedometer::colorLuminance(const QColor& color) {
  return static_cast<int>(0.2126 * color.red() + 0.7152 * color.green() +
                          0.0722 * color.blue());
}

// 新增：根据背景色自动生成高对比度的前景色
QColor Speedometer::getContrastColor(const QColor& bgColor) const {
  int luminance = colorLuminance(bgColor);
  // 背景亮度 < 128（深色）→ 浅色文字；反之→深色文字
  return luminance < 128 ? QColor(220, 220, 220) : QColor(50, 50, 60);
}

// 新增：更新主题配色（暗黑/亮色模式）
void Speedometer::updateThemeColors(bool autoAdaptForeground /* = true */,
                                    bool forceReset /* = false */) {
  // ===== 1. 背景色处理 =====
  if (forceReset || !m_backgroundColor.isValid()) {
    m_backgroundColor = isDark ? QColor(30, 30, 40) : QColor(245, 245, 248);
  }

  // ===== 2. 前景色处理（自动适配背景色）=====
  if (forceReset || !m_foregroundColor.isValid() || autoAdaptForeground) {
    // 如果用户手动设了背景色，自动计算前景色；否则按isDark设置
    m_foregroundColor = getContrastColor(m_backgroundColor);
  }

  // ===== 3. 其他颜色处理（可选：也可适配背景色）=====
  if (forceReset || !m_accentColor.isValid()) {
    m_accentColor = isDark ? QColor(65, 150, 250) : QColor(45, 130, 240);
  }

  if (forceReset || !m_needleColor.isValid()) {
    m_needleColor = isDark ? QColor(255, 100, 100) : QColor(240, 80, 80);
  }

  // 更新渐变和UI
  updateGradient();
  update();
}

void Speedometer::createSpeedTextAnimation() {
  m_speedAnimation = new QPropertyAnimation(this, "currentSpeed", this);
  m_speedAnimation->setDuration(800);
  m_speedAnimation->setEasingCurve(QEasingCurve::OutQuad);
}

void Speedometer::updateGradient() {
  m_gaugeGradient = QLinearGradient(0, 0, 1, 0);
  m_gaugeGradient.setCoordinateMode(QGradient::ObjectBoundingMode);

  // 适配暗黑/亮色模式的渐变颜色
  if (isDark) {
    m_gaugeGradient.setColorAt(0.0, QColor(65, 150, 250));  // 蓝色
    m_gaugeGradient.setColorAt(0.7, QColor(255, 200, 50));  // 黄色
    m_gaugeGradient.setColorAt(1.0, QColor(255, 70, 70));   // 红色
  } else {
    m_gaugeGradient.setColorAt(0.0, QColor(45, 130, 240));  // 浅蓝
    m_gaugeGradient.setColorAt(0.7, QColor(240, 180, 40));  // 浅黄
    m_gaugeGradient.setColorAt(1.0, QColor(240, 60, 60));   // 浅红
  }
}

void Speedometer::setCurrentSpeed(double speed) {
  if (qFuzzyCompare(m_currentSpeed, speed)) return;

  // 使用动画平滑过渡
  if (qAbs(speed - m_currentSpeed) > 10) {
    m_speedAnimation->stop();
    m_speedAnimation->setStartValue(m_currentSpeed);
    m_speedAnimation->setEndValue(speed);
    m_speedAnimation->start();
  } else {
    m_currentSpeed = speed;
    update();
    emit currentSpeedChanged(m_currentSpeed);
  }
}

QSize Speedometer::sizeHint() const {
  return QSize(400, static_cast<int>(400 / m_aspectRatio));
}

QSize Speedometer::minimumSizeHint() const {
  return QSize(300, static_cast<int>(300 / m_aspectRatio));
}

double Speedometer::maxSpeed() const { return m_maxSpeed; }

void Speedometer::setMaxSpeed(double maxSpeed) {
  if (qFuzzyCompare(m_maxSpeed, maxSpeed)) return;
  m_maxSpeed = maxSpeed <= 10 ? 10 : maxSpeed;
  calculateMetrics();
  update();
}

double Speedometer::currentSpeed() const { return m_currentSpeed; }

double Speedometer::minSpeed() const { return m_minSpeed; }

void Speedometer::setMinSpeed(double minSpeed) {
  if (qFuzzyCompare(m_minSpeed, minSpeed)) return;
  m_minSpeed = minSpeed;
  calculateMetrics();
  update();
}

int Speedometer::decimalPlaces() const { return m_decimalPlaces; }

void Speedometer::setDecimalPlaces(int places) {
  m_decimalPlaces = qBound(0, places, 4);
  update();
}

double Speedometer::aspectRatio() const { return m_aspectRatio; }

void Speedometer::setAspectRatio(double ratio) {
  m_aspectRatio = qMax(1.5, qMin(3.0, ratio));
  updateGeometry();  // 通知布局系统尺寸需求已改变
  calculateMetrics();
  update();
}

QColor Speedometer::backgroundColor() const { return m_backgroundColor; }

void Speedometer::setBackgroundColor(const QColor& color) {
  if (m_backgroundColor == color) return;
  m_backgroundColor = color;
  // 关键：设置背景后，自动更新前景色（刻度/文字颜色）
  m_foregroundColor = getContrastColor(m_backgroundColor);
  updateGradient();
  update();
}

QColor Speedometer::foregroundColor() const { return m_foregroundColor; }

void Speedometer::setForegroundColor(const QColor& color) {
  if (m_foregroundColor == color) return;
  m_foregroundColor = color;
  update();
}

QColor Speedometer::accentColor() const { return m_accentColor; }

void Speedometer::setAccentColor(const QColor& color) {
  if (m_accentColor == color) return;
  m_accentColor = color;
  updateGradient();
  update();
}

QColor Speedometer::needleColor() const { return m_needleColor; }

void Speedometer::setNeedleColor(const QColor& color) {
  if (m_needleColor == color) return;
  m_needleColor = color;
  update();
}

void Speedometer::resizeEvent(QResizeEvent* event) {
  Q_UNUSED(event)
  calculateMetrics();
  update();
}

void Speedometer::calculateMetrics() {
  QRect widgetRect = rect();

  // 速度数字区域 (顶部50%)
  m_speedRect = QRect(0, 0, widgetRect.width(), widgetRect.height() * 0.5);

  // 主表盘区域 (剩余70%)
  m_gaugeRect = QRect(0, widgetRect.height() * 0.3, widgetRect.width(),
                      widgetRect.height() * 0.7);

  // 进度条区域 (占表盘高度的50%)
  int barHeight = qMin(m_gaugeRect.height() * 0.5, 30.0);
  int barWidth = m_gaugeRect.width() * 0.9;

  m_scaleRect =
      QRect(m_gaugeRect.width() * 0.05,
            m_gaugeRect.top() + (m_gaugeRect.height() - barHeight) / 2,
            barWidth, barHeight);

  // 刻度文本区域 (进度条下方)
  m_tickTextRect =
      QRect(m_scaleRect.x(), m_scaleRect.bottom() + 5, m_scaleRect.width(),
            m_gaugeRect.bottom() - m_scaleRect.bottom() - 5);

  // 创建字体
  m_speedFont =
      QFont("Arial", qMin(m_speedRect.height() * 0.6, 36.0), QFont::Bold);
  m_scaleFont = QFont("Arial", qMin(m_tickTextRect.height() * 0.8, 12.0));
}

void Speedometer::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event)
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setRenderHint(QPainter::TextAntialiasing);

  // 计算进度百分比
  double progress = (m_currentSpeed - m_minSpeed) / (m_maxSpeed - m_minSpeed);
  progress = qBound(0.0, progress, 1.0);

  // 绘制背景
  painter.fillRect(rect(), m_backgroundColor);

  // ===================== 速度文本区域 =====================
  painter.save();
  painter.setPen(m_foregroundColor);  // 现在用适配后的前景色
  painter.setFont(m_speedFont);

  // 修复：原代码写死了2位小数，改为用m_decimalPlaces
  QString speedText = QString("%1 km/h").arg(
      QString::number(m_currentSpeed, 'f', m_decimalPlaces));

  painter.drawText(m_speedRect, Qt::AlignCenter, speedText);
  painter.restore();

  // ===================== 刻度区域 =====================
  painter.save();

  // 修复：刻度背景色根据背景色亮度适配（不再依赖isDark）
  int cornerRadius = m_scaleRect.height() / 2;
  painter.setPen(Qt::NoPen);
  int bgLuminance = colorLuminance(m_backgroundColor);
  QColor scaleBgColor =
      bgLuminance < 128 ? QColor(60, 60, 70) : QColor(220, 220, 225);
  painter.setBrush(scaleBgColor);
  painter.drawRoundedRect(m_scaleRect, cornerRadius, cornerRadius);

  // 绘制进度条前景
  QRect progressRect(m_scaleRect);
  progressRect.setWidth(static_cast<int>(m_scaleRect.width() * progress));
  painter.setBrush(m_gaugeGradient);
  painter.drawRoundedRect(progressRect, cornerRadius, cornerRadius);

  // 绘制指针
  painter.setPen(Qt::NoPen);
  painter.setBrush(m_needleColor);

  int pointerX = m_scaleRect.left() + progress * m_scaleRect.width();
  int pointerSize = m_scaleRect.height() * 1.2;

  // 修复：指针圆点颜色也根据背景色适配
  QColor pointerDotColor =
      bgLuminance < 128 ? QColor(240, 240, 240) : QColor(50, 50, 60);
  painter.setBrush(pointerDotColor);
  painter.drawEllipse(QPoint(pointerX, m_scaleRect.center().y()),
                      pointerSize / 6, pointerSize / 6);

  // ===================== 刻度文本 =====================
  painter.setPen(m_foregroundColor);  // 适配后的前景色
  painter.setFont(m_scaleFont);

  // 绘制刻度值（原有逻辑不变）
  const int tickCount = 6;
  for (int i = 0; i <= tickCount; i++) {
    double value = m_minSpeed + (i * (m_maxSpeed - m_minSpeed) / tickCount);
    QString text;
    if (m_decimalPlaces == 0) {
      text = QString::number(static_cast<int>(value));
    } else {
      text = QString::number(value, 'f', m_decimalPlaces);
    }

    int x = m_scaleRect.left() + (i * m_scaleRect.width() / tickCount);
    QRect textRect(x - 20, m_tickTextRect.top(), 40, m_tickTextRect.height());
    painter.drawText(textRect, Qt::AlignCenter, text);

    // 绘制刻度标记（线宽也根据背景色适配）
    if (i > 0 && i < tickCount) {
      painter.setPen(QPen(m_foregroundColor, bgLuminance < 128 ? 1 : 1.5));
      painter.drawLine(x, m_scaleRect.top() + 5, x, m_scaleRect.bottom() - 5);
    }
  }

  painter.restore();
}
