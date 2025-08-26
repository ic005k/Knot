#ifndef NOTESLIST_H
#define NOTESLIST_H

#include <QApplication>
#include <QDialog>
#include <QDirIterator>
#include <QFile>
#include <QFutureWatcher>
#include <QInputMethod>
#include <QKeyEvent>
#include <QListWidget>
#include <QListWidgetItem>
#include <QQmlContext>
#include <QQuickWidget>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QStringList>
#include <QTextStream>
#include <QTreeWidgetItem>
#include <QtConcurrent/QtConcurrent>
#include <QtConcurrent/QtConcurrentRun>

#include "database_manager.h"
#include "search_model.h"
#include "src/Comm/TextEditToolbar.h"
#include "src/Notes/MoveTo.h"
#include "src/Notes/NewNoteBook.h"
#include "src/Notes/NoteListModel.h"
#include "src/Notes/note_graph.h"
#include "ui_MoveTo.h"

struct MySearchResult {
  QString filePath;
  QList<int> lineNumbers;
};
using ResultsMap = QMap<QString, MySearchResult>;
MySearchResult searchInFile(const QString &filePath,
                            const QRegularExpression &regex);
QStringList findMarkdownFiles(const QString &dirPath);
void reduceResults(ResultsMap &result, const MySearchResult &partial);
QFuture<ResultsMap> performSearchAsync(const QString &dirPath,
                                       const QString &keyword);
void displayResults(const ResultsMap &results);

namespace Ui {
class NotesList;
}

class NotesList : public QDialog {
  Q_OBJECT

 public:
  explicit NotesList(QWidget *parent = nullptr);
  ~NotesList();
  Ui::NotesList *ui;

  NoteGraphController *m_graphController;  // 图谱控制器
  QDialog *m_RenameNotes = nullptr;
  TextEditToolbar *textToolbarRenameNotes = nullptr;
  QMenu *menuRecentOpen = nullptr;
  MoveTo *m_MoveTo = nullptr;
  NewNoteBook *m_NewNoteBook = nullptr;

  QString noteTitle;

  void saveNotesListIndex();

  NoteListModel *noteModel;

  DatabaseManager m_dbManager;

  QStringList needDelWebDAVFiles;

  QStringList searchResultList;

  QStringList recycleNotesList;

  QString getCurrentNoteNameFromMDFile(QString mdFile);

  QStringList listRecentOpen;
  QList<QTreeWidgetItem *> pNoteBookItems;
  QList<QTreeWidgetItem *> pNoteItems;
  QStringList findResult;
  int findCount;
  QList<QTreeWidgetItem *> findResultList;

  void set_memo_dir();

  bool delFile(QString file);

  void saveNotesList();
  void initNotesList();

  void setWinPos();

  void addItem(QTreeWidget *tw, QTreeWidgetItem *item);

  void saveRecycle();
  void initRecycle();
  void clearFiles();
  void getAllFiles(const QString &foldPath, QStringList &folds,
                   const QStringList &formats);

  void setNoteName(QString name);
  void moveBy(int ud);

  QString getCurrentMDFile();
  void init_NotesListMenu(QMenu *mainMenu);
  void init_NoteBookMenu(QMenu *mainMenu);

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
  void goPrevious();
  void goNext();

  int getNoteBookIndex_twToqml();

  void loadAllRecycle();

  void resetQML_List();

  void localNotesItem();
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
  void setCurrentItemFromMDFile(QString mdFile);
  QStringList extractLocalImagesFromMarkdown(const QString &filePath);

  void showNotesSearchResult();

  void openSearch();

  QString getSearchResultQmlFile();

  QStringList getValidMDFiles();

  void refreshRecentOpen(QString name);

  void showFindNotes();
  void restoreNoteFromRecycle();
  void needDelNotes();

  void updateNoteIndexManager(QString mdFile, int notebookIndex, int noteIndex);
  void updateAllNoteIndexManager();

 protected:
  bool eventFilter(QObject *watch, QEvent *evn) override;

  void closeEvent(QCloseEvent *event) override;

 public slots:
  void mouseClickNoteBook();

  void on_actionCopyNoteLink();

  void clickNoteList();

  void clickNoteBook();

  void on_btnClose_clicked();

  void on_btnNewNoteBook_clicked();

  void on_btnNewNote_clicked();

  void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);

  void on_btnRename_clicked();

  void on_btnDel_clicked();

  bool on_btnImport_clicked();

  void on_btnExport_clicked();

  void on_btnRecycle_clicked();

  void on_btnBack_clicked();

  void on_btnRestore_clicked();

  void on_btnDel_Recycle_clicked();

  void on_KVChanged();

  void on_btnFind_clicked();

  void on_btnUp_clicked();

  void on_btnDown_clicked();

  void on_btnMoveTo_clicked();

  void on_actionRelationshipGraph();
 private slots:

  void on_editFind_textChanged(const QString &arg1);

  void on_editFind_returnPressed();

  void on_actionShareNoteFile();

  void onSearchFinished();

  void onSearchTextChanged(const QString &text);

  void on_actionSetColorFlag();

  void on_actionStatistics();

  void onNoteNodeDoubleClicked(const QString &filePath);

 private:
  bool isImportFilesEnd;

  bool isReadyNoteDataEnd;

  bool isMouseClick = false;

  bool isActColorFlagStatus = false;

  QStringList validMDFiles;

  QStringList mIndexList;

  QInputMethod *pAndroidKeyboard = QApplication::inputMethod();

  QStringList knot_all_files;

  void clearMD_Pic(QTreeWidget *tw);
  void removePicFromMD(QString mdfile);
  void removeFromFiles(QString str);

  void on_actionAdd_NoteBook_triggered();
  void on_actionDel_NoteBook_triggered();
  void on_actionRename_NoteBook_triggered();
  void on_actionMoveUp_NoteBook_triggered();
  void on_actionMoveDown_NoteBook_triggered();

  void on_actionAdd_Note_triggered();
  void on_actionDel_Note_triggered();
  void on_actionRename_Note_triggered();
  void on_actionMoveUp_Note_triggered();
  void on_actionMoveDown_Note_triggered();
  void on_actionImport_Note_triggered();
  void on_actionExport_Note_triggered();
  int rootIndex;

  void goFindResult(int index);

  bool moveItem(QTreeWidget *tw);

  QFutureWatcher<ResultsMap> *watcher;

  QDateTime m_lastIndexTime;  // 记录最后一次索引构建时间
  QMutex m_indexTimeMutex;    // 互斥锁

  bool m_isIndexing = false;  // 标记索引状态

  void clearInvalidMDFile();
  SearchModel m_searchModel;
  void startBackgroundTaskUpdateFilesIndex();

  int getSavedNotesListIndex(int notebookIndex);
  bool safeWriteFile(const QString &filePath, const QString &content);
  void loadNotesListIndex();
  void addItemToQW(QQuickWidget *qw, QString text0, QString text1,
                   QString text2, QString text3, QString text4, int itemH);
  void setColorFlag(QString strColor);
  void setDelNoteFlag(QString mdFile);
  void saveNotesListToFile();

  void initNoteGraphView();
  void readyNotesData(QTreeWidgetItem *topItem);
};

class SearchMapper {
 public:
  using result_type = MySearchResult;  // 必须声明result_type

  explicit SearchMapper(const QRegularExpression &regex) : m_regex(regex) {}

  MySearchResult operator()(const QString &filePath) const {
    return searchInFile(filePath, m_regex);
  }

 private:
  QRegularExpression m_regex;
};

#endif  // NOTESLIST_H
