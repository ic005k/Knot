#pragma once
#include <QModelIndex>
#include <QStyledItemDelegate>

class MyNoteDiffDelegate : public QStyledItemDelegate {
  Q_OBJECT
 public:
  explicit MyNoteDiffDelegate(QObject* parent = nullptr);

  // 1. 绘制列表项（文本 + 按钮）
  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;

  // 2. 设定列表项高度
  QSize sizeHint(const QStyleOptionViewItem& option,
                 const QModelIndex& index) const override;

  // 3. 拦截鼠标事件，处理按钮点击
  bool editorEvent(QEvent* event, QAbstractItemModel* model,
                   const QStyleOptionViewItem& option,
                   const QModelIndex& index) override;

 signals:
  // 当右侧“旧文本”按钮被点击时，发出此信号，并携带行号
  void oldTextButtonClicked(int row);

 private:
  QString m_oldTextBtnLabel;  // ✅ 存储翻译后的按钮文本
  // 辅助函数：计算按钮在列表项中的矩形区域
  QRect getButtonRect(const QStyleOptionViewItem& option) const;
};