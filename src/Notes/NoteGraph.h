#ifndef NOTEGRAPH_H
#define NOTEGRAPH_H

#include <QAbstractItemModel>
#include <QJsonObject>
#include <QObject>
#include <QPointF>
#include <QPointer>
#include <QString>
#include <QVector>

// =================================================================================
// 缓存链接条目（唯一数据载体）
// =================================================================================
struct CachedLink {
  QString fileName;     // 目标/来源文件名（如 "20250813_210313.md"）
  QString displayText;  // [] 中的原始链接文本，空串表示需运行时回退

  QJsonObject toJson() const;
  static CachedLink fromJson(const QJsonObject& obj);
};

// =================================================================================
// 图谱缓存（纯数据 + 序列化，不含任何业务逻辑）
// =================================================================================
class NoteGraphCache {
 public:
  QMap<QString, QVector<CachedLink>> forward;   // 我引用谁
  QMap<QString, QVector<CachedLink>> backward;  // 谁引用我

  void load(const QString& filePath);
  void save(const QString& filePath) const;
  bool isEmpty() const;
  void clear();

  // 按 fileName 精确移除条目（供 Parser 调用）
  static void removeByFileName(QVector<CachedLink>& links,
                               const QString& fileName);
};

// =================================================================================
// 图谱节点 & 关系
// =================================================================================
struct NoteNode {
  QString name;
  QString filePath;
  QPointF position;
  bool isCurrentNote;

  NoteNode(const QString& n = {}, const QString& path = {},
           bool current = false)
      : name(n), filePath(path), position(0, 0), isCurrentNote(current) {}
};

struct NoteRelation {
  int sourceIndex;
  int targetIndex;

  NoteRelation(int src = -1, int tgt = -1)
      : sourceIndex(src), targetIndex(tgt) {}
};

Q_DECLARE_METATYPE(QVector<NoteNode>)
Q_DECLARE_METATYPE(QVector<NoteRelation>)

// =================================================================================
// NoteGraphModel
// =================================================================================
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

  explicit NoteGraphModel(QObject* parent = nullptr);

  QModelIndex index(int row, int column,
                    const QModelIndex& parent = {}) const override;
  QModelIndex parent(const QModelIndex& child) const override;
  int rowCount(const QModelIndex& parent = {}) const override;
  int columnCount(const QModelIndex& parent = {}) const override;
  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;

  Q_INVOKABLE QVariantList getRelations() const;
  Q_INVOKABLE void setNodePosition(int index, qreal x, qreal y);
  void addNode(const NoteNode& node);
  void addRelation(const NoteRelation& relation);
  int findNodeIndex(const QString& filePath) const;
  void clear();

 signals:
  void nodePositionChanged(int index, qreal x, qreal y);
  void modelCleared();

 private:
  QVector<NoteNode> m_nodes;
  QVector<NoteRelation> m_relations;
};

// =================================================================================
// NoteRelationParser
// =================================================================================
class NoteRelationParser : public QObject {
  Q_OBJECT
 public:
  enum CacheAction : int { CACHE_DELETE = 0, CACHE_MODIFY = 1 };

  explicit NoteRelationParser(QObject* parent = nullptr);

  Q_INVOKABLE void parseNoteRelations(NoteGraphModel* model,
                                      const QString& currentNotePath);
  Q_INVOKABLE void invalidateCache();

  void updateNoteCache(const QString& filePath);
  void deleteNoteCache(const QString& filePath);
  void invalidateNoteCache(const QString& filePath, CacheAction action);

 signals:
  void parsingCompleted();
  void parsedDataReady(const QVector<NoteNode>& nodes,
                       const QVector<NoteRelation>& relations);

 private slots:
  void onParsedDataReady(const QVector<NoteNode>& nodes,
                         const QVector<NoteRelation>& relations);

 private:
  // 解析引擎
  void parseNoteReferences(QVector<NoteNode>& nodes,
                           QVector<NoteRelation>& relations,
                           const QString& notePath, int sourceIndex);
  void findReferencingNotes(QVector<NoteNode>& nodes,
                            QVector<NoteRelation>& relations,
                            const QString& dirPath,
                            const QString& currentNotePath,
                            int currentNoteIndex);

  // 缓存构建
  void buildCacheFromNodes(const QVector<NoteNode>& nodes,
                           const QVector<NoteRelation>& relations,
                           const QString& currentFileName);

  // 布局
  void arrangeNodes(NoteGraphModel* model);

  // 工具
  static int findNodeIndex(const QVector<NoteNode>& nodes, const QString& path);
  QString resolveDisplayName(const CachedLink& link,
                             const QString& fullPath) const;

  QPointer<NoteGraphModel> m_model;
  NoteGraphCache m_cache;
  QString m_cachePath;
};

// =================================================================================
// NoteGraphController
// =================================================================================
class NoteGraphController : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString currentNotePath READ currentNotePath WRITE
                 setCurrentNotePath NOTIFY currentNotePathChanged)
  Q_PROPERTY(NoteGraphModel* model READ model NOTIFY modelChanged)
 public:
  explicit NoteGraphController(QObject* parent = nullptr);

  QString currentNotePath() const;
  void setCurrentNotePath(const QString& path);
  NoteGraphModel* model() const;
  NoteRelationParser* parser() const;

 signals:
  void currentNotePathChanged();
  void nodeDoubleClicked(const QString& filePath);
  void modelChanged();

 public slots:
  void handleNodeDoubleClick(const QString& filePath);

 private:
  QString m_currentNotePath;
  NoteGraphModel* m_model;
  NoteRelationParser* m_parser;
};

// 初始化入口
void registerNoteGraphTypes();
void initializeNoteGraph();

#endif  // NOTEGRAPH_H
