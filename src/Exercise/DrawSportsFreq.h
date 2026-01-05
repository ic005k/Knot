#ifndef DRAWSPORTSFREQ_H
#define DRAWSPORTSFREQ_H

#include <QColor>
#include <QVector>
#include <QWidget>

class FrequencyCurveWidget : public QWidget {
  Q_OBJECT
 public:
  explicit FrequencyCurveWidget(const QVector<int>& cyclingCnt,
                                const QVector<int>& hikingCnt,
                                const QVector<int>& runningCnt,
                                bool isDarkTheme, QWidget* parent = nullptr);

 protected:
  void paintEvent(QPaintEvent* event) override;

 private:
  QVector<int> m_cyclingCounts;
  QVector<int> m_hikingCounts;
  QVector<int> m_runningCounts;
  bool m_isDark;
};

#endif  // DRAWSPORTSFREQ_H
