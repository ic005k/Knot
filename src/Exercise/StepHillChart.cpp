#include "StepHillChart.h"

#include <src/defines.h>

#include <QLinearGradient>
#include <QResizeEvent>

StepHillChart::StepHillChart(QWidget* parent) : QFrame(parent) {
  // 安卓端核心设置：无边框、背景透明（适配手机界面）
  setFrameStyle(QFrame::NoFrame);
  setStyleSheet("background: transparent;");
  // 尺寸策略：宽度/高度自适应父容器（安卓屏幕）
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  // 初始数据占位
  m_stepData.resize(90, 0);
}

void StepHillChart::setStepData(const std::vector<int>& stepData) {
  // 截断/补0到90个数据（适配X轴长度）
  m_stepData.clear();
  for (int i = 0; i < 90; ++i) {
    m_stepData.push_back(i < stepData.size() ? stepData[i] : 0);
  }
  // 更新最大值（用于Y轴归一化）
  m_maxValue = 1;  // 避免除0
  for (int val : m_stepData) {
    if (val > m_maxValue) m_maxValue = val;
  }
  // 触发重绘（安卓端异步更新，避免阻塞UI）
  update();
}

QPointF StepHillChart::mapToChartPos(size_t index, int value) {
  // 逻辑完全不变，仅参数类型改size_t
  int drawWidth = width() - 2 * m_margin;
  int drawHeight = height() - 2 * m_topBottomMargin;

  qreal x = m_margin + (index * drawWidth) /
                           (m_stepData.size() - 1);  // index已为size_t，无转换
  qreal normalizedVal = (value * 1.0) / m_maxValue;
  qreal y = m_topBottomMargin + (drawHeight - normalizedVal * drawHeight);

  return QPointF(x, y);
}

void StepHillChart::paintEvent(QPaintEvent* event) {
  QFrame::paintEvent(event);

  QPixmap buffer(size());
  buffer.fill(Qt::transparent);
  QPainter p(&buffer);
  p.setRenderHint(QPainter::Antialiasing, true);

  if (m_stepData.empty() || m_maxValue <= 1) return;

  // 构建路径
  QPainterPath path;
  path.moveTo(mapToChartPos(0, 0));
  for (size_t i = 0; i < m_stepData.size(); ++i) {
    path.lineTo(mapToChartPos(i, m_stepData[i]));
  }
  path.lineTo(mapToChartPos(m_stepData.size() - 1, 0));
  path.closeSubpath();

  // ========== 核心：根据isDark切换配色 ==========
  QColor topColor, bottomColor, borderColor;
  if (isDark) {                                // 暗黑模式
    topColor = QColor(76, 175, 80, 150);       // 暗绿（低亮度、半透明）
    bottomColor = QColor(76, 175, 80, 40);     // 更淡的暗绿
    borderColor = QColor(102, 187, 106, 180);  // 轮廓色（稍亮，保证辨识度）
  } else {                                     // 浅色模式
    topColor = QColor(66, 133, 244, 180);      // 谷歌蓝（原配色）
    bottomColor = QColor(66, 133, 244, 60);
    borderColor = QColor(41, 98, 198, 200);
  }

  // 适配后的渐变
  QLinearGradient grad(0, 0, 0, height());
  grad.setColorAt(0, topColor);
  grad.setColorAt(1, bottomColor);
  p.setBrush(grad);
  p.setPen(QPen(borderColor, 1));

  // 绘制山丘（不变）
  p.drawPath(path);

  QPainter painter(this);
  painter.drawPixmap(0, 0, buffer);
}

void StepHillChart::resizeEvent(QResizeEvent* event) {
  QFrame::resizeEvent(event);
  // 安卓屏幕旋转/尺寸变化时重绘
  update();
}
