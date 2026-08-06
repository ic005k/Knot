#include "NoteGraph.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMetaType>
#include <QQmlEngine>
#include <QRegularExpression>
#include <QtConcurrent>
#include <QtMath>
#include <algorithm>

#include "src/MainWindow.h"
#include "src/defines.h"
// #include "ui_MainWindow.h"

// 链接正则：[显示文本](memo/文件名.md)
static const QRegularExpression linkRegex(
    R"(\[(.*?)\]\((memo\/([^)]+\.md))\))");

// =================================================================================
// CachedLink
// =================================================================================
QJsonObject CachedLink::toJson() const {
  QJsonObject obj;
  obj["f"] = fileName;
  obj["t"] = displayText;
  return obj;
}

CachedLink CachedLink::fromJson(const QJsonObject& obj) {
  CachedLink link;
  link.fileName = obj["f"].toString();
  link.displayText = obj["t"].toString();
  return link;
}

// =================================================================================
// NoteGraphCache
// =================================================================================
bool NoteGraphCache::isEmpty() const {
  return forward.isEmpty() && backward.isEmpty();
}

void NoteGraphCache::clear() {
  forward.clear();
  backward.clear();
}

void NoteGraphCache::removeByFileName(QVector<CachedLink>& links,
                                      const QString& fileName) {
  links.erase(std::remove_if(links.begin(), links.end(),
                             [&fileName](const CachedLink& l) {
                               return l.fileName == fileName;
                             }),
              links.end());
}

void NoteGraphCache::load(const QString& filePath) {
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

  QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
  file.close();

  if (!doc.isObject()) return;
  QJsonObject root = doc.object();

  auto loadMap =
      [](const QJsonObject& obj) -> QMap<QString, QVector<CachedLink>> {
    QMap<QString, QVector<CachedLink>> map;
    for (auto it = obj.begin(); it != obj.end(); ++it) {
      QVector<CachedLink> links;
      for (const QJsonValue& val : it.value().toArray())
        links.append(CachedLink::fromJson(val.toObject()));
      if (!links.isEmpty()) map[it.key()] = links;
    }
    return map;
  };

  forward = loadMap(root["forward"].toObject());
  backward = loadMap(root["backward"].toObject());
}

void NoteGraphCache::save(const QString& filePath) const {
  auto saveMap =
      [](const QMap<QString, QVector<CachedLink>>& map) -> QJsonObject {
    QJsonObject obj;
    for (auto it = map.begin(); it != map.end(); ++it) {
      if (it.value().isEmpty()) continue;
      QJsonArray arr;
      for (const CachedLink& link : it.value()) arr.append(link.toJson());
      obj[it.key()] = arr;
    }
    return obj;
  };

  QJsonObject root;
  root["forward"] = saveMap(forward);
  root["backward"] = saveMap(backward);

  QFile file(filePath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
  file.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
  file.close();
}

// =================================================================================
// 注册 & 初始化
// =================================================================================
void registerNoteGraphTypes() {
  qmlRegisterType<NoteGraphModel>("NoteGraph", 1, 0, "NoteGraphModel");
  qmlRegisterType<NoteRelationParser>("NoteGraph", 1, 0, "NoteRelationParser");
  qRegisterMetaType<QPointF>("QPointF");
  qRegisterMetaType<QVector<NoteNode>>("QVector<NoteNode>");
  qRegisterMetaType<QVector<NoteRelation>>("QVector<NoteRelation>");
}

static QObject* noteGraphControllerSingletonProvider(QQmlEngine*, QJSEngine*) {
  return new NoteGraphController();
}

void initializeNoteGraph() {
  qmlRegisterSingletonType<NoteGraphController>(
      "NoteGraph", 1, 0, "NoteGraphController",
      noteGraphControllerSingletonProvider);
}

// =================================================================================
// NoteGraphModel
// =================================================================================
NoteGraphModel::NoteGraphModel(QObject* parent) : QAbstractItemModel(parent) {}

QModelIndex NoteGraphModel::index(int row, int column,
                                  const QModelIndex& parent) const {
  if (!hasIndex(row, column, parent)) return {};
  return createIndex(row, column);
}

QModelIndex NoteGraphModel::parent(const QModelIndex&) const { return {}; }
int NoteGraphModel::rowCount(const QModelIndex& parent) const {
  return parent.isValid() ? 0 : m_nodes.size();
}
int NoteGraphModel::columnCount(const QModelIndex&) const { return 1; }

QVariant NoteGraphModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.row() >= m_nodes.size()) return {};
  const NoteNode& node = m_nodes[index.row()];
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
      return {};
  }
}

