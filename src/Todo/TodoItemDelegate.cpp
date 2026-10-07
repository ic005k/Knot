#include "TodoItemDelegate.h"

#include <QApplication>
#include <QMouseEvent>
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

// ========== 核心：统一按钮布局计算 ==========
QVector<ToolButtonInfo> TodoItemDelegate::getButtonLayout(
    const QRect& itemRect, const QFont& font) const {
  QVector<ToolButtonInfo> buttons;
  QFontMetrics fm(font);

  // {显示文本, 内部ID}
  QPair<QString, QString> leftBtnDefs[] = {
      {tr("High"), QStringLiteral("high")},
      {tr("Low"), QStringLiteral("low")},
      {tr("Edit"), QStringLiteral("edit")},
      {tr("Schedule"), QStringLiteral("schedule")}};
  QPair<QString, QString> rightBtnDef = {tr("Done"), QStringLiteral("done")};

  int btnH = fm.height() + kButtonPadding * 2;
  int barY = itemRect.bottom() - kToolBarHeight + (kToolBarHeight - btnH) / 2;

  int curX = itemRect.left() + 12;
  for (const auto& [txt, id] : leftBtnDefs) {
    int btnW = fm.horizontalAdvance(txt) + kButtonPadding * 2;
    buttons.append({txt, id, QRect(curX, barY, btnW, btnH), false});
    curX += btnW + kButtonSpacing;
  }

  int rightBtnW = fm.horizontalAdvance(rightBtnDef.first) + kButtonPadding * 2;
  int rightBtnX = itemRect.right() - 10 - rightBtnW;
  buttons.append({rightBtnDef.first, rightBtnDef.second,
                  QRect(rightBtnX, barY, rightBtnW, btnH), true});

  return buttons;
}

void TodoItemDelegate::paint(QPainter* painter,
                             const QStyleOptionViewItem& option,
                             const QModelIndex& index) const {
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);

  bool isExpanded = (m_expandedIndex == index);
  QRect rect = opt.rect;

  // ========== 关键改动：拆分绘制区域 ==========
  QRect contentRect = rect;
  QRect toolBarRect;

  if (isExpanded) {
    // 内容区高度 = 总高度 - 工具栏高度
    contentRect.setHeight(rect.height() - kToolBarHeight);
    // 工具栏区域位于底部
    toolBarRect = QRect(rect.left(), rect.bottom() - kToolBarHeight + 1,
                        rect.width(), kToolBarHeight);
  }

  // 1. 仅对【内容区域】绘制选中/悬停背景
  if (opt.state & QStyle::State_Selected) {
    painter->fillRect(contentRect, opt.palette.highlight());
  } else if (opt.state & QStyle::State_MouseOver) {
    painter->fillRect(contentRect, opt.palette.base().color().darker(105));
  } else {
    painter->fillRect(contentRect, opt.palette.base());
  }

  // 2. 【工具栏区域】始终使用基础背景色（不受选中状态影响）
  if (isExpanded) {
    painter->fillRect(toolBarRect, opt.palette.base());
  }

  // 3. 解析数据
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

  // 4. 布局参数 (⚠️ 删除了重复的 QRect rect，并改用 contentRect 作为基准)
  int leftPadding = 12, barWidth = 5, barGap = 10, rightPadding = 10;
  int topMargin = 8, gapBetweenLines = 4, bottomMargin = 8;

  int textX = contentRect.left() + leftPadding + barWidth + barGap;
  int textW =
      contentRect.width() - leftPadding - barWidth - barGap - rightPadding;

  // 5. 绘制左侧颜色条 (基于 contentRect)
  painter->setPen(Qt::NoPen);
  painter->setBrush(barColor);
  QRect barRect(contentRect.left() + leftPadding, contentRect.top() + topMargin,
                barWidth, contentRect.height() - topMargin - bottomMargin);
  painter->drawRoundedRect(barRect, 2.5, 2.5);

  // 6. 文本颜色
  bool isSelected = (opt.state & QStyle::State_Selected);
  QColor mainTextColor = isSelected ? opt.palette.highlightedText().color()
                                    : opt.palette.text().color();
  QColor subTextColor = isSelected
                            ? QColor(mainTextColor.red(), mainTextColor.green(),
                                     mainTextColor.blue(), 180)
                            : QColor(mainTextColor.red(), mainTextColor.green(),
                                     mainTextColor.blue(), 140);

  // --- 第一行：时间 ---
  QFont timeFont = opt.font;
  timeFont.setPointSize(qMax(8, timeFont.pointSize() - 3));
  QFontMetrics timeFm(timeFont);
  painter->setFont(timeFont);
  painter->setPen(subTextColor);

  int currentY = contentRect.top() + topMargin;
  QRect timeRect(textX, currentY, textW, timeFm.height());
  painter->drawText(timeRect,
                    Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine,
                    timeText);
  currentY += timeFm.height() + gapBetweenLines;

  // --- 第二行：Todo文本 ---
  QFont todoFont = opt.font;
  todoFont.setBold(true);
  QFontMetrics todoFm(todoFont);
  painter->setFont(todoFont);
  painter->setPen(mainTextColor);

  int remainingHeight = contentRect.bottom() - currentY - bottomMargin;
  QRect todoRect(textX, currentY, textW, qMax(0, remainingHeight));
  painter->drawText(todoRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                    todoText);

  // ========== 7. 绘制工具按钮（仅展开时）==========
  if (isExpanded) {
    // 分隔线
    painter->setPen(QColor(128, 128, 128, 60));
    painter->drawLine(toolBarRect.topLeft(),
                      QPoint(toolBarRect.right(), toolBarRect.top()));

    // 按钮 (注意：这里依然传 rect，因为 getButtonLayout 是基于整个 item
    // 矩形定位的)
    auto buttons = getButtonLayout(rect, opt.font);
    QFont btnFont = opt.font;
    btnFont.setPointSize(qMax(8, btnFont.pointSize() - 2));
    painter->setFont(btnFont);

    for (const auto& btn : buttons) {
      // 按钮背景（圆角矩形）
      QColor btnBg =
          QColor(0, 0, 0, 20);  // 工具栏背景不受选中色影响，使用固定浅色
      if (opt.state & QStyle::State_Selected) {
        btnBg = QColor(128, 128, 128, 30);  // 选中时稍微调整一下按钮背景
      }

      painter->setPen(Qt::NoPen);
      painter->setBrush(btnBg);
      painter->drawRoundedRect(btn.rect, 4, 4);

      // 按钮文字 (使用固定的深色或根据主题调整，避免被选中色影响)
      painter->setPen(opt.palette.text().color());
      painter->drawText(btn.rect, Qt::AlignCenter, btn.text);
    }
  }

  painter->restore();
}

