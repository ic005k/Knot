#include "src/Exercise/DrawSportsFreq.h"

#include <QFont>
#include <QList>  // 改为 QList
#include <QPainter>
#include <QPen>
#include <QRect>
#include <QString>
#include <algorithm>

// --- Constructor（核心修复：初始化列表按声明顺序，不重复初始化）---
FrequencyCurveWidget::FrequencyCurveWidget(const QList<int>& cycling,
                                           const QList<int>& hiking,
                                           const QList<int>& running,
                                           bool isDark, QWidget* parent,
                                           TextPosition textPosition)
    : QWidget(parent),  // 先初始化父类
      m_cycling(
          cycling),  // 按 h 文件声明顺序：m_cycling → m_hiking → m_running
      m_hiking(hiking),
      m_running(running),
      m_isDark(isDark),
      m_textPosition(textPosition) {  // 最后初始化新增成员
  setFixedHeight(mh);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  calculateMaxCount();
}

// --- Paint Event（不变，仅类型适配 QList）---
void FrequencyCurveWidget::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setPen(Qt::NoPen);

  QList<int> xPoints = calculateXPoints();  // QVector → QList

  int totalWidth = width();
  for (int i = 0; i < 12; ++i) {
    int monthLeft, monthRight;
    if (i == 0) {
      monthLeft = 0;
      monthRight = (xPoints[0] + xPoints[1]) / 2;
    } else if (i == 11) {
      monthLeft = (xPoints[10] + xPoints[11]) / 2;
      monthRight = totalWidth;
    } else {
      monthLeft = (xPoints[i - 1] + xPoints[i]) / 2;
      monthRight = (xPoints[i] + xPoints[i + 1]) / 2;
    }
    int monthWidth = monthRight - monthLeft;
    drawFrequencyBlocks(painter, monthLeft, monthRight, monthWidth, i);
  }

  QFont font;
  font.setPointSize(6);
  painter.setFont(font);
  QPen pen(m_isDark ? Qt::white : Qt::black);
  painter.setPen(pen);

  for (int i = 0; i < 12; ++i) {
    int monthLeft, monthRight;
    if (i == 0) {
      monthLeft = 0;
      monthRight = (xPoints[0] + xPoints[1]) / 2;
    } else if (i == 11) {
      monthLeft = (xPoints[10] + xPoints[11]) / 2;
      monthRight = totalWidth;
    } else {
      monthLeft = (xPoints[i - 1] + xPoints[i]) / 2;
      monthRight = (xPoints[i] + xPoints[i + 1]) / 2;
    }

    QRect textRect = getTextRect(monthLeft, monthRight);
    painter.drawText(textRect, Qt::AlignCenter, QString::number(i + 1));

    if (i < 11) {
      int tickX = monthRight;
      QLine tickLine = getTickLine(tickX);
      painter.drawLine(tickLine);
    }
  }
}

// --- Calculate X Points（QVector → QList，逻辑不变）---
QList<int> FrequencyCurveWidget::calculateXPoints() const {
  QList<int> xPoints;
  int totalWidth = width();
  for (int i = 0; i < 12; ++i) {
    qreal x = (2 * i + 1) * totalWidth / 24.0;
    xPoints.append(qRound(x));
  }
  return xPoints;
}

// --- Calculate Max Count（QVector → QList，逻辑不变）---
void FrequencyCurveWidget::calculateMaxCount() {
  QList<int> allCounts;
  for (int i = 0; i < 12; ++i) {
    allCounts.append(m_cycling[i]);
    allCounts.append(m_hiking[i]);
    allCounts.append(m_running[i]);
  }
  if (allCounts.isEmpty()) {
    m_maxCount = 1;
    return;
  }
  std::sort(allCounts.begin(), allCounts.end());
  int quantileIndex =
      qMin(qRound(allCounts.size() * 0.95), allCounts.size() - 1);
  m_maxCount = qMax(1, allCounts[quantileIndex]);
}

// --- Get Color（不变）---
QColor FrequencyCurveWidget::getColor(int type) const {
  if (m_isDark) {
    switch (type) {
      case 0:
        return QColor(90, 189, 94, 220);
      case 1:
        return QColor(255, 171, 44, 220);
      case 2:
        return QColor(183, 70, 201, 220);
    }
  } else {
    switch (type) {
      case 0:
        return QColor(76, 175, 80, 220);
      case 1:
        return QColor(255, 152, 0, 220);
      case 2:
        return QColor(156, 39, 176, 220);
    }
  }
  return Qt::black;
}

// --- Get Text Rect（不变）---
QRect FrequencyCurveWidget::getTextRect(int monthLeft, int monthRight) const {
  const int textHeight = 12;
  if (m_textPosition == Top) {
    return QRect(monthLeft, 0, monthRight - monthLeft, textHeight);
  } else {
    return QRect(monthLeft, mh - textHeight, monthRight - monthLeft,
                 textHeight);
  }
}

// --- Get Tick Line（不变）---
QLine FrequencyCurveWidget::getTickLine(int tickX) const {
  const int textHeight = 12;
  const int tickLength = 5;
  if (m_textPosition == Top) {
    return QLine(tickX, textHeight, tickX, textHeight + tickLength);
  } else {
    return QLine(tickX, mh - textHeight - tickLength, tickX, mh - textHeight);
  }
}

// --- Draw Frequency Blocks（QList 适配，逻辑不变）---
void FrequencyCurveWidget::drawFrequencyBlocks(QPainter& painter, int monthLeft,
                                               int monthRight, int monthWidth,
                                               int monthIndex) {
  const int textHeight = 12;
  const int bottomMargin = 2;
  const int outerGap = 2;
  const int innerGap = 1;

  int availableHeight, blockStartY;
  if (m_textPosition == Top) {
    blockStartY = textHeight + 1;
    availableHeight = mh - blockStartY - bottomMargin;
  } else {
    blockStartY = 1;
    availableHeight = (mh - textHeight) - blockStartY - bottomMargin;
  }
  if (availableHeight < 3) availableHeight = 3;

  int totalBlockWidth = monthWidth - 2 * outerGap;
  if (totalBlockWidth <= 0) return;
  int singleBlockWidth = (totalBlockWidth - 2 * innerGap) / 3;
  if (singleBlockWidth <= 0) singleBlockWidth = 1;

  // QList 索引访问（和 QVector 一致，无需修改）
  int cyclingCount =
      (monthIndex < m_cycling.size()) ? m_cycling[monthIndex] : 0;
  int hikingCount = (monthIndex < m_hiking.size()) ? m_hiking[monthIndex] : 0;
  int runningCount =
      (monthIndex < m_running.size()) ? m_running[monthIndex] : 0;

  QList<int> counts = {cyclingCount, hikingCount, runningCount};
  for (int i = 0; i < 3; ++i) {
    int count = counts[i];
    if (count <= 0) continue;

    int blockX = monthLeft + outerGap + i * (singleBlockWidth + innerGap);
    double blockHeight =
        (static_cast<double>(count) / m_maxCount) * availableHeight;
    if (blockHeight < 1) blockHeight = 1;

    int blockY = blockStartY + (availableHeight - blockHeight);
    int blockBottom = mh - bottomMargin - (m_textPosition == Top ? 0 : 1);

    QRectF blockRect(blockX, blockY, singleBlockWidth, blockHeight);
    painter.setBrush(getColor(i));
    painter.drawRoundedRect(blockRect, 1, 1);
  }
}
