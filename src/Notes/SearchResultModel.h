#pragma once
#include <QAbstractListModel>
#include <QDebug>

#include "NotesSearchEngine.h"

class SearchResultModel : public QAbstractListModel {
  Q_OBJECT
 public:
  enum Roles {
    FilePathRole = Qt::UserRole + 1,
    FileTitleRole,
    PreviewTextRole,
    HighlightPositionsRole
  };

  SearchResultModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}

  int rowCount(const QModelIndex &parent = QModelIndex()) const override {
    Q_UNUSED(parent);
    return m_data.size();
  }

  QVariant data(const QModelIndex &index, int role) const override {
    if (!index.isValid() || index.row() >= m_data.size()) return QVariant();

    const SearchResult &item = m_data[index.row()];
    switch (role) {
      case FilePathRole:
        return item.filePath;
      case FileTitleRole:
        return item.fileTitle;
      case PreviewTextRole:
        return item.previewText;
      case HighlightPositionsRole:
        return QVariant::fromValue(item.highlightPos);
      default:
        return QVariant();
    }
  }

  QHash<int, QByteArray> roleNames() const override {
    return {{FilePathRole, "filePath"},
            {FileTitleRole, "fileTitle"},
            {PreviewTextRole, "previewText"},
            {HighlightPositionsRole, "highlightPos"}};
  }

  void updateResults(const QList<SearchResult> &results) {
    beginResetModel();
    m_data = results;
    endResetModel();
    Q_EMIT resultsUpdated();
  }

 signals:
  void resultsUpdated();

 private:
  QList<SearchResult> m_data;  // 直接存储 SearchResult
};
