#ifndef NOTES_H
#define NOTES_H

#include <QApplication>
#include <QChar>
#include <QClipboard>
#include <QMimeData>
#include <QObject>
#include <QShortcut>
#include <QTextEdit>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#ifdef Q_OS_ANDROID
#include <QtCore/private/qandroidextras_p.h>
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
#include <QSettings>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextDocumentFragment>

#include "lib/qsci/Qsci/qscilexercpp.h"
#include "lib/qsci/Qsci/qscilexermarkdown.h"
#include "lib/qsci/Qsci/qsciscintilla.h"
#include "src/Comm/ShowMessage.h"
#include "src/Notes/ColorDialog.h"
#include "src/Notes/LineNumberArea.h"
#include "src/Notes/PrintPDF.h"
#include "src/Notes/QTextEditHighlighter.h"
#include "src/Notes/TextSelector.h"
#include "ui_PrintPDF.h"
#include "ui_TextSelector.h"

class LimitedTextEdit;
class NoteIndexManager;
class MiniMap;

namespace Ui {
class Notes;
}

class Notes : public QDialog {
  Q_OBJECT

 public:
  explicit Notes(QWidget *parent = nullptr);
  ~Notes();
  Ui::Notes *ui;

  NoteIndexManager *m_NoteIndexManager;

  QString new_title;

  QsciScintilla *m_EditSource = nullptr;
  QTextEditHighlighter *m_EditSource1;

  QTimer *timerEditNote;
  int px, py, mx, my;

  bool isTextChange;

  QTimer *timerEditPanel;
  QString htmlBuffer;
  QTextEdit *byTextEdit;
  QLineEdit *byLineEdit;
  int androidKeyH;
  int start;
  int end;

  qreal textHeight;

  QString textMemo;
  void saveMainNotes();
  QString fileName;

  qlonglong curPos;
  qreal sliderPos;
  void loadNoteToQML();

  QString Deciphering(const QString &fileName);

  void init();

  QStringList getImgFileFromHtml(QString htmlfile);
  void zipMemo();

  void selectText(int start, int end);

  void getEditPanel(QTextEdit *textEdit, QEvent *evn);

  qreal getVHeight();

  QString getDateTimeStr();
  void MD2Html(QString mdFile);
  void saveQMLVPos();

  qreal getVPos();
  void unzip(QString zipfile);

  void show_findText();

  void findText();
  void show_findTextBack();

  bool selectPDFFormat(QPrinter *printer);
  void on_btnPDF_clicked();

  bool eventFilterEditTodo(QObject *watch, QEvent *evn);

  bool eventFilterEditRecord(QObject *watch, QEvent *evn);

  QString insertImage(QString fileName, bool isToAndroidView);

  bool eventFilterQwNote(QObject *watch, QEvent *event);

  void setEditorVPos();

  void openAndroidNoteEditor();

  void appendNote(QString str);
  void insertNote(QString str);
  auto getAndroidNoteConfig(QString key);
  void setAndroidNoteConfig(QString key, QString value);

  void delImage();

  void javaNoteToQMLNote();
  QString formatMDText(QString text);

  void init_all_notes();
  void loadEmptyNote();

  void refreshQMLVPos(qreal newPos);

  void setWebViewFile(QString htmlfile);

  void openMDWindow();

  bool isSetNewNoteTitle();

  void saveWebScrollPos(QString mdfilename);

  void openNotesUI();

  void syncToWebDAV();

  QStringList notes_sync_files;

  void openEditUI();

  void openNotes();

  void updateMainnotesIniToSyncLists();
  QDateTime mainnotesLastModi;

  void updateMDFileToSyncLists(QString currentMDFile);

  void initEditor();

  void openBrowserOnce(const QString &htmlPath);

  void init_md();

