#ifndef METHOD_H
#define METHOD_H

#include <lib/quazip/quazip.h>
#include <lib/quazip/quazipfile.h>
#include <lib/quazip/quazipnewinfo.h>
#include <lib/quazip/unzip.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <zlib.h>

#include <QByteArray>
#include <QConicalGradient>
#include <QCryptographicHash>
#include <QDebug>
#include <QDialog>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QInputDialog>
#include <QInputMethod>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif

#include <QLabel>
#include <QPainter>
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

#include "src/Comm/enhancedcolorpicker.h"

namespace Ui {
class Method;
}

class Method : public QDialog {
  Q_OBJECT

 public:
  explicit Method(QWidget *parent = nullptr);
  ~Method();
  Ui::Method *ui;

  EnhancedColorPicker *m_EnColorPicker = nullptr;

  QMenu *menuNoteBook = nullptr;
  QMenu *menuNoteList = nullptr;

  bool androidCopyFile(QString src, QString des);

  static QString getFileSize(const qint64 &size, int precision);

  bool isClickLink = false;
  QWidget *m_widget;
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

  QString listWidgetStyle =
      "QListWidget::indicator{width:25;height:25;right: 5px;}"
      "QListView {outline: none;}"
      "#listWidget::item {background-color: #ffffff;color: #000000;border: "
      "transparent;border-bottom: 1px solid #dbdbdb;padding: 8px;height: 85;}"
      "#listWidget::item:hover {background-color: #f5f5f5;}"
      "#listWidget::item:selected {border-left: 5px solid #777777;}";

  QString setPushButtonQss(
      QPushButton *btn,                              // 按钮对象
      int radius = 5,                                // 圆角半径
      int padding = 8,                               // 间距
      const QString &normalColor = "#34495E",        // 正常颜色
      const QString &normalTextColor = "#FFFFFF",    // 文字颜色
      const QString &hoverColor = "#4E6D8C",         // 悬停颜色
      const QString &hoverTextColor = "#F0F0F0",     // 悬停文字颜色
      const QString &pressedColor = "#2D3E50",       // 按下颜色
      const QString &pressedTextColor = "#B8C6D1");  // 按下文字颜色

  QString setToolButtonQss(
      QToolButton *btn,                              // 按钮对象
      int radius = 5,                                // 圆角半径
      int padding = 8,                               // 间距
      const QString &normalColor = "#34495E",        // 正常颜色
      const QString &normalTextColor = "#FFFFFF",    // 文字颜色
      const QString &hoverColor = "#4E6D8C",         // 悬停颜色
      const QString &hoverTextColor = "#F0F0F0",     // 悬停文字颜色
      const QString &pressedColor = "#2D3E50",       // 按下颜色
      const QString &pressedTextColor = "#B8C6D1");  // 按下文字颜色

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

  QDialog *getProgBar();
  void startSearch();
  void initSearchResults();

  void init();

  void setCurrentIndex(int index);
  void clearAll();
  int getCount();
  void delItem(int index);

  void addItem(QString text_tab, QString text0, QString text1, QString text2,
               QString text3, int itemH);

  void setCurrentIndexFromQW(QQuickWidget *qw, int index);
  void clearAllBakList(QQuickWidget *qw);
  int getCountFromQW(QQuickWidget *qw);
  void delItemFromQW(QQuickWidget *qw, int index);
  void addItemToQW(QQuickWidget *qw, QString text0, QString text1,
                   QString text2, QString text3, int itemH);

  QString getText3(QQuickWidget *qw, int index);
  int getCurrentIndexFromQW(QQuickWidget *qw);
  QString getText0(QQuickWidget *qw, int index);

  void modifyItemText2(QQuickWidget *qw, int index, QString strText);

  void modifyItemText0(QQuickWidget *qw, int index, QString strText);
  void insertItem(QQuickWidget *qw, QString text0, QString text1, QString text2,
                  QString text3, int curIndex);

  void gotoEnd(QQuickWidget *qw);

  void setSCrollPro(QObject *obj);

  void showDelMsgBox(QString title, QString info);

