#ifndef NOTES_H
#define NOTES_H

#include <QApplication>
#include <QChar>
#include <QClipboard>
#include <QMimeData>
#include <QObject>
#include <QShortcut>
#include <QStandardPaths>
#include <QTextEdit>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#ifdef Q_OS_ANDROID
#include <QtCore/private/qandroidextras_p.h>

#include <QJniEnvironment>
#include <QJniObject>
#endif

#include <QColorDialog>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QHash>
#include <QImageReader>
#include <QInputMethod>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QObject>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPrinter>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <algorithm>
#include <cstdlib>  // 用于 getenv
#ifdef __linux__
#include <unistd.h>  // Linux 下的 unsetenv/setenv
#endif

#ifndef Q_OS_ANDROID
#include "lib/qsci/Qsci/qscilexercpp.h"
#include "lib/qsci/Qsci/qscilexermarkdown.h"
#include "lib/qsci/Qsci/qsciscintilla.h"
#endif

#include "src/Comm/ShowMessage.h"
#include "src/Notes/ColorDialog.h"
#include "src/Notes/NoteDiffManager.h"
#include "src/Notes/PrintPDF.h"
#include "src/Notes/note_index_manager.h"
#include "titlegenerator.h"
#include "ui_PrintPDF.h"

class NoteIndexManager1;
class MiniMap;

namespace Ui {
class Notes;
}

class Notes : public QDialog {
  Q_OBJECT

 public:
  explicit Notes(QWidget* parent = nullptr);
  ~Notes();
  Ui::Notes* ui;

  NoteDiffManager m_NoteDiffManager;

  bool isWebDAVError = false;

  void openLocalHtmlFileInAndroid();

  QString htmlFileName;

  bool isRequestOpenNoteEditor = false;

  NoteIndexManager* m_NoteIndexManager;

  QString new_title;

#ifndef Q_OS_ANDROID
  QsciScintilla* m_EditSource = nullptr;
#endif

  QTimer* timerEditNote;
  int px, py, mx, my;

  bool isTextChange;

  QString htmlBuffer;
  QTextEdit* byTextEdit;
  QLineEdit* byLineEdit;
  int androidKeyH;
  int start;
  int end;

  qreal textHeight;

  QString textMemo;
  void saveMainNotes();
  QString fileName;

  qlonglong curPos;
  qreal sliderPos;

  void init();

  qreal getVHeight();

  QString getDateTimeStr();
  void MD2Html(QString mdFile);

  qreal getVPos();

  void show_findText();

  void findText();
  void show_findTextBack();

  bool selectPDFFormat(QPrinter* printer);
  void on_btnPDF_clicked();

  QString insertImage(QString fileName, bool isToAndroidView);

  bool eventFilterQwNote(QObject* watch, QEvent* event);

  void openAndroidNoteEditor();

  void appendNote(QString str);
  void insertNote(QString str);
  auto getAndroidNoteConfig(QString key);
  void setAndroidNoteConfig(QString key, QString value);

  void javaNoteToQMLNote();
  QString formatMDText(QString text);

  void init_all_notes();
  void loadEmptyNote();

  void openMDWindow();

  bool isSetNewNoteTitle();

  void openNotesUI();

  void syncToWebDAV();

  QStringList notes_sync_files;

  void openEditUI();

  void openNotes();

  void updateMainnotesIniToSyncLists();

  bool isSaveNotesConfig = false;

  void updateMDFileToSyncLists();

  void initEditor();

  void openBrowserOnce(const QString& htmlPath);

  void init_md();

  void previewNote();
  void appendToSyncList(QString file);

  void startBackgroundTaskDelAndClear();
  void startBackgroundTaskUpdateNoteIndex(QString mdFile);

  void startBackgroundTaskUpdateNoteIndexes(QStringList mdFileList);

  QStringList orgRemoteFiles;

  QList<QJsonObject> loadAllDiffs(const QString& diffFilePath);

  bool appendDiffToFile(const QString& diffFilePath,
                        const QString& noteFilePath, const QString& strDiff,
                        const QString& diffHtml);
  QString getCurrentJSON(const QString& md);

