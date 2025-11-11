#ifndef FREQUENCYCURVEWIDGET_H
#define FREQUENCYCURVEWIDGET_H

#include <QColor>
#include <QFont>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QVector>
#include <QWidget>

class FrequencyCurveWidget : public QWidget {
  Q_OBJECT
 public:
  explicit FrequencyCurveWidget(const QVector<int>& cycling,
                                const QVector<int>& hiking,
                                const QVector<int>& running, bool isDark,
                                QWidget* parent = nullptr);

 protected:
  void paintEvent(QPaintEvent*) override;

 private:
  QVector<int> calculateXPoints() const;
  void calculateMaxCount();
  QColor getColor(int type) const;
  void drawSmoothCurve(QPainter& painter, const QVector<int>& xPoints,
                       const QVector<int>& counts, const QColor& color);
  double mapCountToY(int count) const;

 private:
  QVector<int> m_cycling;  // 骑行频次数据
  QVector<int> m_hiking;   // 徒步频次数据
  QVector<int> m_running;  // 跑步频次数据
  int m_maxCount;          // 最大频次（用于比例映射）
  bool m_isDark;           // 是否暗模式
  int mh = 25;             // 控件的高度 (修正为整数)
};

#endif  // FREQUENCYCURVEWIDGET_H