QHash<int, QByteArray> NoteGraphModel::roleNames() const {
  return {{NameRole, "name"},
          {FilePathRole, "filePath"},
          {PositionRole, "position"},
          {IsCurrentNoteRole, "isCurrentNote"}};
}

QVariantList NoteGraphModel::getRelations() const {
  QVariantList list;
  for (const auto& rel : m_relations) {
    QVariantMap map;
    map["source"] = rel.sourceIndex;
    map["target"] = rel.targetIndex;
    list.append(map);
  }
  return list;
}

void NoteGraphModel::setNodePosition(int index, qreal x, qreal y) {
  if (index < 0 || index >= m_nodes.size()) return;
  m_nodes[index].position = QPointF(x, y);
  emit dataChanged(createIndex(index, 0), createIndex(index, 0),
                   {PositionRole});
  emit nodePositionChanged(index, x, y);
}

void NoteGraphModel::addNode(const NoteNode& node) {
  beginInsertRows({}, m_nodes.size(), m_nodes.size());
  m_nodes.append(node);
  endInsertRows();
}

void NoteGraphModel::addRelation(const NoteRelation& relation) {
  m_relations.append(relation);
}

int NoteGraphModel::findNodeIndex(const QString& filePath) const {
  for (int i = 0; i < m_nodes.size(); ++i)
    if (m_nodes[i].filePath == filePath) return i;
  return -1;
}

void NoteGraphModel::clear() {
  beginResetModel();
  m_nodes.clear();
  m_relations.clear();
  endResetModel();
  emit modelCleared();
}

// =================================================================================
// NoteRelationParser
// =================================================================================
NoteRelationParser::NoteRelationParser(QObject* parent) : QObject(parent) {
  connect(this, &NoteRelationParser::parsedDataReady, this,
          &NoteRelationParser::onParsedDataReady, Qt::QueuedConnection);
  m_cachePath = privateDir + "notegraph_cache.json";
  m_cache.load(m_cachePath);
}

int NoteRelationParser::findNodeIndex(const QVector<NoteNode>& nodes,
                                      const QString& path) {
  for (int i = 0; i < nodes.size(); ++i)
    if (nodes[i].filePath == path) return i;
  return -1;
}

// ★ 三级名称回退：缓存文本 → 笔记标题 → 文件名
QString NoteRelationParser::resolveDisplayName(const CachedLink& link,
                                               const QString& fullPath) const {
  if (!link.displayText.isEmpty()) return link.displayText;
  QString title = m_Notes->m_NoteManager->getNoteTitle(fullPath);
  if (!title.isEmpty()) return title;
  return QFileInfo(link.fileName).baseName();
}

