#ifndef STEPS_H
#define STEPS_H

#include <QAccelerometer>
#include <QDialog>
#include <QGeoCoordinate>
#include <QGeoPositionInfoSource>
#include <QRegularExpressionValidator>
#include <iomanip>  // 包含 std::setprecision
#include <iostream>

#include "src/Steps/StepsOptions.h"

namespace Ui {
class Steps;
}

class Steps : public QDialog {
  Q_OBJECT

 public:
  explicit Steps(QWidget *parent = nullptr);
  ~Steps();
  Ui::Steps *ui;

  QString btnRoundStyle =
      "QToolButton {"
      "   border-radius: 40px;"  // 设置圆角半径为按钮宽度的一半
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
      "   border-radius: 40px;"  // 设置圆角半径为按钮宽度的一半
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

  int toDayInitSteps = 0;
  double dleInter = 5;
  double dleSlope = 5;
  void saveSteps();
  void init_Steps();
  QString lblStyleLight = "background-color: rgb(25, 239, 21);color:black";

  void addRecord(QString, qlonglong, QString);

  qlonglong getCurrentSteps();

  void setTableSteps(qlonglong steps);

  void releaseWakeLock();
  void acquireWakeLock();

  void setMaxMark();

  void appendSteps(QString date, int steps, QString km);
  int getCount();
  QString getDate(int row);
  int getSteps(int row);

  void delItem(int index);
  QString getKM(int row);
  void clearAll();

  void setScrollBarPos(double pos);

 protected:
  void keyReleaseEvent(QKeyEvent *event) override;

  bool eventFilter(QObject *watch, QEvent *evn) override;

 public:
  void on_btnBack_clicked();

  void on_btnReset_clicked();

  void on_editTangentLineIntercept_textChanged(const QString &arg1);

  void on_editTangentLineSlope_textChanged(const QString &arg1);

  void setTableData(int index, QString date, int steps, QString km);

  void startRecordMotion();
  void stopRecordMotion();

  bool requestLocationPermissions();
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
  bool isCycling, isHiking, isRunning;
  void writeCSV(const QString &filePath, const QList<QStringList> &data);

  void appendToCSV(const QString &filePath, const QStringList &data);
  void updateInfoText(QString strDistance, QString strSpeed);
 public slots:
  void clearAllGpsList();
  void getGpsTrack();
 private slots:
  void positionUpdated(const QGeoPositionInfo &info);

  void updateGetGps();

 private:
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
  double m_TotalDistance;
  double m_speed;
  QDateTime m_startTime;
  QTime m_time;
  QTimer *timer;
  QString strTotalDistance;
  QString strDurationTime;
  double latitude = 59.919023461273;
  double longitude = 10.752109237521;
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
                     QString t3, QString t4, QString t5);
  QString strStartTime, strEndTime;

  QString getGpsListText2(int index);

  void clearTrack();
  void writeGpsPos(double lat, double lon, int i, int count);

  double mySpeed;
  QString strGpsMapDateTime, strGpsMapDistnce, strGpsMapSpeed, strGpsList;
  bool isGpsMapTrackFile;
  double lastLat, lastLon;
  bool isGpsTest = false;
  int nGpsMethod = 1; /* 1：主类中 2:单独的类中 */

  QString getGpsListText0(int index);
 signals:
  void distanceChanged(double distance);
  void timeChanged();
  void speedChanged();
};

#endif  // STEPS_H