  void saveRecycleTabName(QString keyStr, QString tabName);
  QString getRecycleTabName(QString keyStr);

  QFont getNewFont(int maxSize);

  int getFontHeight();

  void closeQtKeyboard();

  void modifyItemText3(QQuickWidget *qw, int index, QString strText);
  QInputDialog *inputDialog(QString windowsTitle, QString lblEdit,
                            QString defaultValue);

  void showGrayWindows();
  void closeGrayWindows();

  QString getRealPathFile(QString strFile);

  int getStrWidth(const QString str);

  void set_ToolButtonStyle2(QObject *parent);

  QString getText2(QQuickWidget *qw, int index);
  QString getText1(QQuickWidget *qw, int index);

  void setScrollBarPos(QQuickWidget *qw, double pos);

  QString getLastModified(QString file);

  void setVPosForQW(QQuickWidget *qw, qreal pos);
  qreal getVPosForQW(QQuickWidget *qw);

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

  void setQLabelImage(QLabel *lbl, int w, int h, QString imgFile);

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

  bool decompressWithPassword(const QString &zipPath, const QString &extractDir,
                              const QString &password);

  bool compressFile(const QString &zipPath, const QString &filePath,
                    const QString &password);

  bool encryptFile(const QString &inputPath, const QString &outputPath,
                   const QString &password);
  bool decryptFile(const QString &inputPath, const QString &outputPath,
                   const QString &password);
  QString useDec(QString enc_file);
  QString useEnc(QString m_file);

  bool encryptFile_Old(const QString &inputPath, const QString &outputPath,
                       const QString &password);

  bool decompressFileWithZlib(const QString &sourcePath,
                              const QString &destPath);
  bool compressFileWithZlib(const QString &sourcePath, const QString &destPath,
                            int level);

  bool compressDirectory(const QString &zipPath, const QString &sourceDir,
                         const QString &password);

  void setOSFlag();

  void setDark(bool dark);

  void setEditLightMode(QTextEdit *textEdit);
  void setEditDarkMode(QTextEdit *textEdit);

  bool createDatabase(const QString &dbFileName);
  static void saveTreeToDB(QTreeWidget *tree, const QString &dbFileName);
  static void loadTreeFromDB(QTreeWidget *tree, const QString &dbFileName);

  void setToolButtonStyle(QToolButton *btn, bool isDark);

  QString setCurrentDateValue();

  QString formatSecondsToHMS(qlonglong seconds);
  QStringList removeDuplicatesFromQStringList(const QStringList &list);
  QString setCurrentDateTimeValue();
  bool getLockScreenStatus();

  void closeAndroidKeyboard();

  void callJavaForceDisconnectInputMethod();

  void set_ToolButtonStyle(QObject *parent);
  void set_PushButtonStyle(QObject *parent);
  QObjectList getAllToolButton(QObjectList lstUIControls);
  QObjectList getAllPushButton(QObjectList lstUIControls);
  QObjectList getAllTreeWidget(QObjectList lstUIControls);
  QObjectList getAllUIControls(QObject *parent);
  QString secondsToTime(ulong totalTime);
  bool copyFileToPath(QString sourceDir, QString toDir, bool coverFileIfExist);

  QString highlightTextInHtml(const QString &originalText,
                              const QString &targetText,
                              const QString &color = "red", bool bold = true);

  QString convertDataToUnicode(QByteArray data);

  void upIniFile(QString tempFile, QString endFile);
  QStringList getMdFilesInDir(const QString &dirPath, bool includeFullPath = false);
  int calculateCenterYForScreen(QWidget *widget);
  protected:
  bool eventFilter(QObject *watchDlgSearch, QEvent *evn) override;

 public slots:

  void showNoteBookMenu(int x, int y);
  void showNotsListMenu(int x, int y);
  void clickMainDate();

  void clickMainDateData();
  void clickMainEventData();
  void reeditMainEventData();
  void setTypeRenameText();
  void okType();
 private slots:

 private:
  void setCellText(int row, int column, QString str, QTableWidget *table);
  void generateData(int count);
  int nProgressBarType = 2;

