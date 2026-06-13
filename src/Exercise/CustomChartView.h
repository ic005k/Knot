#ifndef CUSTOMCHARTVIEW_H
#define CUSTOMCHARTVIEW_H

#include <QChartView>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QRectF>
#include <QToolTip>
#include <QVector>

#include "src/Exercise/Steps.h"
#include "src/defines.h"

class CustomChartView : public QChartView {
  Q_OBJECT
 public:
  explicit CustomChartView(QWidget* parent = nullptr);

  void setMonthData(const QVector<Steps::MonthData>& data);

 protected:
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

  void mouseMoveEvent(QMouseEvent* event) override;

 private:
  void handleChartClick(QMouseEvent* event);

  QVector<Steps::MonthData> m_monthData;

  QLabel* m_tipLabel = nullptr;    // 自定义提示窗口
  QTimer* m_hideTimer = nullptr;   // 延时隐藏定时器
  const int m_tipShowTime = 2500;  // 显示时长 2.5s
};

#endif  // CUSTOMCHARTVIEW_H