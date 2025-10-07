#ifndef READER_H
#define READER_H

#include <QAbstractListModel>
#include <QButtonGroup>
#include <QDialog>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNamedNodeMap>
#include <QEvent>
#include <QFile>
#include <QFontDialog>
#include <QOverload>
#include <QPlainTextEdit>
#include <QProcess>
#include <QProgressBar>
#include <QQmlEngine>
#include <QQuickView>
#include <QQuickWidget>
#include <QRegExp>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSettings>
#include <QStandardItemModel>
#include <QString>
#include <QStringList>
#include <QTextBlock>
#include <QTextBrowser>
#include <QTextCodec>
#include <QVBoxLayout>
#include <QXmlStreamReader>
#include <vector>

#include "src/Reader/DocumentHandler.h"
#include "src/Reader/epubreader.h"

// 目录项结构体（存储章节信息）
struct TocItem {
  QString title;              // 章节标题
  QString href;               // 章节对应的内容文件路径（相对路径）
  QList<TocItem> childItems;  // 子章节列表（嵌套层级）
};

class TextChunkModel;

namespace Ui {
class Reader;
}

class Reader : public QDialog {
  Q_OBJECT

 public:
  explicit Reader(QWidget *parent = nullptr);
  ~Reader();
  Ui::Reader *ui;

  QDialog *dlgAddBookNote = nullptr;
  QDialog *dlgEditBookNote = nullptr;

  bool isShowNote = false;
  int currentNoteListIndex = -1;

  bool isLandscape = false;

  qreal scrollValue = 1;
  bool isOpenBookListClick = false;
  bool isSelText = false;
  int pdfMethod = 2;

  QTimer *tmeAutoRun;
  QDialog *frame;
  QString openfile;
  QString readerStyle;
  QString strStyle2_0 =
      "color: rgb(0, 0, 0);background-color: rgb(255, 255, 255);border: 2px "
      "solid "
      "rgb(0,0,255);border-radius: 4px;";
  QString strStyle2_1 =
      "color: rgb(0, 0, 0);background-color: rgb(255, 255, 255);border: 2px "
      "solid "
      "rgb(255,0,0);border-radius: 4px;";
  QString currentTxt;
  QString currentHtmlFile;
  bool isPageNext = false;
  int mainDirIndex = -1;
  DocumentHandler *myDocHandler;
  QStringList bookList;

  QString currentBookName;
  qreal textPos;
  qreal textHeight;

  void saveReader(QString BookmarkText, bool isSetBookmark);
  void initReader();

  static void openFile(QString fileName);
  qulonglong vpos;

  static QStringList readText(QString textFile);
  void goBookReadPosition();
  void setQMLText(QString);

  void setQMLHtml(QString htmlFile, QString htmlBuffer, QString skipID);
  void setFontSize(int fontSize);
  static void PlainTextEditToFile(QPlainTextEdit *txtEdit, QString fileName);
  void savePageVPos();
  void setPageVPos();
  void showInfo();
  void startOpenFile(QString openfile);
  static QString getUriRealPath(QString uripath);
  static QString getNCX_File(QString path);

  void getReadList();
  void getBookList();

  void backDir();
  static QString get_idref(QString str0);
  void setVPos(qreal pos);

  static QString getCoverPicFile(QString htmlFile);
  qreal getVPos();
  void setReaderStyle();
  qreal getVHeight();

  static QString processHtml(QString htmlFile, bool isWriteFile);

  void on_btnOpen_clicked();

  void setAni();
  void loadQMLText(QString str);

  static bool copyDirectoryFiles(const QString &fromDir, const QString &toDir,
                                 bool coverFileIfExist);

  void clearAllReaderRecords();
  void on_hSlider_sliderReleased(int position);
  void setHtmlSkip(QString htmlFile, QString skipID);

  void removeBookList();
  void readBookDone();
  void setStatusBarHide();
  void setStatusBarShow();
  bool isStatusBarShow;

  void closeSelText();

  QString getBookmarkText();
  QStringList getCurrentBookmarkList();
  void showBookmarkList();

  void ContinueReading();
  QString getQMLText();

  void openMyPDF(QString uri);
  void closeMyPDF();
  void shareBook();
  bool eventFilterReader(QObject *watch, QEvent *evn);
  bool getDefaultOpen();

  void setDefaultOpen(QString value);
  void setTextAreaCursorPos(int nCursorPos);

