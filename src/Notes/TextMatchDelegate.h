// TextMatchDelegate.h
#ifndef TEXTMATCHDELEGATE_H
#define TEXTMATCHDELEGATE_H

#include <QAbstractTextDocumentLayout>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QTextDocument>

class TextMatchDelegate : public QStyledItemDelegate {
 public:
  explicit TextMatchDelegate(QObject* parent = nullptr)
      : QStyledItemDelegate(parent) {}

  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    painter->save();

    // ✅ 1. 先让基类绘制完整的背景（含选中态、悬停态、交替行色等）
    //    这样能自动获得系统/主题正确的选中颜色
    opt.text = QString();  // 清空文本，避免基类再画一遍纯文本
    opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter,
                                     opt.widget);

    // ✅ 2. 获取显示用 HTML
    QString html = index.data(Qt::UserRole + 1).toString();
    if (html.isEmpty()) {
      painter->restore();
      return;
    }

    // ✅ 3. 构建 QTextDocument 并消除多余间距
    QTextDocument doc;
    doc.setHtml(html);
    doc.setDefaultFont(opt.font);
    doc.setDocumentMargin(0);  // 去掉文档自身边距
    doc.setTextWidth(option.rect.width() - 8);

    // 去掉所有段落的额外间距
    QTextCursor cursor(&doc);
    cursor.select(QTextCursor::Document);
    QTextBlockFormat blockFmt;
    blockFmt.setTopMargin(0);
    blockFmt.setBottomMargin(0);
    blockFmt.setLineHeight(0, QTextBlockFormat::SingleHeight);
    cursor.mergeBlockFormat(blockFmt);

    // ✅ 4. 设置绘制上下文，确保选中态文字颜色正确
    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.palette = opt.palette;
    if (opt.state & QStyle::State_Selected) {
      ctx.palette.setColor(QPalette::Text,
                           opt.palette.color(QPalette::HighlightedText));
    }

    // ✅ 5. 垂直居中绘制，补偿合理偏移
    qreal textHeight = doc.size().height();
    qreal yOffset = (opt.rect.height() - textHeight) / 2.0;
    if (yOffset < 0) yOffset = 0;

    painter->translate(opt.rect.left() + 4, opt.rect.top() + qRound(yOffset));
    doc.documentLayout()->draw(painter, ctx);

    painter->restore();
  }

  QSize sizeHint(const QStyleOptionViewItem& option,
                 const QModelIndex& index) const override {
    QString html = index.data(Qt::UserRole + 1).toString();
    if (html.isEmpty()) {
      return QStyledItemDelegate::sizeHint(option, index);
    }

    QTextDocument doc;
    doc.setHtml(html);
    doc.setDefaultFont(option.font);
    doc.setDocumentMargin(0);

    QTextCursor cursor(&doc);
    cursor.select(QTextCursor::Document);
    QTextBlockFormat blockFmt;
    blockFmt.setTopMargin(0);
    blockFmt.setBottomMargin(0);
    blockFmt.setLineHeight(0, QTextBlockFormat::SingleHeight);
    cursor.mergeBlockFormat(blockFmt);

    int w = qCeil(doc.idealWidth()) + 8;
    int h = qCeil(doc.size().height()) + 4;  // 仅保留极小缓冲

    // 不低于默认行高，防止截断
    int defaultH = QStyledItemDelegate::sizeHint(option, index).height();
    if (h < defaultH) h = defaultH;

    return QSize(w, h);
  }
};

#endif  // TEXTMATCHDELEGATE_H