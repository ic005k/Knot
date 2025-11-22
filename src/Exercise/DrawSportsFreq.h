#ifndef FREQUENCYCURVEWIDGET_H
#define FREQUENCYCURVEWIDGET_H

#include <QColor>
#include <QFont>
#include <QList>  // 改为 QList（匹配你调用时的参数类型，错误信息显示是 QList）
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QWidget>

class FrequencyCurveWidget : public QWidget {
  Q_OBJECT
 public:
  // 文本位置枚举（不变）
  enum TextPosition { Top = 0, Bottom = 1 };

  // 核心修复：参数顺序调整！textPosition 放倒数第二位，parent
  // 仍在最后（保持兼容）
  explicit FrequencyCurveWidget(
      const QList<int>& cycling,  // 改为 QList（匹配你的调用）
      const QList<int>& hiking, const QList<int>& running, bool isDark,
      QWidget* parent = nullptr,            // parent 仍在最后，默认 nullptr
      TextPosition textPosition = Bottom);  // 新增参数放倒数第二，默认 Bottom

 protected:
  void paintEvent(QPaintEvent*) override;

 private:
  QList<int> calculateXPoints() const;  // QVector → QList（统一类型）
  void calculateMaxCount();
  QColor getColor(int type) const;

  QRect getTextRect(int monthLeft, int monthRight) const;
  QLine getTickLine(int tickX) const;

 private:
  // 成员变量声明顺序（初始化列表必须按这个顺序！）
  QList<int> m_cycling;  // 改为 QList（匹配参数类型）
  QList<int> m_hiking;
  QList<int> m_running;
  int m_maxCount;
  bool m_isDark;
  int mh = 25;
  TextPosition m_textPosition;  // 新增成员（最后声明）
  void drawFrequencyBlocks(QPainter& painter, int monthLeft, int monthWidth,
                           int monthIndex);

  int m_fontSize = 10;    // 字体大小（默认10，与你cpp中setPointSize(10)一致）
  int m_textHeight = 12;  // 文本高度（适配10号字，可根据字体大小调整）
};

#endif  // FREQUENCYCURVEWIDGET_H
