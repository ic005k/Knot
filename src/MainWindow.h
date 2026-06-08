#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qmath.h>

#include <QAbstractButton>
#include <QAccelerometerReading>
#include <QCheckBox>
#include <QClipboard>
#include <QGeoServiceProvider>
#include <QGeoServiceProviderFactory>
#include <QInputMethod>
#include <QPointingDevice>
#include <QSplashScreen>
#include <QtConcurrent/QtConcurrentRun>

#ifdef Q_OS_ANDROID

#include <QCoreApplication>
#include <QJniEnvironment>
#include <QJniObject>

#endif

#include <QBarCategoryAxis>
#include <QBarSeries>
#include <QBarSet>
#include <QChart>
#include <QChartView>
#include <QComboBox>
#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QGyroscope>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickWidget>
#include <QRandomGenerator>
#include <QScatterSeries>
#include <QScrollBar>
#include <QScroller>
#include <QSensorManager>
#include <QSettings>
#include <QSplineSeries>
#include <QSplitter>
#include <QStringList>
#include <QTextEdit>
#include <QThread>
#include <QTimer>
#include <QToolTip>
#include <QTreeWidgetItem>
#include <QValueAxis>

#include "MainHelper.h"
#include "src/AboutThis.h"
#include "src/CategoryList.h"
#include "src/CloudBackup.h"
#include "src/Comm/Method.h"
#include "src/Comm/ReceiveShare.h"
#include "src/Comm/ShowMessage.h"
#include "src/Comm/TextEditToolbar.h"
#include "src/EditRecord.h"
#include "src/Exercise/Steps.h"
#include "src/Exercise/StepsOptions.h"
#include "src/LoadPic.h"
#include "src/MyThread.h"
#include "src/Notes/Notes.h"
#include "src/Notes/NotesList.h"
#include "src/Preferences.h"
#include "src/Reader/Reader.h"
#include "src/Reader/ReaderSet.h"
#include "src/Report.h"
#include "src/Todo/Todo.h"
#include "src/Todo/TodoAlarm.h"
#include "src/defines.h"
#include "ui_AboutThis.h"
#include "ui_CategoryList.h"
#include "ui_CloudBackup.h"
#include "ui_DateSelector.h"
#include "ui_MainWindow.h"
#include "ui_Notes.h"
#include "ui_NotesList.h"
#include "ui_Preferences.h"
#include "ui_ShowMessage.h"
#include "ui_StepsOptions.h"
#include "ui_Todo.h"
#include "ui_TodoAlarm.h"

class SaveThread;
class ReadChartThread;
class ReadTWThread;
class ReadEBookThread;
class BakDataThread;
class ImportDataThread;
class SearchThread;
class UpdateGpsMapThread;
class GetGpsDataThread;
class SliderButton;

class AndroidTouchFixer : public QObject {
  Q_OBJECT
 public:
  explicit AndroidTouchFixer(QObject* parent = nullptr) : QObject(parent) {}

  static void wakeup() {
#ifdef Q_OS_ANDROID
    // 延迟150ms，等界面完全稳定
    QTimer::singleShot(150, []() {
      QWidget* window = QApplication::activeWindow();
      if (!window) return;

      // =========================================
      // 正确写法：清理鼠标捕获（修复Qt内部状态卡死）
      // =========================================
      QWidget* grabber = QWidget::mouseGrabber();
      if (grabber) {
        grabber->releaseMouse();
      }

      // 发送一组无害的点击，强制重置Qt点击状态机
      // 坐标(1,1)绝对不会点到任何按钮
      QPoint p(1, 1);

      QMouseEvent evtPress(QEvent::MouseButtonPress, p, Qt::LeftButton,
                           Qt::LeftButton, Qt::NoModifier);
      QApplication::sendEvent(window, &evtPress);

      QMouseEvent evtRelease(QEvent::MouseButtonRelease, p, Qt::LeftButton,
                             Qt::LeftButton, Qt::NoModifier);
      QApplication::sendEvent(window, &evtRelease);
    });
#endif
  }
};

