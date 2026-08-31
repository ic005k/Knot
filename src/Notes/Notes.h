#ifndef NOTES_H
#define NOTES_H

#include <QApplication>
#include <QChar>
#include <QClipboard>
#include <QCompleter>
#include <QMimeData>
#include <QObject>
#include <QPointer>
#include <QShortcut>
#include <QStandardPaths>
#include <QTextEdit>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#ifdef Q_OS_ANDROID
// #include <QtCore/private/qandroidextras_p.h>

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

#include "TitleGenerator.h"
#include "defines.h"
#include "src/Comm/ShowMessage.h"
#include "src/Notes/ColorDialog.h"
#include "src/Notes/NoteDiffManager.h"
#include "src/Notes/NoteManager.h"
#include "src/Notes/PrintPDF.h"
#include "ui_PrintPDF.h"

class MiniMap;

namespace Ui {
class Notes;
}

// 自定义 ListWidgetItem，存储搜索结果的位置信息
struct TextMatch {
  int line;          // 行号
  int index;         // 列号（起始位置）
  int length;        // 匹配长度
  QString lineText;  // 该行完整文本
};

struct NoteCounterItem {
  int64_t count = 0;
  int64_t last = 0;
};

struct ContextWords {
  QStringList before;  // 光标前的字词（按从近到远排列）
  QStringList after;   // 光标后的字词（按从近到远排列）
};

class Notes : public QDialog {
  Q_OBJECT

 public:
  explicit Notes(QWidget* parent = nullptr);
  ~Notes();
  Ui::Notes* ui;

  bool isAIQA = false;

  // ✅ 纯数据解析，供JNI调用，不触碰任何UI
  // 返回值约定：
  //   以 "IMG:" 开头 → 图片绝对路径
  //   以 "TXT:" 开头 → 摘要文本
  //   空字符串     → 无可预览内容
  QString parsePreviewData(const QString& lineText, int cursorPos = -1);

  QAtomicInteger<int> m_skipCount{0};

  void loadNotesToUI();

  NoteDiffManager m_NoteDiffManager;

  bool isWebDAVError = false;

  void openLocalHtmlFileInAndroid();

  QString htmlFileName;

  bool isRequestOpenNoteEditor = false;

  NoteManager* m_NoteManager;

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
  void saveMDFile();
  QString fileName;

  qlonglong curPos;
  qreal sliderPos;

  void init();

  qreal getVHeight();

  QString getDateTimeStr();
  void MD2Html(QString mdFile);

  qreal getVPos();

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
  void startBackgroundTaskUpdateNoteGraph(QString mdFile);

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

  bool checkAndUpdateCleanDate();

  bool removeNoteVector(const QString& mdFilePath);

  bool syncNoteVectorsBatchToDb(const QString& mdFilePath);

  QString readNoteFileSafeFromRaw(const QByteArray& raw);

  void popupNoteLinkList(const QString& arg1);

  QString getCursorPosText(const QString& fullText, int cursorPos,
                           int textCount);
  bool isBtnAILinkClicked = false;
  bool isAndroidAILinkGen = false;

  void appendAIResults(QString str);

  void setNotesCounter();
  void saveNotesCounter();
  void loadNotesCounter();
  void refreshRecentOpenByCounter();

  void openNoteWindow();

  void setNoteEntryList();

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
  void syncFinished();

 private slots:

  void editSource_textChanged();

  void on_btnFind_clicked();

  void on_btnPrev_clicked();

  void on_editFind_returnPressed();

  void on_editFind_textChanged(const QString& arg1);

  void on_btnView_clicked();

  void on_editNoteLink_textChanged(const QString& arg1);

  void onPopupItemClicked(QListWidgetItem* item);

  void on_btnReplace_clicked();

  void on_btnFindReplace_clicked();

  void on_btnReplaceAll_clicked();

  void on_btnAILink_clicked();

  void on_btnInsertNoteLink_clicked();

  void on_btnQuestion_clicked();

  void on_btnClearQuestion_clicked();

  void onSearchResultClicked(QListWidgetItem* item);

 private:
  void findAllAndShowResults(const QString& text);
  void updateResultCount(int count);
  QTimer* m_findDebounceTimer = nullptr;
  int m_searchGeneration = 0;
  void startBackgroundSearch(const QString& keyword);
  void applySearchResults(const QList<TextMatch>& matches,
                          const QStringList& htmlList);

#ifdef VECTOR_SEARCH
  QString loadNoteFullText(const QString& mdPath);
#endif

  QHash<QString, NoteCounterItem> m_counterMap;

  void updateImagePreview(const QString& lineText, int cursorPos);
  QString generateSmartSummary(const QString& filePath);
  QRegularExpression m_linkRegex;
  QRegularExpression m_imgRegex;
  QCache<QString, QString> m_summaryCache;  // 默认上限100条

  static inline QMutex s_embMutex;
  static inline QMutex s_vecDbMutex;

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

  void wheelEvent(QWheelEvent* e) override;

  QColor StringToColor(QString mRgbStr);

  void setOpenSearchResultForAndroid(bool isValue, QString strSearchText);

#ifndef Q_OS_ANDROID
  void initMarkdownEditor(QsciScintilla* editor);
#endif

  void searchText(const QString& text, bool forward);
  void searchNext();
  void searchPrevious();

  void searchWithCount(const QString& text);
  QList<QPair<int, int>> m_matchPositions;
  int m_currentMatchIndex = -1;
  void jumpToNextMatch();
  void jumpToPrevMatch();

#ifndef Q_OS_ANDROID
  QsciLexerMarkdown* markdownLexer = nullptr;

  QString takeFirstNTokens(const QString& text, int count = 10);

  int getCursorCharOffset(QsciScintilla* editor);
#endif

  void initMarkdownLexer();
  QString imageToBase64(const QString& path);
  QString addImagePathToHtml(QString strhtml);

  void initEditorScrollBars();

  void saveEditorState(const QString& filePath);
  void restoreEditorState(const QString& filePath);

  QString getFileVersion(const QString& filePath);
  void updateDiff(const QString& oldText, const QString& newText);

  void zipNoteToSyncList();
  void refreshLocalHtmlFileInAndroid();

  void processSingleRemoteFile(const QString& file);
  void startBackgroundProcessRemoteFiles_MultiThread();
  void buildCleanFileList();
  void insertNoteLink(const QString& title, const QString& path);
  void switchMdDarkTheme(bool dark);
  void applyMdLexerTheme(bool darkMode);
  void highlightCurrentResult(int line, int index);
  void buildHighlightedItem(QListWidgetItem* item, int lineNum,
                            const QString& displayPrefix, const QString& prefix,
                            const QString& match, const QString& suffix,
                            const QString& displaySuffix);

  QString lightPCScrollbarStyle = R"(
        /* Light Vertical Scrollbar */
        QScrollBar:vertical {
            background: #F5F5F5;
            width: 22px;
            margin: 2px;
        }
        QScrollBar::handle:vertical {
            background: #C0C0C0;
            border-radius: 4px;
            border: 1px solid #D0D0D0;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background: #A8A8A8;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            background: transparent;
            border: none;
            height: 0px;
        }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            background: transparent;
        })";

  QString darkPCScrollbarStyle = R"(
        /* Dark Vertical Scrollbar */
        QScrollBar:vertical {
            background: #2D2D2D;
            width: 22px;
            margin: 2px;
        }
        QScrollBar::handle:vertical {
            background: #606060;
            border-radius: 4px;
            border: 1px solid #404040;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background: #707070;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            background: transparent;
            border: none;
            height: 0px;
        }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            background: transparent;
        })";
};

#endif  // NOTES_H
