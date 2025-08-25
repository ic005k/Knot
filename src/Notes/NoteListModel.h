// 头文件：NoteListModel.h
#include <QAbstractListModel>
#include <QString>

// 存储单条列表项的数据结构
struct NoteItem {
  QString text0;
  QString text1;
  QString text2;
  QString text3;
  int myh;
  QString time;
  QString dototext;
  int type;
};

class NoteListModel : public QAbstractListModel {
  Q_OBJECT
 public:
  // 1. 定义QML中使用的角色（与QML的ListElement字段对应）
  enum NoteRoles {
    Text0Role = Qt::UserRole + 1,  // 从UserRole开始避免与系统角色冲突
    Text1Role,
    Text2Role,
    Text3Role,
    MyhRole,
    TimeRole,
    DotoTextRole,
    TypeRole
  };

  explicit NoteListModel(QObject *parent = nullptr);

  // 2. 重写QAbstractListModel核心函数（ListView按需调用）
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;
  QHash<int, QByteArray> roleNames()
      const override;  // 关键：映射角色名到QML可访问的字符串

  // 3. 提供数据操作接口（供QML调用，如添加/删除/修改项）
  Q_INVOKABLE void addItem(const QString &text0, const QString &text1,
                           const QString &text2, const QString &text3, int myh);
  Q_INVOKABLE void insertItem(int index, const QString &text0,
                              const QString &text1, const QString &text2,
                              const QString &text3);
  Q_INVOKABLE void removeItem(int index);
  Q_INVOKABLE void modifyItemText0(int index, const QString &text0);
  // ... 其他修改接口（如modifyItemTime、modifyItemType）

  // 批量添加接口：接收NoteItem列表，一次性插入
  Q_INVOKABLE void addBatchItems(const QList<NoteItem> &batchItems);

  // 获取text0的接口（Q_INVOKABLE允许QML调用，也可被C++直接调用）
  Q_INVOKABLE QString getText0(int index) const;

  // 获取text3的接口
  Q_INVOKABLE QString getText3(int index) const;

 private:
  QList<NoteItem> m_items;  // C++侧存储所有数据
};
