// TodoItemDelegate.h
#ifndef TODOITEMDELEGATE_H
#define TODOITEMDELEGATE_H

#include <QPainter>
#include <QStyledItemDelegate>

class TodoItemDelegate : public QStyledItemDelegate {
  Q_OBJECT
 public:
  explicit TodoItemDelegate(QObject* parent = nullptr);

  // 核心：自定义绘制逻辑
  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;

  // 核心：定义每个Item的尺寸
  QSize sizeHint(const QStyleOptionViewItem& option,
                 const QModelIndex& index) const override;

 private:
  QColor getColorByIndex(int index) const;
};

#endif  // TODOITEMDELEGATE_H