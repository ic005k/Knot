#ifndef TODOITEMDELEGATE_H
#define TODOITEMDELEGATE_H

#include <QPainter>
#include <QPersistentModelIndex>
#include <QStyledItemDelegate>

struct ToolButtonInfo {
  QString text;      // 仅用于显示（tr() 包裹）
  QString actionId;  // 内部标识，永不翻译，如 "high", "low", "edit"
  QRect rect;
  bool isRightAligned;
};

class TodoItemDelegate : public QStyledItemDelegate {
  Q_OBJECT
 public:
  explicit TodoItemDelegate(QObject* parent = nullptr);

  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;

  QSize sizeHint(const QStyleOptionViewItem& option,
                 const QModelIndex& index) const override;

  // 核心：拦截鼠标事件以处理按钮点击
  bool editorEvent(QEvent* event, QAbstractItemModel* model,
                   const QStyleOptionViewItem& option,
                   const QModelIndex& index) override;

 signals:
  // 当工具按钮被点击时发出信号，外部连接处理业务逻辑
  void toolButtonClicked(const QModelIndex& index, const QString& actionId);

 private:
  QColor getColorByIndex(int index) const;

  // 统一计算按钮布局，确保绘制和点击判定完全一致
  QVector<ToolButtonInfo> getButtonLayout(const QRect& itemRect,
                                          const QFont& font) const;

  // 按钮区域额外高度
  static constexpr int kToolBarHeight = 36;
  static constexpr int kButtonPadding = 6;
  static constexpr int kButtonSpacing = 4;

  // 记录当前展开的条目（使用 QPersistentModelIndex 防止模型重置后失效）
  QPersistentModelIndex m_expandedIndex;
};

#endif  // TODOITEMDELEGATE_H