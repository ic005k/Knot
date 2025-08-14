#include "note_graph.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaType>
#include <QPointer>  // 用于安全持有模型指针
#include <QQmlEngine>
#include <QRegularExpression>
#include <QtMath>

#include "src/MainWindow.h"

extern MainWindow *mw_one;

void registerNoteGraphTypes() {
  qmlRegisterType<NoteGraphModel>("NoteGraph", 1, 0, "NoteGraphModel");
  qmlRegisterType<NoteRelationParser>("NoteGraph", 1, 0, "NoteRelationParser");
  qRegisterMetaType<QPointF>("QPointF");
  qRegisterMetaType<QVector<NoteNode>>("QVector<NoteNode>");  // 注册自定义类型
  qRegisterMetaType<QVector<NoteRelation>>("QVector<NoteRelation>");
}

static QObject *noteGraphControllerSingletonProvider(QQmlEngine *engine,
                                                     QJSEngine *scriptEngine) {
  Q_UNUSED(engine);
  Q_UNUSED(scriptEngine);
  return new NoteGraphController();
}

void initializeNoteGraph() {
  qmlRegisterSingletonType<NoteGraphController>(
      "NoteGraph", 1, 0, "NoteGraphController",
      noteGraphControllerSingletonProvider);
}

// --- NoteGraphModel 实现（保持不变）---
NoteGraphModel::NoteGraphModel(QObject *parent) : QAbstractItemModel(parent) {}

QModelIndex NoteGraphModel::index(int row, int column,
                                  const QModelIndex &parent) const {
  if (!hasIndex(row, column, parent)) return QModelIndex();
  return createIndex(row, column);
}

QModelIndex NoteGraphModel::parent(const QModelIndex &child) const {
  Q_UNUSED(child);
  return QModelIndex();
}

int NoteGraphModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) return 0;
  return m_nodes.size();
}

int NoteGraphModel::columnCount(const QModelIndex &parent) const {
  Q_UNUSED(parent);
  return 1;
}

QVariant NoteGraphModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= m_nodes.size()) return QVariant();
  const NoteNode &node = m_nodes[index.row()];

  switch (role) {
    case NameRole:
      return node.name;
    case FilePathRole:
      return node.filePath;
    case PositionRole:
      return QVariant::fromValue(node.position);
    case IsCurrentNoteRole:
      return node.isCurrentNote;
    default:
      return QVariant();
  }
}

QHash<int, QByteArray> NoteGraphModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[NameRole] = "name";
  roles[FilePathRole] = "filePath";
  roles[PositionRole] = "position";
  roles[IsCurrentNoteRole] = "isCurrentNote";
  return roles;
}

QVariantList NoteGraphModel::getRelations() const {
  QVariantList relations;
  for (const auto &rel : m_relations) {
    QVariantMap map;
    map["source"] = rel.sourceIndex;
    map["target"] = rel.targetIndex;
    relations.append(map);
  }
  return relations;
}

void NoteGraphModel::setNodePosition(int index, qreal x, qreal y) {
  if (index >= 0 && index < m_nodes.size()) {
    m_nodes[index].position = QPointF(x, y);
    emit dataChanged(createIndex(index, 0), createIndex(index, 0),
                     {PositionRole});
    emit nodePositionChanged(index, x, y);
  }
}

void NoteGraphModel::addNode(const NoteNode &node) {
  beginInsertRows(QModelIndex(), m_nodes.size(), m_nodes.size());
  m_nodes.append(node);
  endInsertRows();
}

void NoteGraphModel::addRelation(const NoteRelation &relation) {
  m_relations.append(relation);
}

int NoteGraphModel::findNodeIndex(const QString &filePath) const {
  for (int i = 0; i < m_nodes.size(); ++i) {
    if (m_nodes[i].filePath == filePath) return i;
  }
  return -1;
}

void NoteGraphModel::clear() {
  beginResetModel();
  m_nodes.clear();
  m_relations.clear();
  endResetModel();
  emit modelCleared();
}

