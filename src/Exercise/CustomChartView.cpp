#include "CustomChartView.h"

CustomChartView::CustomChartView(QWidget* parent) : QChartView(parent) {
  // 启用鼠标跟踪，确保能捕获所有鼠标事件
  setMouseTracking(true);
}

void CustomChartView::setMonthData(const QVector<Steps::MonthData>& data) {
  m_monthData = data;
}

void CustomChartView::mousePressEvent(QMouseEvent* event) {
  // 首先调用基类实现
  QChartView::mousePressEvent(event);
  handleChartClick(event);
  // 接受事件，防止事件继续传播
  event->accept();
}

void CustomChartView::mouseReleaseEvent(QMouseEvent* event) {
  QChartView::mouseReleaseEvent(event);
}

void CustomChartView::handleChartClick(QMouseEvent* event) {
  if (!chart()) return;

  // 获取鼠标位置和绘图区域
  QPoint mousePos = event->pos();
  QRectF plotArea = chart()->plotArea();

  // 计算月份
  int month = -1;

  // 检查点击是否在绘图区域内
  if (plotArea.contains(mousePos)) {
    // 在绘图区域内，计算月份
    double relativeY = mousePos.y() - plotArea.y();
    double totalHeight = plotArea.height();
    double monthHeight = totalHeight / 12.0;
    int monthIndex = static_cast<int>(relativeY / monthHeight);
    month = 12 - monthIndex;  // 底部是1月，顶部是12月
    month = qBound(1, month, 12);
  } else if (mousePos.x() < plotArea.left()) {
    // 在Y轴区域（左侧），计算月份
    // 使用整个视图高度来计算，而不是绘图区域高度
    int viewHeight = height();
    double monthHeight = viewHeight / 12.0;
    int monthIndex = static_cast<int>(mousePos.y() / monthHeight);
    month = 12 - monthIndex;  // 底部是1月，顶部是12月
    month = qBound(1, month, 12);
  }

  // 如果月份有效，显示提示信息
  if (month >= 1 && month <= m_monthData.size()) {
    const Steps::MonthData& data = m_monthData[month - 1];

    // 使用统一的提示信息生成逻辑
    QString tooltip = QString(
        tr("Month") + ": " + QString::number(month) + "\n" + tr("Cycling") +
        ": " + QString::number(data.cyclingDist, 'f', 1) + " km (" +
        QString::number(data.cyclingCount) + " " + tr("times") + ")\n" +
        tr("Hiking") + ": " + QString::number(data.hikingDist, 'f', 1) +
        " km (" + QString::number(data.hikingCount) + " " + tr("times") +
        ")\n" + tr("Running") + ": " +
        QString::number(data.runningDist, 'f', 1) + " km (" +
        QString::number(data.runningCount) + " " + tr("times") + ")");

    // 计算提示框显示位置（鼠标上方）
    QFontMetrics fm(QToolTip::font());
    int lineHeight = fm.height();
    int tooltipHeight = lineHeight * 5 + 10;  // 大约5行高度

    QPoint globalPos = event->globalPosition().toPoint();
    QPoint showPos = globalPos - QPoint(0, tooltipHeight);

    QToolTip::showText(showPos, tooltip);
  }
}