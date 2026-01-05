#include "src/Exercise/DrawSportsFreq.h"

#include <QBrush>
#include <QPainter>
#include <QPen>
#include <algorithm>

FrequencyCurveWidget::FrequencyCurveWidget(const QVector<int>& cyclingCnt,
                                           const QVector<int>& hikingCnt,
                                           const QVector<int>& runningCnt,
                                           bool isDarkTheme, QWidget* parent)
    : QWidget{parent},
      m_cyclingCounts(cyclingCnt),
      m_hikingCounts(hikingCnt),
      m_runningCounts(runningCnt),
      m_isDark(isDarkTheme) {
  // ✅ 永久锁死高度 50px 极致紧凑，频次次要信息，绝不挤压下方里程图
  const int FIX_WIDGET_HEIGHT = 50;
  this->setMinimumHeight(FIX_WIDGET_HEIGHT);
  this->setMaximumHeight(FIX_WIDGET_HEIGHT);
  this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void FrequencyCurveWidget::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);  // 抗锯齿，边缘平滑

  // ==============================================================
  // ✅ ✅ ✅ 核心算法【完全按你的思路实现：全局真实最大值为唯一基准】✅ ✅ ✅
  // 遍历所有月份+三种运动的全部数据，获取【唯一的全局真实最大值】，无封顶、无阈值、无折中
  // ==============================================================
  int maxGlobal = 0;
  for (int val : m_cyclingCounts) maxGlobal = qMax(maxGlobal, val);
  for (int val : m_hikingCounts) maxGlobal = qMax(maxGlobal, val);
  for (int val : m_runningCounts) maxGlobal = qMax(maxGlobal, val);
  if (maxGlobal <= 0) maxGlobal = 1;  // 防除零异常，仅当所有数据都是0时生效

  // ===================== 极简紧凑的绘图边距，适配50px高度，无任何空间浪费
  // =====================
  const int marginLeft = 12;
  const int marginRight = 12;
  const int marginTop = 2;
  const int marginBottom = 18;
  const int drawW = this->width() - marginLeft - marginRight;  // X轴可用宽度
  const int drawH =
      this->height() - marginTop - marginBottom;  // Y轴柱子可用高度【固定值】
  const double yScale =
      (double)drawH / maxGlobal;  // ✅ 唯一映射比例：数值/最大值 = 高度/总高度

  // 12个月份均分宽度，每月一个柱子分组
  const double groupWidth = (double)drawW / 12.0;
  const double barWidth = groupWidth / 4.0;   // 每组3根柱子的宽度，紧凑不拥挤
  const double barSpace = groupWidth / 20.0;  // 柱子之间的微小间距，区分明显

  // ===================== 配色和下方里程图完全一致，无需图例
  // =====================
  QColor cyclingColor =
      m_isDark ? QColor(90, 189, 94) : QColor(76, 175, 80);  // 骑行绿
  QColor hikingColor =
      m_isDark ? QColor(255, 171, 44) : QColor(255, 152, 0);  // 徒步橙
  QColor runningColor =
      m_isDark ? QColor(183, 70, 201) : QColor(156, 39, 176);  // 跑步紫
  QColor textColor = m_isDark ? QColor(230, 230, 230) : QColor(60, 60, 60);

  // ===================== 绘制单根柱子的极简函数 =====================
  auto drawBar = [&](double x, double y, double w, double h, QColor color) {
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(color));
    painter.drawRect(x, y, w, h);
  };

  // ===================== ✅ 小数值保底修复：非0值强制画1px，防止像素吞掉
  // =====================
  auto getRealHeight = [&](int count) -> double {
    double realH = count * yScale;
    if (count > 0 && realH < 1.0)
      realH = 1.0;  // 频次>0但高度<1px时，强制画1px，确保可见
    return realH;
  };

  // ===================== 遍历1-12月，绘制分组柱状图 =====================
  const int baseY = this->height() - marginBottom;  // X轴基线，柱子从下往上绘制
  for (int monthIdx = 0; monthIdx < 12; ++monthIdx) {
    const double groupX = marginLeft + monthIdx * groupWidth;
    // 获取当月三种运动的真实频次
    int cyc = m_cyclingCounts[monthIdx];
    int hik = m_hikingCounts[monthIdx];
    int run = m_runningCounts[monthIdx];
    // 计算精准高度（比例绝对真实）
    double h_cyc = getRealHeight(cyc);
    double h_hik = getRealHeight(hik);
    double h_run = getRealHeight(run);

    // 绘制：骑行 → 徒步 → 跑步 三根柱子
    drawBar(groupX + barSpace, baseY - h_cyc, barWidth, h_cyc, cyclingColor);
    drawBar(groupX + barSpace + barWidth, baseY - h_hik, barWidth, h_hik,
            hikingColor);
    drawBar(groupX + barSpace + barWidth * 2, baseY - h_run, barWidth, h_run,
            runningColor);
  }

  // ===================== 绘制X轴基线 + 1-12月文字标签（紧凑适配50px高度）
  // =====================
  painter.setPen(QPen(textColor, 1));
  painter.drawLine(marginLeft, baseY, this->width() - marginRight, baseY);
  QFont ft = painter.font();
  ft.setPointSize(7);  // 小字体，不拥挤，清晰可见
  painter.setFont(ft);

  // X轴月份标签 精准居中对齐每个月的柱子组
  for (int monthIdx = 0; monthIdx < 12; ++monthIdx) {
    double groupX = marginLeft + monthIdx * groupWidth;
    painter.drawText(groupX + groupWidth / 2 - 4, baseY + 12,
                     QString::number(monthIdx + 1));
  }
}
