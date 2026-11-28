#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qmath.h>

#include <QAbstractButton>
#include <QCheckBox>
#include <QClipboard>
#include <QInputMethod>
#include <QPointingDevice>
#include <QSplashScreen>
#include <QtConcurrent/QtConcurrentRun>

#ifdef Q_OS_ANDROID

#include <QCoreApplication>
#include <QJniEnvironment>
#include <QJniObject>

#endif

#include <QComboBox>
#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QElapsedTimer>
#include <QFileDialog>
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
#include <QRandomGenerator>
#include <QScrollBar>
#include <QScroller>
#include <QSettings>
#include <QSplitter>
#include <QStringList>
#include <QTextEdit>
#include <QThread>
#include <QTimer>
#include <QToolTip>
#include <QTreeWidgetItem>

#include "BakFileList.h"

#include "Exercise/ui_StepsOptions.h"
#include "MainHelper.h"
#include "Notes/ui_Notes.h"
#include "Notes/ui_NotesList.h"
#include "Todo/ui_Todo.h"
#include "Todo/ui_TodoAlarm.h"
#include "src/AboutThis.h"
#include "src/CategoryList.h"
#include "src/CloudBackup.h"
#include "src/Comm/Method.h"
#include "src/Comm/ReceiveShare.h"

#include "src/Comm/TextEditToolbar.h"
#include "src/EditRecord.h"
#include "src/Exercise/Steps.h"
#include "src/Exercise/StepsOptions.h"
#include "src/MyThread.h"
#include "src/Notes/Notes.h"
#include "src/Notes/NotesList.h"
#include "src/Preferences.h"
#include "src/Reader/Reader.h"
#include "src/Report.h"
#include "src/Todo/Todo.h"
#include "src/Todo/TodoAlarm.h"
#include "ui_AboutThis.h"
#include "ui_CategoryList.h"
#include "ui_CloudBackup.h"
#include "ui_DateSelector.h"
#include "ui_MainWindow.h"
#include "ui_Preferences.h"

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
class Method;

#include <QMetaType>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

extern MainWindow *mw_one;
extern QSettings *iniPreferences;

extern CloudBackup *m_CloudBackup;
extern QTreeWidgetItem *parentItem;
extern Reader *m_Reader;
extern Method *m_Method;
extern Notes *m_Notes;
extern NotesList *m_NotesList;
extern StepsOptions *m_StepsOptions;
extern Steps *m_Steps;
extern QTabWidget *tabData;
extern CategoryList *m_CategoryList;
extern ColorDialog *colorDlg;
extern PrintPDF *m_PrintPDF;
extern QTreeWidget *twrb, *tw;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();
  Ui::MainWindow *ui;

  QStringList listMyEventTitle, listMainDate, listMainDateDetail;

  void emitAndroidBackSignal() { emit androidBackSignal(); }

  QMenu *mainMenu = nullptr;

  int x, y, w, h;

  QInputDialog *m_RenameDlg = nullptr;

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

  QString strLatestModify = tr("None");

  QDialog *dlgProg = nullptr;

  void setMini();

  int days = 45;
  int newHeight;

  QString treeStyle = "QTreeWidget::item {height: 28;}";

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

  QString listStyle = "QListWidget{item-background: "
                      "rgb(244,23,24); item-color:rgb(255,255,255); "
                      "border-radius:2px;border:2px "
                      "solid red;}";
  int sRate = 0;
  qreal ax, ay, az, gx, gy, gz;
  int testCount1 = 0;
  int testCount = 0;
  int smallCount = 0;

  qlonglong CurTableCount = 0;

  static void get_Today(QTreeWidget *);

  SaveThread *mySaveThread;
  ReadChartThread *myReadChartThread;
  ReadTWThread *m_ReadTWThread;
  ReadEBookThread *myReadEBookThread;
  BakDataThread *myBakDataThread;
  ImportDataThread *myImportDataThread;
  SearchThread *mySearchThread;
  UpdateGpsMapThread *myUpdateGpsMapThread;
  GetGpsDataThread *myGetGpsDataThread;
  BakFileList *m_BakFileList;

  QThread *m_workerThread;

  static void ReadChartData();
  static int get_Day(QString date);
  static QString get_Year(QString date);
  static QString get_Month(QString date);
  void init_Options();

  QStringList chartCategories;
  QList<QVariant> qmlFreqValues;
  QList<QVariant> qmlAmountValues;

  AboutThis *m_AboutThis;
  EditRecord *m_EditRecord;
  Todo *m_Todo;
  Report *m_Report;
  Preferences *m_Preferences;

  TodoAlarm *m_TodoAlarm;
  DateSelector *m_DateSelector = nullptr;

  ReceiveShare *m_ReceiveShare;
  MainHelper *m_MainHelper;

  QList<QTreeWidgetItem *> findItemList;
  bool isFindTextChange = false;
  int findPos = 0;

  QTimer *tmeStartRecordAudio;

  static void readData(QTreeWidget *);

  static void saveTab();
  bool isSlide = false;
  void init_TotalData();

  bool del_Data(QTreeWidget *);
  static QTreeWidget *get_tw(int tabIndex);

  static QStringList get_MonthList(QString strY, QString strM);
  static void drawMonthChart();

  static void SaveFile(QString);

  static void init_Stats(QTreeWidget *);

  void startSave(QString);
  void startRead(QString);
  static void drawDayChart();
  static void readDataInThread(int ExceptIndex);

  void stopJavaTimer();

  QString getYMD(QString date);
  bool bakData();
  bool importBakData(QString fileName);

  QString getTabText();

  int calcStringPixelWidth(QString s_str, QFont font, int n_font_size);
  int calcStringPixelHeight(QFont font, int n_font_size);

  void reloadMain();

  void gotoEnd();

  void startInitReport();

  int getMaxDay(QString sy, QString sm);
  void showProgress();

  void execDeskShortcut();

  void on_DelayCloseProgressBar();

  void clearWidgetFocus();

  int max_day = 31;

  void execNeedSyncNotes();

  void saveNeedSyncNotes();

  void on_btnDownload_clicked();

  void init_ButtonStyle();

  void setToolButtonAnimation(QToolButton *btn, bool setMyStyle);
  void init_Menu(QMenu *mainMenu);
  void init_Theme();
  void init_UIWidget();
  QTreeWidget *init_TreeWidget(QString name);

  void sendAiChatRequest(const AiSingleRecord &cfg,
                         const QString &userQuestion);
  void aiChatQuery(const QString &userQuestion);
  void checkAiConnectivity(const AiSingleRecord &cfg,
                           std::function<void()> onSuccess);

  void setVectorStatus(int status, int current, int total);

  void clickMainTab(int index);

  void on_btnAIExerciseSuggestions_clicked();

  void addMyTab(const QString &customTabText);

  void renameTab(const QString &newName);

  void delTab();
  void clickMainDate(int index);

  void cleanOldBackups(const QString &dirPath, const QString &pattern,
                       int maxKeep);

  void GetGpsDataThreadDone();
  void readEBookDone();

