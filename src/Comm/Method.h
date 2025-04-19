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
#include <QLabel>
#include <QPainter>
#include <QPropertyAnimation>
#include <QQuickWidget>
#include <QStandardPaths>
#include <QString>
#include <QTableWidget>
#include <QTimer>
#include <QWidget>

/*
#define MZ_ZIP_USE_CRYPTO  // 启用 AES 加密支持
#define MZ_ZIP_USE_UTF8    // 强制使用 UTF-8 编码
#include "mz.h"
#include "mz_os.h"
#include "mz_strm.h"
#include "mz_strm_os.h"
#include "mz_strm_zlib.h"
#include "mz_zip.h"
#include "mz_zip_rw.h"
*/

namespace Ui {
class Method;
}

class Method : public QDialog {
  Q_OBJECT

 public:
  explicit Method(QWidget *parent = nullptr);
  ~Method();
  Ui::Method *ui;

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

  QString vsbarStyle =
      "QScrollBar:vertical{"  // 垂直滑块整体
      "width:30px;"
      "background:#FFFFFF;"                 // 背景色
      "padding-top:25px;"                   // 上预留位置（放置向上箭头）
      "padding-bottom:25px;"                // 下预留位置（放置向下箭头）
      "padding-left:3px;"                   // 左预留位置（美观）
      "padding-right:3px;"                  // 右预留位置（美观）
      "border-left:1px solid #d7d7d7;}"     // 左分割线
      "QScrollBar::handle:vertical{"        // 滑块样式
      "background:#dbdbdb;"                 // 滑块颜色
      "border-radius:6px;"                  // 边角圆润
      "min-height:60px;}"                   // 滑块最小高度
      "QScrollBar::handle:vertical:hover{"  // 鼠标触及滑块样式
      "background:#d0d0d0;}"                // 滑块颜色
      "QScrollBar::add-line:vertical{"      // 向下箭头样式
      "background:url(:/src/down.png) bottom no-repeat;}"
      "QScrollBar::sub-line:vertical{"  // 向上箭头样式
      "background:url(:/src/up.png) top no-repeat;}";

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
      "border-radius: 3px; }"
      "QMenu::item {"
      "border-bottom: 1px solid rgb(172, 172, 172);"
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

  void closeKeyboard();

  void modifyItemText3(QQuickWidget *qw, int index, QString strText);
  QInputDialog *inputDialog(QString windowsTitle, QString lblEdit,
                            QString defaultValue);

  void setDark(QString strDark);

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

  bool compressDirectoryNG(const QString &zipPath, const QString &sourceDir,
                           const QString &password);
  bool compressFileNG(const QString &zipPath, const QString &filePath,
                      const QString &password);
  bool decompressWithPasswordNG(const QString &zipPath,
                                const QString &extractDir,
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
};

class IOSCircularProgress : public QWidget {
  Q_OBJECT
  Q_PROPERTY(int rotationAngle READ rotationAngle WRITE setRotationAngle)
  Q_PROPERTY(qreal progress READ progress WRITE setProgress)
 public:
  explicit IOSCircularProgress(QWidget *parent = nullptr)
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
