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

  bool eventFilter(QObject* watch, QEvent* evn) override;

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

// ----------------------
// ---------------------- 自定义频次曲线Widget（必须定义在函数外部，Qt
// MOC才能处理）----------------------
class FrequencyCurveWidget : public QWidget {
  Q_OBJECT
 public:
  explicit FrequencyCurveWidget(const QVector<int>& cycling,
                                const QVector<int>& hiking,
                                const QVector<int>& running, bool isDark,
                                QWidget* parent = nullptr)
      : QWidget(parent),
        m_cycling(cycling),
        m_hiking(hiking),
        m_running(running),
        m_isDark(isDark) {
    setFixedHeight(20);  // 固定10px高度，核心需求
    setSizePolicy(QSizePolicy::Expanding,
                  QSizePolicy::Fixed);  // 宽度跟随父窗口
    calculateMaxCount();                // 计算最大频次，用于比例映射
  }

 protected:
  // 重写paintEvent，手动绘制曲线
  void paintEvent(QPaintEvent*) override {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 平滑曲线，山丘感核心
    painter.setPen(Qt::NoPen);  // 取消默认画笔，避免多余线条

    // 1. 计算12个月份的水平坐标（平分父窗口宽度）
    QVector<int> xPoints = calculateXPoints();

    // 2. 绘制三条运动曲线（骑行、徒步、跑步）
    drawSmoothCurve(painter, xPoints, m_cycling, getColor(0));
    drawSmoothCurve(painter, xPoints, m_hiking, getColor(1));
    drawSmoothCurve(painter, xPoints, m_running, getColor(2));
  }

 private:
  // 计算12个月份的水平坐标（平分宽度）
  QVector<int> calculateXPoints() const {
    QVector<int> xPoints;
    int totalWidth = width();
    int monthSpacing = totalWidth / 12;  // 每个月占据的宽度
    for (int i = 0; i < 12; ++i) {
      // 取每个月的中心点作为曲线节点X坐标
      xPoints.append(monthSpacing * i + monthSpacing / 2);
    }
    return xPoints;
  }

  // 计算所有运动的最大频次（用于垂直比例映射）
  void calculateMaxCount() {
    m_maxCount = 1;  // 避免除以0
    for (int i = 0; i < 12; ++i) {
      int currentMax = qMax(m_cycling[i], qMax(m_hiking[i], m_running[i]));
      m_maxCount = qMax(m_maxCount, currentMax);
    }
  }

  // 根据运动类型获取对应颜色（适配明暗主题）
  QColor getColor(int type) const {
    if (m_isDark) {
      // 暗模式：亮一点的颜色，提高对比度
      switch (type) {
        case 0:
          return QColor(90, 189, 94, 200);  // 骑行绿
        case 1:
          return QColor(255, 171, 44, 200);  // 徒步橙
        case 2:
          return QColor(183, 70, 201, 200);  // 跑步紫
      }
    } else {
      // 亮模式：标准颜色
      switch (type) {
        case 0:
          return QColor(76, 175, 80, 200);  // 骑行绿
        case 1:
          return QColor(255, 152, 0, 200);  // 徒步橙
        case 2:
          return QColor(156, 39, 176, 200);  // 跑步紫
      }
    }
    return Qt::black;
  }

  // 绘制单条平滑曲线（贝塞尔插值，山丘状）
  void drawSmoothCurve(QPainter& painter, const QVector<int>& xPoints,
                       const QVector<int>& counts, const QColor& color) {
    QPainterPath path;
    int pointCount = xPoints.size();
    if (pointCount <= 1) return;

    // 1. 初始化曲线起点
    int startX = xPoints[0];
    double startY = mapCountToY(counts[0]);  // 频次→10px高度映射
    path.moveTo(startX, startY);

    // 2. 贝塞尔曲线插值，确保平滑
    for (int i = 1; i < pointCount; ++i) {
      int currentX = xPoints[i];
      double currentY = mapCountToY(counts[i]);
      int prevX = xPoints[i - 1];
      double prevY = mapCountToY(counts[i - 1]);

      // 控制点：取两个点的中点，确保曲线自然过渡
      int ctrlX = (prevX + currentX) / 2;
      double ctrlY = (prevY + currentY) / 2;

      // 画二次贝塞尔曲线（平滑山丘核心）
      path.quadTo(ctrlX, ctrlY, currentX, currentY);
    }

    // 3. 设置画笔样式（3px粗，半透明）
    QPen pen(color);
    pen.setWidth(3);  // 确保10px高度下清晰可见
    painter.setPen(pen);
    painter.drawPath(path);
  }

  // 频次→Y坐标映射（最大频次对应10px顶端，0频次对应底部）
  double mapCountToY(int count) const {
    if (m_maxCount == 0) return 10.0;
    // Y轴向下为正，所以高频次→Y值小→向上凸
    return 20.0 - (count * 20.0) / m_maxCount;  // 20.0为高度
  }

 private:
  QVector<int> m_cycling;  // 骑行频次数据
  QVector<int> m_hiking;   // 徒步频次数据
  QVector<int> m_running;  // 跑步频次数据
  int m_maxCount;          // 最大频次（用于比例映射）
  bool m_isDark;           // 是否暗模式
};
// --------------------------------------------------------------------------------

#endif  // STEPS_H
