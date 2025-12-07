#include "CompassWidget.h"

#include <QBrush>
#include <QFont>
#include <QFontMetrics>
#include <QPen>
#include <QtMath>

#include "src/defines.h"  // 包含全局变量定义

CompassWidget::CompassWidget(QWidget* parent) : QWidget(parent) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setMinimumSize(150, 120);
  setContentsMargins(0, 0, 0, 0);
}

void CompassWidget::setBearing(double bearing) {
  if (std::fabs(m_bearing - bearing) < 0.1) return;
  m_bearing = bearing;
  update();
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

  // 如果调整后的绘制区域太小，不绘制
  if (totalWidth < 20 || totalHeight < 20) {
    return;
  }

  // 计算罗盘和指针的大小
  int maxCompassSize =
      qMin(totalWidth * 0.7, totalHeight * 0.9);   // 罗盘占70%宽度或90%高度
  int compassDiameter = qMax(20, maxCompassSize);  // 确保最小尺寸
  int compassRadius = compassDiameter / 2;

  // 计算指针宽度（罗盘直径的40%）
  int pointerWidth = qMax(20, static_cast<int>(compassDiameter * 0.4));
  int pointerHeight = qMax(20, static_cast<int>(compassDiameter * 0.5));

  // 计算总内容宽度
  int contentWidth = pointerWidth + compassDiameter;
  int contentHeight = compassDiameter;

  // 确保内容尺寸不超过可用空间
  if (contentWidth > totalWidth) {
    // 按比例缩小
    float scale = static_cast<float>(totalWidth) / contentWidth;
    contentWidth = totalWidth;
    pointerWidth = static_cast<int>(pointerWidth * scale);
    compassDiameter = static_cast<int>(compassDiameter * scale);
    compassRadius = compassDiameter / 2;
    contentHeight = compassDiameter;
    pointerHeight = static_cast<int>(pointerHeight * scale);
  }

  if (contentHeight > totalHeight) {
    // 按比例缩小
    float scale = static_cast<float>(totalHeight) / contentHeight;
    contentHeight = totalHeight;
    compassDiameter = static_cast<int>(compassDiameter * scale);
    compassRadius = compassDiameter / 2;
    pointerWidth = static_cast<int>(pointerWidth * scale);
    pointerHeight = static_cast<int>(pointerHeight * scale);
    contentWidth = pointerWidth + compassDiameter;
  }

  // 计算居中位置
  int startX = (totalWidth - contentWidth) / 2;
  int startY = (totalHeight - contentHeight) / 2;

  // 指针位置（左侧）
  int pointerCenterX = startX + pointerWidth / 2;
  int pointerCenterY = startY + compassRadius;

  // 罗盘位置（右侧）
  int compassCenterX = startX + pointerWidth + compassRadius;
  int compassCenterY = startY + compassRadius;

  // 罗盘内部半径定义
  int scaleOuterRadius = qMax(5, compassRadius - 5);     // 刻度外半径
  int scaleInnerRadius = qMax(10, compassRadius - 20);   // 刻度内半径
  int textRadius = qMax(15, compassRadius - 30);         // 文字半径
  int centerCircleRadius = qMax(5, compassRadius - 40);  // 中心圆半径

  // 确保内部半径合理
  if (scaleInnerRadius <= scaleOuterRadius) {
    scaleInnerRadius = scaleOuterRadius - 5;
  }
  if (textRadius <= scaleInnerRadius) {
    textRadius = scaleInnerRadius - 5;
  }
  if (centerCircleRadius <= 0) {
    centerCircleRadius = compassRadius / 2;
  }

  // ========== 2. 根据暗黑模式选择颜色 ==========
  QColor backgroundColor, borderColor, centerColor, textColor,
      textBackgroundColor;
  QColor scaleMajorColor, scaleMinorColor, pointerColor, pointerOutlineColor;
  QColor pointerCenterColor, pointerCenterBrush, forwardTextColor;

  if (isDark) {  // 使用全局变量 isDark
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
  }

  // ========== 3. 绘制罗盘背景和外框 ==========
  // 绘制罗盘外圆
  painter.setPen(QPen(borderColor, 2));
  painter.setBrush(backgroundColor);
  painter.drawEllipse(QPoint(compassCenterX, compassCenterY), compassRadius,
                      compassRadius);

  // 绘制罗盘中心区
  painter.setPen(QPen(centerColor, 1));
  painter.setBrush(isDark ? QColor(50, 50, 50) : Qt::white);
  if (centerCircleRadius > 0) {
    painter.drawEllipse(QPoint(compassCenterX, compassCenterY),
                        centerCircleRadius, centerCircleRadius);
  }

  // ========== 4. 绘制旋转的罗盘刻度盘 ==========
  painter.save();
  painter.translate(compassCenterX, compassCenterY);

  // 罗盘刻度盘旋转，使当前航向指向正上方
  painter.rotate(-m_bearing);

  // 简化刻度：只绘制主刻度（每30度）和中等刻度（每10度）
  for (int angle = 0; angle < 360; angle += 10) {
    double rad = qDegreesToRadians(static_cast<double>(angle));

    double sinVal = qSin(rad);
    double cosVal = qCos(rad);

    // 计算刻度线端点
    int x1 = static_cast<int>(scaleInnerRadius * sinVal);
    int y1 = static_cast<int>(-scaleInnerRadius * cosVal);
    int x2, y2;

    // 主刻度（每30度）
    if (angle % 30 == 0) {
      x2 = static_cast<int>(scaleOuterRadius * sinVal);
      y2 = static_cast<int>(-scaleOuterRadius * cosVal);

      // 绘制主刻度线
      painter.setPen(QPen(scaleMajorColor, 2));
      painter.drawLine(x1, y1, x2, y2);
    }
    // 中等刻度（每10度）
    else {
      int midRadius =
          scaleInnerRadius + (scaleOuterRadius - scaleInnerRadius) * 0.7;
      x2 = static_cast<int>(midRadius * sinVal);
      y2 = static_cast<int>(-midRadius * cosVal);

      painter.setPen(QPen(scaleMinorColor, 1));
      painter.drawLine(x1, y1, x2, y2);
    }
  }

  // ========== 5. 在罗盘内部绘制方向文字 ==========
  QFont dirFont;
  dirFont.setFamily("Arial");
  dirFont.setPointSize(10);
  dirFont.setBold(true);
  painter.setFont(dirFont);
  QFontMetrics fm(dirFont);

  // 四个主要方向
  QString directions[4] = {tr("N"), tr("E"), tr("S"), tr("W")};
  int directionAngles[4] = {0, 90, 180, 270};

  for (int i = 0; i < 4; i++) {
    double angle = directionAngles[i];
    double rad = qDegreesToRadians(angle);

    // 计算文字位置
    int x = static_cast<int>(textRadius * qSin(rad));
    int y = static_cast<int>(-textRadius * qCos(rad));

    QString text = directions[i];
    int textW = fm.horizontalAdvance(text);
    int textH = fm.height();

    // 绘制文字背景（确保清晰）
    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(textBackgroundColor);
    painter.drawEllipse(QPoint(x, y), textW / 2 + 2, textH / 2 + 2);

    // 绘制文字
    painter.setPen(textColor);
    painter.drawText(QRect(x - textW / 2, y - textH / 2, textW, textH),
                     Qt::AlignCenter, text);
    painter.restore();
  }

  painter.restore();

  // ========== 6. 在左侧绘制独立的指针 ==========
  painter.save();
  painter.translate(pointerCenterX, pointerCenterY);

  // 绘制指针主体（指向正上方）
  QPolygonF pointer;
  pointer << QPointF(0, -pointerHeight / 2)                 // 箭头顶端
          << QPointF(-pointerWidth / 2, pointerHeight / 2)  // 左下角
          << QPointF(0, pointerHeight / 4)                  // 中间底部
          << QPointF(pointerWidth / 2, pointerHeight / 2)   // 右下角
          << QPointF(0, -pointerHeight / 2);                // 回到顶端

  painter.setPen(QPen(pointerOutlineColor, 2));
  painter.setBrush(pointerColor);
  painter.drawPolygon(pointer);

  // 绘制指针中心线
  painter.setPen(QPen(pointerOutlineColor, 1));
  painter.drawLine(0, -pointerHeight / 2, 0, pointerHeight / 2);

  // 绘制指针中心圆点
  painter.setPen(QPen(pointerCenterColor, 2));
  painter.setBrush(pointerCenterBrush);
  painter.drawEllipse(QPointF(0, 0), 3, 3);

  painter.restore();

  // ========== 7. 绘制前进方向指示文本 ==========
  QFont indicatorFont;
  indicatorFont.setPointSize(9);
  indicatorFont.setBold(true);
  painter.setFont(indicatorFont);

  QString forwardText = tr("Forward");
  int textW = painter.fontMetrics().horizontalAdvance(forwardText);
  int textH = painter.fontMetrics().height();

  QRect textRect(pointerCenterX - textW / 2,
                 pointerCenterY + pointerHeight / 2 + 5, textW, textH);

  painter.setPen(forwardTextColor);
  painter.drawText(textRect, Qt::AlignCenter, forwardText);
}