  void delRemoteFile(const QStringList& Files);

  bool openUrl(const QString& url);

  void renameTitle(bool isOk);

  void refreshNote();

 protected:
  void keyReleaseEvent(QKeyEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;
  bool eventFilter(QObject* obj, QEvent* event) override;
  void paintEvent(QPaintEvent* pEvent) override;
  void closeEvent(QCloseEvent* event) override;
  void showEvent(QShowEvent* event) override;

 public slots:
  void on_btnNext_clicked();

  void editVSBarValueChanged();

  void on_btnDone_clicked();

  void on_KVChanged();

  void on_btnPic_clicked();

  void on_btnInsertTable_clicked();

  void on_btnS1_clicked();

  void on_btnS2_clicked();

  void on_btnS3_clicked();

  void on_btnS4_clicked();

  void on_btnColor_clicked();

  void on_btnS5_clicked();

  void on_btnPaste_clicked();

  void editNote();
  void showNoteList();
  void on_editNote();

  void delLink(QString link);

 signals:

 private slots:

  void editSource_textChanged();

  void on_btnFind_clicked();

  void on_btnPrev_clicked();

  void on_editFind_returnPressed();

  void on_editFind_textChanged(const QString& arg1);

  void on_btnView_clicked();

 private:
  bool isReceiveRemoteFile = false;

  QString oldText, newText;

  QString currentJSON;

  bool m_initialized = false;

  QString m_lastSearchText;
  int m_searchFlags = 0;

  QList<QDateTime> orgRemoteDateTime;
  QList<QString> remoteFiles;

  int x_left, x_right, y_left, y_right;

  int y1;
  QString pdfFileName;
  bool isMouseRelease = false;
  bool isMousePress = false;
  bool isMouseMove = false;

  bool isFunShow;

  int newHeight = 0;

  QInputMethod* pAndroidKeyboard = QApplication::inputMethod();
  void wheelEvent(QWheelEvent* e) override;

  QColor StringToColor(QString mRgbStr);

  void setOpenSearchResultForAndroid(bool isValue, QString strSearchText);

#ifndef Q_OS_ANDROID
  void initMarkdownEditor(QsciScintilla* editor);
#endif

  void searchText(const QString& text, bool forward);
  void searchNext();
  void searchPrevious();

  int getSearchMatchCount(const QString& text);
  void searchWithCount(const QString& text);
  QList<QPair<int, int>> m_matchPositions;
  int m_currentMatchIndex = -1;
  void jumpToNextMatch();
  void jumpToPrevMatch();

#ifndef Q_OS_ANDROID
  QsciLexerMarkdown* markdownLexer;
#endif

  void initMarkdownLexer();
  QString imageToBase64(const QString& path);
  QString addImagePathToHtml(QString strhtml);
  void initMarkdownLexerDark();

  void initEditorScrollBars();
  void saveEditorState(const QString& filePath);
  void restoreEditorState(const QString& filePath);
  void processRemoteFiles(QStringList remoteFiles);
  void startBackgroundProcessRemoteFiles();

  QString getFileVersion(const QString& filePath);
  void updateDiff(const QString& oldText, const QString& newText);

  void zipNoteToSyncList();
  void refreshLocalHtmlFileInAndroid();
  void openNotesUI_1();
};

///////////////////////////////////////////////////////////////////////
class NoteIndexManager1 : public QObject {
  Q_OBJECT
 public:
  explicit NoteIndexManager1(QObject* parent = nullptr);

  // 加载/保存索引
  bool loadIndex(const QString& indexPath);
  bool saveIndex(const QString& indexPath);

  // 名称操作
  QString getNoteTitle(const QString& filePath) const;
  void setNoteTitle(const QString& filePath, const QString& title);

 private:
  QHash<QString, QString> m_index;  // 内存哈希表加速查询
  QString m_currentIndexPath;
};
////////////////////////////////////////////////////////////////////////

#endif  // NOTES_H
