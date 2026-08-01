#pragma once

#include <QAbstractListModel>
#include <QFileInfo>

#include "DatabaseManager.h"

class SearchModel : public QAbstractListModel {
  Q_OBJECT
 public:
  enum Roles { TitleRole = Qt::UserRole + 1, PathRole, PreviewRole };

  explicit SearchModel(QObject* parent = nullptr);

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  void setResults(const QVector<SearchResult>& results);

 signals:
  // 模型数据重置完成、且存在数据时触发，通知QML选中第0行
  void selectFirstItem();

 private:
  QVector<SearchResult> m_results;
};
