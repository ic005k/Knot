#ifndef NOTE_INDEX_MANAGER_H
#define NOTE_INDEX_MANAGER_H

#include <QDir>
#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QString>

// 存储笔记的元数据：标题、所在笔记本索引、在笔记本中的索引
struct NoteMetadata {
  QString title;           // 笔记标题
  int notebookIndex = -1;  // 所在笔记本的索引（-1表示未关联）
  int noteIndex = -1;      // 在笔记本中的索引（-1表示未关联）

  // 转换为JSON对象（用于持久化）
  QJsonObject toJson() const {
    QJsonObject obj;
    obj["title"] = title;
    obj["notebookIndex"] = notebookIndex;
    obj["noteIndex"] = noteIndex;
    return obj;
  }

  // 从JSON对象恢复
  static NoteMetadata fromJson(const QJsonObject &obj) {
    NoteMetadata metadata;
    metadata.title = obj["title"].toString();
    metadata.notebookIndex = obj["notebookIndex"].toInt(-1);
    metadata.noteIndex = obj["noteIndex"].toInt(-1);
    return metadata;
  }
};

class NoteIndexManager : public QObject {
  Q_OBJECT
 public:
  explicit NoteIndexManager(QObject *parent = nullptr);

  // 加载/保存索引（包含标题、笔记本索引、笔记索引）
  bool loadIndex(const QString &indexPath);
  bool saveIndex(const QString &indexPath);

  // 标题操作（兼容原有功能）
  QString getNoteTitle(const QString &filePath) const;
  void setNoteTitle(const QString &filePath, const QString &title);

  // 笔记本索引操作
  int getNotebookIndex(const QString &filePath) const;
  void setNotebookIndex(const QString &filePath, int notebookIndex);

  // 笔记在笔记本中的索引操作
  int getNoteIndex(const QString &filePath) const;
  void setNoteIndex(const QString &filePath, int noteIndex);

  // 当前激活的索引（全局状态）
  int currentNotebookIndex() const { return m_currentNotebookIndex; }
  int currentNoteIndex() const { return m_currentNoteIndex; }
  void setCurrentIndexes(int notebookIndex, int noteIndex);

  // 通过文件路径反查完整元数据
  NoteMetadata getNoteMetadata(const QString &filePath) const;

 private:
  // 路径→元数据的映射（核心索引表）
  QHash<QString, NoteMetadata> m_metadataMap;
  QString m_currentIndexPath;  // 当前索引文件路径

  // 当前激活的索引（全局状态）
  int m_currentNotebookIndex = -1;
  int m_currentNoteIndex = -1;

  // 工具方法：标准化文件路径（确保一致性）
  QString normalizePath(const QString &filePath) const {
    return QDir::cleanPath(filePath);
  }
};

#endif  // NOTE_INDEX_MANAGER_H
