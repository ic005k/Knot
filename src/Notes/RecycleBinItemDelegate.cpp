// RecycleBinItemDelegate.cpp
#include "RecycleBinItemDelegate.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>

namespace Layout {
constexpr int LeftPadding = 12;
constexpr int RightPadding = 12;
constexpr int TopMargin = 10;
constexpr int BottomMargin = 10;
constexpr int CheckBoxSize = 18;
constexpr int CheckBoxGap = 12;
constexpr int LineGap = 4;
}  // namespace Layout

RecycleBinItemDelegate::RecycleBinItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {}

void RecycleBinItemDelegate::paint(QPainter* painter,
                                   const QStyleOptionViewItem& option,
                                   const QModelIndex& index) const {
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  // 1. 获取状态
  bool isMouseOver = opt.state & QStyle::State_MouseOver;
  bool isChecked = index.data(Qt::UserRole).toBool();

  // 清除系统默认状态，防止默认边框、焦点框和刺眼的高亮色干扰自定义绘制
  opt.state &= ~(QStyle::State_Selected | QStyle::State_HasFocus |
                 QStyle::State_MouseOver);

  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);

  // 2. 绘制背景
  // 【关键】背景高亮 ONLY 取决于复选框是否打勾，彻底剥离系统选中状态残留
  if (isChecked) {
    QColor highlightColor = opt.palette.highlight().color();
    highlightColor.setAlpha(45);  // 稍淡的高亮效果
    painter->fillRect(opt.rect, highlightColor);
  } else if (isMouseOver) {
    painter->fillRect(opt.rect, opt.palette.base().color().darker(103));
  } else {
    painter->fillRect(opt.rect, opt.palette.base());
  }

  // 3. 绘制复选框
  QRect rect = opt.rect;
  int checkBoxX = rect.left() + Layout::LeftPadding;
  int checkBoxY = rect.top() + (rect.height() - Layout::CheckBoxSize) / 2;
  QRect checkBoxRect(checkBoxX, checkBoxY, Layout::CheckBoxSize,
                     Layout::CheckBoxSize);

  QStyleOptionButton checkOpt;
  checkOpt.rect = checkBoxRect;
  checkOpt.state = QStyle::State_Enabled;
  if (isChecked) checkOpt.state |= QStyle::State_On;

  // 使用系统原生样式绘制复选框，保证跨平台一致性
  QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkOpt, painter);

  // 4. 计算文本区域
  int textX = checkBoxX + Layout::CheckBoxSize + Layout::CheckBoxGap;
  int textW = rect.width() - Layout::LeftPadding - Layout::CheckBoxSize -
              Layout::CheckBoxGap - Layout::RightPadding;

  // 解析数据 (名称===路径)
  QString rawData = index.data(Qt::DisplayRole).toString();
  QStringList parts = rawData.split("===");
  QString title = parts.size() > 0 ? parts[0].trimmed() : "";
  QString path = parts.size() > 1 ? parts[1].trimmed() : "";

  // 5. 绘制第一行：笔记名称（正常字体）
  QFont titleFont = opt.font;
  titleFont.setBold(false);
  QFontMetrics titleFm(titleFont);

  int titleY = rect.top() + Layout::TopMargin;
  QRect titleRect(textX, titleY, textW, titleFm.height());

  painter->setFont(titleFont);
  painter->setPen(opt.palette.text().color());
  painter->drawText(
      titleRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, title);

  // 6. 绘制第二行：文件路径（小号字体，字色稍淡）
  QFont pathFont = opt.font;
  int pathFontSize = qMax(8, pathFont.pointSize() - 2);
  pathFont.setPointSize(pathFontSize);
  QFontMetrics pathFm(pathFont);

  int pathY = titleY + titleFm.height() + Layout::LineGap;
  QRect pathRect(textX, pathY, textW, pathFm.height());

  painter->setFont(pathFont);
  // 字色稍淡：使用 text 颜色并降低透明度
  QColor pathColor = opt.palette.text().color();
  pathColor.setAlpha(140);
  painter->setPen(pathColor);
  painter->drawText(
      pathRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, path);

  painter->restore();
}

QSize RecycleBinItemDelegate::sizeHint(const QStyleOptionViewItem& option,
                                       const QModelIndex& index) const {
  QFont titleFont = option.font;
  QFontMetrics titleFm(titleFont);

  QFont pathFont = option.font;
  pathFont.setPointSize(qMax(8, pathFont.pointSize() - 2));
  QFontMetrics pathFm(pathFont);

  int height = Layout::TopMargin + titleFm.height() + Layout::LineGap +
               pathFm.height() + Layout::BottomMargin;

  // 保证最小高度，避免内容被裁剪
  return QSize(option.rect.width(), qMax(56, height));
}

bool RecycleBinItemDelegate::editorEvent(QEvent* event,
                                         QAbstractItemModel* model,
                                         const QStyleOptionViewItem& option,
                                         const QModelIndex& index) {
  // 处理单击事件，切换复选框状态
  if (event->type() == QEvent::MouseButtonRelease) {
    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
    if (mouseEvent->button() == Qt::LeftButton &&
        option.rect.contains(mouseEvent->pos())) {
      bool currentChecked = index.data(Qt::UserRole).toBool();
      model->setData(index, !currentChecked, Qt::UserRole);

      // 【关键】手动发射 dataChanged 信号，强制视图重绘该行
      // QListWidget 对 UserRole 的 setData 不会自动触发重绘，必须手动通知
      emit model->dataChanged(index, index, {Qt::UserRole});

      return true;  // 消费事件
    }
  }
  return QStyledItemDelegate::editorEvent(event, model, option, index);
}