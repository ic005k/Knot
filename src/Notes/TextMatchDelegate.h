#ifndef TEXTMATCHDELEGATE_H
#define TEXTMATCHDELEGATE_H

#include <QAbstractTextDocumentLayout>
#include <QCache>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QTextBlock>
#include <QTextDocument>

class TextMatchDelegate : public QStyledItemDelegate {
 public:
  explicit TextMatchDelegate(QObject* parent = nullptr)
      : QStyledItemDelegate(parent), m_normalCache(200), m_selectedCache(200) {}

  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    painter->save();

    opt.text = QString();
    opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter,
                                     opt.widget);

    QString html = index.data(Qt::UserRole + 1).toString();
    if (html.isEmpty()) {
      painter->restore();
      return;
    }

    bool selected = (opt.state & QStyle::State_Selected);
    QTextDocument* doc = getDoc(html, opt.font, selected);

    doc->setTextWidth(option.rect.width() - 8);

    qreal textHeight = doc->size().height();
    qreal yOffset = (opt.rect.height() - textHeight) / 2.0;
    if (yOffset < 0) yOffset = 0;

    painter->translate(opt.rect.left() + 4, opt.rect.top() + qRound(yOffset));

    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.palette = opt.palette;
    doc->documentLayout()->draw(painter, ctx);

    painter->restore();
  }

  QSize sizeHint(const QStyleOptionViewItem& option,
                 const QModelIndex& index) const override {
    QString html = index.data(Qt::UserRole + 1).toString();
    if (html.isEmpty()) return QStyledItemDelegate::sizeHint(option, index);

    // sizeHint 与选中态无关，用 normal 版本
    QTextDocument* doc = getDoc(html, option.font, false);
    doc->setTextWidth(option.rect.width() - 8);

    int w = qCeil(doc->idealWidth()) + 8;
    int h = qCeil(doc->size().height()) + 4;
    int defaultH = QStyledItemDelegate::sizeHint(option, index).height();
    return QSize(w, qMax(h, defaultH));
  }

 private:
  mutable QCache<QString, QTextDocument> m_normalCache;
  mutable QCache<QString, QTextDocument> m_selectedCache;

  QTextDocument* getDoc(const QString& html, const QFont& font,
                        bool selected) const {
    auto& cache = selected ? m_selectedCache : m_normalCache;

    if (auto* cached = cache.object(html)) {
      cached->setDefaultFont(font);
      return cached;
    }

    auto* doc = new QTextDocument();
    doc->setDefaultFont(font);
    doc->setDocumentMargin(0);

    if (selected) {
      // 选中版 HTML：所有文字用白色，关键词保持黄底黑字
      QColor hl = QColor(0xFF, 0xFF, 0xFF);  // 或从 palette 取
      QString css = QStringLiteral(
                        "<style>"
                        "body{margin:0;padding:0}"
                        ".ln,.ctx{color:%1}"
                        ".hl{background:#FFEB3B;color:#000;font-weight:bold;"
                        "padding:1px 2px;border-radius:2px}"
                        "</style>")
                        .arg(hl.name());
      doc->setHtml(css + html);
    } else {
      // 普通版 HTML
      QString css = QStringLiteral(
          "<style>"
          "body{margin:0;padding:0}"
          ".ln{color:#888}"
          ".ctx{color:#aaa}"
          ".hl{background:#FFEB3B;color:#000;font-weight:bold;"
          "padding:1px 2px;border-radius:2px}"
          "</style>");
      doc->setHtml(css + html);
    }

    // 消除段落间距
    QTextCursor cursor(doc);
    cursor.select(QTextCursor::Document);
    QTextBlockFormat bf;
    bf.setTopMargin(0);
    bf.setBottomMargin(0);
    bf.setLineHeight(0, QTextBlockFormat::SingleHeight);
    cursor.mergeBlockFormat(bf);

    cache.insert(html, doc);
    return cache.object(html);
  }
};

#endif  // TEXTMATCHDELEGATE_H