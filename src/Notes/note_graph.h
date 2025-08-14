#ifndef NOTE_GRAPH_H
#define NOTE_GRAPH_H

#include <QAbstractItemModel>
#include <QObject>
#include <QPointF>
#include <QPointer>  // 新增：用于安全持有模型指针
#include <QString>
#include <QVector>

// 结构体定义
struct NoteNode {
  QString name;
  QString filePath;
  QPointF position;
  bool isCurrentNote;

  NoteNode(const QString &n, const QString &path, bool isCurrent = false)
      : name(n), filePath(path), isCurrentNote(isCurrent) {}
};

struct NoteRelation {
  int sourceIndex;
  int targetIndex;

  NoteRelation(int src, int tgt) : sourceIndex(src), targetIndex(tgt) {}
};

// 声明元类型（用于跨线程传递）
Q_DECLARE_METATYPE(QVector<NoteNode>)
Q_DECLARE_METATYPE(QVector<NoteRelation>)

class NoteGraphModel : public QAbstractItemModel {
  Q_OBJECT
  Q_ENUMS(NodeRoles)
 public:
  enum NodeRoles {
    NameRole = Qt::UserRole + 1,
    FilePathRole,
    PositionRole,
    IsCurrentNoteRole
  };
  explicit NoteGraphModel(QObject *parent = nullptr);

  // 重写QAbstractItemModel方法
  QModelIndex index(int row, int column,
                    const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &child) const override;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;

  // 自定义方法
  Q_INVOKABLE QVariantList getRelations() const;
  Q_INVOKABLE void setNodePosition(int index, qreal x, qreal y);
  void addNode(const NoteNode &node);
  void addRelation(const NoteRelation &relation);
  int findNodeIndex(const QString &filePath) const;
  void clear();

 signals:
  void nodePositionChanged(int index, qreal x, qreal y);
  void modelCleared();

 private:
  QVector<NoteNode> m_nodes;
  QVector<NoteRelation> m_relations;
};

class NoteRelationParser : public QObject {
  Q_OBJECT
 public:
  explicit NoteRelationParser(QObject *parent = nullptr);
  Q_INVOKABLE void parseNoteRelations(NoteGraphModel *model,
                                      const QString &currentNotePath);

 signals:
  void parsingCompleted();
  // 新增：后台解析完成后传递数据的信号（跨线程使用）
  void parsedDataReady(const QVector<NoteNode> &nodes,
                       const QVector<NoteRelation> &relations);
  void sendDataToQml(const QVariantList &nodesArray,
                     const QVariantList &relationsArray);

 private slots:
  // 新增：主线程处理后台返回数据的槽函数
  void onParsedDataReady(const QVector<NoteNode> &nodes,
                         const QVector<NoteRelation> &relations);

 private:
  // 修改：原parseNoteReferences改为后台数据收集函数（参数调整）
  void parseNoteReferences(QVector<NoteNode> &nodes,
                           QVector<NoteRelation> &relations,
                           const QString &notePath, int sourceIndex);
  // 修改：原findReferencingNotes改为后台数据收集函数（参数调整）
  void findReferencingNotes(QVector<NoteNode> &nodes,
                            QVector<NoteRelation> &relations,
                            const QString &dirPath,
                            const QString &currentNotePath,
                            int currentNoteIndex);
  void arrangeNodes(NoteGraphModel *model);
  // 新增：持有模型指针（使用QPointer确保安全）
  QPointer<NoteGraphModel> m_model;
};

class NoteGraphController : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString currentNotePath READ currentNotePath WRITE
                 setCurrentNotePath NOTIFY currentNotePathChanged)
  Q_PROPERTY(NoteGraphModel *model READ model NOTIFY
                 modelChanged)  // 新增：暴露model给QML
 public:
  explicit NoteGraphController(QObject *parent = nullptr);
  QString currentNotePath() const;
  void setCurrentNotePath(const QString &path);
  NoteGraphModel *model() const;  // 保持原getter
  NoteRelationParser *parser() const;

 signals:
  void currentNotePathChanged();
  void nodeDoubleClicked(const QString &filePath);
  void modelChanged();  // 新增：模型初始化/变更信号

 public slots:
  void handleNodeDoubleClick(const QString &filePath);

 private:
  QString m_currentNotePath;
  NoteGraphModel *m_model;
  NoteRelationParser *m_parser;
};

// 初始化函数声明
void initializeNoteGraph();
void registerNoteGraphTypes();  // <-- 新增：声明 registerNoteGraphTypes 函数

#endif  // NOTE_GRAPH_H
