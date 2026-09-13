#include "TodoRecycleBinDelegate.h"

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QPainter>
#include <QTextDocument>
#include <QtMath>  // for qCeil

namespace Layout {
constexpr int LeftPadding = 12;
constexpr int RightPadding = 10;
constexpr int TopMargin = 10;
constexpr int BottomMargin = 10;
constexpr int GapBetweenLines = 6;
}  // namespace Layout

TodoRecycleBinDelegate::TodoRecycleBinDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {}

void TodoRecycleBinDelegate::paint(QPainter* painter,
                                   const QStyleOptionViewItem& option,
                                   const QModelIndex& index) const {
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);

  // 1. 绘制背景
  if (opt.state & QStyle::State_Selected) {
    painter->fillRect(opt.rect, opt.palette.highlight());
  } else if (opt.state & QStyle::State_MouseOver) {
    painter->fillRect(opt.rect, opt.palette.base().color().darker(105));
  } else {
    painter->fillRect(opt.rect, opt.palette.base());
  }

  // 2. 解析数据
  QString rawData = index.data(Qt::DisplayRole).toString();
  QStringList parts = rawData.split("|==|", Qt::KeepEmptyParts);
  if (parts.size() < 2) {
    painter->restore();
    return;
  }

  QString timeText = parts[0].trimmed();
  QString todoText = parts[1].trimmed();

  // 3. 统一布局参数（与 sizeHint 严格一致）
  QRect rect = opt.rect;
  int textX = rect.left() + Layout::LeftPadding;
  int textW = rect.width() - Layout::LeftPadding - Layout::RightPadding;

  // 4. 动态计算颜色
  bool isSelected = (opt.state & QStyle::State_Selected);
  QColor mainTextColor =
      isSelected ? QColor("#FFFFFF") : opt.palette.text().color();
  QColor subTextColor = isSelected
                            ? QColor(255, 255, 255, 180)
                            : opt.palette.color(QPalette::PlaceholderText);

  // ========== 流式布局绘制（与 sizeHint 完全对齐）==========

  // --- 第一行：时间 ---
  QFont timeFont = opt.font;
  timeFont.setPointSize(qMax(8, timeFont.pointSize() - 2));
  timeFont.setBold(false);
  QFontMetrics timeFm(timeFont);

  painter->setFont(timeFont);
  painter->setPen(subTextColor);

  int currentY = rect.top() + Layout::TopMargin;
  QRect timeRect(textX, currentY, textW, timeFm.height());
  painter->drawText(timeRect,
                    Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine,
                    timeText);

  // Y 坐标下移
  currentY += timeFm.height() + Layout::GapBetweenLines;

  // --- 第二行：Todo文本（自动换行）---
  QFont todoFont = opt.font;
  todoFont.setBold(false);
  QFontMetrics todoFm(todoFont);

  painter->setFont(todoFont);
  painter->setPen(mainTextColor);

  // 剩余高度 = 总高度 - 当前Y - 底部边距
  // 不再使用 qMax(0, ...) 包裹，让 drawText 自然处理边界
  int remainingHeight = rect.bottom() - currentY - Layout::BottomMargin;
  QRect todoRect(textX, currentY, textW, remainingHeight);

  painter->drawText(todoRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                    todoText);

  painter->restore();
}

QSize TodoRecycleBinDelegate::sizeHint(const QStyleOptionViewItem& option,
                                       const QModelIndex& index) const {
  QString rawData = index.data(Qt::DisplayRole).toString();
  QStringList parts = rawData.split("|==|", Qt::KeepEmptyParts);

  int baseHeight = qMax(56, option.fontMetrics.height() * 2 + 24);
  if (parts.size() < 2) return QSize(option.rect.width(), baseHeight);

  // 字体参数与 paint 完全一致
  QFont timeFont = option.font;
  timeFont.setPointSize(qMax(8, timeFont.pointSize() - 2));
  timeFont.setBold(false);
  QFontMetrics timeFm(timeFont);

  QFont todoFont = option.font;
  todoFont.setBold(false);

  int textW = option.rect.width() - Layout::LeftPadding - Layout::RightPadding;
  QString todoText = parts[1].trimmed();

  // ✅ 【核心】使用 QTextDocument 计算真实排版高度
  // QTextDocument 与 drawText(TextWordWrap) 使用同一套排版引擎
  QTextDocument doc;
  doc.setDefaultFont(todoFont);
  doc.setTextWidth(qMax(1, textW));
  doc.setPlainText(todoText);
  // documentLayout()->documentSize() 返回的是精确的排版高度
  // 包含所有行的 ascent + descent + leading，与 drawText 像素级对齐
  qreal realTextHeight = doc.documentLayout()->documentSize().height();

  int totalHeight =
      Layout::TopMargin + timeFm.height() + Layout::GapBetweenLines +
      static_cast<int>(qCeil(realTextHeight)) + Layout::BottomMargin;

  return QSize(option.rect.width(), qMax(baseHeight, totalHeight));
}