class SearchWorker : public QObject {
  Q_OBJECT
 public:
  QList<QString> resultsList;

 public slots:
  void startSearch(QList<SearchItem> data, const QString& searchStr) {
    // resultsList.clear();

    QList<QString> localResults;

    for (const SearchItem& item : data) {
      QString tabStr = item.tabName;
      QString strYear = item.strYear;
      QString weeks = item.weeks;
      QString day = item.day;
      QString strTime = item.strTime;
      QString txt1 = item.txt1;
      QString txt2 = item.txt2;
      QString txt3 = item.txt3;

      QString txt0;
      if (strTime.split(".").count() == 2) {
        txt0 = strYear + " " + day + " " + weeks + " " +
               strTime.split(".").at(1).trimmed();
      }

      QStringList list;
      bool isYes = false;

      if (searchStr.contains("&")) {
        list = searchStr.split("&");
        bool is0 = false, is1 = false, is2 = false, is3 = false;

        for (int n = 0; n < list.count(); n++) {
          QString str = list.at(n);
          str = str.trimmed();

          if (str.length() > 0) {
            if (strYear.contains(str) || day.contains(str) ||
                weeks.contains(str)) {
              is0 = true;
              txt0 = m_Method->highlightTextInHtml(txt0, str);
            }
            if (txt1.contains(str)) {
              is1 = true;
              txt1 = m_Method->highlightTextInHtml(txt1, str);
            }
            if (txt2.contains(str)) {
              is2 = true;
              txt2 = m_Method->highlightTextInHtml(txt2, str);
            }
            if (txt3.contains(str)) {
              is3 = true;
              txt3 = m_Method->highlightTextInHtml(txt3, str);
            }
          }
        }

        if (list.count() == 2) {
          if (is0 && is1) isYes = true;
          if (is0 && is2) isYes = true;
          if (is0 && is3) isYes = true;
          if (is1 && is2) isYes = true;
          if (is1 && is3) isYes = true;
          if (is2 && is3) isYes = true;
        }

        if (list.count() == 3) {
          if (is0 && is1 && is2) isYes = true;
          if (is0 && is1 && is3) isYes = true;
          if (is0 && is2 && is3) isYes = true;
          if (is1 && is2 && is3) isYes = true;
        }

        if (list.count() >= 4) {
          if (is0 && is1 && is2 && is3) isYes = true;
        }

        QString s_total = txt0 + txt1 + txt2 + txt3;
        int n_count = 0;
        for (int x = 0; x < list.count(); x++) {
          QString str = list.at(x);
          if (str.length() > 0) {
            if (s_total.contains(str)) {
              n_count++;
            }
          }
        }

        if (isYes) {
          if (n_count < list.count()) isYes = false;
        }

      } else {
        if (txt1.contains(searchStr) || txt2.contains(searchStr) ||
            txt3.contains(searchStr)) {
          isYes = true;

          if (txt1.contains(searchStr)) {
            txt1 = m_Method->highlightTextInHtml(txt1, searchStr);
          }
          if (txt2.contains(searchStr)) {
            txt2 = m_Method->highlightTextInHtml(txt2, searchStr);
          }
          if (txt3.contains(searchStr)) {
            txt3 = m_Method->highlightTextInHtml(txt3, searchStr);
          }
        }
      }

      if (isYes) {
        localResults.append(tabStr + "=|=" + txt0 + "=|=" + txt1 +
                            "=|=" + txt2 + "=|=" + txt3);
      }
    }

    emit searchFinished(localResults);
  }

 signals:
  void searchFinished(const QList<QString>& results);
};

