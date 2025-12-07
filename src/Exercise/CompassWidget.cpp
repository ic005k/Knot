#include "CompassWidget.h"

#include <QBrush>
#include <QFont>
#include <QFontMetrics>
#include <QPen>
#include <QtMath>

CompassWidget::CompassWidget(QWidget* parent) : QWidget(parent) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setMinimumSize(50, 30);
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

  // ========== 1. 基础绘制区域 ==========
  QRect drawRect =
      rect().adjusted(m_padding, m_padding, -m_padding, -m_padding);
  int centerX = drawRect.center().x();
  int centerY = drawRect.center().y();

  // ========== 2. 绘制外框 ==========
  painter.setPen(QPen(Qt::black, 2));
  painter.setBrush(Qt::white);
  painter.drawRect(drawRect);

  // ========== 3. 绘制固定的刻度（字不偏转） ==========
  painter.setPen(QPen(Qt::darkGray, 2));
  // 正北刻度（上边框中点）
  painter.drawLine(centerX, drawRect.top(), centerX, drawRect.top() + 10);
  // 正南刻度（下边框中点）
  painter.drawLine(centerX, drawRect.bottom(), centerX, drawRect.bottom() - 10);
  // 正东刻度（右边框中点）
  painter.drawLine(drawRect.right(), centerY, drawRect.right() - 10, centerY);
  // 正西刻度（左边框中点）
  painter.drawLine(drawRect.left(), centerY, drawRect.left() + 10, centerY);

  // ========== 4. 绘制固定的N/E/S/W（15号字，不偏转，防超出） ==========
  QFont dirFont;
  dirFont.setFamily("sans-serif");  // 跨平台无衬线字体
  dirFont.setPointSize(15);         // 字号15
  dirFont.setBold(true);
  painter.setFont(dirFont);
  QFontMetrics fm(dirFont);
  int textH = fm.height();
  int textW = fm.horizontalAdvance(tr("W"));
  int gap = 8;

  // 北（N）：固定在上边框上方，不超出
  int n_y = qMax(gap, drawRect.top() - textH - gap);
  QRect nRect(centerX - textW / 2, n_y, textW, textH);
  // 南（S）：固定在下边框下方，不超出
  int s_y = qMin(height() - textH - gap, drawRect.bottom() + gap);
  QRect sRect(centerX - textW / 2, s_y, textW, textH);
  // 东（E）：固定在右边框右侧，不超出
  int e_x = qMin(width() - textW - gap, drawRect.right() + gap);
  QRect eRect(e_x, centerY - textH / 2, textW, textH);
  // 西（W）：固定在左边框左侧，不超出
  int w_x = qMax(gap, drawRect.left() - textW - gap);
  QRect wRect(w_x, centerY - textH / 2, textW, textH);

  // 绘制文字（白色背景+黑色字，清晰）
  painter.setPen(Qt::black);
  drawDirText(&painter, tr("N"), nRect);
  drawDirText(&painter, tr("S"), sRect);
  drawDirText(&painter, tr("E"), eRect);
  drawDirText(&painter, tr("W"), wRect);

  // ========== 5. 绘制旋转的指针（指向运动方向） ==========
  painter.save();
  painter.translate(centerX, centerY);
  painter.rotate(m_bearing);  // 只有指针旋转，指向当前方位

  int pointerLen = qMin(drawRect.width(), drawRect.height()) / 3;
  QPolygonF pointer;
  pointer << QPointF(0, -pointerLen)  // 指针顶端（指向运动方向）
          << QPointF(-8, 10)          // 左下
          << QPointF(8, 10);          // 右下

  painter.setPen(QPen(Qt::red, 2));
  painter.setBrush(Qt::red);
  painter.drawPolygon(pointer);
  painter.restore();
}

void CompassWidget::drawDirText(QPainter* painter, const QString& text,
                                const QRect& rect) {
  painter->save();
  painter->fillRect(rect, Qt::white);  // 白色背景防重叠
  painter->setPen(Qt::black);
  painter->drawText(rect, Qt::AlignCenter, text);
  painter->restore();
}