QSize TodoItemDelegate::sizeHint(const QStyleOptionViewItem& option,
                                 const QModelIndex& index) const {
  QString rawData = index.data(Qt::DisplayRole).toString();
  QStringList parts = rawData.split("|==|");

  bool isExpanded = (m_expandedIndex == index);

  int baseHeight = qMax(64, option.fontMetrics.height() * 2 + 20);
  if (parts.size() < 3)
    return QSize(option.rect.width(),
                 baseHeight + (isExpanded ? kToolBarHeight : 0));

  QFont timeFont = option.font;
  timeFont.setPointSize(qMax(8, timeFont.pointSize() - 3));
  QFontMetrics timeFm(timeFont);

  QFont todoFont = option.font;
  todoFont.setBold(true);
  QFontMetrics todoFm(todoFont);

  int leftPadding = 12, barWidth = 5, barGap = 10, rightPadding = 10;
  int topMargin = 8, gapBetweenLines = 4, bottomMargin = 8;

  int textW =
      option.rect.width() - leftPadding - barWidth - barGap - rightPadding;
  QString todoText = parts[2].trimmed();
  QRect boundingRect = todoFm.boundingRect(
      QRect(0, 0, qMax(1, textW), 10000),
      Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, todoText);

  int contentHeight = topMargin + timeFm.height() + gapBetweenLines +
                      boundingRect.height() + bottomMargin;

  int totalHeight = qMax(baseHeight, contentHeight);

  // ⚠️ 展开时追加工具栏高度
  if (isExpanded) {
    totalHeight += kToolBarHeight;
  }

  return QSize(option.rect.width(), totalHeight);
}

// ========== 核心：鼠标事件处理 ==========
bool TodoItemDelegate::editorEvent(QEvent* event, QAbstractItemModel* model,
                                   const QStyleOptionViewItem& option,
                                   const QModelIndex& index) {
  if (event->type() == QEvent::MouseButtonRelease) {
    auto* mouseEvent = static_cast<QMouseEvent*>(event);
    QPoint clickPos = mouseEvent->pos();

    bool wasExpanded = (m_expandedIndex == index);

    if (wasExpanded) {
      // 检查是否点击了某个按钮
      auto buttons = getButtonLayout(option.rect, option.font);
      for (const auto& btn : buttons) {
        if (btn.rect.contains(clickPos)) {
          emit toolButtonClicked(index, btn.actionId);
          return true;  // 消费事件，不触发选择切换
        }
      }
    }

    // 点击条目本身 → 切换展开状态
    if (wasExpanded) {
      m_expandedIndex = QPersistentModelIndex();  // 收起
    } else {
      m_expandedIndex =
          QPersistentModelIndex(index);  // 展开新项（自动收起旧项）
    }

    // ⚠️ 关键：通知 View 重新计算布局并重绘
    const_cast<QAbstractItemModel*>(model)->dataChanged(index, index);

    // 提示：如果视图没有自动更新高度，请在外部连接 dataChanged 信号并调用
    // view->doItemsLayout()
    return true;
  }

  return QStyledItemDelegate::editorEvent(event, model, option, index);
}