#include <QMetaType>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QWidget* parent = nullptr);
  ~MainWindow();
  Ui::MainWindow* ui;

  void emitAndroidBackSignal() { emit androidBackSignal(); }

  QMenu* mainMenu = nullptr;

  int x, y, w, h;

  QInputDialog* m_RenameDlg = nullptr;

  QString labelNormalStyleSheet = R"(/* 动态适配明暗模式 */
    QLabel {
        background-color: qlineargradient(
            x1:0, y1:0, x2:1, y2:0,
            stop:0 palette(light),  /* 明色端 */
            stop:1 palette(mid)     /* 过渡色 */
        );
        color: palette(window-text); /* 跟随系统文本色 */
        padding: 5px;
        border-radius: 4px;         /* 可选：增加圆角提升质感 */
    })";

  QString labelEnSyncStyleSheet =
      "background-color:qlineargradient(spread:pad,x1:1,y1:0,x2:0,y2:0,stop:0 "
      "#87CEFF,stop:1 #FFAEB9); color:black;selection-background-color: "
      "lightblue;border-radius: 4px;";

  QString labelEncStyleSheet =
      "QLabel { "
      "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, "
      "stop:0 rgba(255, 204, 204, 255), stop:1 rgba(220, 220, 220, 255)); "
      "color: black;border-radius: 4px;}";

  QString labelSyncStyleSheet =
      "QLabel { "
      "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, "
      "stop:0 #87CEFF, stop:1 rgba(220, 220, 220, 255)); "
      "color: black;border-radius: 4px;}";

  QString mySearchText = "";

  bool isOpenSearchResult = false;

  bool isEditItem = false;
  bool isDelItem = false;

  QString strLatestModify = tr("None");

  QDialog* dlgProg = nullptr;

  void setMini();

  int days = 45;
  int newHeight;

  bool isMemoVisible;
  bool isReaderVisible;
  QString treeStyle = "QTreeWidget::item {height: 28;}";
  int mainHeight = 0;

  QString strText;
  QString strRefreshUrl;
  QString CurrentYear = "20xx";
  qreal curx = 0;
  bool isMousePress = false;
  bool isMouseMove = false;

  bool initMain = false;

  QString LatestTime = tr("Latest Time");
  QStringList timeLines;
  QString loginTime;

  int mwh;

  QString listStyle =
      "QListWidget{item-background: "
      "rgb(244,23,24); item-color:rgb(255,255,255); "
      "border-radius:2px;border:2px "
      "solid red;}";
  int sRate = 0;
  qreal ax, ay, az, gx, gy, gz;
  int testCount1 = 0;
  int testCount = 0;
  int smallCount = 0;

  qlonglong CurTableCount = 0;

  static void get_Today(QTreeWidget*);

  SaveThread* mySaveThread;
  ReadChartThread* myReadChartThread;
  ReadTWThread* m_ReadTWThread;
  ReadEBookThread* myReadEBookThread;
  BakDataThread* myBakDataThread;
  ImportDataThread* myImportDataThread;
  SearchThread* mySearchThread;
  UpdateGpsMapThread* myUpdateGpsMapThread;
  GetGpsDataThread* myGetGpsDataThread;

  QThread* m_workerThread;
  SearchWorker* m_searchWorker;

  static void ReadChartData();
  static int get_Day(QString date);
  static QString get_Year(QString date);
  static QString get_Month(QString date);
  void init_Options();

  QStringList chartCategories;
  QList<QVariant> qmlFreqValues;
  QList<QVariant> qmlAmountValues;

  AboutThis* m_AboutThis;
  EditRecord* m_EditRecord;
  Todo* m_Todo;
  Report* m_Report;
  Preferences* m_Preferences;

  Reader* m_Reader;
  TodoAlarm* m_TodoAlarm;
  DateSelector* m_DateSelector = nullptr;

  ReceiveShare* m_ReceiveShare;
  MainHelper* m_MainHelper;

  QList<QTreeWidgetItem*> findItemList;
  bool isFindTextChange = false;
  int findPos = 0;

  QTimer* timer;

  QTimer* timerMousePress;
  QTimer* timerSyncData;
  QTimer* tmeStartRecordAudio;

  static void readData(QTreeWidget*);

  static void saveTab();
  bool isSlide = false;
  void init_TotalData();
  void modify_Data();
  void add_Data(QTreeWidget*, QString, QString, QString);
  bool del_Data(QTreeWidget*);
  static QTreeWidget* get_tw(int tabIndex);

  static QStringList get_MonthList(QString strY, QString strM);
  static void drawMonthChart();

  static void SaveFile(QString);

  static void init_Stats(QTreeWidget*);

  void startSave(QString);
  void startRead(QString);
  static void drawDayChart();
  static void readDataInThread(int ExceptIndex);

  void stopJavaTimer();

  QString getYMD(QString date);
  bool bakData();
  bool importBakData(QString fileName);

  QString getTabText();

  void refreshMainUI();

  QString getSelectedText();

  int calcStringPixelWidth(QString s_str, QFont font, int n_font_size);
  int calcStringPixelHeight(QFont font, int n_font_size);

  void delItem(int index);
  int getCount();
  void clearAll();
  void reloadMain();
  void setCurrentIndex(int index);
  void gotoEnd();
  void gotoIndex(int index);
  void setItemHeight(int h);

  QString getTop(int index);
  QString getText0(int index);
  int getItemType(int index);
  int getCurrentIndex();
  bool setTWCurrentItem();
  QString getText1(int index);
  QString getText2(int index);

  void startInitReport();

  void startSyncData();

  int getMaxDay(QString sy, QString sm);
  void showProgress();

  void setScrollBarPos(double pos);

  void addItem(QString text0, QString text1, QString text2, QString text3,
               int itemH);

  void execDeskShortcut();

  void on_DelayCloseProgressBar();

  void clearWidgetFocus();

  int max_day = 31;

  void execNeedSyncNotes();

  void saveNeedSyncNotes();

  void updateMainTab();

  void on_btnPageUp_pressed();
  void on_btnPageNext_pressed();
  void on_btnBackDir_pressed();

  void on_btnUpload_pressed();
  void on_btnDownload_pressed();

  void sendFakeTouch();

 protected:
  void closeEvent(QCloseEvent* event) override;
  bool eventFilter(QObject* watch, QEvent* evn) override;
  void paintEvent(QPaintEvent* event) override;
  void changeEvent(QEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;

  void hideEvent(QHideEvent* event) override;

  void showEvent(QShowEvent* event) override;
 public slots:
  void on_btnCatalogue_pressed();

  void on_btnHome_pressed();

  void GetGpsDataThreadDone();

  void on_btnAutoStop_pressed();

  void on_btnMove();

  void on_btnSelText();

  void on_hSlider_sliderMoved(int position);
  void clickMainTab();

  void on_btnTodo_pressed();
  void readEBookDone();

  void on_btnShowFindNotes_pressed();

  void on_btnRename_pressed();

  void on_actionOneDriveBackupData();

  void on_actionReport_triggered();

  void on_btnCopy_pressed();

  void on_btnSearch_pressed();

  void on_btnCancelSel_pressed();

  void on_timerSyncData();
  void timerUpdate();

  void on_actionRename_triggered();

  void on_actionAdd_Tab_triggered();

  void on_actionDel_Tab_triggered();

  void on_tabWidget_currentChanged(int index);

  void on_btnModifyRecord();

  void on_actionExport_Data_triggered();

  void on_actionImport_Data_triggered();

  void on_twItemClicked();

  void on_twItemDoubleClicked();

  void on_actionAbout();

  void on_btnFind_pressed();

  void on_actionFind_triggered();

  void saveDone();

  void bakDataDone();

  void importDataDone();

  void readChartDone();

  void readTWDone();

  void searchDone();

  void on_actionPreferences_triggered();

  void on_btnSteps_pressed();

  void on_rbSteps();

  void on_btnSelTab_pressed();

  void on_btnBackReader_pressed();

  void on_btnAddTodo_clicked();
  void on_btnAddTodo_pressed();
  void on_btnReader_pressed();

  void on_btnOpen_pressed();

  void on_btnPages_pressed();

  void on_btnReadList_pressed();

  void on_actionBakFileList();

  void on_btnBack_One_pressed();

  void on_btnBackNotesGraph_pressed();

  void on_btnNotes_pressed();

  void on_btnZoomIn_pressed();

  void on_btnZoomOut_pressed();

  void on_openKnotBakDir();

  void reeditData();

  void clickData();

  void on_btnStartSearch_pressed();

  void on_btnChart();

  void on_btnBack_NotesSearchResult_pressed();

  void on_btnOpenSearchResult_pressed();

  void on_btnEditNote_pressed();

  void on_btnOpenNote_pressed();

  void on_btnStartDate_pressed();

  void on_btnEndDate_pressed();

  void on_ExecShortcut();

  void on_btnFindNotes_pressed();

  void on_btnShowBookmark_pressed();

  void on_btnBackBookList_pressed();

  void on_btnBackEditRecord_pressed();

  void on_btnBackNoteList_pressed();

  void on_btnBackReaderSet_pressed();

  void on_btnBackNoteRecycle_clicked();

  void on_timerMousePress();

  void on_btnBackImg_pressed();

  void on_btnReport();

  void on_btnAdd_pressed();

  void on_btnDel_pressed();

  void on_btnBackTodo_pressed();

  void on_btnHigh();

  void on_btnLow();

  void on_btnSetTime();

  void on_btnRecycle();

  void on_btnReturnRecycle_pressed();

  void on_btnClearRecycle_pressed();

  void on_btnDelRecycle_pressed();

  void on_btnRestoreRecycle_pressed();

  void on_editTodo_textChanged();

  void on_btnBackSteps_clicked();

  void on_btnReset_pressed();

  void on_btnBack_Report_pressed();

  void on_btnYear_pressed();

  void on_btnMonth_pressed();

  void on_btnCategory_pressed();

  void closeProgress();

  void selTab();

  void updateGpsMapDone();

  void on_btnBackTabRecycle_pressed();

  void on_btnBackSearch_pressed();

  void on_btnBackBakList_pressed();

  void on_btnBackSetTab_pressed();

  void on_btnBack_Tree_pressed();

  void on_btnCancelType_pressed();

  void on_actionTabRecycle();

  void on_actionShareFile();

  void on_btnUpMove_pressed();

  void on_btnDownMove_pressed();

  void on_btnDelNote_NoteBook_pressed();

  void on_btnMoveTo_pressed();

  void on_btnToPDF_pressed();

  void on_btnManagement_pressed();

  void on_btnNoteRecycle_pressed();

 private slots:
  void onAndroidBackHandle();

  void on_btnMenu_pressed();

  void on_btnSync_pressed();

  void on_btnPasteTodo_pressed();

  void on_btnClearSearchText_pressed();

  void on_btnImportBakList_pressed();

  void on_btnOkViewCate_pressed();

  void on_btnDelTabRecycle_pressed();

  void on_btnRestoreTab_pressed();

  void on_btnDelBakFile_pressed();

  void on_btnDelNoteRecycle_clicked();

  void on_btnRestoreNoteRecycle_clicked();

  void on_btnFindPreviousNote_pressed();

  void on_btnFindNextNote_pressed();

  void on_btnClearNoteFindText_pressed();

  void on_btnNoteBookMenu_pressed();

  void on_btnNoteMenu_pressed();

  void on_btnOkType_pressed();

  void on_btnDelType_pressed();

  void on_btnRenameType_pressed();

  void on_btnType_pressed();

  void on_btnOkEditRecord_pressed();

  void on_btnClearType_pressed();

  void on_btnClearDetails_pressed();

  void on_btnClearAmount_pressed();

  void on_editAmount_textChanged(const QString& arg1);

  void on_editCategory_textChanged(const QString& arg1);

  void on_editDetails_textChanged();

  void on_hsH_valueChanged(int value);

  void on_hsM_valueChanged(int value);

  void on_btn7_pressed();

  void on_btn8_pressed();

  void on_btn9_pressed();

  void on_btn4_pressed();

  void on_btn5_pressed();

  void on_btn6_pressed();

  void on_btn1_pressed();

  void on_btn2_pressed();

  void on_btn3_pressed();

  void on_btn0_pressed();

  void on_btnDot_pressed();

  void on_btnDel_Number_pressed();

  void on_btnOkBookList_pressed();

  void on_btnClearAllRecords_pressed();

  void on_btnAnd_pressed();

  void on_btnClear_pressed();

  void on_btnModify_pressed();

  void on_btnTabMoveUp_pressed();

  void on_btnTabMoveDown_pressed();

  void on_btnHideFind_pressed();

  void on_btnStepsOptions_clicked();

  void on_btnRecentOpen_pressed();

  void on_btnMenuReport_pressed();

  void on_btnRemoveBookList_pressed();

  void on_ReceiveShare();

  void on_btnShareImage_pressed();

  void on_btnDelImage_pressed();

  void on_btnSetBookmark_pressed();

  void on_btnFontLess_pressed();

  void on_btnFontPlus_pressed();

  void on_btnFont_pressed();

  void on_btnBackgroundColor_pressed();

  void on_btnForegroundColor_pressed();

  void on_editBackgroundColor_textChanged(const QString& arg1);

  void on_editForegroundColor_textChanged(const QString& arg1);

  void on_btnStyle1_pressed();

  void on_btnStyle2_pressed();

  void on_btnStyle3_pressed();

  void on_btnGoPage_pressed();

  void on_hSlider_sliderReleased();

  void on_CloseProgressBar();

  void on_btnShareBook_pressed();

  void slotSetBookmark();
  void on_btnAutoRun_pressed();

  void on_btnLessen_pressed();

  void on_btnDefault_pressed();

  void on_btnPlus_pressed();

  void on_btnAddTodo_released();

  void on_tmeFlash();

  void on_btnClearReaderFont_pressed();

  void on_StartRecordAudio();

  void on_sliderPlayAudio_sliderPressed();

  void on_sliderPlayAudio_sliderReleased();

  void on_btnGPS_pressed();

  void on_btnSelGpsDate_pressed();

  void on_btnGetGpsListData_pressed();

  void on_rbCycling_pressed();

  void on_rbHiking_pressed();

  void on_rbRunning_pressed();

  void on_btnWebDAVBackup_pressed();

  void on_btnWebDAVRestore_pressed();

  void on_chkWebDAV_pressed();

  void on_editFindNote_returnPressed();

  void on_btnClearSearchResults_pressed();

  void on_btnFindNotes2_pressed();

  void on_btnOpenEditFind_pressed();

  void on_btnTools_pressed();

  void on_btnCopyNoteLink_pressed();

  void on_cboxWebDAV_currentTextChanged(const QString& arg1);

  void on_btnShowCboxList_clicked();

  void on_btnRotation_pressed();

  void on_btnBackNoteDiff_pressed();

  void on_btnSendEmail();

  void on_btnShareBakFile_pressed();

  void on_btnNewNote_pressed();

  void on_btnShareBookText_pressed();

  void on_btnAddBookNote_pressed();

  void on_btnViewBookNote_pressed();

  void on_btnMap_clicked();

  void on_btnSportsChart_pressed();

  void on_btnSpeak_pressed();

  void on_btnStopSpeak_pressed();

  void on_chkPlayRunVoice_clicked(bool checked);

  void on_tabMotion_currentChanged(int index);

  void on_btnPause_pressed();

  void on_btnTestWebDav_pressed();

  void on_btnShowPassword_pressed();

  void on_btnShowPassword_released();

  void on_btnShowValidate_pressed();

  void on_btnShowValidate_released();

  void on_chkZip_pressed();

  void on_editPassword_textChanged(const QString& arg1);

  void on_editValidate_textChanged(const QString& arg1);

  void on_editAutoStopTTS_textChanged(const QString& arg1);

  void on_chkAutoStopTTS_clicked(bool checked);

 private:
  bool isMoveEntry;
  QTimer* tmeFlash;
  int nFlashCount = 0;
  QString keyType;
  bool isShowDetails = false;
  QString strShowDetails;
  bool isTabChanged = false;

  int frameChartHeight;

  qreal aoldX, aoldY, aoldZ;
  int countOne = 0;

  void resetWinPos();

  void init_Instance();

  void getMainTabs();

  QString strTime, strAmount, strCategory, strDetails;

  void init_Thread_Timer();

 signals:
  void androidBackSignal();
};

#endif  // MAINWINDOW_H
