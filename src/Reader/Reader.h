#ifndef READER_H
#define READER_H

#include <QAbstractListModel>
#include <QDialog>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNamedNodeMap>
#include <QEvent>
#include <QFile>
#include <QFontDialog>
#include <QPlainTextEdit>
#include <QProcess>
#include <QProgressBar>
#include <QQmlEngine>
#include <QQuickView>
#include <QQuickWidget>
#include <QSaveFile>
#include <QString>
#include <QStringList>
#include <QTextBlock>
#include <QTextBrowser>
#include <QTextCodec>
#include <QVBoxLayout>
#include <QXmlStreamReader>

#include "src/Reader/DocumentHandler.h"
#include "src/Reader/File.h"

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

  qreal scrollValue = 1;
  bool isOpenBookListClick = false;
  bool isSelText = false;
  int pdfMethod = 2;
  QTimer *tmeShowEpubMsg;
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

  static QString GetCorrectUnicode(const QByteArray &text);
  void getReadList();
  void getBookList();

  void backDir();
  static QString get_idref(QString str0);
  void setVPos(qreal pos);

  static QString getCoverPicFile(QString htmlFile);
  qreal getVPos();
  void setReaderStyle();
  qreal getVHeight();
  qreal getNewVPos(qreal pos1, qreal h1, qreal h2);
  static QString processHtml(QString htmlFile, bool isWriteFile);

  void goNextPage();
  void goUpPage();
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
  bool eventFilterReaderAndroid(QObject *watch, QEvent *evn);
  void showOrHideBookmark();
  public slots:
  void setPageScroll0();
  void setPageScroll1();
  void setEpubPagePosition(int index, QString htmlFile);

  void showCatalogue();
  void initLink(QString htmlFile);
  void selectText();
  void openCataList(QString htmlFile);

  void clickBookmarkList(int i);

  void setPanelVisible();

 protected:
  bool eventFilter(QObject *obj, QEvent *evn) override;
  void keyReleaseEvent(QKeyEvent *event) override;
  void closeEvent(QCloseEvent *event) override;
  void paintEvent(QPaintEvent *event) override;

 public slots:
  void openBookListItem();

 private slots:
  void autoRun();
  void showEpubMsg();

 private:
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
  static bool isUtf8(const QByteArray &data);
  static bool isValidText(const QString &text);
  void handleDoubleClick(const QPointF &globalPos);
  bool handleTouchRelease(const QPointF &globalPos);
  bool handleTouchMove(const QPointF &globalPos);
  bool handleTouchPress(const QPointF &globalPos);
};

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

#endif  // READER_H
