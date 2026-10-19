// BakFileDelegate.h
#ifndef BAKFILEDELEGATE_H
#define BAKFILEDELEGATE_H

#include <QStyledItemDelegate>

class BakFileDelegate : public QStyledItemDelegate {
  Q_OBJECT
 public:
  explicit BakFileDelegate(QObject* parent = nullptr);

 protected:
  // 核心：重写绘制函数，实现中间省略
  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;
  QSize sizeHint(const QStyleOptionViewItem& option,
                 const QModelIndex& index) const override;
};

#endif  // BAKFILEDELEGATE_H