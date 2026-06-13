#include "CustomChartView.h"

CustomChartView::CustomChartView(QWidget* parent) : QChartView(parent) {
  // 启用鼠标跟踪，确保能捕获所有鼠标事件
  setMouseTracking(true);

  // ========= 初始化自定义提示Label =========
  m_tipLabel = new QLabel(this);
  // 悬浮、置顶、无边框
  if (isAndroid)
    m_tipLabel->setWindowFlags(Qt::FramelessWindowHint);
  else
    m_tipLabel->setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint |
                               Qt::WindowStaysOnTopHint);

  if (isDark) {
    // 暗黑模式：深色背景、浅边框、浅色文字
    m_tipLabel->setStyleSheet(R"(
          background-color: #2b2b2b;
          color: #f0f0f0;
          border: 1px solid #555555;
          padding: 6px;
      )");
  } else {
    // 亮色模式：白底、灰色边框、内边距、字体
    m_tipLabel->setStyleSheet(R"(
        background-color: white;
        border: 1px solid #aaaaaa;
        padding: 6px;
    )");
  }

  m_tipLabel->hide();  // 默认隐藏

  // ========= 初始化延时隐藏定时器 =========
  m_hideTimer = new QTimer(this);
  m_hideTimer->setSingleShot(true);
  m_hideTimer->setInterval(m_tipShowTime);
  connect(m_hideTimer, &QTimer::timeout, this,
          [this]() { m_tipLabel->hide(); });
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

void CustomChartView::mouseMoveEvent(QMouseEvent* event) {
  QChartView::mouseMoveEvent(event);
  if (m_hideTimer->isActive()) m_hideTimer->stop();
  m_tipLabel->hide();
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

    // 每次弹窗前，强制关闭已有提示
    if (m_tipLabel->isVisible()) {
      m_tipLabel->hide();
    }
    if (m_hideTimer->isActive()) {
      m_hideTimer->stop();
    }

    // 设置文本
    m_tipLabel->setText(tooltip);
    // 自适应大小
    m_tipLabel->adjustSize();

    // 计算显示位置（鼠标上方）
    QPoint tipPos;
#ifdef Q_OS_ANDROID
    tipPos = event->pos() - QPoint(0, m_tipLabel->height() + 10);
    // 限制在当前控件内部，防止裁剪
    tipPos.setX(qBound(0, tipPos.x(), width() - m_tipLabel->width()));
    tipPos.setY(qBound(0, tipPos.y(), height() - m_tipLabel->height()));
#else
    QPoint globalPos = event->globalPosition().toPoint();
    tipPos = globalPos - QPoint(0, m_tipLabel->height() + 10);

    // 桌面端保留屏幕边界检测
    QRect screenRect = this->screen()->availableGeometry();
    if (tipPos.x() + m_tipLabel->width() > screenRect.right())
      tipPos.setX(screenRect.right() - m_tipLabel->width());
    if (tipPos.y() < screenRect.top()) tipPos = globalPos + QPoint(10, 10);
#endif

    // 显示提示 + 重置定时器
    m_tipLabel->move(tipPos);
    m_tipLabel->show();

    m_hideTimer->start();
  } else {
    // 无效点击，隐藏提示
    m_tipLabel->hide();
    if (m_hideTimer->isActive()) m_hideTimer->stop();
  }
}
