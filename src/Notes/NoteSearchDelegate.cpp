#include "NoteSearchDelegate.h"

#include <QApplication>
#include <QFontMetrics>
#include <QPainter>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>

NoteSearchDelegate::NoteSearchDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {}

QSize NoteSearchDelegate::sizeHint(const QStyleOptionViewItem& option,
                                   const QModelIndex& index) const {
  // sizeHint 无需修改，与之前完全一致
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  QString rawData = index.data(Qt::DisplayRole).toString();
  QStringList parts = rawData.split("===");
  if (parts.size() < 3) return QSize(0, 0);

  int padding = 10;
  int availableWidth =
      opt.rect.width() > 0 ? opt.rect.width() - padding * 2 : 400;

  QFont titleFont = opt.font;
  titleFont.setBold(true);
  QFontMetrics fmTitle(titleFont);
  int height = padding + fmTitle.height() + 6;

  QTextDocument doc;
  doc.setHtml(parts[1]);
  doc.setDefaultFont(opt.font);
  doc.setTextWidth(availableWidth);
  height += doc.size().height() + 6;

  QFont pathFont = opt.font;
  int smallerSize = pathFont.pointSize() > 2 ? pathFont.pointSize() - 2 : 10;
  if (pathFont.pointSize() > 0) pathFont.setPointSize(smallerSize);
  QFontMetrics fmPath(pathFont);
  height += fmPath.height() + padding;

  return QSize(option.rect.width(), height);
}

void NoteSearchDelegate::paint(QPainter* painter,
                               const QStyleOptionViewItem& option,
                               const QModelIndex& index) const {
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  bool isSelected = opt.state & QStyle::State_Selected;

  const QWidget* widget = option.widget;
  QStyle* style = widget ? widget->style() : QApplication::style();
  style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, widget);

  QString rawData = index.data(Qt::DisplayRole).toString();
  QStringList parts = rawData.split("===");
  if (parts.size() < 3) return;

  QString title = parts[0];
  QString summaryHtml = parts[1];
  QString filePath = parts[2];

  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);

  int padding = 10;
  int x = opt.rect.x() + padding;
  int y = opt.rect.y() + padding;
  int availableWidth = opt.rect.width() - padding * 2;

  // --- 1. 笔记标题 ---
  QFont titleFont = opt.font;
  titleFont.setBold(true);
  painter->setFont(titleFont);
  // 完全交由调色板，Qt 自动区分选中/未选中、明亮/暗黑
  painter->setPen(opt.palette.color(isSelected ? QPalette::HighlightedText
                                               : QPalette::Text));

  QFontMetrics fmTitle(titleFont);
  QString elidedTitle =
      fmTitle.elidedText(title, Qt::ElideRight, availableWidth);
  painter->drawText(x, y + fmTitle.ascent(), elidedTitle);
  y += fmTitle.height() + 6;

  // --- 2. 笔记摘要 (HTML) ---
  QTextDocument doc;
  doc.setHtml(summaryHtml);
  doc.setDefaultFont(opt.font);
  doc.setTextWidth(availableWidth);

  // 【零干预】不设 CSS、不改 Format、不动调色板
  // QTextDocument 完美继承上下文颜色，搜索高亮标签原样保留

  painter->translate(x, y);
  doc.drawContents(painter);
  painter->translate(-x, -y);

  y = opt.rect.y() + padding + fmTitle.height() + 6 + doc.size().height() + 6;

  painter->restore();
  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);

  // --- 3. 文件路径（唯一需要特殊处理的部分）---
  QFont pathFont = opt.font;
  int smallerSize = pathFont.pointSize() > 2 ? pathFont.pointSize() - 2
                                             : pathFont.pixelSize() - 2;
  if (pathFont.pointSize() > 0)
    pathFont.setPointSize(smallerSize);
  else
    pathFont.setPixelSize(smallerSize);

  painter->setFont(pathFont);

  // 基础颜色交给调色板，仅做透明度分层
  QColor pathColor = opt.palette.color(isSelected ? QPalette::HighlightedText
                                                  : QPalette::PlaceholderText);
  pathColor.setAlpha(isSelected ? 180 : 255);
  painter->setPen(pathColor);

  QFontMetrics fmPath(pathFont);
  QString elidedPath =
      fmPath.elidedText(filePath, Qt::ElideMiddle, availableWidth);
  painter->drawText(x, y + fmPath.ascent(), elidedPath);

  painter->restore();
}