#ifndef NOTEGRAPH_H
#define NOTEGRAPH_H

#include <QJsonObject>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QVector>

// =================================================================================
// 1. 缓存数据结构 (纯数据，无 UI 依赖)
// =================================================================================
struct CachedLink {
  QString fileName;     // 文件名 (如 "project_a.md")
  QString displayText;  // 链接显示文本 (如 "项目A计划")

  QJsonObject toJson() const;
  static CachedLink fromJson(const QJsonObject& obj);
};

class NoteGraphCache {
 public:
  QMap<QString, QVector<CachedLink>> forward;   // 我引用谁 (出链)
  QMap<QString, QVector<CachedLink>> backward;  // 谁引用我 (入链)

  void load(const QString& filePath);
  void save(const QString& filePath) const;
  bool isEmpty() const;
  void clear();
  static void removeByFileName(QVector<CachedLink>& links,
                               const QString& fileName);
};

// =================================================================================
// 2. 图谱解析引擎 (核心：只产出 JSON 字符串)
// =================================================================================
class NoteRelationParser : public QObject {
  Q_OBJECT
 public:
  enum CacheAction : int { CACHE_DELETE = 0, CACHE_MODIFY = 1 };

  explicit NoteRelationParser(QObject* parent = nullptr);

  // ★ 核心接口：生成跨平台中性 JSON 字符串
  // 返回格式: { "current": "memo/xxx.md", "links": [ { "file": "...", "title":
  // "...", "dir": "out|in|both" } ] }
  QString generateGraphJson(const QString& currentNotePath);

  // 缓存管理接口 (供外部笔记保存/删除时调用)
  void updateNoteCache(const QString& filePath);
  void deleteNoteCache(const QString& filePath);
  void invalidateNoteCache(const QString& filePath, CacheAction action);
  void invalidateCache();

 signals:
  void graphJsonReady(const QString& jsonData);

 private:
  // 内部解析逻辑
  void parseOutLinks(const QString& notePath,
                     QMap<QString, QPair<QString, QString>>& linkMap);
  void parseInLinks(const QString& dirPath, const QString& currentNotePath,
                    const QString& currentFileName,
                    QMap<QString, QPair<QString, QString>>& linkMap);

  // 缓存构建
  void buildCacheFromMap(const QMap<QString, QPair<QString, QString>>& linkMap,
                         const QString& currentFileName);

  // 工具方法
  QString resolveDisplayName(const QString& linkText, const QString& fullPath,
                             const QString& fileName) const;

  NoteGraphCache m_cache;
  QString m_cachePath;
  mutable QMutex m_cacheMutex;
};

// =================================================================================
// 3. 控制器 (极简：仅负责触发和传递数据)
// =================================================================================
class NoteGraphController : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString currentNotePath READ currentNotePath WRITE
                 setCurrentNotePath NOTIFY currentNotePathChanged)
  Q_PROPERTY(QString graphJson READ graphJson NOTIFY graphJsonChanged)

 public:
  explicit NoteGraphController(QObject* parent = nullptr);

  QString currentNotePath() const;
  void setCurrentNotePath(const QString& path);

  QString graphJson() const;  // ★ 供 QML 或 C++ 直接读取
  NoteRelationParser* parser() const;

 signals:
  void currentNotePathChanged();
  void graphJsonChanged();                          // ★ 数据就绪信号
  void nodeDoubleClicked(const QString& filePath);  // 保留供桌面端使用

 public slots:
  void handleNodeDoubleClick(const QString& filePath);

 private slots:
  void onGraphJsonReady(const QString& jsonData);

 private:
  QString m_currentNotePath;
  QString m_graphJson;
  NoteRelationParser* m_parser;
};

#endif  // NOTEGRAPH_H