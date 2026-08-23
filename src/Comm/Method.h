#ifndef METHOD_H
#define METHOD_H

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <quazip.h>
#include <quazipfile.h>
#include <quazipnewinfo.h>
#include <unzip.h>
#include <zlib.h>

#include <QAbstractSocket>
#include <QByteArray>
#include <QConicalGradient>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QDialog>
#include <QDir>
#include <QDirIterator>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QHostAddress>
#include <QInputDialog>
#include <QInputMethod>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkInterface>
#include <QNetworkReply>
#include <QTimeZone>
#include <QTimer>
#include <QtGui/QEventPoint>
#include <QtGui/QTouchEvent>
#include <functional>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif

#include <QLabel>
#include <QPainter>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QQuickWidget>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QString>
#include <QTableWidget>
#include <QTextEdit>
#include <QTimer>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QWidget>

#include "src/Comm/EnhancedColorPicker.h"
#include "src/Comm/TextEditToolbar.h"

struct SearchItem {
  QString tabName;
  QString strYear;
  QString strMonthDay;
  QString weeks;
  QString day;
  QString strTime;
  QString txt1;
  QString txt2;
  QString txt3;
};

namespace Ui {
class Method;
}

class Method : public QDialog {
  Q_OBJECT

 public:
  explicit Method(QWidget* parent = nullptr);
  ~Method();
  Ui::Method* ui;

  QDialog* infoWindow = nullptr;
  QProgressBar* infoProgBar = nullptr;
  QTextEdit* lblInfo = nullptr;
  void showInfoWindow(const QString& info);
  void closeInfoWindow();
  void setInfoText(const QString& newText);

  EnhancedColorPicker* m_EnColorPicker = nullptr;

  bool androidCopyFile(QString src, QString des);

  static QString getFileSize(const qint64& size, int precision);

  QWidget* m_widget = nullptr;
  QString qssSlider;
  QString ColorToString(QColor v_color);

  QString lblStyle =
      "QLabel{border: 0px solid gray;border-radius: "
      "0px;background-color:qlineargradient(spread:pad,x1:1,y1:0,x2:0,y2:0,"
      "stop:0 #FFAEB9,stop:1 #87CEFF);color:black;selection-background-color: "
      "lightblue;}";

  QString lblStyle0 =
      "QLabel{border: 0px solid gray;border-radius: "
      "0px;background-color:#3498DB;color:white;selection-background-color: "
      "lightblue;}";

  QString btnStyle =
      "QToolButton {background-color: rgb(236, 236, 236);color: black; "
      "border-radius:10px; "
      "border:1px solid gray; } QToolButton:pressed { background-color: "
      "rgb(220,220,230);}";

  QString btnStyleDark =
      "QToolButton {background-color: rgb(51, 51, 51);color: white; "
      "border-radius:10px; "
      "border:1px solid gray; } QToolButton:pressed { background-color: "
      "rgb(22,22,23);}";

  QString pushbtnStyle =
      "QPushButton {background-color: rgb(236, 236, 236);border-radius:10px; "
      "border:1px solid gray; } QPushButton:pressed { background-color: "
      "rgb(220,220,230);}";

  QString listWidgetDarkStyle = R"(
QListWidget {
    background-color: #2C2C2C;
    color: #E0E0E0;
    border: 1px solid #555555;
    border-top: none;
    padding: 4px;
    outline: none;
}
QListWidget::item:selected {
    background-color: #007ACC;
    color: white;
}
QListWidget::item:selected:hover {
    background-color: #0068B5;
    color: white;
}
QListWidget::item:hover {
    background-color: #444444;
    color: #E0E0E0;
}

/* --- 垂直滚动条 --- */
QScrollBar:vertical {
    background: #333333;
    width: 8px;
    margin: 0px;
    border:none;
}
QScrollBar::handle:vertical {
    background: #777777;
    border-radius: 4px;
    min-height: 25px;
    border:none;
}
QScrollBar::handle:vertical:hover {
    background: #999999;
}
QScrollBar::add-line:vertical,
QScrollBar::sub-line:vertical {
    height: 0px;
}
QScrollBar::add-page:vertical,
QScrollBar::sub-page:vertical {
    background: none;
}

/* --- 水平滚动条 --- */
QScrollBar:horizontal {
    background: #333333;
    height: 8px;
    margin: 0px;
    border:none;
}
QScrollBar::handle:horizontal {
    background: #777777;
    border-radius: 4px;
    min-width: 25px;
    border:none;
}
QScrollBar::handle:horizontal:hover {
    background: #999999;
}
QScrollBar::add-line:horizontal,
QScrollBar::sub-line:horizontal {
    width: 0px;
}
QScrollBar::add-page:horizontal,
QScrollBar::sub-page:horizontal {
    background: none;
}
)";

  QString listWidgetLightStyle = R"(
