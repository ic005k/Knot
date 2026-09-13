#include "RecentNoteDelegate.h"

#include <QApplication>
#include <QPainter>

namespace Layout {
constexpr int LeftPadding = 12;
constexpr int RightPadding = 10;
constexpr int TopMargin = 8;
constexpr int BottomMargin = 8;
constexpr int GapBetweenLines = 4;
}  // namespace Layout

RecentNoteDelegate::RecentNoteDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {}

void RecentNoteDelegate::paint(QPainter* painter,
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

  // 2. 解析数据 (格式: "笔记名称|==|笔记路径")
  QString rawData = index.data(Qt::DisplayRole).toString();
  QStringList parts = rawData.split("===", Qt::KeepEmptyParts);
  if (parts.size() < 2) {
    painter->restore();
    return;
  }

  QString nameText = parts[0].trimmed();
  QString pathText = parts[1].trimmed();

  // 3. 统一布局参数
  QRect rect = opt.rect;
  int textX = rect.left() + Layout::LeftPadding;
  int textW = rect.width() - Layout::LeftPadding - Layout::RightPadding;

  // 4. 动态计算颜色
  bool isSelected = (opt.state & QStyle::State_Selected);
  QColor mainTextColor =
      isSelected ? QColor("#FFFFFF") : opt.palette.text().color();
  QColor subTextColor = isSelected
                            ? QColor(255, 255, 255, 160)
                            : opt.palette.color(QPalette::PlaceholderText);

  // ========== 绘制第一行：笔记名称（正常字体，末尾省略） ==========
  QFont nameFont = opt.font;
  nameFont.setBold(false);
  QFontMetrics nameFm(nameFont);

  painter->setFont(nameFont);
  painter->setPen(mainTextColor);

  int currentY = rect.top() + Layout::TopMargin;
  // 使用 QFontMetrics::elidedText 处理过长文本，末尾加 "..."
  QString elidedName = nameFm.elidedText(nameText, Qt::ElideRight, textW);

  QRect nameRect(textX, currentY, textW, nameFm.height());
  painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, elidedName);

  // Y 坐标下移
  currentY += nameFm.height() + Layout::GapBetweenLines;

  // ========== 绘制第二行：笔记路径（小字体，较淡字色，中间省略） ==========
  QFont pathFont = opt.font;
  // 小字体：点数减小 2，最小为 8
  pathFont.setPointSize(qMax(8, pathFont.pointSize() - 2));
  pathFont.setBold(false);
  QFontMetrics pathFm(pathFont);

  painter->setFont(pathFont);
  painter->setPen(subTextColor);

  // 使用 Qt::ElideMiddle 实现“两头显示，中间省略” (例如: C:/Users/.../note.md)
  QString elidedPath = pathFm.elidedText(pathText, Qt::ElideMiddle, textW);

  QRect pathRect(textX, currentY, textW, pathFm.height());
  painter->drawText(pathRect, Qt::AlignLeft | Qt::AlignVCenter, elidedPath);

  painter->restore();
}

QSize RecentNoteDelegate::sizeHint(const QStyleOptionViewItem& option,
                                   const QModelIndex& index) const {
  // 字体参数与 paint 完全一致
  QFont nameFont = option.font;
  nameFont.setBold(false);
  QFontMetrics nameFm(nameFont);

  QFont pathFont = option.font;
  pathFont.setPointSize(qMax(8, pathFont.pointSize() - 2));
  pathFont.setBold(false);
  QFontMetrics pathFm(pathFont);

  // 固定高度计算：上边距 + 名称高度 + 间距 + 路径高度 + 下边距
  int totalHeight = Layout::TopMargin + nameFm.height() +
                    Layout::GapBetweenLines + pathFm.height() +
                    Layout::BottomMargin;

  return QSize(option.rect.width(), totalHeight);
}