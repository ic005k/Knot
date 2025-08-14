#include "note_graph.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaType>
#include <QQmlEngine>
#include <QRegularExpression>
#include <QtMath>  // <-- 添加这个用于 M_PI

// --- 将 qmlRegisterType 移到这里 ---
void registerNoteGraphTypes() {
  // 注册普通 QML 类型
  qmlRegisterType<NoteGraphModel>("NoteGraph", 1, 0, "NoteGraphModel");
  qmlRegisterType<NoteRelationParser>("NoteGraph", 1, 0, "NoteRelationParser");
  // 注册元类型
  qRegisterMetaType<QPointF>("QPointF");
  // 注意：单例注册在 initializeNoteGraph 中
}

// --- 保持 initializeNoteGraph 专注于单例注册 ---
static QObject *noteGraphControllerSingletonProvider(QQmlEngine *engine,
                                                     QJSEngine *scriptEngine) {
  Q_UNUSED(engine);
  Q_UNUSED(scriptEngine);
  return new NoteGraphController();
}

void initializeNoteGraph() {
  // 注册 NoteGraphController 单例
  qmlRegisterSingletonType<NoteGraphController>(
      "NoteGraph", 1, 0, "NoteGraphController",
      noteGraphControllerSingletonProvider);  // 使用命名函数更清晰
}

// --- NoteGraphModel 实现 ---
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
    if (m_nodes[i].filePath == filePath) {
      return i;
    }
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

// --- NoteRelationParser 实现 ---
NoteRelationParser::NoteRelationParser(QObject *parent) : QObject(parent) {}

void NoteRelationParser::parseNoteRelations(NoteGraphModel *model,
                                            const QString &currentNotePath) {
  if (!model || currentNotePath.isEmpty()) return;

  model->clear();

  QString currentNoteName = QFileInfo(currentNotePath).baseName();
  model->addNode(NoteNode(currentNoteName, currentNotePath, true));

  parseNoteReferences(model, currentNotePath, true);

  QString notesDir = QFileInfo(currentNotePath).absolutePath();
  findReferencingNotes(model, notesDir, currentNotePath);

  arrangeNodes(model);

  emit parsingCompleted();
}

void NoteRelationParser::parseNoteReferences(NoteGraphModel *model,
                                             const QString &notePath,
                                             bool isSource) {
  QFile file(notePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

  QString content = file.readAll();
  file.close();

  QRegularExpression regex(R"(\[(.*?)\]\((.*?\.md)\))");
  QRegularExpressionMatchIterator it = regex.globalMatch(content);

  int sourceIndex = model->findNodeIndex(notePath);
  if (sourceIndex == -1) return;

  while (it.hasNext()) {
    QRegularExpressionMatch match = it.next();
    QString name = match.captured(1);
    QString path = match.captured(2);

    if (!QFileInfo(path).isAbsolute()) {
      path = QFileInfo(QFileInfo(notePath).absolutePath() + "/" + path)
                 .absoluteFilePath();
    }

    int targetIndex = model->findNodeIndex(path);
    if (targetIndex == -1) {
      model->addNode(NoteNode(name, path));
      targetIndex = model->findNodeIndex(path);
    }

    model->addRelation(NoteRelation(sourceIndex, targetIndex));
  }
}

void NoteRelationParser::findReferencingNotes(NoteGraphModel *model,
                                              const QString &dirPath,
                                              const QString &currentNotePath) {
  QDir dir(dirPath);
  if (!dir.exists()) return;

  QStringList mdFiles =
      dir.entryList(QStringList() << "*.md", QDir::Files | QDir::Readable);
  int currentNoteIndex = model->findNodeIndex(currentNotePath);
  if (currentNoteIndex == -1) return;

  // --- 修改：使用范围 for 循环 ---
  for (const QString &fileName : mdFiles) {
    QString filePath = dir.filePath(fileName);
    if (filePath == currentNotePath) continue;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) continue;

    QString content = file.readAll();
    file.close();

    QString currentNoteFileName = QFileInfo(currentNotePath).fileName();
    if (content.contains(currentNoteFileName)) {
      QString noteName = QFileInfo(filePath).baseName();
      int sourceIndex = model->findNodeIndex(filePath);
      if (sourceIndex == -1) {
        model->addNode(NoteNode(noteName, filePath));
        sourceIndex = model->findNodeIndex(filePath);
      }
      model->addRelation(NoteRelation(sourceIndex, currentNoteIndex));
      // parseNoteReferences(model, filePath, false); // 可选：递归解析引用
    }
  }

  QStringList subDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
  // --- 修改：使用范围 for 循环 ---
  for (const QString &subDir : subDirs) {
    findReferencingNotes(model, dir.filePath(subDir), currentNotePath);
  }
}

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

  // --- 修改：使用范围 for 循环 ---
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

// --- NoteGraphController 实现 ---
NoteGraphController::NoteGraphController(QObject *parent) : QObject(parent) {
  m_model = new NoteGraphModel(this);
  m_parser = new NoteRelationParser(this);
  // --- 修改：移除了 qmlRegisterType 调用 ---
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
    // 可选：在解析完成后通知 QML 更新
    // emit modelChanged();
  }
}

NoteGraphModel *NoteGraphController::model() const { return m_model; }

NoteRelationParser *NoteGraphController::parser() const { return m_parser; }

void NoteGraphController::handleNodeDoubleClick(const QString &filePath) {
  emit nodeDoubleClicked(filePath);
}
