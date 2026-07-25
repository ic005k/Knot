#ifndef NOTESLIST_H
#define NOTESLIST_H

#include <QApplication>
#include <QDialog>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfoList>
#include <QFutureWatcher>
#include <QInputMethod>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMetaObject>
#include <QMutex>
#include <QQmlContext>
#include <QQuickWidget>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QStringList>
#include <QTextStream>
#include <QTreeWidgetItem>
#include <QVariantList>
#include <QtConcurrent/QtConcurrent>
#include <QtConcurrent/QtConcurrentRun>
#include <algorithm>
#include <utility>

#include "VectorSearchService.h"
#include "database_manager.h"
#include "qtreewidgetproxymodel.h"
#include "search_model.h"
#include "src/AI/GlobalAI.h"
#include "src/Comm/TextEditToolbar.h"
#include "src/MainWindow.h"
#include "src/Notes/MoveTo.h"
#include "src/Notes/NoteListModel.h"
#include "src/Notes/note_graph.h"
#include "src/defines.h"
#include "ui_MainWindow.h"
#include "ui_MoveTo.h"
#include "ui_NotesList.h"

struct MySearchResult {
  QString filePath;
  QList<int> lineNumbers;
};
using ResultsMap = QMap<QString, MySearchResult>;
MySearchResult searchInFile(const QString& filePath,
                            const QRegularExpression& regex);
QStringList findMarkdownFiles(const QString& dirPath);
void reduceResults(ResultsMap& result, const MySearchResult& partial);

struct ExactMatchResult {
  QString filePath;
  QString title;
  QString preview;            // 已清洗、截断的上下文摘要
  int lineNumber;             // 首次匹配行号（用于跳转）
  int matchCount;             // 匹配总数
  QList<int> allLineNumbers;  // 所有匹配行号（用于文件内上下条跳转）
};

namespace Ui {
class NotesList;
}

class NotesList : public QDialog {
  Q_OBJECT

 public:
  explicit NotesList(QWidget* parent = nullptr);
  ~NotesList();
  Ui::NotesList* ui;

  QVector<SearchResult> adaptedResults;

  VectorSearchService* m_vectorSearchService = nullptr;

  QPointer<QMenu> menuNoteBook;
  QPointer<QMenu> menuNoteList;
  QPointer<QMenu> menuRecentOpen;

  bool isAINoteRename = false;

  NoteGraphController* m_graphController;  // 图谱控制器
  QDialog* m_RenameNotes = nullptr;

  MoveTo* m_MoveTo = nullptr;

  void startBackgroundTaskUpdateFilesIndex();

  QString noteTitle;

  void saveNotesListIndex();

  NoteListModel* noteModel;

  QStringList searchResultList;

  QStringList recycleNotesList;

  QString getCurrentNoteNameFromMDFile(QString mdFile);

  QStringList listRecentOpen;
  QList<QTreeWidgetItem*> pNoteBookItems;
  QList<QTreeWidgetItem*> pNoteItems;
  QStringList findResult;
  int findCount;
  QList<QTreeWidgetItem*> findResultList;

  void set_memo_dir();

  bool delFile(QString file);

  void saveNotesList();
  void initNotesList();

  void setWinPos();

  void addItem(QTreeWidget* tw, QTreeWidgetItem* item);

  void initRecycle();
  QStringList needDelFiles;
  bool isDelNoteRecycle = false;
  void clearFiles();
  void getAllFiles(const QString& foldPath, QStringList& folds,
                   const QStringList& formats);

  void setNoteName(QString name);
  void moveBy(int ud);

  QString getCurrentMDFile();
  void init_NotesListMenu(QMenu* mainMenu);
  void init_NoteBookMenu(QMenu* mainMenu);

  int getNoteBookCurrentIndex();
  int getNotesListCurrentIndex();
  void setNoteBookCurrentIndex(int index);
  void setNotesListCurrentIndex(int index);
  int getNoteBookCount();
  int getNotesListCount();
  void loadAllNoteBook();

