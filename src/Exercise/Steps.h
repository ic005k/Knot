#ifndef STEPS_H
#define STEPS_H

#include <QAccelerometer>
#include <QDialog>
#include <QGeoCoordinate>
#include <QGeoPositionInfo>
#include <QGeoPositionInfoSource>
#include <QGeoServiceProvider>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlApplicationEngine>
#include <QRegularExpressionValidator>
#include <iomanip>  // 包含 std::setprecision
#include <iostream>

#include "Speedometer.h"
#include "src/Comm/GeoAddressResolver.h"
#include "src/Exercise/StepsOptions.h"
#include "src/Exercise/WeatherFetcher.h"

namespace Ui {
class Steps;
}

class Steps : public QDialog {
  Q_OBJECT

 public:
  explicit Steps(QWidget *parent = nullptr);
  ~Steps();
  Ui::Steps *ui;

  // GCJ02 转换常量（国测局标准）
  static const double PI;
  static const double EARTH_RADIUS;
  static const double ECCENTRICITY_SQUARE;
  static const double GCJ02_LON_MIN;
  static const double GCJ02_LON_MAX;
  static const double GCJ02_LAT_MIN;
  static const double GCJ02_LAT_MAX;

  GeoAddressResolver *addressResolver;
  QString m_lastAddress;
  bool isShowRoute = true;

  Speedometer *m_speedometer;
  WeatherFetcher *weatherFetcher;

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

 protected:
  void keyReleaseEvent(QKeyEvent *event) override;

  bool eventFilter(QObject *watch, QEvent *evn) override;

 public:
  void closeSteps();

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

  void writeCSV(const QString &filePath, const QList<QStringList> &data);

  void appendToCSV(const QString &filePath, const QStringList &data);
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
  public slots:
  void clearAllGpsList();
  void getGpsTrack();
  void openMapWindow();
  void getRouteList(const QString &strGpsTime);
 private slots:
  void positionUpdated(const QGeoPositionInfo &info);

  void updateGetGps();

 private:
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
  QGeoPositionInfoSource *m_positionSource;
  QGeoCoordinate lastPosition;
  double m_distance;
  double m_TotalDistance, oldTotalDistance;
  double m_speed;
  QDateTime m_startTime;
  QTime m_time;
  QTimer *timer;
  QString strTotalDistance;
  QString strDurationTime;
  double latitude = 59.919023461273;
  double longitude = 10.752109237521;
  double latRoute, lonRoute;
  QString timeRoute;
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

  QString t0, str1, str2, str3, str4, str5, str6, str7;

  void insertGpsList(int curIndex, QString t0, QString t1, QString t2,
                     QString t3, QString t4, QString t5, QString t6);
  QString strStartTime, strEndTime;

  QString getGpsListText2(int index);

  void clearTrack();
  void writeGpsPos(double lat, double lon, int i, int count);

  double mySpeed;
  QString strGpsMapDateTime, strGpsMapDistnce, strGpsMapSpeed, strGpsList;
  bool isGpsMapTrackFile;
  double lastLat, lastLon;
  bool isGpsTest = false;

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
  void setDateLabelToAndroid(const QString &str);
  void setInfoLabelToAndroid(const QString &str);

  void saveRoute(const QString &file, const QString &time, double lat,
                 double lon, const QString &address);
  void setAddressResolverConnect();
  QString strJsonRouteFile;
  QStringList readRoute(const QString &file);
  void getAddress(double lat, double lon);
  bool isInChina(double lat, double lon);
 signals:
  void distanceChanged(double distance);
  void timeChanged();
  void speedChanged();
};

#endif  // STEPS_H
