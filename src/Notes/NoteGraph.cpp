#include "NoteGraph.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QtConcurrent>

#include "defines.h"
#include "src/MainWindow.h"

// 链接正则：[显示文本](memo/文件名.md)
static const QRegularExpression linkRegex(
    R"(\[(.*?)\]\((memo\/([^)]+\.md))\))");

// =================================================================================
// CachedLink & NoteGraphCache (保持原有高效逻辑)
// =================================================================================
QJsonObject CachedLink::toJson() const {
  QJsonObject obj;
  obj["f"] = fileName;
  obj["t"] = displayText;
  return obj;
}

CachedLink CachedLink::fromJson(const QJsonObject& obj) {
  return {obj["f"].toString(), obj["t"].toString()};
}

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
  if (!file.open(QIODevice::ReadOnly)) return;
  QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();
  file.close();

  auto loadMap = [](const QJsonObject& obj) {
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
  auto saveMap = [](const QMap<QString, QVector<CachedLink>>& map) {
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
  if (!file.open(QIODevice::WriteOnly)) return;
  file.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
  file.close();
}

// =================================================================================
// NoteRelationParser (重构核心：直接生成 JSON)
// =================================================================================
NoteRelationParser::NoteRelationParser(QObject* parent) : QObject(parent) {
  m_cachePath = privateDir + "notegraph_cache.json";
  m_cache.load(m_cachePath);
}

QString NoteRelationParser::resolveDisplayName(const QString& linkText,
                                               const QString& fullPath,
                                               const QString& fileName) const {
  if (!linkText.isEmpty() && linkText != fileName) return linkText;
  QString title = m_Notes->m_NoteManager->getNoteTitle(fullPath);
  if (!title.isEmpty()) return title;
  return QFileInfo(fileName).baseName();
}

// ★ 核心入口：生成中性 JSON 字符串
QString NoteRelationParser::generateGraphJson(const QString& currentNotePath) {
  if (currentNotePath.isEmpty()) return "{}";

  QString currentFileName = QFileInfo(currentNotePath).fileName();

  // 使用 Map 自动去重，Key: 文件名, Value: <显示标题, 方向("out"/"in"/"both")>
  QMap<QString, QPair<QString, QString>> linkMap;

  QMutexLocker locker(&m_cacheMutex);

  // 1. 尝试从缓存快速构建
  bool cacheHit =
      !m_cache.isEmpty() && m_cache.forward.contains(currentFileName);

  if (cacheHit) {
    for (const auto& link : m_cache.forward.value(currentFileName)) {
      // ★ 缓存中的标题可能为空，需要回退解析
      QString displayText = link.displayText;
      if (displayText.isEmpty()) {
        QString fullPath = iniDir + "memo/" + link.fileName;
        displayText = resolveDisplayName("", fullPath, link.fileName);
      }
      linkMap[link.fileName] = {displayText, "out"};
    }
    for (const auto& link : m_cache.backward.value(currentFileName)) {
      QString displayText = link.displayText;
      if (displayText.isEmpty()) {
        QString fullPath = iniDir + "memo/" + link.fileName;
        displayText = resolveDisplayName("", fullPath, link.fileName);
      }

      if (linkMap.contains(link.fileName)) {
        linkMap[link.fileName].second = "both";
      } else {
        linkMap[link.fileName] = {displayText, "in"};
      }
    }
  }
  // 2. 缓存未命中，后台全量解析 (这里为了简化直接同步返回，若需异步可改用信号)
  else {
    parseOutLinks(currentNotePath, linkMap);
    parseInLinks(QFileInfo(currentNotePath).absolutePath(), currentNotePath,
                 currentFileName, linkMap);
    buildCacheFromMap(linkMap, currentFileName);
  }

  // 3. 组装 JSON
  QJsonObject root;
  // current 使用相对路径，方便跨平台识别
  root["current"] = "memo/" + currentFileName;

  QJsonArray linksArray;
  for (auto it = linkMap.begin(); it != linkMap.end(); ++it) {
    QJsonObject linkObj;
    linkObj["file"] = "memo/" + it.key();
    linkObj["title"] = it.value().first;
    linkObj["dir"] = it.value().second;
    linksArray.append(linkObj);
  }
  root["links"] = linksArray;

  QString jsonData = QJsonDocument(root).toJson(QJsonDocument::Compact);
  emit graphJsonReady(jsonData);
  return jsonData;
}

// ---------- 解析出链 ----------
void NoteRelationParser::parseOutLinks(
    const QString& notePath, QMap<QString, QPair<QString, QString>>& linkMap) {
  QFile file(notePath);
  if (!file.open(QIODevice::ReadOnly)) return;
  QString content = file.readAll();
  file.close();

  auto it = linkRegex.globalMatch(content);
  while (it.hasNext()) {
    auto match = it.next();
    QString linkText = match.captured(1);
    QString fileName = match.captured(3);
    QString fullPath = iniDir + match.captured(2);

    if (!linkMap.contains(fileName)) {
      linkMap[fileName] = {resolveDisplayName(linkText, fullPath, fileName),
                           "out"};
    }
  }
}

// ---------- 解析入链 ----------
void NoteRelationParser::parseInLinks(
    const QString& dirPath, const QString& currentNotePath,
    const QString& currentFileName,
    QMap<QString, QPair<QString, QString>>& linkMap) {
  QDir dir(dirPath);
  if (!dir.exists()) return;

  for (const QString& fileName : dir.entryList({"*.md"}, QDir::Files)) {
    QString mdFilePath = dir.filePath(fileName);
    if (mdFilePath == currentNotePath) continue;

    QFile file(mdFilePath);
    if (!file.open(QIODevice::ReadOnly)) continue;
    QString content = file.readAll();
    file.close();

    auto it = linkRegex.globalMatch(content);
    while (it.hasNext()) {
      if (it.next().captured(3) == currentFileName) {
        QString title = m_Notes->m_NoteManager->getNoteTitle(mdFilePath);
        if (title.isEmpty()) title = QFileInfo(fileName).baseName();

        if (linkMap.contains(fileName)) {
          linkMap[fileName].second = "both";
        } else {
          linkMap[fileName] = {title, "in"};
        }
        break;
      }
    }
  }

  for (const QString& subDir : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
    parseInLinks(dir.filePath(subDir), currentNotePath, currentFileName,
                 linkMap);
}

// ---------- 构建缓存 ----------
void NoteRelationParser::buildCacheFromMap(
    const QMap<QString, QPair<QString, QString>>& linkMap,
    const QString& currentFileName) {
  QVector<CachedLink> outgoing, incoming;
  for (auto it = linkMap.begin(); it != linkMap.end(); ++it) {
    if (it.value().second == "out" || it.value().second == "both")
      outgoing.append({it.key(), it.value().first});
    if (it.value().second == "in" || it.value().second == "both")
      incoming.append({it.key(), it.value().first});
  }
  m_cache.forward[currentFileName] = outgoing;
  m_cache.backward[currentFileName] = incoming;
  m_cache.save(m_cachePath);
}

// ---------- 缓存管理 (保持原有逻辑，略作精简) ----------
void NoteRelationParser::updateNoteCache(const QString& filePath) {
  QPointer<NoteRelationParser> safeThis(this);  // ★ 安全守卫

  // QtConcurrent::run([safeThis, filePath]() {
  QThreadPool::globalInstance()->start([safeThis, filePath]() {
    // 子线程只做纯文本正则匹配
    QMap<QString, QString> rawLinks;
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
      QString content = file.readAll();
      file.close();
      auto it = linkRegex.globalMatch(content);
      while (it.hasNext()) {
        auto match = it.next();
        rawLinks[match.captured(3)] = match.captured(1);
      }
    }

    // ★ 回到主线程前检查对象是否还活着
    if (!safeThis) return;

    QMetaObject::invokeMethod(
        safeThis.data(),  // 使用安全指针
        [safeThis, filePath, rawLinks]() {
          if (!safeThis) return;  // ★ 双重检查

          QString fileName = QFileInfo(filePath).fileName();
          QMutexLocker locker(&safeThis->m_cacheMutex);

          QVector<CachedLink> old = safeThis->m_cache.forward.take(fileName);
          QVector<CachedLink> newOutgoing;

          for (auto it = rawLinks.begin(); it != rawLinks.end(); ++it) {
            QString fullPath = iniDir + "memo/" + it.key();
            QString title =
                safeThis->resolveDisplayName(it.value(), fullPath, it.key());
            newOutgoing.append({it.key(), title});
          }

          safeThis->m_cache.forward[fileName] = newOutgoing;

          for (const CachedLink& link : old)
            NoteGraphCache::removeByFileName(
                safeThis->m_cache.backward[link.fileName], fileName);
          for (const CachedLink& link : newOutgoing) {
            auto& bwdList = safeThis->m_cache.backward[link.fileName];
            if (!std::any_of(bwdList.begin(), bwdList.end(),
                             [&fileName](const CachedLink& l) {
                               return l.fileName == fileName;
                             }))
              bwdList.append({fileName, {}});
          }
          safeThis->m_cache.save(safeThis->m_cachePath);
        },
        Qt::QueuedConnection);
  });
}

void NoteRelationParser::deleteNoteCache(const QString& filePath) {
  QString fileName = QFileInfo(filePath).fileName();
  QMutexLocker locker(&m_cacheMutex);
  QVector<CachedLink> myForward = m_cache.forward.take(fileName);
  for (const CachedLink& link : myForward)
    NoteGraphCache::removeByFileName(m_cache.backward[link.fileName], fileName);
  QVector<CachedLink> myBackward = m_cache.backward.take(fileName);
  for (const CachedLink& link : myBackward)
    NoteGraphCache::removeByFileName(m_cache.forward[link.fileName], fileName);
  m_cache.save(m_cachePath);
}

void NoteRelationParser::invalidateNoteCache(const QString& filePath,
                                             CacheAction action) {
  QString fileName = QFileInfo(filePath).fileName();
  QMutexLocker locker(&m_cacheMutex);
  if (action == CACHE_MODIFY) {
    if (m_cache.forward.remove(fileName) | m_cache.backward.remove(fileName))
      m_cache.save(m_cachePath);
    return;
  }
  QVector<CachedLink> oldFwd = m_cache.forward.take(fileName);
  QVector<CachedLink> oldBwd = m_cache.backward.take(fileName);
  for (const CachedLink& link : std::as_const(oldFwd))
    NoteGraphCache::removeByFileName(m_cache.backward[link.fileName], fileName);
  for (const CachedLink& link : std::as_const(oldBwd))
    NoteGraphCache::removeByFileName(m_cache.forward[link.fileName], fileName);
  if (!oldFwd.isEmpty() || !oldBwd.isEmpty()) m_cache.save(m_cachePath);
}

void NoteRelationParser::invalidateCache() {
  QMutexLocker locker(&m_cacheMutex);
  m_cache.clear();
  m_cache.save(m_cachePath);
}

// =================================================================================
// NoteGraphController (极简控制器)
// =================================================================================
NoteGraphController::NoteGraphController(QObject* parent) : QObject(parent) {
  m_parser = new NoteRelationParser(this);
  connect(m_parser, &NoteRelationParser::graphJsonReady, this,
          &NoteGraphController::onGraphJsonReady);
}

QString NoteGraphController::currentNotePath() const {
  return m_currentNotePath;
}

void NoteGraphController::setCurrentNotePath(const QString& path) {
  m_currentNotePath = path;
  emit currentNotePathChanged();

  // ★ 确保解析始终在主线程执行，避免与 invokeMethod 回调产生竞态
  if (QThread::currentThread() == m_parser->thread()) {
    m_parser->generateGraphJson(m_currentNotePath);
  } else {
    QMetaObject::invokeMethod(
        m_parser, [this]() { m_parser->generateGraphJson(m_currentNotePath); },
        Qt::QueuedConnection);
  }
}

QString NoteGraphController::graphJson() const { return m_graphJson; }
NoteRelationParser* NoteGraphController::parser() const { return m_parser; }

void NoteGraphController::handleNodeDoubleClick(const QString& filePath) {
  emit nodeDoubleClicked(filePath);
}

void NoteGraphController::onGraphJsonReady(const QString& jsonData) {
  m_graphJson = jsonData;
  emit graphJsonChanged();
}