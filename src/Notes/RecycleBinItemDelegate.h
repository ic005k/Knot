// RecycleBinItemDelegate.h
#ifndef RECYCLEBINITEMDELEGATE_H
#define RECYCLEBINITEMDELEGATE_H

#include <QStyledItemDelegate>

class RecycleBinItemDelegate : public QStyledItemDelegate {
  Q_OBJECT
 public:
  explicit RecycleBinItemDelegate(QObject* parent = nullptr);

  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;
  QSize sizeHint(const QStyleOptionViewItem& option,
                 const QModelIndex& index) const override;
  bool editorEvent(QEvent* event, QAbstractItemModel* model,
                   const QStyleOptionViewItem& option,
                   const QModelIndex& index) override;
};

#endif  // RECYCLEBINITEMDELEGATE_H