  int x, y, w, h;

  void setMainTabCurrentIndex();
  QString quazipErrorString(int code);
  bool addFilesToZip(void *zip_handle, const QString &currentDir,
                     const QString &baseDir);
  QByteArray generateRandomBytes(int length);
  QByteArray deriveKey(const QString &password, const QByteArray &salt,
                       int keyLength);

  static bool isUtf8(const QByteArray &data);
  static bool isValidText(const QString &text);
};

class IOSCircularProgress : public QWidget {
  Q_OBJECT
  Q_PROPERTY(int rotationAngle READ rotationAngle WRITE setRotationAngle)
  Q_PROPERTY(qreal progress READ progress WRITE setProgress)
 public:
  explicit IOSCircularProgress(QWidget *parent = nullptr)
      : QWidget(parent),
        m_rotation(0),
        m_progress(0),
        m_penWidth(5),
        m_seconds(0) {
    setFixedSize(60, 60);

    // 旋转动画
    m_rotateAnimation = new QPropertyAnimation(this, "rotationAngle", this);
    m_rotateAnimation->setDuration(1500);
    m_rotateAnimation->setLoopCount(-1);
    m_rotateAnimation->setStartValue(0);
    m_rotateAnimation->setEndValue(360);
    m_rotateAnimation->start();

    // 进度动画（示例用）
    QPropertyAnimation *progressAnim =
        new QPropertyAnimation(this, "progress", this);
    progressAnim->setDuration(3000);
    progressAnim->setLoopCount(-1);
    progressAnim->setStartValue(0);
    progressAnim->setEndValue(1);
    progressAnim->start();

    // 初始化计时器，每秒更新一次
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this,
            &IOSCircularProgress::updateSeconds);
    m_timer->start(1000);  // 1000毫秒 = 1秒
  }

  int rotationAngle() const { return m_rotation; }
  void setRotationAngle(int angle) {
    m_rotation = angle;
    update();
  }

  qreal progress() const { return m_progress; }
  void setProgress(qreal p) {
    m_progress = qBound<qreal>(0, p, 1);
    update();
  }

 protected:
  void paintEvent(QPaintEvent *) override {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const qreal outerRadius = qMin(width(), height()) * 0.5 - m_penWidth;
    const QPointF center = rect().center();

    // 绘制背景环
    QPen bgPen(QColor("#E5E5E5"));
    bgPen.setWidth(m_penWidth);
    bgPen.setCapStyle(Qt::RoundCap);
    p.setPen(bgPen);
    p.drawEllipse(center, outerRadius, outerRadius);

    // 绘制进度环
    if (m_progress > 0) {
      QConicalGradient gradient(center, -m_rotation);
      gradient.setColorAt(0.0, QColor("#007AFF"));
      gradient.setColorAt(0.5, QColor("#34C759"));
      gradient.setColorAt(1.0, QColor("#007AFF"));

      QPen progressPen(QBrush(gradient), m_penWidth);
      progressPen.setCapStyle(Qt::RoundCap);
      p.setPen(progressPen);

      int startAngle = (-m_rotation + 90) * 16;
      int spanAngle = -m_progress * 360 * 16;

      p.drawArc(QRectF(center.x() - outerRadius, center.y() - outerRadius,
                       outerRadius * 2, outerRadius * 2),
                startAngle, spanAngle);
    }

    // 绘制高光
    QRadialGradient highlight(center, outerRadius * 2);
    highlight.setColorAt(0.0, QColor(255, 255, 255, 150));
    highlight.setColorAt(0.3, QColor(255, 255, 255, 50));
    highlight.setColorAt(1.0, Qt::transparent);
    p.setBrush(highlight);
    p.setPen(Qt::NoPen);
    p.drawEllipse(center, outerRadius + m_penWidth / 2,
                  outerRadius + m_penWidth / 2);

    // 绘制中央秒数
    p.setPen(QColor("#333333"));  // 文本颜色
    QFont font = p.font();
    // 调整字体大小以确保两位数不会超出圆圈
    font.setPointSize(12);
    p.setFont(font);

    // 文本居中显示
    p.drawText(rect(), Qt::AlignCenter, QString::number(m_seconds));
  }

 private slots:
  // 更新秒数
  void updateSeconds() {
    m_seconds++;
    // 如果需要，可以在这里添加重置逻辑，例如超过99秒后重置
    // if (m_seconds > 99) m_seconds = 0;
    update();  // 触发重绘
  }

 private:
  QPropertyAnimation *m_rotateAnimation;
  QTimer *m_timer;  // 计时器
  int m_rotation;
  qreal m_progress;
  int m_penWidth;
  int m_seconds;  // 秒数计数器
};