  void localItem();

  void modifyNoteBookText0(QString text0, int index);
  void modifyNotesListText0(QString text0, int index);
  QString getNoteBookText0(int index);
  QString getNotesListText0(int index);
  void setNoteLabel();

  void setNoteBookCurrentItem();

  void startFind(QString strFind);

  int getNoteBookIndex_twToqml();

  void loadAllRecycle();

  void resetQML_List();

  QVariant addQmlTreeTopItem(QString strItem);
  QVariant addQmlTreeChildItem(QVariant parentItem, QString strChildItem,
                               QString iconFile);
  void initQmlTree();

  void clearQmlTree();
  void resetQML_Recycle();
  void setTWCurrentItem();
  void setTWRBCurrentItem();

  void saveRecentOpen();
  void initRecentOpen();
  void saveCurrentNoteInfo();
  void genRecentOpenMenu();

  void genCursorText();
  void renameCurrentItem(QString title);
  bool setCurrentItemFromMDFile(QString mdFile);
  QStringList extractLocalImagesFromMarkdown(const QString& filePath);

  QString getSearchResultQmlFile();

  QStringList getValidMDFiles();

  void refreshRecentOpen(QString name);

  void showFindNotes();
  void restoreNoteFromRecycle();
  void needDelNotes();

  void updateNoteIndexManager(QString mdFile, int notebookIndex, int noteIndex);
  void updateAllNoteIndexManager();

  void moveToFirst();

  void initUnclassified();

  void startBackgroundTaskDelFilesIndex(const QStringList& files);
  QStringList getRecycleNoteFiles();

  void on_btnNewNote_clicked();

  void on_btnBack_clicked();

  void closeNoteDiff();

  void on_actionAdd_Note_triggered();

  void on_btnBatchDel_Recycle_clicked();

  void on_btnBatchRestore_clicked();

  void delRemoteWebDAVFiles();

  void on_btnRename_clicked();

  QStringList getAllNotePaths();

  void initVectorSearchService();

  void rebuilderNotesVector();

  void cancelRebuildNotesVector();

  bool waitForRebuildFinished(int timeoutMs);
  void safeExitLlama();

 protected:
  bool eventFilter(QObject* watch, QEvent* evn) override;

  void closeEvent(QCloseEvent* event) override;

 public slots:
  void showNoteBookMenu(int x, int y);
  void showNotsListMenu(int x, int y);

  void mouseClickNoteBook();

  void on_actionCopyNoteLink();

  void clickNoteList();

  void clickNoteBook();

  void on_btnClose_clicked();

  void on_btnDel_clicked();

  int on_btnImport_clicked();

  void on_btnExport_clicked();

  void on_btnRecycle();

  void on_btnRestore_clicked();

  void on_btnDel_Recycle_clicked();

  void on_btnUp_clicked();

  void on_btnDown_clicked();

  void on_btnMoveTo_clicked();

  void on_actionRelationshipGraph();

  void qmlOpenEdit();

  void getNoteDiffHtml();
  void newtextToOldtextFromDiffStr();

  void slotCreateSubNotebook(int qmlIndex);
  void show_NoteBookPopMenu(int qmlIndex);
  void mouseClickNoteList();

 private slots:

  void on_actionShareNoteFile();

  void onSearchFinished();

  void onSearchTextChanged(const QString& text);

  void on_actionSetColorFlag();

  void on_actionStatistics();

  void onNoteNodeDoubleClicked(const QString& filePath);

  void on_actionModificationHistory();

 signals:
  void rebuildProgressChanged(int current, int total);

 private:
  // 精准搜索导航缓存（与 m_searchModel 平行存在）
  QVector<ExactMatchResult> m_exactMatchCache;
  int m_currentExactMatchIndex = -1;

  bool isReadyNotesEnd = false;
  bool isExecRecentOpen = false;

