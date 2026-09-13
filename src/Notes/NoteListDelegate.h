#pragma once

#include <QStyledItemDelegate>

class NoteListDelegate : public QStyledItemDelegate {
  Q_OBJECT
 public:
  explicit NoteListDelegate(QObject* parent = nullptr);

  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;

  QSize sizeHint(const QStyleOptionViewItem& option,
                 const QModelIndex& index) const override;
};