// ---------- 核心入口 ----------
void NoteRelationParser::parseNoteRelations(NoteGraphModel* model,
                                            const QString& currentNotePath) {
  if (!model || currentNotePath.isEmpty()) return;

  m_model = model;
  model->clear();

  QString currentNoteName =
      m_Notes->m_NoteManager->getNoteTitle(currentNotePath);
  QString currentFileName = QFileInfo(currentNotePath).fileName();

  // ====== 缓存命中 ======
  if (!m_cache.isEmpty() && m_cache.forward.contains(currentFileName)) {
    QVector<NoteNode> nodes;
    QVector<NoteRelation> relations;
    nodes.append(NoteNode(currentNoteName, currentNotePath, true));

    auto addLinks = [&](const QVector<CachedLink>& links, bool isForward) {
      for (const CachedLink& link : links) {
        QString path = iniDir + "memo/" + link.fileName;
        int idx = findNodeIndex(nodes, path);
        if (idx == -1) {
          nodes.append(NoteNode(resolveDisplayName(link, path), path, false));
          idx = nodes.size() - 1;
        }
        relations.append(isForward ? NoteRelation(0, idx)
                                   : NoteRelation(idx, 0));
      }
    };

    addLinks(m_cache.forward[currentFileName], true);
    if (m_cache.backward.contains(currentFileName))
      addLinks(m_cache.backward[currentFileName], false);

    emit parsedDataReady(nodes, relations);
    return;
  }

  // ====== 缓存未命中 → 后台全量解析 ======
  QFuture<void> future = QtConcurrent::run([=]() {
    QVector<NoteNode> nodes;
    QVector<NoteRelation> relations;
    nodes.append(NoteNode(currentNoteName, currentNotePath, true));

    parseNoteReferences(nodes, relations, currentNotePath, 0);
    findReferencingNotes(nodes, relations,
                         QFileInfo(currentNotePath).absolutePath(),
                         currentNotePath, 0);

    buildCacheFromNodes(nodes, relations, currentFileName);
    emit parsedDataReady(nodes, relations);
  });

  auto* watcher = new QFutureWatcher<void>(this);
  connect(watcher, &QFutureWatcher<void>::finished, watcher,
          &QFutureWatcher<void>::deleteLater);
  watcher->setFuture(future);
}

// ---------- 解析当前笔记出链 ----------
void NoteRelationParser::parseNoteReferences(QVector<NoteNode>& nodes,
                                             QVector<NoteRelation>& relations,
                                             const QString& notePath,
                                             int sourceIndex) {
  QFile file(notePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
  QString content = file.readAll();
  file.close();

  QRegularExpressionMatchIterator it = linkRegex.globalMatch(content);
  while (it.hasNext()) {
    auto match = it.next();
    QString linkText = match.captured(1);
    QString fullLink = match.captured(2);
    QString fileName = match.captured(3);

    int targetIndex = findNodeIndex(nodes, fullLink);
    if (targetIndex == -1) {
      QString nodeName =
          linkText.isEmpty() ? QFileInfo(fileName).baseName() : linkText;
      nodes.append(NoteNode(nodeName, fullLink, false));
      targetIndex = nodes.size() - 1;
    }
    relations.append(NoteRelation(sourceIndex, targetIndex));
  }
}

// ---------- 查找反向引用 ----------
void NoteRelationParser::findReferencingNotes(QVector<NoteNode>& nodes,
                                              QVector<NoteRelation>& relations,
                                              const QString& dirPath,
                                              const QString& currentNotePath,
                                              int currentNoteIndex) {
  QString currentFileName = QFileInfo(currentNotePath).fileName();
  QDir dir(dirPath);
  if (!dir.exists()) return;

  QStringList mdFiles =
      dir.entryList({"*.md"}, QDir::Files | QDir::Readable | QDir::NoSymLinks);
  for (const QString& fileName : mdFiles) {
    QString mdFilePath = dir.filePath(fileName);
    if (mdFilePath == currentNotePath) continue;

    QFile file(mdFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) continue;
    QString content = file.readAll();
    file.close();

    QRegularExpressionMatchIterator it = linkRegex.globalMatch(content);
    while (it.hasNext()) {
      auto match = it.next();
      if (match.captured(3) == currentFileName) {
        QString noteName = m_Notes->m_NoteManager->getNoteTitle(mdFilePath);
        if (noteName.isEmpty()) noteName = QFileInfo(mdFilePath).baseName();

        int sourceIndex = findNodeIndex(nodes, mdFilePath);
        if (sourceIndex == -1) {
          nodes.append(NoteNode(noteName, mdFilePath, false));
          sourceIndex = nodes.size() - 1;
        }
        relations.append(NoteRelation(sourceIndex, currentNoteIndex));
        break;  // 同一文件只记一条反向边
      }
    }
  }

  // 递归子目录
  for (const QString& subDir : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot |
                                             QDir::Readable | QDir::NoSymLinks))
    findReferencingNotes(nodes, relations, dir.filePath(subDir),
                         currentNotePath, currentNoteIndex);
}

