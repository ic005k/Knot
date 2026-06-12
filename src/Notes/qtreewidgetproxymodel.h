#ifndef QTREEWIDGETPROXYMODEL_H
#define QTREEWIDGETPROXYMODEL_H

#include <QAbstractItemModel>
#include <QTreeWidget>
#include <QTreeWidgetItem>

class QTreeWidgetProxyModel : public QAbstractItemModel {
  Q_OBJECT
 public:
  explicit QTreeWidgetProxyModel(QTreeWidget* treeWidget,
                                 QObject* parent = nullptr);

  // 树形模型必实现接口
  QModelIndex index(int row, int column,
                    const QModelIndex& parent) const override;
  QModelIndex parent(const QModelIndex& child) const override;
  int rowCount(const QModelIndex& parent) const override;
  int columnCount(const QModelIndex& parent) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;

  // 对外刷新接口
  void resetAll();
  // QML 调用：同步选中
  Q_INVOKABLE void setCurrentIndex(const QModelIndex& idx);

  bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;

 private:
  // 工具方法：获取 item 在父节点中的行号
  int getItemRow(QTreeWidgetItem* item) const;
  // 索引 <-> QTreeWidgetItem 互转
  static QTreeWidgetItem* indexToItem(const QModelIndex& idx);
  QModelIndex itemToIndex(QTreeWidgetItem* item) const;

  QTreeWidget* m_treeWidget = nullptr;
};

#endif  // QTREEWIDGETPROXYMODEL_H