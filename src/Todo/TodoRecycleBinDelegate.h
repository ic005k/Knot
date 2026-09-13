#ifndef TODORECYCLEBINDELEGATE_H
#define TODORECYCLEBINDELEGATE_H

#include <QStyledItemDelegate>

class TodoRecycleBinDelegate : public QStyledItemDelegate {
  Q_OBJECT
 public:
  explicit TodoRecycleBinDelegate(QObject* parent = nullptr);

  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;

  QSize sizeHint(const QStyleOptionViewItem& option,
                 const QModelIndex& index) const override;
};

#endif  // TODORECYCLEBINDELEGATE_H