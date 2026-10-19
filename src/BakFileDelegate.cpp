#include "BakFileDelegate.h"

#include <QApplication>
#include <QPainter>

BakFileDelegate::BakFileDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {}

// ⭐ 重写 sizeHint 控制条目高度和间隙
QSize BakFileDelegate::sizeHint(const QStyleOptionViewItem& option,
                                const QModelIndex& index) const {
  // 获取基类计算的默认尺寸（已包含文本高度、图标高度等）
  QSize baseSize = QStyledItemDelegate::sizeHint(option, index);

  // 在默认高度基础上增加额外间距（例如上下各加 6px，总共 +12px）
  // 可根据实际视觉效果调整这个值
  int extraPadding = 12;
  baseSize.setHeight(baseSize.height() + extraPadding);

  return baseSize;
}

void BakFileDelegate::paint(QPainter* painter,
                            const QStyleOptionViewItem& option,
                            const QModelIndex& index) const {
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  QString originalText = opt.text;
  opt.text.clear();  // 清空文本防止任何底层绘制

  // ✅ 仅绘制背景和高亮框，完全跳过 CE_ItemViewItem 中的文本子元素
  painter->save();
  opt.widget->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt,
                                     painter, opt.widget);
  painter->restore();

  // 手动绘制中间省略文本
  if (!originalText.isEmpty()) {
    painter->save();
    painter->setFont(opt.font);

    // 颜色处理
    QColor textColor = opt.palette.color((opt.state & QStyle::State_Selected)
                                             ? QPalette::HighlightedText
                                             : QPalette::Text);
    if (!(opt.state & QStyle::State_Enabled))
      textColor = opt.palette.color(QPalette::Disabled, QPalette::Text);
    painter->setPen(textColor);

    // ✅ 增加垂直居中补偿（配合 sizeHint 增加的 extraPadding）
    QRect textRect = opt.widget->style()->subElementRect(
        QStyle::SE_ItemViewItemText, &opt, opt.widget);

    // 如果 subElementRect 因为某些原因失效，手动兜底计算
    if (textRect.isNull() || !textRect.isValid()) {
      int margin = opt.widget->style()->pixelMetric(
                       QStyle::PM_FocusFrameHMargin, &opt, opt.widget) +
                   1;
      textRect = opt.rect.adjusted(margin, 0, -margin, 0);
    }

    QString elidedText = opt.fontMetrics.elidedText(
        originalText, Qt::ElideMiddle, textRect.width());

    painter->drawText(textRect, opt.displayAlignment | Qt::AlignVCenter,
                      elidedText);
    painter->restore();
  }
}