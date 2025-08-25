#include "NoteListModel.h"

NoteListModel::NoteListModel(QObject *parent) : QAbstractListModel(parent) {}

// 返回列表项总数
int NoteListModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) return 0;
  return m_items.size();
}

// ListView按需获取数据
QVariant NoteListModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= m_items.size()) return QVariant();

  const NoteItem &item = m_items[index.row()];
  switch (role) {
    case Text0Role:
      return item.text0;
    case Text1Role:
      return item.text1;
    case Text2Role:
      return item.text2;
    case Text3Role:
      return item.text3;
    case MyhRole:
      return item.myh;
    case TimeRole:
      return item.time;
    case DotoTextRole:
      return item.dototext;
    case TypeRole:
      return item.type;
    default:
      return QVariant();
  }
}

// 角色名映射
QHash<int, QByteArray> NoteListModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[Text0Role] = "text0";
  roles[Text1Role] = "text1";
  roles[Text2Role] = "text2";
  roles[Text3Role] = "text3";
  roles[MyhRole] = "myh";
  roles[TimeRole] = "time";
  roles[DotoTextRole] = "dototext";
  roles[TypeRole] = "type";
  return roles;
}

// 添加项
void NoteListModel::addItem(const QString &text0, const QString &text1,
                            const QString &text2, const QString &text3,
                            int myh) {
  beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
  m_items.append({text0, text1, text2, text3, myh, "", "", 0});
  endInsertRows();
}

// 插入项
void NoteListModel::insertItem(int index, const QString &text0,
                               const QString &text1, const QString &text2,
                               const QString &text3) {
  if (index < 0 || index > m_items.size()) return;
  beginInsertRows(QModelIndex(), index, index);
  m_items.insert(index, {text0, text1, text2, text3, 0, "", "", 0});
  endInsertRows();
}

// 删除项
void NoteListModel::removeItem(int index) {
  if (index < 0 || index >= m_items.size()) return;
  beginRemoveRows(QModelIndex(), index, index);
  m_items.removeAt(index);
  endRemoveRows();
}

// 修改项文本（已修复命名冲突问题）
void NoteListModel::modifyItemText0(
    int rowIndex, const QString &text0) {  // 参数名改为rowIndex
  if (rowIndex < 0 || rowIndex >= m_items.size()) return;
  m_items[rowIndex].text0 = text0;
  // 使用this->index()明确调用成员函数，参数为行号和列号（列表模型通常用0列）
  emit dataChanged(this->index(rowIndex, 0), this->index(rowIndex, 0),
                   {Text0Role});
}

void NoteListModel::addBatchItems(const QList<NoteItem> &batchItems) {
  if (batchItems.isEmpty()) return;  // 空数据直接返回，避免无效操作

  // 1. 通知QML：即将插入批量数据（起始行=当前数据末尾，结束行=末尾+批量数-1）
  int startRow = m_items.size();
  beginInsertRows(QModelIndex(), startRow, startRow + batchItems.size() - 1);

  // 2. C++侧批量添加数据（仅1次内存操作，无频繁拷贝）
  m_items.append(batchItems);

  // 3. 通知QML：批量插入完成，视图统一更新（仅1次刷新）
  endInsertRows();
}

// 获取text0：直接从m_items中读取
QString NoteListModel::getText0(int index) const {
  // 检查索引有效性（避免越界）
  if (index < 0 || index >= m_items.size()) {
    qWarning() << "getText0: 索引越界，index=" << index
               << "，总条数=" << m_items.size();
    return "";
  }
  // 直接返回存储的数据（无需经过data()函数的角色判断）
  return m_items[index].text0;
}

// 获取text3
QString NoteListModel::getText3(int index) const {
  if (index < 0 || index >= m_items.size()) {
    qWarning() << "getText3: 索引越界，index=" << index
               << "，总条数=" << m_items.size();
    return "";
  }
  return m_items[index].text3;
}