// ---------- 从解析结果构建缓存 ----------
void NoteRelationParser::buildCacheFromNodes(
    const QVector<NoteNode>& nodes, const QVector<NoteRelation>& relations,
    const QString& currentFileName) {
  QVector<CachedLink> outgoing, incoming;
  QSet<QString> seenOut, seenIn;

  for (const auto& rel : relations) {
    if (rel.sourceIndex == 0) {
      QString targetFile =
          QFileInfo(nodes[rel.targetIndex].filePath).fileName();
      if (targetFile != currentFileName && !seenOut.contains(targetFile)) {
        seenOut.insert(targetFile);
        outgoing.append({targetFile, nodes[rel.targetIndex].name});
      }
    }
    if (rel.targetIndex == 0) {
      QString sourceFile =
          QFileInfo(nodes[rel.sourceIndex].filePath).fileName();
      if (sourceFile != currentFileName && !seenIn.contains(sourceFile)) {
        seenIn.insert(sourceFile);
        incoming.append({sourceFile, nodes[rel.sourceIndex].name});
      }
    }
  }

  m_cache.forward[currentFileName] = outgoing;
  m_cache.backward[currentFileName] = incoming;
  m_cache.save(m_cachePath);
}

// ---------- 增量更新缓存（笔记保存后调用）----------
void NoteRelationParser::updateNoteCache(const QString& filePath) {
  QtConcurrent::run([=]() {
    QVector<NoteNode> nodes;
    QVector<NoteRelation> relations;
    nodes.append(NoteNode({}, filePath, true));
    parseNoteReferences(nodes, relations, filePath, 0);

    QString fileName = QFileInfo(filePath).fileName();

    // 构建新出链
    QVector<CachedLink> newOutgoing;
    QSet<QString> seen;
    for (const auto& rel : relations) {
      if (rel.sourceIndex == 0) {
        QString targetFile =
            QFileInfo(nodes[rel.targetIndex].filePath).fileName();
        if (targetFile != fileName && !seen.contains(targetFile)) {
          seen.insert(targetFile);
          newOutgoing.append({targetFile, nodes[rel.targetIndex].name});
        }
      }
    }

    // 替换 forward
    QVector<CachedLink> old = m_cache.forward.value(fileName);
    m_cache.forward[fileName] = newOutgoing;

    // 清理旧目标的 backward
    for (const CachedLink& link : old)
      NoteGraphCache::removeByFileName(m_cache.backward[link.fileName],
                                       fileName);

    // 添加新目标的 backward（displayText 留空，待对方解析时修正）
    for (const CachedLink& link : newOutgoing) {
      auto& bwdList = m_cache.backward[link.fileName];
      bool exists = std::any_of(
          bwdList.begin(), bwdList.end(),
          [&fileName](const CachedLink& l) { return l.fileName == fileName; });
      if (!exists) bwdList.append({fileName, {}});
    }

    m_cache.save(m_cachePath);
  });
}

// ---------- 删除笔记缓存 ----------
void NoteRelationParser::deleteNoteCache(const QString& filePath) {
  QString fileName = QFileInfo(filePath).fileName();

  QVector<CachedLink> myForward = m_cache.forward.take(fileName);
  for (const CachedLink& link : myForward)
    NoteGraphCache::removeByFileName(m_cache.backward[link.fileName], fileName);

  QVector<CachedLink> myBackward = m_cache.backward.take(fileName);
  for (const CachedLink& link : myBackward)
    NoteGraphCache::removeByFileName(m_cache.forward[link.fileName], fileName);

  m_cache.save(m_cachePath);
}

