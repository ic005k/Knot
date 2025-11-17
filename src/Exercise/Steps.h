#ifndef STEPS_H
#define STEPS_H

#include <QAbstractAxis>
#include <QAbstractBarSeries>
#include <QAccelerometer>
#include <QBarSeries>
#include <QBarSet>
#include <QCategoryAxis>
#include <QChart>
#include <QChartView>
#include <QDebug>
#include <QDialog>
#include <QGeoCoordinate>
#include <QGeoPositionInfo>
#include <QGeoPositionInfoSource>
#include <QGeoServiceProvider>
#include <QHorizontalBarSeries>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QQmlApplicationEngine>
#include <QRegularExpressionValidator>
#include <QSettings>
#include <QStringList>
#include <QToolTip>
#include <QVBoxLayout>
#include <QValueAxis>
#include <iomanip>  // 包含 std::setprecision
#include <iostream>

#include "Speedometer.h"
#include "src/Comm/GeoAddressResolver.h"
#include "src/Exercise/StepsOptions.h"
#include "src/Exercise/WeatherFetcher.h"

#ifdef Q_OS_ANDROID
#include <QJniEnvironment>
#include <QJniObject>
#endif

namespace Ui {
class Steps;
}

class Steps : public QDialog {
  Q_OBJECT

 public:
  explicit Steps(QWidget* parent = nullptr);
  ~Steps();
  Ui::Steps* ui;

  struct MonthData {
    double cyclingDist = 0.0;
    int cyclingCount = 0;
    double hikingDist = 0.0;
    int hikingCount = 0;
    double runningDist = 0.0;
    int runningCount = 0;
  };

  QString strMapKeyTestInfo;

  QTimer* timer;

  GeoAddressResolver* addressResolver = nullptr;
  QString m_lastAddress;
  bool isShowRoute = false;

  Speedometer* m_speedometer;
  WeatherFetcher* weatherFetcher;

  QString btnRoundStyle =
      "QToolButton {"
      "   border-radius: 40px;"        // 设置圆角半径为按钮宽度的一半
      "   background-color: #00AF50;"  // 设置背景颜色
      "   color: white;"               // 设置文字颜色
      "   min-width: 80px;"            // 设置最小宽度
      "   min-height: 80px;"           // 设置最小高度
      "}"
      "QToolButton:hover {"
      "   background-color: #00AF50;"  // 设置鼠标悬停时的背景颜色
      "}"
      "QToolButton:pressed {"
      "   background-color: #3d8b40;"  // 设置按钮按下时的背景颜色
      "}";

  QString btnRoundStyleRed =
      "QToolButton {"
      "   border-radius: 40px;"        // 设置圆角半径为按钮宽度的一半
      "   background-color: #FF0000;"  // 设置背景颜色
      "   color: white;"               // 设置文字颜色
      "   min-width: 80px;"            // 设置最小宽度
      "   min-height: 80px;"           // 设置最小高度
      "}"
      "QToolButton:hover {"
      "   background-color: #FF0000;"  // 设置鼠标悬停时的背景颜色
      "}"
      "QToolButton:pressed {"
      "   background-color: #3d8b40;"  // 设置按钮按下时的背景颜色
      "}";

  double dleInter = 5;
  double dleSlope = 5;
  void saveSteps();
  void loadStepsToTable();
  QJsonObject rootObj;
  QString lblStyleLight = "background-color: rgb(25, 239, 21);color:black";

  void addRecord(QString, qlonglong, QString);

  qlonglong getCurrentSteps();

  void setTableSteps(qlonglong steps);

  void setMaxMark();

  void appendSteps(QString date, int steps, QString km);
  int getCount();
  QString getDate(int row);
  int getSteps(int row);

  QString getKM(int row);
  void clearAll();

  void setScrollBarPos(double pos);

  void getAddress(double lat, double lon);

 protected:
  void keyReleaseEvent(QKeyEvent* event) override;

  bool eventFilter(QObject* obj, QEvent* event) override;

 public:
  void closeSteps();

  QDialog* statsDialog = nullptr;

  void on_btnReset_clicked();

  void setTableData(int index, QString date, int steps, QString km);

  void startRecordMotion();
  void stopRecordMotion();

