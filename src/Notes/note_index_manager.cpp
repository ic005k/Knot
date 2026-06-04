#include "note_index_manager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

NoteIndexManager::NoteIndexManager(QObject* parent) : QObject(parent) {}

bool NoteIndexManager::loadIndex(const QString& indexPath) {
  QFile file(indexPath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return false;
  }

  QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
  if (doc.isNull()) {
    return false;
  }

  QJsonObject root = doc.object();
  // 检查版本（可选，用于后续兼容）
  if (root["version"].toDouble() != 1.0) {
    return false;
  }

  QJsonObject data = root["data"].toObject();
  m_metadataMap.clear();

  // 从JSON恢复元数据
  for (auto it = data.begin(); it != data.end(); ++it) {
    QString filePath = it.key();
    NoteMetadata metadata = NoteMetadata::fromJson(it.value().toObject());
    m_metadataMap.insert(normalizePath(filePath), metadata);
  }

  m_currentIndexPath = indexPath;
  return true;
}

bool NoteIndexManager::saveIndex(const QString& indexPath) {
  QJsonObject root;
  root["version"] = 1.0;

  QJsonObject data;
  // 将元数据写入JSON
  for (auto it = m_metadataMap.begin(); it != m_metadataMap.end(); ++it) {
    data.insert(it.key(), it.value().toJson());
  }
  root["data"] = data;

  QFile file(indexPath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    return false;
  }

  file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
  m_currentIndexPath = indexPath;
  return true;
}

QString NoteIndexManager::getNoteTitle(const QString& filePath) const {
  QString normalized = normalizePath(filePath);
  if (m_metadataMap.contains(normalized)) {
    return m_metadataMap[normalized].title;
  }
  // 未找到时返回文件名作为默认标题
  return QFileInfo(filePath).baseName();
}

void NoteIndexManager::setNoteTitle(const QString& filePath,
                                    const QString& title) {
  QString normalized = normalizePath(filePath);
  NoteMetadata metadata =
      m_metadataMap.value(normalized);  // 不存在则返回默认值
  metadata.title = title;
  m_metadataMap[normalized] = metadata;
}

int NoteIndexManager::getNotebookIndex(const QString& filePath) const {
  QString normalized = normalizePath(filePath);
  return m_metadataMap.value(normalized).notebookIndex;
}

void NoteIndexManager::setNotebookIndex(const QString& filePath,
                                        int notebookIndex) {
  QString normalized = normalizePath(filePath);
  NoteMetadata metadata = m_metadataMap.value(normalized);
  metadata.notebookIndex = notebookIndex;
  m_metadataMap[normalized] = metadata;
}

int NoteIndexManager::getNoteIndex(const QString& filePath) const {
  QString normalized = normalizePath(filePath);
  return m_metadataMap.value(normalized).noteIndex;
}

void NoteIndexManager::setNoteIndex(const QString& filePath, int noteIndex) {
  QString normalized = normalizePath(filePath);
  NoteMetadata metadata = m_metadataMap.value(normalized);
  metadata.noteIndex = noteIndex;
  m_metadataMap[normalized] = metadata;
}

void NoteIndexManager::setCurrentIndexes(int notebookIndex, int noteIndex) {
  m_currentNotebookIndex = notebookIndex;
  m_currentNoteIndex = noteIndex;
  // 可在此处发射信号，通知UI更新当前选中状态
  // emit currentIndexesChanged(notebookIndex, noteIndex);
}

NoteMetadata NoteIndexManager::getNoteMetadata(const QString& filePath) const {
  return m_metadataMap.value(normalizePath(filePath));
}

// 获取所有笔记的标题（用于补全弹窗列表）
QStringList NoteIndexManager::getAllNoteTitles() const {
  QStringList titles;
  for (auto it = m_metadataMap.begin(); it != m_metadataMap.end(); ++it) {
    titles.append(it.value().title);
  }
  return titles;
}

// 根据标题 → 返回对应的笔记路径
QString NoteIndexManager::getFilePathByTitle(const QString& title) const {
  for (auto it = m_metadataMap.begin(); it != m_metadataMap.end(); ++it) {
    if (it.value().title == title) {
      return it.key();  // 返回完整路径
    }
  }
  return "";
}

// 输入关键词 → 返回所有匹配的标题列表（自动补全用）
QStringList NoteIndexManager::searchTitles(const QString& keyword) const {
  QStringList result;
  for (auto it = m_metadataMap.begin(); it != m_metadataMap.end(); ++it) {
    if (it.value().title.contains(keyword, Qt::CaseInsensitive)) {
      result.append(it.value().title);
    }
  }
  return result;
}