  std::atomic<bool> m_rebuildCancelled{false};
  QMutex m_rebuildMutex;  // 防止重复触发重建

  // 递归遍历 QTreeWidget 节点，填充到 QML + 维护映射表
  void traverseTreeItem(QTreeWidgetItem* item, int parentQmlIndex, int level);

  void addItemToQW_Level(QObject* qmlRoot, const QString& t0, const QString& t1,
                         const QString& t2, const QString& t3,
                         const QString& t4, int fontSize, int level,
                         int parentIndex, bool isExpand = true);

  QTreeWidgetProxyModel* m_treeProxyModel = nullptr;

  QMutex m_saveMutex;       // 保存锁
  bool m_isSaving = false;  // 保存状态

  QStringList noteDiffTime, noteDiffHtml, noteDiffPatch;

  QString notebookName;

  QString noteName;

  QStringList noteFiles, recycleFiles;

  bool isImportFilesEnd;

  bool isImportNotes = false;

  bool isMouseClick = false;

  bool isMouseClickNoteBook = false;

  bool isActColorFlagStatus = false;

  QStringList validMDFiles;

  QStringList mIndexList;

  QInputMethod* pAndroidKeyboard = QApplication::inputMethod();

  QStringList knot_all_files;

  void clearMD_Pic();

  void removeFromFiles(QString str);

  void on_actionAdd_NoteBook_triggered();
  void on_actionDel_NoteBook_triggered();
  void on_actionRename_NoteBook_triggered();
  void on_actionMoveUp_NoteBook_triggered();
  void on_actionMoveDown_NoteBook_triggered();

  void on_actionDel_Note_triggered();
  void on_actionRename_Note_triggered();
  void on_actionMoveUp_Note_triggered();
  void on_actionMoveDown_Note_triggered();
  void on_actionImport_Note_triggered();
  void on_actionExport_Note_triggered();

  bool moveItem(QTreeWidget* tw);

  QFutureWatcher<QVector<ExactMatchResult>>* watcher = nullptr;

  QDateTime m_lastIndexTime;  // 记录最后一次索引构建时间
  QMutex m_indexTimeMutex;    // 互斥锁

  bool m_isIndexing = false;  // 标记索引状态

  SearchModel m_searchModel;

  int getSavedNotesListIndex(int notebookIndex);
  bool safeWriteFile(const QString& filePath, const QString& content);
  void loadNotesListIndex();
  void addItemToQW(QQuickWidget* qw, QString text0, QString text1,
                   QString text2, QString text3, QString text4, int itemH);
  void setColorFlag(QString strColor);
  void setDelNoteFlag(QString mdFile);
  void saveNotesListToFile();
  // 递归序列化笔记本节点（包含自身笔记 + 下级子笔记本）
  QJsonObject serializeNotebookItem(QTreeWidgetItem* item);

  void initNoteGraphView();
  void readyNotesData(QTreeWidgetItem* topItem);

  int getSelectedVersionIndex();
  void setNoteDiffHtmlToQML(const QString& html);

  QFuture<QVector<ExactMatchResult>> performSearchAsync(const QString& dirPath,
                                                        const QString& keyword);

  int countMdFilesImages(const QString& dirPath);
  void loadSubNotebook(const QJsonObject& bookObj, QTreeWidgetItem* parentItem,
                       int parentRow, int& totalNotes);
  void moveChildToRecycle(QTreeWidgetItem* parentItem, QString iniDir,
                          QVector<QString>& delFilesIndex, QTreeWidget* twrb);
};

class SearchMapper {
 public:
  using result_type = MySearchResult;  // 必须声明result_type

  explicit SearchMapper(const QRegularExpression& regex) : m_regex(regex) {}

  MySearchResult operator()(const QString& filePath) const {
    return searchInFile(filePath, m_regex);
  }

 private:
  QRegularExpression m_regex;
};

#endif  // NOTESLIST_H