// --- NoteRelationParser 实现（核心优化部分）---
NoteRelationParser::NoteRelationParser(QObject *parent) : QObject(parent) {
  // 连接后台数据到主线程处理槽函数
  connect(this, &NoteRelationParser::parsedDataReady, this,
          &NoteRelationParser::onParsedDataReady,
          Qt::QueuedConnection);  // 跨线程信号槽（排队执行）
}

// 启动解析：主线程触发，后台执行耗时操作
void NoteRelationParser::parseNoteRelations(NoteGraphModel *model,
                                            const QString &currentNotePath) {
  if (!model || currentNotePath.isEmpty()) return;

  // 保存模型指针（使用QPointer避免悬垂指针）
  m_model = model;
  // 主线程先清空模型
  model->clear();

  // 获取当前笔记名称
  QString currentNoteName =
      mw_one->m_Notes->m_NoteIndexManager->getNoteTitle(currentNotePath);

  // 启动后台任务（使用QtConcurrent线程池）
  QtConcurrent::run([=]() {
    QVector<NoteNode> nodes;
    QVector<NoteRelation> relations;

    // 添加当前笔记节点（作为第一个节点，索引为0）
    nodes.append(NoteNode(currentNoteName, currentNotePath, true));

    // 后台解析当前笔记的引用
    parseNoteReferences(nodes, relations, currentNotePath, 0);

    // 后台查找引用当前笔记的文件
    QString notesDir = QFileInfo(currentNotePath).absolutePath();
    findReferencingNotes(nodes, relations, notesDir, currentNotePath, 0);

    // 发送结果到主线程（触发parsedDataReady信号）
    emit parsedDataReady(nodes, relations);
  });
}

// 后台任务：解析当前笔记中的引用（仅收集数据）
void NoteRelationParser::parseNoteReferences(QVector<NoteNode> &nodes,
                                             QVector<NoteRelation> &relations,
                                             const QString &notePath,
                                             int sourceIndex) {
  QFile file(notePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

  QString content = file.readAll();
  file.close();

  // 正则匹配markdown链接（如 [name](path.md)）
  QRegularExpression regex(R"(\[(.*?)\]\((.*?\.md)\))");
  QRegularExpressionMatchIterator it = regex.globalMatch(content);

  while (it.hasNext()) {
    QRegularExpressionMatch match = it.next();
    QString name = match.captured(1);
    QString path = match.captured(2);

    // 处理相对路径
    if (!QFileInfo(path).isAbsolute()) {
      path = QFileInfo(QFileInfo(notePath).absolutePath() + "/" + path)
                 .absoluteFilePath();
    }

    // 查找节点是否已存在，不存在则添加
    int targetIndex = -1;
    for (int i = 0; i < nodes.size(); ++i) {
      if (nodes[i].filePath == path) {
        targetIndex = i;
        break;
      }
    }
    if (targetIndex == -1) {
      nodes.append(NoteNode(name, path));
      targetIndex = nodes.size() - 1;
    }

    // 添加关系
    relations.append(NoteRelation(sourceIndex, targetIndex));
  }
}

// 后台任务：递归查找引用当前笔记的文件（仅收集数据）
void NoteRelationParser::findReferencingNotes(QVector<NoteNode> &nodes,
                                              QVector<NoteRelation> &relations,
                                              const QString &dirPath,
                                              const QString &currentNotePath,
                                              int currentNoteIndex) {
  QDir dir(dirPath);
  if (!dir.exists()) return;

  // 遍历当前目录的.md文件
  QStringList mdFiles =
      dir.entryList(QStringList() << "*.md", QDir::Files | QDir::Readable);
  for (const QString &fileName : mdFiles) {
    QString filePath = dir.filePath(fileName);
    if (filePath == currentNotePath) continue;  // 跳过当前笔记

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) continue;

    QString content = file.readAll();
    file.close();

    // 检查是否引用当前笔记
    QString currentNoteFileName =
        mw_one->m_Notes->m_NoteIndexManager->getNoteTitle(currentNotePath);
    // QFileInfo(currentNotePath).fileName();

    if (content.contains(currentNoteFileName)) {
      QString noteName = QFileInfo(filePath).baseName();

      // 查找节点是否已存在
      int sourceIndex = -1;
      for (int i = 0; i < nodes.size(); ++i) {
        if (nodes[i].filePath == filePath) {
          sourceIndex = i;
          break;
        }
      }
      if (sourceIndex == -1) {
        nodes.append(NoteNode(noteName, filePath));
        sourceIndex = nodes.size() - 1;
      }

      // 添加关系（引用当前笔记）
      relations.append(NoteRelation(sourceIndex, currentNoteIndex));
    }
  }

  // 递归遍历子目录
  QStringList subDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
  for (const QString &subDir : subDirs) {
    findReferencingNotes(nodes, relations, dir.filePath(subDir),
                         currentNotePath, currentNoteIndex);
  }
}