  void loadGpsList(int nYear, int nMonth);
  int getGpsListCount();
  void selGpsListYearMonth();
  void getGpsListDataFromYearMonth();
  void delGpsListItem(int index);
  void allGpsTotal();
  void appendTrack(double lat, double lon);
  void updateGpsMapUi();
  void updateGpsTrack();
  void updateTrackData(double lat, double lon);
  void updateMapTrackUi(double lat, double lon);
  void saveMovementType();
  void setVibrate();

  void writeCSV(const QString& filePath, const QList<QStringList>& data);

  void appendToCSV(const QString& filePath, const QStringList& data);
  void updateInfoText(QString strDistance, QString strSpeed);
  QString getCurrentDate();
  void updateHardSensorSteps();
  void initTodayInitSteps();
  void getHardStepSensor();
  void openStepsUI();
  void setCurrentGpsSpeed(double speed, double maxSpeed);

  QString getFullDate();

  QGeoCoordinate wgs84ToGcj02(double wgs84Lat, double wgs84Lon);
  void closeRouteDialog();
  bool isRouteShow();
  void setMapKey();
  void showSportsChart();
 public slots:
  void clearAllGpsList();
  void getGpsTrack();
  void openMapWindow();
  void getRouteList(const QString& strGpsTime);
 private slots:
  void positionUpdated(const QGeoPositionInfo& info);

  void updateGetGps();

 private:
  QDateTime m_lastGetAddressTime;    // 上次获取地址的时间
  QDateTime m_lastSaveSpeedTime;     // 上次保存路线的时间
  QDateTime m_lastFetchWeatherTime;  // 上次请求天气的时间
  bool isInitTime;

  QString m_monthlyStatsText;
  QString m_yearlyStatsText;

  bool isOne = false;
  QString strCurrentTemp, strCurrentWeatherIcon;

  double maxSpeed = 0.00;
  qlonglong totalSeconds;

  int isHardStepSensor = -1;

  qlonglong initTodaySteps, resetSteps;

  qlonglong CurrentSteps = 0;

  QDateTime startDT;
  QDateTime endDT;
  QString strCSVFile;
  QBrush brush1 = QBrush(QColor(255, 228, 225));
  QBrush brush2 = QBrush(QColor(245, 222, 179));
  QBrush brushMax = QBrush(QColor(245, 222, 79));
  int maxCount = 90;

  double distance() const { return m_distance; }
  QGeoPositionInfoSource* m_positionSource;
  QGeoCoordinate lastPosition;
  double m_distance;
  double m_TotalDistance, oldTotalDistance;
  double m_speed;
  QDateTime m_startTime;
  QTime m_time;

  QString strTotalDistance;
  QString strDurationTime;
  double latitude = 59.919023461273;
  double longitude = 10.752109237521;
  double latRoute, lonRoute;
  QString timeRoute;
  QString distanceRoute, speedRoute;
  double oldLat;
  double oldLon;
  QString strGpsStatus;
  QString strGpsInfoShow;
  QString lblStyle;
  QString lblStartStyle =
      "QLabel {background-color: #FF0000;color: #ECEFF4;font-family: 'Segoe "
      "UI', sans-serif;font-weight: bold;border: 2px solid "
      "#4C566A;border-radius: 10px;padding: 10px 20px;text-align: "
      "center;}";

  QString t0, str1, str2, str3, str4, strAltitude, str6, str7;

  void insertGpsList(int curIndex, QString t0, QString t1, QString t2,
                     QString t3, QString t4, QString t5, QString t6,
                     QVariantList speedData);
  QString strStartTime, strEndTime;
  QDateTime startDt, endDt;

  QString getGpsListText2(int index);

  void clearTrack();
  void writeGpsPos(double lat, double lon, int i, int count);

  double mySpeed;
  QString strGpsMapDateTime, strGpsMapDistnce, strGpsMapSpeed, strGpsList;
  bool isGpsMapTrackFile;
  double lastLat, lastLon;

  QString getGpsListText0(int index);
  void refreshMotionData();
  void sendMsg(int CurTableCount);
  qlonglong getAndroidSteps();

  void refreshTotalDistance();
  qlonglong getTodaySteps();
  qlonglong getOldSteps();
  QString getCurrentYear();
  QString getCurrentMonth();

  void clearTrackAndroid();
  void appendTrackPointAndroid(double latitude, double longitude);
  void addTrackDataToAndroid(double latitude, double longitude);
  void clearTrackDataToAndroid();
  void setDateLabelToAndroid(const QString& str);
  void setInfoLabelToAndroid(const QString& str);

