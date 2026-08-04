#ifndef NOTEMANAGER_H
#define NOTEMANAGER_H

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
  static NoteMetadata fromJson(const QJsonObject& obj) {
    NoteMetadata metadata;
    metadata.title = obj["title"].toString();
    metadata.notebookIndex = obj["notebookIndex"].toInt(-1);
    metadata.noteIndex = obj["noteIndex"].toInt(-1);
    return metadata;
  }
};

class NoteManager : public QObject {
  Q_OBJECT
 public:
  explicit NoteManager(QObject* parent = nullptr);

  // 模糊搜索标题，返回【标题，文件路径】配对列表
  QList<QPair<QString, QString>> searchTitleWithPath(
      const QString& keyword) const;

  // 加载/保存索引（包含标题、笔记本索引、笔记索引）
  bool loadIndex(const QString& indexPath);
  bool saveIndex();

  // 标题操作（兼容原有功能）
  QString getNoteTitle(const QString& filePath) const;
  void setNoteTitle(const QString& filePath, const QString& title);

  // 通过文件路径反查完整元数据
  NoteMetadata getNoteMetadata(const QString& filePath) const;

  QStringList getAllNoteTitles() const;
  QString getFilePathByTitle(const QString& title) const;
  QStringList searchTitles(const QString& keyword) const;

  void removeNote(const QString& filePath);

  // ✅ 获取所有已索引的文件路径列表
  QStringList getAllFilePaths() const { return m_metadataMap.keys(); }

  // ✅ 批量获取元数据（避免多次加锁/查找）
  QHash<QString, NoteMetadata> getAllMetadata() const { return m_metadataMap; }

 signals:
  void noteMetaChanged(const QString& filePath, const NoteMetadata& meta);
  void noteRemoved(const QString& filePath);
  void indexReloaded();  // 索引整体重载完成

 private:
  // 路径→元数据的映射（核心索引表）
  QHash<QString, NoteMetadata> m_metadataMap;
  QString m_currentIndexPath;  // 当前索引文件路径

  // 工具方法：标准化文件路径（确保一致性）
  QString normalizePath(const QString& filePath) const {
    return QDir::cleanPath(filePath);
  }
};

#endif  // NOTEMANAGER_H
