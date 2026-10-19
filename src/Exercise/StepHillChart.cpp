#include "StepHillChart.h"

#include <src/defines.h>

#include <QFontMetrics>
#include <QLinearGradient>
#include <QPainterPath>
#include <QResizeEvent>

StepHillChart::StepHillChart(QWidget* parent) : QFrame(parent) {
  setFrameStyle(QFrame::NoFrame);
  setStyleSheet("background: transparent;");
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_stepData.resize(90, 0);

  // ⭐ 缓存翻译文本，避免 paintEvent 中重复查找
  m_titleText = tr("Steps");
}

void StepHillChart::setStepData(const std::vector<int>& stepData) {
  m_stepData.clear();
  for (int i = 0; i < 90; ++i) {
    m_stepData.push_back(i < stepData.size() ? stepData[i] : 0);
  }
  m_maxValue = 1;
  for (int val : m_stepData) {
    if (val > m_maxValue) m_maxValue = val;
  }
  update();
}

QPointF StepHillChart::mapToChartPos(size_t index, int value) {
  int drawWidth = width() - 2 * m_margin;
  int drawHeight = height() - 2 * m_topBottomMargin;

  qreal x = m_margin + (index * drawWidth) / (m_stepData.size() - 1);
  qreal normalizedVal = (value * 1.0) / m_maxValue;
  qreal y = m_topBottomMargin + (drawHeight - normalizedVal * drawHeight);

  return QPointF(x, y);
}

void StepHillChart::paintEvent(QPaintEvent* event) {
  QFrame::paintEvent(event);

  // ⭐ 核心：创建 HiDPI 感知的 buffer
  qreal dpr = devicePixelRatioF();
  if (dpr <= 0.0) dpr = 1.0;

  QPixmap buffer(size() * dpr);     // 物理像素尺寸
  buffer.setDevicePixelRatio(dpr);  // ⭐ 关键：告诉 QPainter 坐标系仍是逻辑像素
  buffer.fill(Qt::transparent);

  QPainter p(&buffer);
  p.setRenderHint(QPainter::Antialiasing, true);
  p.setRenderHint(QPainter::SmoothPixmapTransform, true);  // 可选：提升渐变质量

  if (m_stepData.empty() || m_maxValue <= 1) return;

  // ========== 构建山丘路径 ==========
  QPainterPath path;
  path.moveTo(mapToChartPos(0, 0));
  for (size_t i = 0; i < m_stepData.size(); ++i) {
    path.lineTo(mapToChartPos(i, m_stepData[i]));
  }
  path.lineTo(mapToChartPos(m_stepData.size() - 1, 0));
  path.closeSubpath();

  // ========== 配色（保持原有逻辑）==========
  QColor topColor, bottomColor, borderColor;
  if (isDark) {
    topColor = QColor(76, 175, 80, 150);
    bottomColor = QColor(76, 175, 80, 40);
    borderColor = QColor(102, 187, 106, 180);

  } else {
    topColor = QColor(66, 133, 244, 180);
    bottomColor = QColor(66, 133, 244, 60);
    borderColor = QColor(41, 98, 198, 200);
  }

  // ========== 绘制山丘 ==========
  QLinearGradient grad(0, 0, 0, height());
  grad.setColorAt(0, topColor);
  grad.setColorAt(1, bottomColor);
  p.setBrush(grad);
  p.setPen(QPen(borderColor, 1));
  p.drawPath(path);

  // ========== ⭐ 左上角标题：同色系衬底，覆盖图形时更融合 ==========
  QFont titleFont = font();
  titleFont.setPixelSize(12);
  titleFont.setBold(true);
  p.setFont(titleFont);

  QFontMetrics fm(titleFont);
  int padX = 4, padY = 2;
  int textW = fm.horizontalAdvance(m_titleText);
  int textH = fm.height();
  int x = m_margin + padX;
  int y = m_topBottomMargin + padY + 8;

  // ⭐ 关键改动：衬底使用山丘同色系，而非纯黑/纯白
  // 高不透明度确保文字始终清晰，同色系确保与山丘融合自然
  QColor bgColor, textColor;
  if (isDark) {
    bgColor = QColor(30, 70, 35, 160);  // 深绿底，与暗绿山丘同源
    textColor = QColor(180, 230, 180);  // 柔光绿字
  } else {
    bgColor = QColor(220, 235, 255, 180);  // 淡蓝底，与谷歌蓝山丘同源
    textColor = QColor(25, 55, 110);       // 深蓝灰字
  }

  p.setPen(Qt::NoPen);
  p.setBrush(bgColor);
  p.drawRoundedRect(QRectF(x - padX, y - fm.ascent() - padY, textW + padX * 2,
                           textH + padY * 2),
                    3, 3);

  p.setPen(textColor);
  p.drawText(x, y, m_titleText);

  // ========== 输出到屏幕 ==========
  QPainter painter(this);
  painter.drawPixmap(0, 0, buffer);
}

void StepHillChart::resizeEvent(QResizeEvent* event) {
  QFrame::resizeEvent(event);
  update();
}