  void saveRoute(const QString& file, const QString& time, double lat,
                 double lon, const QString& address);
  void setAddressResolverConnect();
  QString strJsonRouteFile, strJsonSpeedFile;
  QStringList readRoute(const QString& file);

  bool isInChina(double lat, double lon);
  void setMapKeyError();
  bool isChina = false;
  void setMapType();
  QGeoCoordinate wgs84ToGcj02_cpp(double wgs84Lat, double wgs84Lon);

  QJsonArray routeMemoryCache;

  void updateGpsList(int curIndex, QString t0, QString t1, QString t2,
                     QString t3, QString t4, QString t5, QString t6);
  void refreshRoute();
  void saveSpeedData(const QString& jsonFile, double speed);
  QVariantList getSpeedData(const QString& jsonFile);
  QString getJsonRouteFile(const QString& strGpsList);

 signals:
  void distanceChanged(double distance);
  void timeChanged();
  void speedChanged();
};

//////////////////////////////////
class CustomChartView : public QChartView {
  Q_OBJECT
 public:
  explicit CustomChartView(QWidget* parent = nullptr) : QChartView(parent) {
    // 启用鼠标跟踪，确保能捕获所有鼠标事件
    setMouseTracking(true);
  }

  void setMonthData(const QVector<Steps::MonthData>& data) {
    m_monthData = data;
  }

 protected:
  void mousePressEvent(QMouseEvent* event) override {
    // 首先调用基类实现
    QChartView::mousePressEvent(event);

    // 处理所有区域的点击事件，包括空白区域
    // handleChartClick(event);

    // 接受事件，防止事件继续传播
    event->accept();
  }

  void mouseReleaseEvent(QMouseEvent* event) override {
    QChartView::mouseReleaseEvent(event);
    // 移除 handleChartClick 调用，避免重复处理
    handleChartClick(event);
  }

 private:
  void handleChartClick(QMouseEvent* event) {
    if (!chart()) return;

    // 获取鼠标位置和绘图区域
    QPoint mousePos = event->pos();
    QRectF plotArea = chart()->plotArea();

    // 计算月份
    int month = -1;

    // 检查点击是否在绘图区域内
    if (plotArea.contains(mousePos)) {
      // 在绘图区域内，计算月份
      double relativeY = mousePos.y() - plotArea.y();
      double totalHeight = plotArea.height();
      double monthHeight = totalHeight / 12.0;
      int monthIndex = static_cast<int>(relativeY / monthHeight);
      month = 12 - monthIndex;  // 底部是1月，顶部是12月
      month = qBound(1, month, 12);
    } else if (mousePos.x() < plotArea.left()) {
      // 在Y轴区域（左侧），计算月份
      // 使用整个视图高度来计算，而不是绘图区域高度
      int viewHeight = height();
      double monthHeight = viewHeight / 12.0;
      int monthIndex = static_cast<int>(mousePos.y() / monthHeight);
      month = 12 - monthIndex;  // 底部是1月，顶部是12月
      month = qBound(1, month, 12);
    }

    // 如果月份有效，显示提示信息
    if (month >= 1 && month <= m_monthData.size()) {
      const Steps::MonthData& data = m_monthData[month - 1];

      // 使用统一的提示信息生成逻辑
      QString tooltip = QString(
          tr("Month") + ": " + QString::number(month) + "\n" + tr("Cycling") +
          ": " + QString::number(data.cyclingDist, 'f', 1) + " km (" +
          QString::number(data.cyclingCount) + " " + tr("times") + ")\n" +
          tr("Hiking") + ": " + QString::number(data.hikingDist, 'f', 1) +
          " km (" + QString::number(data.hikingCount) + " " + tr("times") +
          ")\n" + tr("Running") + ": " +
          QString::number(data.runningDist, 'f', 1) + " km (" +
          QString::number(data.runningCount) + " " + tr("times") + ")");

      // 计算提示框显示位置（鼠标上方）
      // 计算提示框的大致高度（基于行数和字体大小）
      QFontMetrics fm(QToolTip::font());
      int lineHeight = fm.height();
      int tooltipHeight = lineHeight * 5 + 10;  // 大约5行高度

      QPoint globalPos = event->globalPos();
      QPoint showPos = globalPos - QPoint(0, tooltipHeight);  // 向上偏移30像素

      QToolTip::showText(showPos, tooltip);
    }
  }

 private:
  QVector<Steps::MonthData> m_monthData;
};

#endif  // STEPS_H