class IOSCircularProgress_NoTimer : public QWidget {
  Q_OBJECT
  Q_PROPERTY(int rotationAngle READ rotationAngle WRITE setRotationAngle)
  Q_PROPERTY(qreal progress READ progress WRITE setProgress)
 public:
  explicit IOSCircularProgress_NoTimer(QWidget *parent = nullptr)
      : QWidget(parent), m_rotation(0), m_progress(0), m_penWidth(5) {
    setFixedSize(60, 60);

    // 旋转动画
    m_rotateAnimation = new QPropertyAnimation(this, "rotationAngle", this);
    m_rotateAnimation->setDuration(1500);
    m_rotateAnimation->setLoopCount(-1);
    m_rotateAnimation->setStartValue(0);
    m_rotateAnimation->setEndValue(360);
    m_rotateAnimation->start();

    // 进度动画（示例用）
    QPropertyAnimation *progressAnim =
        new QPropertyAnimation(this, "progress", this);
    progressAnim->setDuration(3000);
    progressAnim->setLoopCount(-1);
    progressAnim->setStartValue(0);
    progressAnim->setEndValue(1);
    progressAnim->start();
  }

  int rotationAngle() const { return m_rotation; }
  void setRotationAngle(int angle) {
    m_rotation = angle;
    update();
  }

  qreal progress() const { return m_progress; }
  void setProgress(qreal p) {
    m_progress = qBound<qreal>(0, p, 1);
    update();
  }

 protected:
  void paintEvent(QPaintEvent *) override {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const qreal outerRadius = qMin(width(), height()) * 0.5 - m_penWidth;
    const QPointF center = rect().center();

    // 绘制背景环
    QPen bgPen(QColor("#E5E5E5"));
    bgPen.setWidth(m_penWidth);
    bgPen.setCapStyle(Qt::RoundCap);
    p.setPen(bgPen);
    p.drawEllipse(center, outerRadius, outerRadius);

    // 绘制进度环
    if (m_progress > 0) {
      QConicalGradient gradient(center, -m_rotation);
      gradient.setColorAt(0.0, QColor("#007AFF"));
      gradient.setColorAt(0.5, QColor("#34C759"));
      gradient.setColorAt(1.0, QColor("#007AFF"));

      QPen progressPen(QBrush(gradient), m_penWidth);
      progressPen.setCapStyle(Qt::RoundCap);
      p.setPen(progressPen);

      int startAngle = (-m_rotation + 90) * 16;
      int spanAngle = -m_progress * 360 * 16;

      p.drawArc(QRectF(center.x() - outerRadius, center.y() - outerRadius,
                       outerRadius * 2, outerRadius * 2),
                startAngle, spanAngle);
    }

    // 绘制高光
    QRadialGradient highlight(center, outerRadius * 2);
    highlight.setColorAt(0.0, QColor(255, 255, 255, 150));
    highlight.setColorAt(0.3, QColor(255, 255, 255, 50));
    highlight.setColorAt(1.0, Qt::transparent);
    p.setBrush(highlight);
    p.setPen(Qt::NoPen);
    p.drawEllipse(center, outerRadius + m_penWidth / 2,
                  outerRadius + m_penWidth / 2);
  }

 private:
  QPropertyAnimation *m_rotateAnimation;
  int m_rotation;
  qreal m_progress;
  int m_penWidth;
};

#endif  // METHOD_H
