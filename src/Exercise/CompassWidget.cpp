#include "CompassWidget.h"

#include <QBrush>
#include <QFont>
#include <QFontMetrics>
#include <QPen>
#include <QtMath>

#include "src/defines.h"

CompassWidget::CompassWidget(QWidget* parent) : QWidget(parent) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setMinimumSize(150, 120);
  setContentsMargins(0, 0, 0, 0);

  // 初始化默认值
  m_speed = 0.0;
  m_speedUnit = "km/h";
}

void CompassWidget::setBearing(double bearing) {
  if (std::fabs(m_bearing - bearing) < 0.1) return;
  m_bearing = bearing;
  update();
}

void CompassWidget::setSpeed(double speed) {
  if (std::fabs(m_speed - speed) < 0.1) return;
  m_speed = speed;
  update();
}

void CompassWidget::setSpeedUnit(const QString& unit) {
  if (m_speedUnit != unit) {
    m_speedUnit = unit;
    update();
  }
}

void CompassWidget::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // 如果父控件太小，不绘制
  if (rect().width() < 10 || rect().height() < 10) {
    return;
  }

  // ========== 1. 计算布局 ==========
  QRect drawRect =
      rect().adjusted(m_padding, m_padding, -m_padding, -m_padding);
  int totalWidth = drawRect.width();
  int totalHeight = drawRect.height();

  if (totalWidth < 20 || totalHeight < 20) {
    return;
  }

  // 计算罗盘大小
  int compassDiameter = qMin(totalWidth, totalHeight) * 0.8;
  compassDiameter = qMax(20, compassDiameter);
  int compassRadius = compassDiameter / 2;

  // 计算罗盘中心位置
  int compassCenterX = totalWidth / 2;
  int compassCenterY = totalHeight / 2;

  // 罗盘底部Y坐标
  int compassBottomY = compassCenterY + compassRadius;

  // 罗盘内部半径
  int scaleOuterRadius = qMax(5, compassRadius - 5);
  int scaleInnerRadius = qMax(10, compassRadius - 20);
  int textRadius = qMax(15, compassRadius - 30);
  int centerCircleRadius = qMax(5, compassRadius - 40);

  // 2. 颜色设置
  QColor backgroundColor, borderColor, centerColor, textColor,
      textBackgroundColor;
  QColor scaleMajorColor, scaleMinorColor, pointerColor, pointerOutlineColor;
  QColor pointerCenterColor, pointerCenterBrush, forwardTextColor;
  QColor speedTextColor, unitTextColor;

  if (isDark) {
    // 暗黑模式颜色
    backgroundColor = QColor(40, 40, 40);
    borderColor = QColor(100, 100, 100);
    centerColor = QColor(60, 60, 60);
    textColor = QColor(220, 220, 220);
    textBackgroundColor = QColor(60, 60, 60, 200);
    scaleMajorColor = QColor(180, 180, 180);
    scaleMinorColor = QColor(100, 100, 100);
    pointerColor = QColor(255, 100, 100);
    pointerOutlineColor = QColor(255, 150, 150);
    pointerCenterColor = QColor(220, 220, 220);
    pointerCenterBrush = QColor(180, 180, 180);
    forwardTextColor = QColor(255, 150, 150);
    speedTextColor = QColor(100, 200, 255);
    unitTextColor = QColor(180, 180, 180);
  } else {
    // 明亮模式颜色
    backgroundColor = QColor(240, 240, 240);
    borderColor = QColor(100, 100, 100);
    centerColor = QColor(200, 200, 200);
    textColor = QColor(0, 0, 0);
    textBackgroundColor = QColor(255, 255, 255, 220);
    scaleMajorColor = QColor(0, 0, 0);
    scaleMinorColor = QColor(100, 100, 100);
    pointerColor = QColor(255, 100, 100);
    pointerOutlineColor = QColor(255, 0, 0);
    pointerCenterColor = QColor(0, 0, 0);
    pointerCenterBrush = QColor(0, 0, 0);
    forwardTextColor = QColor(200, 0, 0);
    speedTextColor = QColor(0, 100, 255);
    unitTextColor = QColor(100, 100, 100);
  }

  // 3. 绘制罗盘
  painter.setPen(QPen(borderColor, 2));
  painter.setBrush(backgroundColor);
  painter.drawEllipse(QRect(compassCenterX - compassRadius,
                            compassCenterY - compassRadius, 2 * compassRadius,
                            2 * compassRadius));

  // 中心圆
  painter.setPen(QPen(centerColor, 1));
  painter.setBrush(isDark ? QColor(50, 50, 50) : Qt::white);
  painter.drawEllipse(QRect(compassCenterX - centerCircleRadius,
                            compassCenterY - centerCircleRadius,
                            2 * centerCircleRadius, 2 * centerCircleRadius));

  // 4. 绘制旋转的刻度盘
  painter.save();
  painter.translate(compassCenterX, compassCenterY);
  painter.rotate(-m_bearing);

  for (int angle = 0; angle < 360; angle += 10) {
    double rad = qDegreesToRadians(static_cast<double>(angle));
    double sinVal = qSin(rad);
    double cosVal = qCos(rad);

    int x1 = static_cast<int>(scaleInnerRadius * sinVal);
    int y1 = static_cast<int>(-scaleInnerRadius * cosVal);
    int x2, y2;

    if (angle % 30 == 0) {
      x2 = static_cast<int>(scaleOuterRadius * sinVal);
      y2 = static_cast<int>(-scaleOuterRadius * cosVal);
      painter.setPen(QPen(scaleMajorColor, 2));
      painter.drawLine(x1, y1, x2, y2);
    } else {
      int midRadius =
          scaleInnerRadius + (scaleOuterRadius - scaleInnerRadius) * 0.7;
      x2 = static_cast<int>(midRadius * sinVal);
      y2 = static_cast<int>(-midRadius * cosVal);
      painter.setPen(QPen(scaleMinorColor, 1));
      painter.drawLine(x1, y1, x2, y2);
    }
  }

  // 方向文字
  QFont dirFont;
  dirFont.setFamily("Arial");
  dirFont.setPointSize(10);
  dirFont.setBold(true);
  painter.setFont(dirFont);
  QFontMetrics fm(dirFont);

  QString directions[4] = {tr("N"), tr("E"), tr("S"), tr("W")};
  int directionAngles[4] = {0, 90, 180, 270};

  for (int i = 0; i < 4; i++) {
    double angle = directionAngles[i];
    double rad = qDegreesToRadians(angle);
    int x = static_cast<int>(textRadius * qSin(rad));
    int y = static_cast<int>(-textRadius * qCos(rad));
    QString text = directions[i];
    int textW = fm.horizontalAdvance(text);
    int textH = fm.height();

    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(textBackgroundColor);

    int ellipseWidth = textW / 2 + 2;
    int ellipseHeight = textH / 2 + 2;
    painter.drawEllipse(QRect(x - ellipseWidth, y - ellipseHeight,
                              2 * ellipseWidth, 2 * ellipseHeight));

    painter.setPen(textColor);
    painter.drawText(QRect(x - textW / 2, y - textH / 2, textW, textH),
                     Qt::AlignCenter, text);
    painter.restore();
  }

  painter.restore();

  // 5. 在中心显示速度
  if (centerCircleRadius > 10) {
    painter.save();

    // 格式化速度文本，保留一位小数
    QString speedStr = QString::number(m_speed, 'f', 1);

    // 获取速度文本的尺寸
    QFont speedFont;
    speedFont.setFamily("Arial");
    speedFont.setPointSize(qMax(8, centerCircleRadius / 2));
    speedFont.setBold(true);
    painter.setFont(speedFont);
    painter.setPen(speedTextColor);

    int speedTextHeight = painter.fontMetrics().height();
    int speedTextWidth = painter.fontMetrics().horizontalAdvance(speedStr);

    // 获取单位文本的尺寸
    QFont unitFont;
    unitFont.setFamily("Arial");
    unitFont.setPointSize(qMax(6, centerCircleRadius / 4));
    unitFont.setBold(false);
    painter.setFont(unitFont);
    int unitTextHeight = painter.fontMetrics().height();
    int unitTextWidth = painter.fontMetrics().horizontalAdvance(m_speedUnit);

    // 计算文本垂直布局
    int textSpacing = unitFont.pointSize() / 2;  // 速度和单位之间的间距
    int totalTextHeight = speedTextHeight + textSpacing + unitTextHeight;

    // 计算起始Y坐标，使整体垂直居中
    int speedTextY = compassCenterY - totalTextHeight / 2;
    int unitTextY = speedTextY + speedTextHeight + textSpacing;

    // 绘制速度值
    painter.setFont(speedFont);
    painter.setPen(speedTextColor);
    painter.drawText(QRect(compassCenterX - speedTextWidth / 2, speedTextY,
                           speedTextWidth, speedTextHeight),
                     Qt::AlignCenter, speedStr);

    // 绘制单位
    painter.setFont(unitFont);
    painter.setPen(unitTextColor);
    painter.drawText(QRect(compassCenterX - unitTextWidth / 2, unitTextY,
                           unitTextWidth, unitTextHeight),
                     Qt::AlignCenter, m_speedUnit);

    painter.restore();
  }

  // 6. 在右下角绘制小的方向指示箭头
  int arrowSize = compassRadius * 0.18;      // 箭头大小为罗盘半径的18%
  arrowSize = qMax(6, qMin(arrowSize, 20));  // 限制大小范围

  // 计算箭头高度
  int arrowHeight = arrowSize;
  int arrowWidth = arrowSize * 0.6;  // 箭头宽度为高度的0.6倍

  // 计算文本尺寸
  QFont indicatorFont;
  indicatorFont.setFamily("Arial");
  indicatorFont.setPointSize(7);
  indicatorFont.setBold(true);
  painter.setFont(indicatorFont);
  QString forwardText = tr("Forward");
  int textW = painter.fontMetrics().horizontalAdvance(forwardText);
  int textH = painter.fontMetrics().height();

  // 文本与箭头之间的间距
  int arrowTextSpacing = 3;

  // 计算箭头和文本的整体高度
  int arrowAndTextHeight = arrowHeight + arrowTextSpacing + textH;

  // 箭头中心X位置 - 罗盘右侧外部
  int arrowCenterX = compassCenterX + compassRadius + arrowWidth;
  arrowCenterX = qMin(arrowCenterX, totalWidth - arrowWidth / 2 - 5);

  // 计算箭头中心Y位置，使文本底部与罗盘底部对齐
  int arrowCenterY = compassBottomY - arrowAndTextHeight + arrowHeight / 2;

  // 计算文本底部位置
  int textBottomY = arrowCenterY + arrowHeight / 2 + arrowTextSpacing + textH;

  // 如果文本底部低于罗盘底部，调整箭头位置
  if (textBottomY > compassBottomY) {
    int diff = textBottomY - compassBottomY;
    arrowCenterY -= diff;
  }

  // 确保箭头不超出控件边界
  arrowCenterY = qMax(
      arrowHeight / 2 + 5,
      qMin(arrowCenterY, totalHeight - arrowAndTextHeight + arrowHeight / 2));

  painter.save();
  painter.translate(arrowCenterX, arrowCenterY);

  // 绘制小的方向指示箭头
  QPolygonF arrow;
  arrow << QPointF(0, -arrowHeight / 2.0)                 // 箭头尖端
        << QPointF(-arrowWidth / 2.0, arrowHeight / 2.0)  // 左下角
        << QPointF(0, arrowHeight / 4.0)                  // 中间底部
        << QPointF(arrowWidth / 2.0, arrowHeight / 2.0)   // 右下角
        << QPointF(0, -arrowHeight / 2.0);                // 回到顶端

  painter.setPen(QPen(pointerOutlineColor, 1));
  painter.setBrush(pointerColor);
  painter.drawPolygon(arrow);

  painter.restore();

  // 7. 在箭头下方绘制"Forward"文本
  int textX = arrowCenterX - textW / 2;
  int textY = arrowCenterY + arrowHeight / 2 + arrowTextSpacing;

  QRect textRect(textX, textY, textW, textH);
  painter.setPen(forwardTextColor);
  painter.drawText(textRect, Qt::AlignCenter, forwardText);
}
