#ifndef STEPHILLCHART_H
#define STEPHILLCHART_H

#include <QFrame>
#include <QPainter>
#include <QPainterPath>
#include <QString>  // ⭐ 新增：用于 m_titleText
#include <vector>

class StepHillChart : public QFrame {
  Q_OBJECT
 public:
  explicit StepHillChart(QWidget* parent = nullptr);
  void setStepData(const std::vector<int>& stepData);

  QSize minimumSizeHint() const override { return QSize(0, 30); }
  QSize sizeHint() const override { return QSize(0, 30); }

 protected:
  void paintEvent(QPaintEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;

 private:
  QPointF mapToChartPos(size_t index, int value);

  std::vector<int> m_stepData;
  int m_maxValue = 1;
  const int m_margin = 5;
  const int m_topBottomMargin = 2;

  QString
      m_titleText;  // ⭐ 新增：缓存翻译文本，解决 undeclared identifier 错误
};

#endif  // STEPHILLCHART_H