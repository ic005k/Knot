#include "NoteListDelegate.h"

#include <QAbstractTextDocumentLayout>
#include <QPainter>
#include <QTextDocument>
#include <QtMath>

namespace Layout {
constexpr int LeftPadding = 12;
constexpr int RightPadding = 10;
constexpr int TopMargin = 10;
constexpr int BottomMargin = 10;
}  // namespace Layout

NoteListDelegate::NoteListDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {}

void NoteListDelegate::paint(QPainter* painter,
                             const QStyleOptionViewItem& option,
                             const QModelIndex& index) const {
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  // 【核心】保存原始交互状态后，立即清除所有可能触发系统边框的标志
  bool isSelected = opt.state & QStyle::State_Selected;
  bool isMouseOver = opt.state & QStyle::State_MouseOver;
  opt.state &= ~(QStyle::State_Selected | QStyle::State_HasFocus |
                 QStyle::State_MouseOver);

  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);

  // 1. 完全手动绘制背景（无任何边框），对齐 TodoItemDelegate 的三态逻辑
  if (isSelected) {
    painter->fillRect(opt.rect, opt.palette.highlight());
  } else if (isMouseOver) {
    painter->fillRect(opt.rect, opt.palette.base().color().darker(105));
  } else {
    painter->fillRect(opt.rect, opt.palette.base());
  }

  // 2. 获取纯文本数据
  QString text = index.data(Qt::DisplayRole).toString().trimmed();
  if (text.isEmpty()) {
    painter->restore();
    return;
  }

  // 3. 布局参数（与 sizeHint 严格一致）
  QRect rect = opt.rect;
  int textX = rect.left() + Layout::LeftPadding;
  int textW = rect.width() - Layout::LeftPadding - Layout::RightPadding;
  int textY = rect.top() + Layout::TopMargin;
  int textH = rect.height() - Layout::TopMargin - Layout::BottomMargin;

  // 4. 根据原始选中状态动态计算颜色
  QColor textColor =
      isSelected ? QColor("#FFFFFF") : opt.palette.text().color();

  // 5. 绘制自动换行文本
  QFont font = opt.font;
  font.setBold(false);

  painter->setFont(font);
  painter->setPen(textColor);

  QRect textRect(textX, textY, textW, textH);
  painter->drawText(textRect,
                    Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap, text);

  painter->restore();
}

QSize NoteListDelegate::sizeHint(const QStyleOptionViewItem& option,
                                 const QModelIndex& index) const {
  QString text = index.data(Qt::DisplayRole).toString().trimmed();

  // 最小高度保底
  int baseHeight = qMax(40, option.fontMetrics.height() + Layout::TopMargin +
                                Layout::BottomMargin);

  if (text.isEmpty()) {
    return QSize(option.rect.width(), baseHeight);
  }

  // 字体参数与 paint 完全一致
  QFont font = option.font;
  font.setBold(false);

  int textW = option.rect.width() - Layout::LeftPadding - Layout::RightPadding;

  // ✅ 使用 QTextDocument 精确计算自动换行后的真实高度
  QTextDocument doc;
  doc.setDefaultFont(font);
  doc.setTextWidth(qMax(1, textW));
  doc.setPlainText(text);

  qreal realTextHeight = doc.documentLayout()->documentSize().height();

  int totalHeight = Layout::TopMargin +
                    static_cast<int>(qCeil(realTextHeight)) +
                    Layout::BottomMargin;

  return QSize(option.rect.width(), qMax(baseHeight, totalHeight));
}