  void showOrHideBookmark();

  void initInfoShowFont();

  static QStringList readText(QByteArray data);
  QString getBookmarkTextFromQML();
  void setQmlLandscape(bool isValue);

  void closeReader();
  void openReader();
  QString getReadTotalTime();

  void updateReaderProperty(int currentPage, int totalPages);
  void showBookPageNext();
  void showBookPageUp();
  void closeBookPage();
  void addBookNote();
  void readReadNote(int page);
  void viewBookNote();
 public slots:
  void on_SetReaderFunVisible();

  void setPageScroll0();
  void setPageScroll1();
  void setEpubPagePosition(int index, QString htmlFile);

  void showCatalogue();
  void initLink(QString htmlFile);
  void selectText();
  void openCataList(QString htmlFile);

  void clickBookmarkList(int i);

 protected:
  bool eventFilter(QObject *obj, QEvent *evn) override;
  void keyReleaseEvent(QKeyEvent *event) override;
  void closeEvent(QCloseEvent *event) override;
  void paintEvent(QPaintEvent *event) override;

 signals:
  void notesLoaded(const QVariantList &notes);

 public slots:
  void openBookListItem();

  void showTextFun();

  void goNextPage();
  void goUpPage();

  void delReadNote(int index);
  void editBookNote(int index, int page, const QString &content);
  void closeViewBookNote();
  void setShowNoteValue(bool value);
  void setNoteListCurrentIndexValue(int value);
 private slots:
  void autoRun();

 private:
  QString strColor;
  QDateTime startDateTime, endDateTime;
  double totalHours = 0.0;
  bool isOne = false;
  QString customCss;
  int x, y, w, h;
  bool isInitReader = false;
  QString strSpace = "";
  QString strEndFlag;
  static void SplitFile(QString qfile);
  static QString get_href(QString idref, QStringList opfList);
  static void proceImg();
  void getLines();
  static QStringList ncx2html();
  QString getSkipText(QString htmlFile, QString skipID);
  QString strFind;
  void gotoCataList(QString htmlFile);
  int currentCataIndex = 0;

  QString updateContent();

  void handleDoubleClick(const QPointF &globalPos);

  bool handleTouchPress(const QPointF &globalPos);

  static QString getNavFileInternalPath(const QByteArray &opfContent);
  static QList<TocItem> parseTocFromNavFile(const QByteArray &navContent);
  static QList<TocItem> parseOlElement(QXmlStreamReader &reader);
  static TocItem parseLiElement(QXmlStreamReader &reader);
  static void debugPrintTocItems(const QList<TocItem> &tocItems, int level);
  static bool isDcTitleElement(const QXmlStreamReader &xml);
  static QString getEpub3Title(const QString &opfFile);

  bool isGetBookmarkText = false;
  QString getFirstThreeLines(QTextEdit *textEdit);
  bool getLandscape();
  bool getQmlReadyEnd();
  double readTotalHours();
  bool writeTotalHours(double value);
  void saveReadNote(int page, int start, int end, const QString &color,
                    const QString &content, const QString &quote);
  int cPage;
  void updateReadNote(int page, int index, const QString &content);
  void appendNoteDataToQmlList();
  QStandardItemModel *notesModel = nullptr;
  void initBookNoteValue(int cindex, int cpage);
  void modifyText2(int currentIndex, const QString &text);
};
//////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
class TextChunkModel : public QAbstractListModel {
  Q_OBJECT
 public:
  explicit TextChunkModel(QObject *parent = nullptr);

  // 定义角色枚举
  enum CustomRoles {
    TextRole = Qt::UserRole + 1  // 从UserRole开始分配自定义角色
  };

  // 必须实现的接口
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  // 业务方法
  Q_INVOKABLE void splitContent(const QString &fullText);
  Q_INVOKABLE void appendChunks(const QStringList &chunks);
  Q_INVOKABLE void clear();
  Q_INVOKABLE QVariantMap get(int index) const;

 signals:
  void chunksChanged(const QVector<QString> &chunks);

 private:
  QHash<int, QByteArray> m_roleNames;  // 存储角色定义
  QStringList m_chunks;                // 存储分割后的文本块
  void handleComplexStructure(QString &text, int &currentPos);
  bool isValidNesting(const QString &htmlBlock);
};
///////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////

#endif  // READER_H
