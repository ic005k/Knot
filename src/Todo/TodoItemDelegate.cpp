#include "TodoItemDelegate.h"

#include <QApplication>
#include <QStyle>

TodoItemDelegate::TodoItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {}

QColor TodoItemDelegate::getColorByIndex(int index) const {
  switch (index) {
    case 0:
      return QColor("gray");
    case 1:
      return QColor("red");
    case 2:
      return QColor("orange");
    case 3:
      return QColor("#3498DB");
    default:
      return QColor("black");
  }
}

void TodoItemDelegate::paint(QPainter* painter,
                             const QStyleOptionViewItem& option,
                             const QModelIndex& index) const {
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);

  // 1. 仅绘制背景，不绘制默认文本
  if (opt.state & QStyle::State_Selected) {
    painter->fillRect(opt.rect, opt.palette.highlight());
  } else if (opt.state & QStyle::State_MouseOver) {
    painter->fillRect(opt.rect, opt.palette.base().color().darker(105));
  } else {
    painter->fillRect(opt.rect, opt.palette.base());
  }

  // 2. 解析数据
  QString rawData = index.data(Qt::DisplayRole).toString();
  QStringList parts = rawData.split("|==|");
  if (parts.size() < 3) {
    painter->restore();
    return;
  }

  QString timeText = parts[0].trimmed();
  int colorIndex = parts[1].trimmed().toInt();
  QString todoText = parts[2].trimmed();
  QColor barColor = getColorByIndex(colorIndex);

  // 3. 统一布局参数（与 sizeHint 严格一致）
  QRect rect = opt.rect;
  int leftPadding = 12;
  int barWidth = 5;
  int barGap = 10;
  int rightPadding = 10;
  int topMargin = 8;
  int gapBetweenLines = 4;
  int bottomMargin = 8;

  int textX = rect.left() + leftPadding + barWidth + barGap;
  int textW = rect.width() - leftPadding - barWidth - barGap - rightPadding;

  // 4. 绘制左侧垂直颜色条（贯穿整个条目）
  painter->setPen(Qt::NoPen);
  painter->setBrush(barColor);
  QRect barRect(rect.left() + leftPadding, rect.top() + topMargin, barWidth,
                rect.height() - topMargin - bottomMargin);
  painter->drawRoundedRect(barRect, 2.5, 2.5);

  // 5. 文本颜色
  bool isSelected = (opt.state & QStyle::State_Selected);
  QColor mainTextColor = isSelected ? opt.palette.highlightedText().color()
                                    : opt.palette.text().color();
  QColor subTextColor = isSelected
                            ? QColor(mainTextColor.red(), mainTextColor.green(),
                                     mainTextColor.blue(), 180)
                            : QColor(mainTextColor.red(), mainTextColor.green(),
                                     mainTextColor.blue(), 140);

  // ========== 流式布局绘制（与 sizeHint 完全对齐）==========

  // --- 第一行：定时文本 ---
  QFont timeFont = opt.font;
  timeFont.setPointSize(qMax(8, timeFont.pointSize() - 3));
  timeFont.setBold(false);
  QFontMetrics timeFm(timeFont);

  painter->setFont(timeFont);
  painter->setPen(subTextColor);

  int currentY = rect.top() + topMargin;
  QRect timeRect(textX, currentY, textW, timeFm.height());
  painter->drawText(timeRect,
                    Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine,
                    timeText);

  // Y 坐标下移
  currentY += timeFm.height() + gapBetweenLines;

  // --- 第二行：Todo文本（自动换行）---
  QFont todoFont = opt.font;
  todoFont.setBold(true);
  QFontMetrics todoFm(todoFont);

  painter->setFont(todoFont);
  painter->setPen(mainTextColor);

  // ⚠️ 关键：剩余高度 = 总高度 - 当前Y - 底部边距
  // 不再使用 midY 平分，而是用尽所有剩余空间
  int remainingHeight = rect.bottom() - currentY - bottomMargin;
  QRect todoRect(textX, currentY, textW, qMax(0, remainingHeight));

  painter->drawText(todoRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                    todoText);

  painter->restore();
}

QSize TodoItemDelegate::sizeHint(const QStyleOptionViewItem& option,
                                 const QModelIndex& index) const {
  QString rawData = index.data(Qt::DisplayRole).toString();
  QStringList parts = rawData.split("|==|");

  int baseHeight = qMax(64, option.fontMetrics.height() * 2 + 20);
  if (parts.size() < 3) return QSize(option.rect.width(), baseHeight);

  // 字体参数与 paint 完全一致
  QFont timeFont = option.font;
  timeFont.setPointSize(qMax(8, timeFont.pointSize() - 3));
  timeFont.setBold(false);
  QFontMetrics timeFm(timeFont);

  QFont todoFont = option.font;
  todoFont.setBold(true);
  QFontMetrics todoFm(todoFont);

  // 布局参数与 paint 完全一致
  int leftPadding = 12;
  int barWidth = 5;
  int barGap = 10;
  int rightPadding = 10;
  int topMargin = 8;
  int gapBetweenLines = 4;
  int bottomMargin = 8;

  int textW =
      option.rect.width() - leftPadding - barWidth - barGap - rightPadding;

  QString todoText = parts[2].trimmed();
  QRect boundingRect = todoFm.boundingRect(
      QRect(0, 0, qMax(1, textW), 10000),
      Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, todoText);

  int totalHeight = topMargin + timeFm.height() + gapBetweenLines +
                    boundingRect.height() + bottomMargin;

  return QSize(option.rect.width(), qMax(baseHeight, totalHeight));
}