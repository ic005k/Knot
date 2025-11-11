#include "src/Exercise/DrawSportsFreq.h"

#include <QFont>
#include <QPainter>
#include <QPen>
#include <QRect>
#include <QString>
#include <QVector>
#include <algorithm>  // For std::max (or qMax if Qt headers handle it)

// --- Constructor ---
FrequencyCurveWidget::FrequencyCurveWidget(const QVector<int>& cycling,
                                           const QVector<int>& hiking,
                                           const QVector<int>& running,
                                           bool isDark, QWidget* parent)
    : QWidget(parent),
      m_cycling(cycling),
      m_hiking(hiking),
      m_running(running),
      m_isDark(isDark) {
  setFixedHeight(mh);  // 固定高度
  setSizePolicy(QSizePolicy::Expanding,
                QSizePolicy::Fixed);  // 宽度跟随父窗口
  calculateMaxCount();                // 计算最大频次，用于比例映射
}

// --- Paint Event ---
void FrequencyCurveWidget::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);  // 平滑曲线，山丘感核心
  painter.setPen(Qt::NoPen);                      // 取消默认画笔，避免多余线条

  // 1. 计算12个月份的水平坐标（平分父窗口宽度）
  QVector<int> xPoints = calculateXPoints();

  // 2. 绘制三条运动曲线（骑行、徒步、跑步）
  drawSmoothCurve(painter, xPoints, m_cycling, getColor(0));
  drawSmoothCurve(painter, xPoints, m_hiking, getColor(1));
  drawSmoothCurve(painter, xPoints, m_running, getColor(2));

  // 设置字体和颜色
  QFont font;
  font.setPointSize(6);  // 将字体大小设置为6 (已按要求修改)
  painter.setFont(font);
  QPen pen;
  pen.setColor(m_isDark ? Qt::white : Qt::black);  // 根据模式选择颜色
  painter.setPen(pen);

  // 绘制月份刻度尺及标记
  int totalWidth = width();
  int segmentWidth = totalWidth / 12;  // 每个分段（包含块和间隙）的宽度

  for (int i = 0; i < 12; ++i) {
    // 计算第 i+1 个月份标记的X坐标（即第 i+1 个块的中心）
    // 修正：使用 i 而不是 (i+0) 以保持一致性 (i 从0开始)
    int xPoint =
        segmentWidth * i + segmentWidth / 2;  // 第 i 个块的中心 (i 从0开始)

    // 绘制月份数字于分块中央的上方
    // 使用 QRect(xPoint - segmentWidth/2, 0, segmentWidth, 15) 来定位文本区域
    painter.drawText(QRect(xPoint - segmentWidth / 2, 0, segmentWidth, 15),
                     Qt::AlignCenter, QString::number(i + 1));  // 显示月份 1-12

    // 不绘制最后一个标记 (i == 11 对应月份 12)
    if (i == 11) {
      continue;  // 跳过最后一个刻度线的绘制
    }
    // 绘制刻度线于分块的底部
    // 刻度线绘制在块的边界上（即第 i+1 个分段的起点）
    painter.drawLine(segmentWidth * (i + 1), height() - 5,
                     segmentWidth * (i + 1), height());
  }
}

// --- Calculate X Points ---
QVector<int> FrequencyCurveWidget::calculateXPoints() const {
  QVector<int> xPoints;
  int totalWidth = width();
  int monthSpacing = totalWidth / 12;
  for (int i = 0; i < 12; ++i) {
    // 取每个月的中心点作为曲线节点X坐标
    xPoints.append(monthSpacing * i + monthSpacing / 2);
  }
  return xPoints;
}

// --- Calculate Max Count ---
void FrequencyCurveWidget::calculateMaxCount() {
  m_maxCount = 1;  // 避免除以0
  for (int i = 0; i < 12; ++i) {
    int currentMax = qMax(m_cycling[i], qMax(m_hiking[i], m_running[i]));
    m_maxCount = qMax(m_maxCount, currentMax);
  }
}

// --- Get Color ---
QColor FrequencyCurveWidget::getColor(int type) const {
  if (m_isDark) {
    // 暗模式：亮一点的颜色，提高对比度
    switch (type) {
      case 0:
        return QColor(90, 189, 94, 200);  // 骑行绿
      case 1:
        return QColor(255, 171, 44, 200);  // 徒步橙
      case 2:
        return QColor(183, 70, 201, 200);  // 跑步紫
    }
  } else {
    // 亮模式：标准颜色
    switch (type) {
      case 0:
        return QColor(76, 175, 80, 200);  // 骑行绿
      case 1:
        return QColor(255, 152, 0, 200);  // 徒步橙
      case 2:
        return QColor(156, 39, 176, 200);  // 跑步紫
    }
  }
  return Qt::black;
}

// --- Draw Smooth Curve ---
void FrequencyCurveWidget::drawSmoothCurve(QPainter& painter,
                                           const QVector<int>& xPoints,
                                           const QVector<int>& counts,
                                           const QColor& color) {
  QPainterPath path;
  int pointCount = xPoints.size();
  if (pointCount <= 1) return;

  // 1. 初始化曲线起点
  int startX = xPoints[0];
  double startY = mapCountToY(counts[0]);  // 频次→10px高度映射
  path.moveTo(startX, startY);

  // 2. 贝塞尔曲线插值，确保平滑
  for (int i = 1; i < pointCount; ++i) {
    int currentX = xPoints[i];
    double currentY = mapCountToY(counts[i]);
    int prevX = xPoints[i - 1];
    double prevY = mapCountToY(counts[i - 1]);

    // 控制点：取两个点的中点，确保曲线自然过渡
    int ctrlX = (prevX + currentX) / 2;
    double ctrlY = (prevY + currentY) / 2;

    // 画二次贝塞尔曲线（平滑山丘核心）
    path.quadTo(ctrlX, ctrlY, currentX, currentY);
  }

  // 3. 设置画笔样式（3px粗，半透明）
  QPen pen(color);
  pen.setWidth(3);  // 确保10px高度下清晰可见
  painter.setPen(pen);
  painter.drawPath(path);
}

// --- Map Count to Y ---
double FrequencyCurveWidget::mapCountToY(int count) const {
  if (m_maxCount == 0) return 10.0;
  // Y轴向下为正，所以高频次→Y值小→向上凸
  return mh - (count * mh) / m_maxCount;  // mh为高度
}
