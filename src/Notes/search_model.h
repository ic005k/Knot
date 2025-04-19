#pragma once
#include <QAbstractListModel>
#include "database_manager.h"
#include <QFileInfo>

class SearchModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        TitleRole = Qt::UserRole + 1,
        PathRole,
        PreviewRole
    };

    explicit SearchModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setResults(const QVector<DatabaseManager::SearchResult> &results);

private:
    QVector<DatabaseManager::SearchResult> m_results;
};