protected:
  void closeEvent(QCloseEvent *event) override;
  bool eventFilter(QObject *watch, QEvent *evn) override;
  void paintEvent(QPaintEvent *event) override;
  void changeEvent(QEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

  void hideEvent(QHideEvent *event) override;

  void showEvent(QShowEvent *event) override;

public slots:
  void btnAISteps_clicked();
  void btnRecentOpen_clicked();
  void btnAIBookExplanation_clicked(const QString &selectedText);
  void btnHome_clicked();

  void on_tabWidget_currentChanged(int index);
  void safeCloseProgress();
  void on_actionOneDriveBackupData();

  void on_actionPreferences_triggered();

  void on_btnSteps_clicked();

  void btnStartDate_clicked();

  void btnEndDate_clicked();

  void on_actionExport_Data_triggered();

  void on_actionImport_Data_triggered();

  void on_twItemClicked();

  void on_twItemDoubleClicked();

  void on_actionAbout();

  void saveDone();

  void bakDataDone();

  void importDataDone();

  void readChartDone();

  void readTWDone();

  void searchDone();

  void on_actionBakFileList();

  void on_btnBack_One_clicked();

  void on_btnNotes_clicked();

  void btnStartSearch_clicked(const QString &searchStr);

  void on_ExecShortcut();

  void backNoteList();

  void btnDel_clicked();

  void closeProgress();

  void updateGpsMapDone();

  void btnAIReportAnalysis_clicked();

private slots:
  void on_StartRecordAudio();

  void onAndroidBackHandle();

  void on_btnMenu_clicked();

  void on_ReceiveShare();

  void on_CloseProgressBar();

  void on_btnWebDAVBackup_clicked();

  void on_btnWebDAVRestore_clicked();

  void on_cboxWebDAV_currentTextChanged(const QString &arg1);

  void on_btnShowCboxList_clicked();

  void on_btnTestWebDav_clicked();

  void on_btnShowPassword_pressed();

  void on_btnShowPassword_released();

  void on_btnShowValidate_pressed();

  void on_btnShowValidate_released();

  void on_chkZip_clicked();

  void on_editPassword_textChanged(const QString &arg1);

  void on_editValidate_textChanged(const QString &arg1);

  void on_btnReader_clicked();

  void on_btnTodo_clicked();

private:
  QNetworkAccessManager *m_ainetMgr;
  QUrl buildAiApiUrl(const QString &rawEndpoint);
  bool isMoveEntry;

  int nFlashCount = 0;
  QString keyType;
  bool isShowDetails = false;
  QString strShowDetails;
  bool isTabChanged = false;

  int frameChartHeight;

  qreal aoldX, aoldY, aoldZ;
  int countOne = 0;

  void init_Instance();

  QString strTime, strAmount, strCategory, strDetails;

  void init_Thread_Timer();

signals:
  void androidBackSignal();
};

#endif // MAINWINDOW_H