// ---------- 精确失效 ----------
void NoteRelationParser::invalidateNoteCache(const QString& filePath,
                                             CacheAction action) {
  QString fileName = QFileInfo(filePath).fileName();

  if (action == CACHE_MODIFY) {
    bool changed =
        m_cache.forward.remove(fileName) | m_cache.backward.remove(fileName);
    if (changed) {
      m_cache.save(m_cachePath);
      qDebug() << "[Graph] 缓存失效(修改):" << fileName;
    }
    return;
  }

  // DELETE
  QVector<CachedLink> oldFwd = m_cache.forward.take(fileName);
  QVector<CachedLink> oldBwd = m_cache.backward.take(fileName);

  for (const CachedLink& link : std::as_const(oldFwd))
    NoteGraphCache::removeByFileName(m_cache.backward[link.fileName], fileName);
  for (const CachedLink& link : std::as_const(oldBwd))
    NoteGraphCache::removeByFileName(m_cache.forward[link.fileName], fileName);

  if (!oldFwd.isEmpty() || !oldBwd.isEmpty()) {
    m_cache.save(m_cachePath);
    qDebug() << "[Graph] 缓存失效(删除):" << fileName
             << "| out:" << oldFwd.size() << "in:" << oldBwd.size();
  }
}

void NoteRelationParser::invalidateCache() {
  m_cache.clear();
  m_cache.save(m_cachePath);
  qDebug() << "[Graph] 全量缓存已清除";
}

// ---------- 主线程接收解析结果 ----------
void NoteRelationParser::onParsedDataReady(
    const QVector<NoteNode>& nodes, const QVector<NoteRelation>& relations) {
  if (!m_model) {
    mw_one->safeCloseProgress();
    return;
  }
  mw_one->safeCloseProgress();

  for (const auto& node : nodes) m_model->addNode(node);
  for (const auto& rel : relations) m_model->addRelation(rel);

  qDebug() << "[Graph] 节点:" << nodes.size() << "关系:" << relations.size();

  QFont font = mw_one->font();
  font.setPointSize(fontSize - 1);
  mui->lblNoteGraphView->setFont(font);
  mui->lblNoteGraphView->setText(
      tr("Nodes") + ": " + QString::number(nodes.size()) + "  " +
      tr("Relations") + ": " + QString::number(relations.size()));

  arrangeNodes(m_model);
  emit parsingCompleted();
}

// ---------- 径向布局 ----------
void NoteRelationParser::arrangeNodes(NoteGraphModel* model) {
  int currentIndex = -1;
  for (int i = 0; i < model->rowCount(); ++i) {
    if (model->data(model->index(i, 0), NoteGraphModel::IsCurrentNoteRole)
            .toBool()) {
      currentIndex = i;
      break;
    }
  }
  if (currentIndex == -1) return;

  model->setNodePosition(currentIndex, 0, 0);

  QVariantList rels = model->getRelations();
  QVector<int> referenced, referencing;
  for (const QVariant& v : rels) {
    QVariantMap r = v.toMap();
    int src = r["source"].toInt(), tgt = r["target"].toInt();
    if (src == currentIndex)
      referenced.append(tgt);
    else if (tgt == currentIndex)
      referencing.append(src);
  }

  const qreal radius = 200;
  auto placeRing = [&](const QVector<int>& indices, qreal startAngle) {
    if (indices.isEmpty()) return;
    qreal step = 2 * M_PI / qMax(1, indices.size());
    for (int i = 0; i < indices.size(); ++i) {
      qreal angle = startAngle + i * step;
      if (indices.size() == 1) angle = startAngle;
      model->setNodePosition(indices[i], radius * cos(angle),
                             radius * sin(angle));
    }
  };

  placeRing(referenced, -M_PI / 2);
  placeRing(referencing, M_PI / 2);
}

// =================================================================================
// NoteGraphController
// =================================================================================
NoteGraphController::NoteGraphController(QObject* parent) : QObject(parent) {
  m_model = new NoteGraphModel(this);
  m_parser = new NoteRelationParser(this);
  QTimer::singleShot(0, this, &NoteGraphController::modelChanged);
}

QString NoteGraphController::currentNotePath() const {
  return m_currentNotePath;
}

void NoteGraphController::setCurrentNotePath(const QString& path) {
  m_currentNotePath = path;
  emit currentNotePathChanged();
  m_parser->parseNoteRelations(m_model, m_currentNotePath);
}

NoteGraphModel* NoteGraphController::model() const { return m_model; }
NoteRelationParser* NoteGraphController::parser() const { return m_parser; }

void NoteGraphController::handleNodeDoubleClick(const QString& filePath) {
  emit nodeDoubleClicked(filePath);
}