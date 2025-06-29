#include "Speedometer.h"

#include <QDebug>
#include <QPainter>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <cmath>

Speedometer::Speedometer(QWidget *parent) : QWidget(parent) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setMinimumSize(300, 120);
  createSpeedTextAnimation();
  updateGradient();
}

void Speedometer::createSpeedTextAnimation() {
  m_speedAnimation = new QPropertyAnimation(this, "currentSpeed", this);
  m_speedAnimation->setDuration(800);
  m_speedAnimation->setEasingCurve(QEasingCurve::OutQuad);
}

void Speedometer::updateGradient() {
  m_gaugeGradient = QLinearGradient(0, 0, 1, 0);
  m_gaugeGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
  m_gaugeGradient.setColorAt(0.0, QColor(65, 150, 250));  // 蓝色
  m_gaugeGradient.setColorAt(0.7, QColor(255, 200, 50));  // 黄色
  m_gaugeGradient.setColorAt(1.0, QColor(255, 70, 70));   // 红色
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

void Speedometer::setBackgroundColor(const QColor &color) {
  if (m_backgroundColor == color) return;
  m_backgroundColor = color;
  update();
}

QColor Speedometer::foregroundColor() const { return m_foregroundColor; }

void Speedometer::setForegroundColor(const QColor &color) {
  if (m_foregroundColor == color) return;
  m_foregroundColor = color;
  update();
}

QColor Speedometer::accentColor() const { return m_accentColor; }

void Speedometer::setAccentColor(const QColor &color) {
  if (m_accentColor == color) return;
  m_accentColor = color;
  updateGradient();
  update();
}

QColor Speedometer::needleColor() const { return m_needleColor; }

void Speedometer::setNeedleColor(const QColor &color) {
  if (m_needleColor == color) return;
  m_needleColor = color;
  update();
}

void Speedometer::resizeEvent(QResizeEvent *event) {
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

void Speedometer::paintEvent(QPaintEvent *event) {
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
  painter.setPen(m_foregroundColor);
  painter.setFont(m_speedFont);

  // 创建速度文本
  QString speedText =
      QString("%1 km/h").arg(QString::number(m_currentSpeed, 'f', 2));

  // 绘制速度文本
  painter.drawText(m_speedRect, Qt::AlignCenter, speedText);
  painter.restore();

  // ===================== 刻度区域 =====================
  painter.save();

  // 绘制刻度线背景
  int cornerRadius = m_scaleRect.height() / 2;
  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor(60, 60, 70));
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

  // 指针中心圆点
  painter.setBrush(QColor(240, 240, 240));
  painter.drawEllipse(QPoint(pointerX, m_scaleRect.center().y()),
                      pointerSize / 6, pointerSize / 6);

  // ===================== 刻度文本 =====================
  painter.setPen(m_foregroundColor);
  painter.setFont(m_scaleFont);

  // 绘制刻度值
  const int tickCount = 6;
  for (int i = 0; i <= tickCount; i++) {
    double value = m_minSpeed + (i * (m_maxSpeed - m_minSpeed) / tickCount);

    // 格式化文本
    QString text;
    if (m_decimalPlaces == 0) {
      text = QString::number(static_cast<int>(value));
    } else {
      text = QString::number(value, 'f', m_decimalPlaces);
    }

    // 计算位置
    int x = m_scaleRect.left() + (i * m_scaleRect.width() / tickCount);
    QRect textRect(x - 20, m_tickTextRect.top(), 40, m_tickTextRect.height());

    painter.drawText(textRect, Qt::AlignCenter, text);

    // 绘制刻度标记
    if (i > 0 && i < tickCount) {
      painter.setPen(QPen(m_foregroundColor, 1));
      painter.drawLine(x, m_scaleRect.top() + 5, x, m_scaleRect.bottom() - 5);
    }
  }

  painter.restore();
}