// 主线程：处理后台返回的结果并更新模型
void NoteRelationParser::onParsedDataReady(
    const QVector<NoteNode> &nodes, const QVector<NoteRelation> &relations) {
  if (!m_model) return;

  // 批量添加节点和关系到模型（主线程操作）
  for (const auto &node : nodes) {
    m_model->addNode(node);
  }
  for (const auto &rel : relations) {
    m_model->addRelation(rel);
  }

  // 排列节点位置（主线程执行，计算量小）
  arrangeNodes(m_model);

  emit parsingCompleted();

  mw_one->closeProgress();
}

// 主线程：排列节点位置（保持不变）
void NoteRelationParser::arrangeNodes(NoteGraphModel *model) {
  int currentIndex = -1;
  for (int i = 0; i < model->rowCount(); ++i) {
    QModelIndex idx = model->index(i, 0);
    if (model->data(idx, NoteGraphModel::IsCurrentNoteRole).toBool()) {
      currentIndex = i;
      break;
    }
  }
  if (currentIndex == -1) return;

  model->setNodePosition(currentIndex, 0, 0);

  QVariantList relations = model->getRelations();
  QVector<int> referencedNodes, referencingNodes;

  for (const QVariant &relVar : relations) {
    QVariantMap rel = relVar.toMap();
    int source = rel["source"].toInt();
    int target = rel["target"].toInt();

    if (source == currentIndex)
      referencedNodes.append(target);
    else if (target == currentIndex)
      referencingNodes.append(source);
  }

  const qreal radius = 200;
  const qreal angleStep = 2 * M_PI / qMax(1, referencedNodes.size());
  for (int i = 0; i < referencedNodes.size(); ++i) {
    qreal angle = -M_PI / 2 + i * angleStep;
    if (referencedNodes.size() == 1) angle = 0;
    model->setNodePosition(referencedNodes[i], radius * cos(angle),
                           radius * sin(angle));
  }

  for (int i = 0; i < referencingNodes.size(); ++i) {
    qreal angle = M_PI / 2 + i * angleStep;
    if (referencingNodes.size() == 1) angle = M_PI;
    model->setNodePosition(referencingNodes[i], -radius * cos(angle - M_PI),
                           radius * sin(angle - M_PI));
  }
}

// --- NoteGraphController 实现（保持不变）---
NoteGraphController::NoteGraphController(QObject *parent) : QObject(parent) {
  m_model = new NoteGraphModel(this);
  m_parser = new NoteRelationParser(this);
  emit modelChanged();
}

QString NoteGraphController::currentNotePath() const {
  return m_currentNotePath;
}

void NoteGraphController::setCurrentNotePath(const QString &path) {
  if (m_currentNotePath != path) {
    m_currentNotePath = path;
    emit currentNotePathChanged();
    m_parser->parseNoteRelations(m_model, m_currentNotePath);
  } else {
    mw_one->closeProgress();
  }
}

NoteGraphModel *NoteGraphController::model() const { return m_model; }

NoteRelationParser *NoteGraphController::parser() const { return m_parser; }

void NoteGraphController::handleNodeDoubleClick(const QString &filePath) {
  emit nodeDoubleClicked(filePath);
}