QListWidget {
    background-color: #FFFFFF;
    color: #2C2C2C;
    border: 1px solid #CCCCCC;
    border-top: none;
    padding: 4px;
    outline: none;
}
QListWidget::item:selected {
    background-color: #007ACC;
    color: white;
}
QListWidget::item:selected:hover {
    background-color: #0068B5;  /* 比 #007ACC 略深，提供微反馈 */
    color: white;
}
QListWidget::item:hover {
    background-color: #EEEEEE;
    color: #2C2C2C;
}

/* --- 垂直滚动条 --- */
QScrollBar:vertical {
    background: #F5F5F5;
    width: 8px;
    margin: 0px;
    border:none;
}
QScrollBar::handle:vertical {
    background: #CCCCCC;
    border-radius: 4px;
    min-height: 25px;
    border:none;
}
QScrollBar::handle:vertical:hover {
    background: #AAAAAA;
}
QScrollBar::add-line:vertical,
QScrollBar::sub-line:vertical {
    height: 0px;
}
QScrollBar::add-page:vertical,
QScrollBar::sub-page:vertical {
    background: none;
}

/* --- 水平滚动条 --- */
QScrollBar:horizontal {
    background: #F5F5F5;
    height: 8px;
    margin: 0px;
    border:none;
}
QScrollBar::handle:horizontal {
    background: #CCCCCC;
    border-radius: 4px;
    min-width: 25px;
    border:none;
}
QScrollBar::handle:horizontal:hover {
    background: #AAAAAA;
}
QScrollBar::add-line:horizontal,
QScrollBar::sub-line:horizontal {
    width: 0px;
}
QScrollBar::add-page:horizontal,
QScrollBar::sub-page:horizontal {
    background: none;
}
)";

  QString setPushButtonQss(
      QPushButton* btn,                              // 按钮对象
      int radius = 5,                                // 圆角半径
      int padding = 8,                               // 间距
      const QString& normalColor = "#34495E",        // 正常颜色
      const QString& normalTextColor = "#FFFFFF",    // 文字颜色
      const QString& hoverColor = "#4E6D8C",         // 悬停颜色
      const QString& hoverTextColor = "#F0F0F0",     // 悬停文字颜色
      const QString& pressedColor = "#2D3E50",       // 按下颜色
      const QString& pressedTextColor = "#B8C6D1");  // 按下文字颜色

  QString setToolButtonQss(
      QToolButton* btn,                              // 按钮对象
      int radius = 5,                                // 圆角半径
      int padding = 8,                               // 间距
      const QString& normalColor = "#34495E",        // 正常颜色
      const QString& normalTextColor = "#FFFFFF",    // 文字颜色
      const QString& hoverColor = "#4E6D8C",         // 悬停颜色
      const QString& hoverTextColor = "#F0F0F0",     // 悬停文字颜色
      const QString& pressedColor = "#2D3E50",       // 按下颜色
      const QString& pressedTextColor = "#B8C6D1");  // 按下文字颜色

  QString lightScrollbarStyle =
      "/* Light Vertical Scrollbar */"
      "QScrollBar:vertical {"
      "    background: #F5F5F5;"
      "    width: 10px;"
      "    margin: 2px;"
      "}"
      "QScrollBar::handle:vertical {"
      "    background: #C0C0C0;"
      "    border-radius: 4px;"
      "    border: 1px solid #D0D0D0;"
      "    min-height: 30px;"
      "}"
      "QScrollBar::handle:vertical:hover {"
      "    background: #A8A8A8;"
      "}"
      "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
      "    background: transparent;"
      "    border: none;"
      "    height: 0px;"
      "}"
      "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
      "    background: transparent;"
      "}";

  QString darkScrollbarStyle =
      "/* Dark Vertical Scrollbar */"
      "QScrollBar:vertical {"
      "    background: #2D2D2D;"
      "    width: 10px;"
      "    margin: 2px;"
      "}"
      "QScrollBar::handle:vertical {"
      "    background: #606060;"
      "    border-radius: 4px;"
      "    border: 1px solid #404040;"
      "    min-height: 30px;"
      "}"
      "QScrollBar::handle:vertical:hover {"
      "    background: #707070;"
      "}"
      "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
      "    background: transparent;"
      "    border: none;"
      "    height: 0px;"
      "}"
      "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
      "    background: transparent;"
      "}";

  QString vsbarStyleSmall =
      "QScrollBar:vertical{"  // 垂直滑块整体
      "width:6px;"
      "background:rgb(255,255,255);"        // 背景色
      "padding-top:0px;"                    // 上预留位置（放置向上箭头）
      "padding-bottom:0px;"                 // 下预留位置（放置向下箭头）
      "padding-left:1px;"                   // 左预留位置（美观）
      "padding-right:1px;"                  // 右预留位置（美观）
      "border-left:0px solid #d7d7d7;}"     // 左分割线
      "QScrollBar::handle:vertical{"        // 滑块样式
      "background:rgb(202,197,191);"        // 滑块颜色
      "border-radius:2px;"                  // 边角圆润
      "min-height:60px;}"                   // 滑块最小高度
      "QScrollBar::handle:vertical:hover{"  // 鼠标触及滑块样式
      "background:#d0d0d0;}"                // 滑块颜色
      "QScrollBar::add-line:vertical{"      // 向下箭头样式
      "background:url() center no-repeat;}"
      "QScrollBar::sub-line:vertical{"  // 向上箭头样式
      "background:url() center no-repeat;}";

  QString vsbarStyleBig =
      "QScrollBar:vertical{"
      "width:48px;"              // 触摸区域大
      "background:transparent;"  // 背景透明
      "margin:0px;"
      "padding:0px;"
      "}"
      "QScrollBar::handle:vertical{"
      "background:rgba(150,150,150,180);"  // 半透明
      "border-radius:2px;"                 // 平时细
      "min-height:60px;"
      "margin-left:22px;"  // 居中
      "margin-right:22px;"
      "}"
      "QScrollBar::handle:vertical:hover{"
      "background:rgba(120,120,120,220);"
      "border-radius:6px;"  // 触摸时变宽
      "margin-left:18px;"
      "margin-right:18px;"
      "}"
      "QScrollBar::handle:vertical:pressed{"
      "background:rgba(100,100,100,255);"
      "}"
      "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical{"
      "height:0px;"  // 隐藏按钮
      "}";

  QString listStyleMain =
      "QListWidget{outline:0px;}"
      "QListWidget::item:selected{background:rgb(255,0,0); border:0px "
      "blue;margin:0px,0px,0px,0px;border-radius:5;"
      "color:white}";

  QString qssMenu =
      "QMenu {"
      "border: 1px solid rgb(172, 172, 172);"
      "border-radius: 0px; }"
      "QMenu::item {"
      "border-bottom: 0px solid rgb(172, 172, 172);"
      "padding:10px 10px;"
      "margin:0px 0px; }"
      "QMenu::item:selected {"
      "background-color: rgb(62, 186, 231); }";

  QDialog* getProgBar();
  void startSearch();
  void initSearchResults();

  void init();

  void setCurrentIndex(int index);
  void clearAll();
  int getCount();
  void delItem(int index);

  void addItem(QString text_tab, QString text0, QString text1, QString text2,
               QString text3, int itemH);

  void setCurrentIndexFromQW(QQuickWidget* qw, int index);
  void clearAllBakList(QQuickWidget* qw);
  int getCountFromQW(QQuickWidget* qw);
  void delItemFromQW(QQuickWidget* qw, int index);
  void addItemToQW(QQuickWidget* qw, QString text0, QString text1,
                   QString text2, QString text3, int itemH);

  QString getText3(QQuickWidget* qw, int index);
  int getCurrentIndexFromQW(QQuickWidget* qw);
  QString getText0(QQuickWidget* qw, int index);

  void modifyItemText2(QQuickWidget* qw, int index, QString strText);

  void modifyItemText0(QQuickWidget* qw, int index, QString strText);
  void insertItem(QQuickWidget* qw, QString text0, QString text1, QString text2,
                  QString text3, int curIndex);

  void gotoEnd(QQuickWidget* qw);

  void setSCrollPro(QObject* obj);

  void showDelMsgBox(QString title, QString info);

  void saveRecycleTabName(QString keyStr, QString tabName);
  QString getRecycleTabName(QString keyStr);

  QFont getNewFont(int maxSize);

  int getFontHeight();

  void modifyItemText3(QQuickWidget* qw, int index, QString strText);
  QInputDialog* inputDialog(QString windowsTitle, QString lblEdit,
                            QString defaultValue);

  void showGrayWindows();
  void closeGrayWindows();

  QString getRealPathFile(QString strFile);

  int getStrWidth(const QString str);

  QString getText2(QQuickWidget* qw, int index);
  QString getText1(QQuickWidget* qw, int index);

  void setScrollBarPos(QQuickWidget* qw, double pos);

  QString getLastModified(QString file);

  void setVPosForQW(QQuickWidget* qw, qreal pos);
  qreal getVPosForQW(QQuickWidget* qw);

  QString getCustomColor();

  QString getExecDone();

  void setExecDone(QString execDone);
  void Delay_MSec(unsigned int msec);
  void Sleep(int msec);
  void showToastMessage(QString msg);

  void openFilePicker();
  void closeFilePicker();
  void showAndroidProgressBar();
  void closeAndroidProgressBar();

  QString getKeyType();
  void setAndroidProgressInfo(QString info);

  void setQLabelImage(QLabel* lbl, int w, int h, QString imgFile);

  void playMyText(QString text);
  void stopPlayMyText();
  void startRecord(QString file);
  void stopRecord();
  void playRecord(QString file);
  void stopPlayRecord();

  QString FormatHHMMSS(qint32 total);

  void openDateTimePicker();

  void setDateTimePickerFlag(QString flag, int y, int m, int d, int h, int mm,
                             QString dateFlag);

  QStringList getDateTimePickerValue();

  QString getDateTimeFlag();
  double updateMicStatus();
  int getPlayDuration();
  int getPlayPosition();
  bool getPlaying();

  void seekTo(QString strPos);
  void startPlay();
  void pausePlay();
  void showTempActivity();
  void delay_MSec(unsigned int msec);

  int checkRecordAudio();

  void setMDTitle(QString strTitle);
  void setMDFile(QString strMDFile);
  void setAndroidFontSize(int nSize);

  bool decompressWithPassword(const QString& zipPath, const QString& extractDir,
                              const QString& password);

  bool compressFile(const QString& zipPath, const QString& filePath,
                    const QString& password);

  bool encryptFile(const QString& inputPath, const QString& outputPath,
                   const QString& password);
  bool decryptFile(const QString& inputPath, const QString& outputPath,
                   const QString& password);
  QString useDec(QString enc_file);
  QString useEnc(QString m_file);

  bool encryptFile_Old(const QString& inputPath, const QString& outputPath,
                       const QString& password);

  bool decompressFileWithZlib(const QString& sourcePath,
                              const QString& destPath);
  bool compressFileWithZlib(const QString& sourcePath, const QString& destPath,
                            int level);

  bool compressDirectory(const QString& zipPath, const QString& sourceDir,
                         const QString& password);

  void setOSFlag();

  void setDark(bool dark);

  void setEditLightMode(QTextEdit* textEdit);
  void setEditDarkMode(QTextEdit* textEdit);

  bool createDatabase(const QString& dbFileName);
  static void saveTreeToDB(QTreeWidget* tree, const QString& dbFileName);
  static void loadTreeFromDB(QTreeWidget* tree, const QString& dbFileName);

  void setToolButtonStyle(QToolButton* btn, bool isDark);

  QString setCurrentDateValue();

  QString formatSecondsToHMS(qlonglong seconds);
  QStringList removeDuplicatesFromQStringList(const QStringList& list);
  QString setCurrentDateTimeValue();
  bool getLockScreenStatus();

  void closeAndroidKeyboard();

  void set_ToolButtonStyle(QObject* parent);
  void set_PushButtonStyle(QObject* parent);
  QObjectList getAllToolButton(QObjectList lstUIControls);
  QObjectList getAllPushButton(QObjectList lstUIControls);
  QObjectList getAllTreeWidget(QObjectList lstUIControls);
  QObjectList getAllUIControls(QObject* parent);
  QString secondsToTime(ulong totalTime);
  bool copyFileToPath(QString sourceDir, QString toDir, bool coverFileIfExist);

  QString highlightTextInHtml(const QString& originalText,
                              const QString& targetText,
                              const QString& color = "red", bool bold = true);

  QString convertDataToUnicode(QByteArray data);

  void upIniFile(QString tempFile, QString endFile);
  QStringList getMdFilesInDir(const QString& dirPath,
                              bool includeFullPath = false);

  int getFlagToday(QTreeWidget* tw);
  QString getFileUTCString(const QString& file);
  QString getBaseFlag(const QString& file);

  void setAccessCount(int count);

  int getAccessCount();
  bool sendMailWithAttachment(const QString& recipient = "",
                              const QString& filePath = "");

  QObjectList getAllTextEdit(QObjectList lstUIControls);
  QObjectList getAllLineEdit(QObjectList lstUIControls);
  void setLineEditToolBar(QObject* parent, EditEventFilter* editFilter);
  void setTextEditToolBar(QObject* parent, EditEventFilter* editFilter);

  bool isInChina();

  void gotoBegin(QQuickWidget* qw);

  QList<SearchItem> exportAllDataForSearch();
  QList<SearchItem> data_for_search;
  void startSearch(QList<SearchItem> data, const QString& searchStr);

  void delayDelFile(const QString& filePath);

  float getSystemFontScale();

  void sendTouch(QQuickWidget* quickWidget);

  void clearAllNotesList(QQuickWidget* qw);

  QString generateRandom3();

  void safeCloseInfoWindow(Method* m);

  QString escapeAllHtml(const QString& src);
  void setLocalAIModelEnabled(bool isLocalAI);
  void setAIAPIEnabled(bool isAIAPI);

  void exitSystemFullscreen();
  protected:
  bool eventFilter(QObject* watchDlgSearch, QEvent* evn) override;

 signals:
  void sigUpdateProgressAndText(const QString& showText, int progMax,
                                int progVal);

 public slots:

  void clickMainDate();

  void clickMainDateData();
  void clickMainEventData();
  void reeditMainEventData();

 private slots:

 private:
  int count1 = 0;
  int count2 = 0;

  int secondCounter = 0;

  void setCellText(int row, int column, QString str, QTableWidget* table);
  void generateData(int count);
  int nProgressBarType = 2;

  int x, y, w, h;

  void setMainTabCurrentIndex();
  QString quazipErrorString(int code);
  bool addFilesToZip(void* zip_handle, const QString& currentDir,
                     const QString& baseDir);
  QByteArray generateRandomBytes(int length);
  QByteArray deriveKey(const QString& password, const QByteArray& salt,
                       int keyLength);

  static bool isUtf8(const QByteArray& data);
  static bool isValidText(const QString& text);
  bool isInChinaOnline(int timeout = 3000);
  QString getLocalIP();
};

#endif  // METHOD_H
