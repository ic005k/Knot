#include "src/Notes/Notes.h"

bool NoteIndexManager1::loadIndex(const QString& indexPath) {
  QFile file(indexPath);
  if (!file.open(QIODevice::ReadOnly)) return false;
  QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
  QJsonObject root = doc.object();
  QJsonObject data = root["data"].toObject();
  m_index.clear();
  for (auto it = data.begin(); it != data.end(); ++it) {
    m_index.insert(it.key(), it.value().toString());
  }
  m_currentIndexPath = indexPath;
  return true;
}

bool NoteIndexManager1::saveIndex(const QString& indexPath) {
  QJsonObject root;
  root["version"] = 1.0;
  QJsonObject data;
  for (auto it = m_index.constBegin(); it != m_index.constEnd(); ++it) {
    data.insert(it.key(), it.value());
  }
  root["data"] = data;
  QFile file(indexPath);
  if (!file.open(QIODevice::WriteOnly)) return false;
  file.write(QJsonDocument(root).toJson());
  return true;
}

QString NoteIndexManager1::getNoteTitle(const QString& filePath) const {
  return m_index.value(QDir::cleanPath(filePath),
                       QFileInfo(filePath).baseName());
}

void NoteIndexManager1::setNoteTitle(const QString& filePath,
                                     const QString& title) {
  QString cleanPath = QDir::cleanPath(filePath);
  if (title.isEmpty())
    m_index.remove(cleanPath);
  else
    m_index[cleanPath] = title;
}