 protected:
  void keyReleaseEvent(QKeyEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  bool eventFilter(QObject *obj, QEvent *event) override;
  void paintEvent(QPaintEvent *pEvent) override;
  void closeEvent(QCloseEvent *event) override;
  void showEvent(QShowEvent *event) override;

 public slots:
  void on_btnNext_clicked();

  void setVPos();

  void editVSBarValueChanged();

  void timerSlot();

  void on_showEditPanel();

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

  void showTextSelector();
  void delLink(QString link);
 signals:
  void sendUpdate();

 private slots:

  void on_editSource_textChanged();

  void on_btnFind_clicked();

  void on_btnPrev_clicked();

  void on_editFind_returnPressed();

  void on_editFind_textChanged(const QString &arg1);

  void on_btnView_clicked();

 private:
  bool m_initialized = false;

  QString m_lastSearchText;
  int m_searchFlags = 0;

  QStringList orgRemoteFiles;
  QList<QDateTime> orgRemoteDateTime;
  QList<QString> remoteFiles;

  int x_left, x_right, y_left, y_right;
  QString strNoteText;
  int y1;
  QString pdfFileName;
  bool isMouseRelease = false;
  bool isMousePress = false;
  bool isMouseMove = false;

  QTimer *timerCur;
  QPainter *pPainter;
  bool bCursorVisible;

  bool isFunShow;

  int newHeight = 0;

  QInputMethod *pAndroidKeyboard = QApplication::inputMethod();
  void wheelEvent(QWheelEvent *e) override;
  QString imgDir = "==Image==";
  QColor StringToColor(QString mRgbStr);

  void setOpenSearchResultForAndroid(bool isValue, QString strSearchText);
  void initMarkdownEditor(QsciScintilla *editor);
  void searchText(const QString &text, bool forward);
  void searchNext();
  void searchPrevious();

  int getSearchMatchCount(const QString &text);
  void searchWithCount(const QString &text);
  QList<QPair<int, int>> m_matchPositions;
  int m_currentMatchIndex = -1;
  void jumpToNextMatch();
  void jumpToPrevMatch();
  QsciLexerMarkdown *markdownLexer;
  void initMarkdownLexer();
  QString imageToBase64(const QString &path);
  QString addImagePathToHtml(QString strhtml);
  void initMarkdownLexerDark();

  void initEditorScrollBars();
  void saveEditorState(const QString &filePath);
  void restoreEditorState(const QString &filePath);
};

class LimitedTextEdit : public QTextEdit {
  Q_OBJECT

 public:
  explicit LimitedTextEdit(QWidget *parent = nullptr)
      : QTextEdit(parent), maxLength(1000) {}

  void setMaxLength(int length) { maxLength = length; }
  int getMaxLength() const { return maxLength; }

 protected:
  void insertFromMimeData(const QMimeData *source) override {
    if (source->hasText()) {
      QString pasteText = source->text();
      int currentLength = toPlainText().length();
      int available = maxLength - currentLength;

      if (available <= 0) {
        // 无可用空间，拒绝粘贴
        return;
      }

      if (pasteText.length() > available) {
        // 截断文本至可用长度
        pasteText = pasteText.left(available);
      }

      // 创建新的MIME数据并插入
      QMimeData *newData = new QMimeData();
      newData->setText(pasteText);
      QTextEdit::insertFromMimeData(newData);
      delete newData;
    } else {
      // 非文本内容，按默认处理
      QTextEdit::insertFromMimeData(source);
    }
  }

 private:
  int maxLength;
};

class NoteIndexManager : public QObject {
  Q_OBJECT
 public:
  explicit NoteIndexManager(QObject *parent = nullptr);

  // 加载/保存索引
  bool loadIndex(const QString &indexPath);
  bool saveIndex(const QString &indexPath);

  // 名称操作
  QString getNoteTitle(const QString &filePath) const;
  void setNoteTitle(const QString &filePath, const QString &title);

 private:
  QHash<QString, QString> m_index;  // 内存哈希表加速查询
  QString m_currentIndexPath;
};

#endif  // NOTES_H
