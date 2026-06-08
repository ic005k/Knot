#pragma once

#include <QAbstractListModel>
#include <QFileInfo>

#include "database_manager.h"

class SearchModel : public QAbstractListModel {
  Q_OBJECT
 public:
  enum Roles { TitleRole = Qt::UserRole + 1, PathRole, PreviewRole };

  explicit SearchModel(QObject* parent = nullptr);

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  void setResults(const QVector<SearchResult>& results);

 private:
  QVector<SearchResult> m_results;
};
