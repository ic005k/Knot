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

// 将按钮纳入选中状态
/*void TodoItemDelegate::paint(QPainter* painter,
                             const QStyleOptionViewItem& option,
                             const QModelIndex& index) const {
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);

  bool isExpanded = (m_expandedIndex == index);
  QRect rect = opt.rect;

  // ========== 核心改动：统一背景绘制 ==========
  // 不再拆分 contentRect 和 toolBarRect 的背景
  // 整个条目（无论是否展开）使用同一个选中/悬停背景
  if (opt.state & QStyle::State_Selected) {
    painter->fillRect(rect, opt.palette.highlight());
  } else if (opt.state & QStyle::State_MouseOver) {
    painter->fillRect(rect, opt.palette.base().color().darker(105));
  } else {
    painter->fillRect(rect, opt.palette.base());
  }

  // ========== 内容区域计算（仅用于文本和颜色条布局）==========
  QRect contentRect = rect;
  QRect toolBarRect;

  if (isExpanded) {
    contentRect.setHeight(rect.height() - kToolBarHeight);
    toolBarRect = QRect(rect.left(), rect.bottom() - kToolBarHeight + 1,
                        rect.width(), kToolBarHeight);
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

  // 4. 布局参数
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

  // 6. 文本颜色（保持原有逻辑不变）
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
    // 分隔线：在选中状态下降低透明度，避免与高亮背景冲突
    QColor lineColor =
        isSelected ? QColor(255, 255, 255, 40) : QColor(128, 128, 128, 60);
    painter->setPen(lineColor);
    painter->drawLine(toolBarRect.topLeft(),
                      QPoint(toolBarRect.right(), toolBarRect.top()));

    auto buttons = getButtonLayout(rect, opt.font);
    QFont btnFont = opt.font;
    btnFont.setPointSize(qMax(8, btnFont.pointSize() - 2));
    painter->setFont(btnFont);

    for (const auto& btn : buttons) {
      // ✅ 按钮背景：根据选中状态自适应，保持与整体背景的层次感
      QColor btnBg;
      if (isSelected) {
        // 选中时：使用半透明白色，在高亮背景上形成微妙层次
        btnBg = QColor(255, 255, 255, 35);
      } else {
        // 未选中时：保持原有的浅色
        btnBg = QColor(0, 0, 0, 20);
      }

      painter->setPen(Qt::NoPen);
      painter->setBrush(btnBg);
      painter->drawRoundedRect(btn.rect, 4, 4);

      // ✅ 按钮文字：选中时使用高亮文字色，保持可读性
      QColor btnTextColor = isSelected ? opt.palette.highlightedText().color()
                                       : opt.palette.text().color();
      painter->setPen(btnTextColor);
      painter->drawText(btn.rect, Qt::AlignCenter, btn.text);
    }
  }

  painter->restore();
}*/

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
  // ✅ 核心修复：拦截鼠标按下事件，阻止 QListWidget 默认的提前选中行为
  if (event->type() == QEvent::MouseButtonPress) {
    auto* mouseEvent = static_cast<QMouseEvent*>(event);
    // 仅左键点击条目内容区时拦截（右键/中键不影响）
    if (mouseEvent->button() == Qt::LeftButton) {
      // 返回 true 消费掉 Press 事件，QListWidget 就不会在按下时设置选中态
      // 注意：不要在这里做任何视觉更新，保持当前画面不变
      return true;
    }
  }

  if (event->type() == QEvent::MouseButtonRelease) {
    auto* mouseEvent = static_cast<QMouseEvent*>(event);
    // 只处理左键释放
    if (mouseEvent->button() != Qt::LeftButton) {
      return QStyledItemDelegate::editorEvent(event, model, option, index);
    }

    QPoint clickPos = mouseEvent->pos();
    bool wasExpanded = (m_expandedIndex == index);

    // 如果已展开，优先检查是否点击了按钮
    if (wasExpanded) {
      auto buttons = getButtonLayout(option.rect, option.font);
      for (const auto& btn : buttons) {
        if (btn.rect.contains(clickPos)) {
          emit toolButtonClicked(index, btn.actionId);
          return true;  // 按钮点击不改变选中/展开状态
        }
      }
    }

    // ✅ 关键：在同一帧内同时完成【选中】+【展开切换】+【重绘】
    // 1. 手动设置选中状态（替代被我们拦截掉的默认行为）
    // auto* selectionModel = const_cast<QAbstractItemModel*>(model)
    //                           ->parent() /* 获取 view 的方式可能因架构而异
    //                           */;
    // ⚠️ 更稳妥的做法：通过外部连接或直接操作 view
    // 这里推荐在 Delegate 中保存 view 指针，或通过信号让外部处理

    // 2. 切换展开状态
    if (wasExpanded) {
      m_expandedIndex = QPersistentModelIndex();
    } else {
      m_expandedIndex = QPersistentModelIndex(index);
    }

    // 3. 通知重绘（此时选中态和展开态已同步更新）
    const_cast<QAbstractItemModel*>(model)->dataChanged(index, index);
    emit expandedIndexChanged(wasExpanded ? QModelIndex() : index);

    return true;
  }

  return QStyledItemDelegate::editorEvent(event, model, option, index);
}