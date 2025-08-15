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
extern QString iniDir;

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

// 解析当前笔记中符合格式的链接（[任意文本](memo/xxx.md)）
void NoteRelationParser::parseNoteReferences(QVector<NoteNode> &nodes,
                                             QVector<NoteRelation> &relations,
                                             const QString &notePath,
                                             int sourceIndex) {
  QFile file(notePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "[parseNoteReferences] 无法打开文件：" << notePath;
    return;
  }
  QString content = file.readAll();
  file.close();

  // 正则：严格匹配 (memo/xxx.md) 格式的链接，捕获显示文本和文件名
  QRegularExpression linkRegex(R"(\[(.*?)\]\((memo\/([^)]+\.md))\))");
  QRegularExpressionMatchIterator it = linkRegex.globalMatch(content);

  while (it.hasNext()) {
    QRegularExpressionMatch match = it.next();
    QString linkText =
        match.captured(1);  // 链接显示文本（如"一个优秀的笔记软件..."）
    QString fullLink = match.captured(
        2);  // 完整链接路径（如"memo/20250322_092204_513504935.md"）
    QString fileName =
        match.captured(3);  // 文件名（如"20250322_092204_513504935.md"）

    qDebug() << "[parseNoteReferences] 找到有效链接：" << fullLink;

    // 构建节点（使用完整链接路径作为标识，确保唯一性）
    int targetIndex = -1;
    for (int i = 0; i < nodes.size(); ++i) {
      // 比较完整链接路径，避免同名文件冲突
      if (nodes[i].filePath == fullLink) {
        targetIndex = i;
        break;
      }
    }
    if (targetIndex == -1) {
      // 节点名称优先使用链接显示文本，为空则用文件名（不含.md）
      QString nodeName =
          linkText.isEmpty() ? QFileInfo(fileName).baseName() : linkText;
      nodes.append(NoteNode(nodeName, fullLink, false));
      targetIndex = nodes.size() - 1;
      qDebug() << "[parseNoteReferences] 新增节点：" << nodeName << "("
               << fullLink << ")";
    }

    // 添加引用关系（当前笔记 → 目标笔记）
    relations.append(NoteRelation(sourceIndex, targetIndex));
  }
}

// 查找其他笔记中是否包含指向当前笔记的链接（memo/当前文件名.md）
void NoteRelationParser::findReferencingNotes(QVector<NoteNode> &nodes,
                                              QVector<NoteRelation> &relations,
                                              const QString &dirPath,
                                              const QString &currentNotePath,
                                              int currentNoteIndex) {
  // 1. 获取当前笔记的文件名（如"20250814_194722_1451366970.md"）
  QString currentFileName = QFileInfo(currentNotePath).fileName();
  qDebug() << "[findReferencingNotes] 当前笔记文件名：" << currentFileName;

  QDir dir(dirPath);
  if (!dir.exists()) {
    qDebug() << "[findReferencingNotes] 目录不存在：" << dirPath;
    return;
  }

  // 2. 遍历目录下的所有.md文件（排除当前笔记）
  QStringList mdFiles = dir.entryList(
      QStringList() << "*.md", QDir::Files | QDir::Readable | QDir::NoSymLinks);
  for (const QString &fileName : mdFiles) {
    QString mdFilePath = dir.filePath(fileName);
    // 跳过当前笔记自身
    if (mdFilePath == currentNotePath) {
      continue;
    }

    // 3. 读取文件内容，提取所有memo/xxx.md格式的链接
    QFile file(mdFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qDebug() << "[findReferencingNotes] 无法打开文件：" << mdFilePath;
      continue;
    }
    QString content = file.readAll();
    file.close();

    // 4. 正则匹配所有(memo/xxx.md)格式的链接
    QRegularExpression linkRegex(R"(\[(.*?)\]\((memo\/([^)]+\.md))\))");
    QRegularExpressionMatchIterator it = linkRegex.globalMatch(content);

    bool isReferencing = false;
    QString refLinkText;  // 引用链接的显示文本

    while (it.hasNext()) {
      QRegularExpressionMatch match = it.next();
      QString fullLink = match.captured(
          2);  // 完整链接（如"memo/20250526_113843_1750251688.md"）
      QString linkFileName = match.captured(
          3);  // 链接中的文件名（如"20250526_113843_1750251688.md"）

      // 5. 检查链接中的文件名是否与当前笔记的文件名一致
      if (linkFileName == currentFileName) {
        isReferencing = true;
        refLinkText = match.captured(1);  // 记录链接显示文本
        qDebug() << "[findReferencingNotes] 找到引用：" << mdFilePath
                 << "中的链接" << fullLink << "匹配当前笔记";
        break;  // 找到一个匹配即可
      }
    }

    // 6. 若存在引用，记录文件名和名称
    if (isReferencing) {
      // 获取笔记名称（通过NoteIndexManager）
      QString noteName =
          mw_one->m_Notes->m_NoteIndexManager->getNoteTitle(mdFilePath);
      // 若获取失败，使用文件名作为备选
      if (noteName.isEmpty()) {
        noteName = QFileInfo(mdFilePath).baseName();
        qDebug() << "[findReferencingNotes] 无法获取笔记名称，使用文件名："
                 << noteName;
      }

      // 添加节点（使用文件路径作为唯一标识）
      int sourceIndex = -1;
      for (int i = 0; i < nodes.size(); ++i) {
        if (nodes[i].filePath == mdFilePath) {
          sourceIndex = i;
          break;
        }
      }
      if (sourceIndex == -1) {
        nodes.append(NoteNode(noteName, mdFilePath, false));
        sourceIndex = nodes.size() - 1;
        qDebug() << "[findReferencingNotes] 新增引用节点：" << noteName << "("
                 << mdFilePath << ")";
      }

      // 记录引用关系
      relations.append(NoteRelation(sourceIndex, currentNoteIndex));
    }
  }

  // 7. 递归遍历子目录
  QStringList subDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot |
                                      QDir::Readable | QDir::NoSymLinks);
  for (const QString &subDir : subDirs) {
    QString subDirPath = dir.filePath(subDir);
    qDebug() << "[findReferencingNotes] 进入子目录：" << subDirPath;
    findReferencingNotes(nodes, relations, subDirPath, currentNotePath,
                         currentNoteIndex);
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

  // 新增：汇总打印所有引用关系
  qDebug() << "\n[解析完成] 总节点数：" << nodes.size() << "，总关系数："
           << relations.size();
  qDebug() << "引用关系列表：";
  for (const auto &rel : relations) {
    QString sourceName = nodes[rel.sourceIndex].name;
    QString targetName = nodes[rel.targetIndex].name;
    qDebug() << "  " << sourceName << " → " << targetName;
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

  // 延迟发射信号，确保对象完全构造
  QTimer::singleShot(0, this, &NoteGraphController::modelChanged);
}

QString NoteGraphController::currentNotePath() const {
  return m_currentNotePath;
}

void NoteGraphController::setCurrentNotePath(const QString &path) {
  // if (m_currentNotePath != path) {
  m_currentNotePath = path;
  emit currentNotePathChanged();
  m_parser->parseNoteRelations(m_model, m_currentNotePath);
  //} else {
  //  mw_one->closeProgress();
  //}
}

NoteGraphModel *NoteGraphController::model() const { return m_model; }

NoteRelationParser *NoteGraphController::parser() const { return m_parser; }

void NoteGraphController::handleNodeDoubleClick(const QString &filePath) {
  emit nodeDoubleClicked(filePath);
}
