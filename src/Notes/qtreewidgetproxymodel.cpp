#include "qtreewidgetproxymodel.h"

QTreeWidgetProxyModel::QTreeWidgetProxyModel(QTreeWidget* treeWidget,
                                             QObject* parent)
    : QAbstractItemModel(parent), m_treeWidget(treeWidget) {
  if (!m_treeWidget) return;

  // 节点数据变化
  connect(m_treeWidget, &QTreeWidget::itemChanged, this,
          [this] { resetAll(); });

  // 展开 / 折叠
  connect(m_treeWidget, &QTreeWidget::itemExpanded, this,
          [this] { resetAll(); });

  connect(m_treeWidget, &QTreeWidget::itemCollapsed, this,
          [this] { resetAll(); });

  // Qt 6 正确监听结构变化
  connect(m_treeWidget->model(), &QAbstractItemModel::rowsInserted, this,
          [this] { resetAll(); });

  connect(m_treeWidget->model(), &QAbstractItemModel::rowsRemoved, this,
          [this] { resetAll(); });

  // 可选但强烈推荐
  connect(m_treeWidget->model(), &QAbstractItemModel::modelReset, this,
          [this] { resetAll(); });
}

int QTreeWidgetProxyModel::getItemRow(QTreeWidgetItem* item) const {
  if (!item || !m_treeWidget) return -1;
  QTreeWidgetItem* parentItem = item->parent();
  if (!parentItem) return m_treeWidget->indexOfTopLevelItem(item);
  return parentItem->indexOfChild(item);
}

QTreeWidgetItem* QTreeWidgetProxyModel::indexToItem(const QModelIndex& idx) {
  if (!idx.isValid()) return nullptr;
  return static_cast<QTreeWidgetItem*>(idx.internalPointer());
}

QModelIndex QTreeWidgetProxyModel::itemToIndex(QTreeWidgetItem* item) const {
  if (!item) return {};
  int row = getItemRow(item);
  return createIndex(row, 0, item);
}

QModelIndex QTreeWidgetProxyModel::index(int row, int column,
                                         const QModelIndex& parent) const {
  // 新增：列合法性校验
  if (!m_treeWidget || row < 0 || column < 0 || column >= columnCount(parent)) {
    return {};
  }

  QTreeWidgetItem* parentItem = indexToItem(parent);
  QTreeWidgetItem* childItem = nullptr;

  if (parentItem)
    childItem = parentItem->child(row);
  else
    childItem = m_treeWidget->topLevelItem(row);

  if (!childItem) return {};
  return createIndex(row, column, childItem);
}

QModelIndex QTreeWidgetProxyModel::parent(const QModelIndex& child) const {
  if (!child.isValid()) return {};
  QTreeWidgetItem* childItem = indexToItem(child);
  if (!childItem) return {};

  QTreeWidgetItem* parentItem = childItem->parent();
  if (!parentItem) return {};

  // 标准写法：通过 item 生成父索引
  return itemToIndex(parentItem);
}

int QTreeWidgetProxyModel::rowCount(const QModelIndex& parent) const {
  if (!m_treeWidget) return 0;
  QTreeWidgetItem* parentItem = indexToItem(parent);
  return parentItem ? parentItem->childCount()
                    : m_treeWidget->topLevelItemCount();
}

int QTreeWidgetProxyModel::columnCount(const QModelIndex& parent) const {
  Q_UNUSED(parent)
  return 1;  // TreeView 单文本列，固定为1
}

QVariant QTreeWidgetProxyModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || !m_treeWidget) return {};
  QTreeWidgetItem* item = indexToItem(index);
  if (!item) return {};

  if (role == Qt::DisplayRole) return item->text(0);

  return {};
}

Qt::ItemFlags QTreeWidgetProxyModel::flags(const QModelIndex& index) const {
  if (!index.isValid()) return Qt::ItemIsEnabled;

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
}

void QTreeWidgetProxyModel::resetAll() {
  beginResetModel();
  endResetModel();
}

void QTreeWidgetProxyModel::setCurrentIndex(const QModelIndex& idx) {
  if (!m_treeWidget) return;
  QTreeWidgetItem* item = indexToItem(idx);
  if (item) m_treeWidget->setCurrentItem(item);
}

bool QTreeWidgetProxyModel::hasChildren(const QModelIndex& parent) const {
  if (!m_treeWidget) return false;

  QTreeWidgetItem* item = indexToItem(parent);
  if (!item) return m_treeWidget->topLevelItemCount() > 0;

  return item->childCount() > 0;
}