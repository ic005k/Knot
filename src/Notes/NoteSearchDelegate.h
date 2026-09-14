#ifndef NOTESEARCHDELEGATE_H
#define NOTESEARCHDELEGATE_H

#include <QStyledItemDelegate>

class NoteSearchDelegate : public QStyledItemDelegate {
  Q_OBJECT
 public:
  explicit NoteSearchDelegate(QObject* parent = nullptr);

  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;

  QSize sizeHint(const QStyleOptionViewItem& option,
                 const QModelIndex& index) const override;
};

#endif  // NOTESEARCHDELEGATE_H