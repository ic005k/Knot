#include "NoteSearchDelegate.h"

#include <QApplication>
#include <QFontMetrics>
#include <QPainter>
#include <QTextDocument>

// 选中状态的淡蓝背景色
static const QColor SELECTED_BG_COLOR(214, 232, 250);  // #D6E8FA
// 选中状态的统一黑色文字
static const QColor SELECTED_TEXT_COLOR(30, 30,
                                        30);  // 接近纯黑，比 #000 更柔和

NoteSearchDelegate::NoteSearchDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {}

void NoteSearchDelegate::paint(QPainter* painter,
                               const QStyleOptionViewItem& option,
                               const QModelIndex& index) const {
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  bool isSelected = opt.state & QStyle::State_Selected;

  // 【关键】选中时使用自定义淡蓝背景，否则走系统默认
  if (isSelected) {
    opt.palette.setColor(QPalette::Highlight, SELECTED_BG_COLOR);
    // 同时设置 HighlightedText 为黑色，供可能的子控件使用
    opt.palette.setColor(QPalette::HighlightedText, SELECTED_TEXT_COLOR);
  }

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
  // 选中/未选中都使用黑色系，未选中时用调色板保证主题适配
  QColor titleColor =
      isSelected ? SELECTED_TEXT_COLOR : opt.palette.color(QPalette::Text);
  painter->setPen(titleColor);

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

  // 选中时确保默认文字为黑色（高亮标签本身是黑色，无需额外处理）
  if (isSelected) {
    doc.setDefaultStyleSheet("body { color: #1E1E1E; }");
  }

  painter->translate(x, y);
  doc.drawContents(painter);
  painter->translate(-x, -y);

  y = opt.rect.y() + padding + fmTitle.height() + 6 + doc.size().height() + 6;

  painter->restore();
  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);

  // --- 3. 文件路径 ---
  QFont pathFont = opt.font;
  int smallerSize = pathFont.pointSize() > 2 ? pathFont.pointSize() - 2
                                             : pathFont.pixelSize() - 2;
  if (pathFont.pointSize() > 0)
    pathFont.setPointSize(smallerSize);
  else
    pathFont.setPixelSize(smallerSize);

  painter->setFont(pathFont);

  QColor pathColor;
  if (isSelected) {
    // 选中时：黑色但降低不透明度，形成层次
    pathColor = QColor(30, 30, 30, 180);
  } else {
    pathColor = opt.palette.color(QPalette::PlaceholderText);
    if (pathColor == QColor(Qt::black)) pathColor = QColor(128, 128, 128);
  }
  painter->setPen(pathColor);

  QFontMetrics fmPath(pathFont);
  QString elidedPath =
      fmPath.elidedText(filePath, Qt::ElideMiddle, availableWidth);
  painter->drawText(x, y + fmPath.ascent(), elidedPath);

  painter->restore();
}

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