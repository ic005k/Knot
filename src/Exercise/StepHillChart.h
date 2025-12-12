#ifndef STEPHILLCHART_H
#define STEPHILLCHART_H

#include <QFrame>
#include <QPainter>
#include <QPainterPath>
#include <vector>

// 安卓适配版：90天步数山丘图（无坐标系、最小高度50、自适应宽度）
class StepHillChart : public QFrame {
  Q_OBJECT
 public:
  explicit StepHillChart(QWidget* parent = nullptr);

  // 设置步数数据（int[]可直接转vector）
  void setStepData(const std::vector<int>& stepData);

  // 重写最小尺寸：适配安卓，最小高度50
  QSize minimumSizeHint() const override { return QSize(0, 55); }
  QSize sizeHint() const override { return QSize(0, 55); }

 protected:
  void paintEvent(QPaintEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;

 private:
  // 映射数据坐标到绘图坐标（核心适配手机端）
  QPointF mapToChartPos(size_t index, int value);

  // 数据与参数（极简边距，适配50px高度）
  std::vector<int> m_stepData;      // 步数数据（长度90）
  int m_maxValue = 1;               // 步数最大值（避免除0）
  const int m_margin = 5;           // 左右边距5px
  const int m_topBottomMargin = 2;  // 上下边距2px
};

#endif  // STEPHILLCHART_H
