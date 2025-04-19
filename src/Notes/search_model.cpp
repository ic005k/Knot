#include "search_model.h"

SearchModel::SearchModel(QObject *parent) : QAbstractListModel(parent) {}

int SearchModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : m_results.size();
}

QVariant SearchModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_results.size())
        return QVariant();

    const auto &item = m_results[index.row()];
    switch (role) {
    case TitleRole:
        //return QFileInfo(item.path).fileName();
        return item.title; // 直接返回预先生成的标题
    case PathRole:
        return item.filePath;
    case PreviewRole:
        return item.preview;
    }
    return QVariant();
}

QHash<int, QByteArray> SearchModel::roleNames() const {
    return {
        {TitleRole, "title"},
        {PathRole, "path"},
        {PreviewRole, "preview"}
    };
}

void SearchModel::setResults(const QVector<DatabaseManager::SearchResult> &results) {
    beginResetModel();
    m_results = results;
    endResetModel();
}
