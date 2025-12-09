#include "Steps.h"

#include <QVector>
#include <cmath>

#include "src/Exercise/DrawSportsFreq.h"
#include "src/MainWindow.h"
#include "src/defines.h"
#include "ui_MainWindow.h"
#include "ui_StepsOptions.h"

bool isGpsTest = false;

// 常量完全同步Java优化版，无任何修改
const double PI = 3.14159265358979323846;            // 与Java Math.PI一致
const double EARTH_RADIUS_TENCENT = 6378137.0;       // 腾讯官方地球半径
const double AXIS_TENCENT = 0.00669342162296594323;  // 官方轴长系数
const double PRECISION_COMPENSATION = 0.0000001;     // 精度补偿值
const double GCJ02_LON_MIN =
    73.66;  // 同步Java的73.66（原C++是73.55，关键差异点）
const double GCJ02_LON_MAX =
    135.05;  // 同步Java的135.05（原C++是135.08，关键差异点）
const double GCJ02_LAT_MIN = 3.86;
const double GCJ02_LAT_MAX = 53.55;

// 弧度转换函数
inline double degreesToRadians(double degrees) { return degrees * PI / 180.0; }

// 辅助函数：完全复刻Java的transformLat（逐行对齐）
inline double transformLat(double x, double y) {
  double ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y +
               0.2 * sqrt(fabs(x));
  ret += ((20.0 * sin(6.0 * x * PI) + 20.0 * sin(2.0 * x * PI)) * 2.0) / 3.0;
  ret += ((20.0 * sin(y * PI) + 40.0 * sin((y / 3.0) * PI)) * 2.0) / 3.0;
  ret += ((160.0 * sin((y / 12.0) * PI) + 320.0 * sin((y * PI) / 30.0)) * 2.0) /
         3.0;
  return ret;
}

// 辅助函数：完全复刻Java的transformLng（逐行对齐）
inline double transformLng(double x, double y) {
  double ret =
      300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * sqrt(fabs(x));
  ret += ((20.0 * sin(6.0 * x * PI) + 20.0 * sin(2.0 * x * PI)) * 2.0) / 3.0;
  ret += ((20.0 * sin(x * PI) + 40.0 * sin((x / 3.0) * PI)) * 2.0) / 3.0;
  ret += ((150.0 * sin((x / 12.0) * PI) + 300.0 * sin((x / 30.0) * PI)) * 2.0) /
         3.0;
  return ret;
}

QVector<GPSCoordinate> gaussianFilter(const QVector<GPSCoordinate>& rawData,
                                      int windowSize, double sigma);
QVector<GPSCoordinate> detectAndCorrectOutliers(
    const QVector<GPSCoordinate>& data, double threshold, double altThreshold);
double calculateHaversineDistance(double lat1, double lon1, double lat2,
                                  double lon2);

#ifdef Q_OS_ANDROID
QJniObject m_activity;
#endif

Steps::Steps(QWidget* parent) : QDialog(parent) {
  this->installEventFilter(this);

  mui->lblSingle->adjustSize();
  QString date = QString::number(QDate::currentDate().month()) + "-" +
                 QString::number(QDate::currentDate().day());
  mui->lblCurrent->setText(date + " " + QTime::currentTime().toString());

  QFont font0 = m_Method->getNewFont(15);
  mui->lblSteps->setFont(font0);

  font0.setPointSize(13);
  mui->lblGpsDateTime->setFont(font0);
  font0.setBold(true);
  mui->lblCurrent->setFont(font0);
  mui->lblToNow->setFont(font0);
  mui->lblNow->setFont(font0);

  mui->tabMotion->setTabVisible(3, false);

  QFont font1 = m_Method->getNewFont(17);
  font1.setBold(true);
  mui->lblKM->setFont(font1);
  mui->lblSingle->setFont(font1);

  mui->lblGpsInfo->setStyleSheet(lblStyle);

  if (!isAndroid)
    font1.setPointSize(8);
  else
    font1.setPointSize(17);
  mui->lblGpsInfo->setFont(font1);

  mui->lblYearTotal->setStyleSheet(mui->lblMonthTotal->styleSheet());
  if (isAndroid)
    font1.setPointSize(13);
  else
    font1.setPointSize(9);
  mui->lblYearTotal->setFont(font1);
  mui->lblMonthTotal->setFont(font1);
  mui->lblYearTotal->hide();
  mui->lblMonthTotal->hide();
  mui->btnGetGpsListData->hide();

  QFontMetrics fm(this->font());
  int textHeight = fm.height();
  int iconSize = static_cast<int>(textHeight * 0.9);
  // 确保图标大小是合理的 (不小于 20px)
  iconSize = qMax(iconSize, 20);
  mui->btnSelGpsDate->setIconSize(QSize(iconSize, iconSize));
  mui->btnSportsChart->setIconSize(QSize(iconSize, iconSize));

  timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &Steps::updateGetGps);

  QDir gpsdir;
  QString gpspath = iniDir + "/memo/gps/";
  if (!gpsdir.exists(gpspath)) gpsdir.mkpath(gpspath);

  getHardStepSensor();

  // Speed
  m_speedometer = new Speedometer(this);
  mui->f_speed->setFixedHeight(120);
  m_speedometer->setMaxSpeed(10.00);  // 最高时速(km/h)
  m_speedometer->setMinSpeed(0);      // 最低时速(km/h)
  m_speedometer->setCurrentSpeed(0.0);
  m_speedometer->setBackgroundColor(QColor(30, 30, 30));  // 背景色
  mui->f_speed->layout()->setSpacing(0);
  mui->f_speed->layout()->setContentsMargins(0, 0, 0, 0);
  mui->f_speed->layout()->addWidget(m_speedometer);
  mui->f_speed->hide();

  // Compass
  compass = new CompassWidget();
  mui->vboxCompass->addWidget(compass);
  mui->vboxCompass->setContentsMargins(0, 0, 0, 0);
  mui->frame_3->setContentsMargins(0, 0, 0, 0);
  if (isAndroid)
    mui->frame_3->setFixedHeight(300);
  else
    mui->frame_3->setFixedHeight(200);

  // Weather
  weatherFetcher = new WeatherFetcher(this);
  // 连接信号时，通过天气代码转换为Unicode符号
  connect(weatherFetcher,
          static_cast<void (WeatherFetcher::*)(double, int, const QString&)>(
              &WeatherFetcher::weatherUpdated),
          this, [this](double temp, int code, const QString& desc) {
            // 1. 将天气代码转换为枚举
            WeatherFetcher::WeatherCondition condition =
                WeatherFetcher::weatherCodeToCondition(code);
            // 2. 获取对应的Unicode符号
            QString weatherIcon = WeatherFetcher::conditionToUnicode(condition);
            strCurrentWeatherIcon = weatherIcon;

            // 3. 组合显示（符号+温度+描述）
            strCurrentTemp = "\n" + tr("Weather") + ": " +
                             QString("%1℃  %2")  // 只保留温度和描述的占位符
                                 .arg(temp)      // 体感温度
                                 .arg(desc);     // 天气描述（中文/英文）

            qDebug() << strCurrentTemp << strCurrentWeatherIcon;
          });

  connect(weatherFetcher, &WeatherFetcher::errorOccurred, this,
          [this](const QString& error) {
            // 处理错误
            qDebug() << "天气获取错误:" << error;
            strCurrentTemp = "";
          });

  // Route
  addressResolver = new GeoAddressResolver(this);
  setMapKey();
  isChina = m_Method->isInChina();
  if (isChina) {
    latitude = 39.9042;
    longitude = 116.4074;
  }
  // 连接信号槽，获取结果
  setAddressResolverConnect();
}

Steps::~Steps() {
  delete timer;
  delete addressResolver;
  delete m_speedometer;
  delete weatherFetcher;
  delete compass;
}

void Steps::setAddressResolverConnect() {
  if (!isOne) {
    if (isChina) {
      connect(addressResolver, &GeoAddressResolver::addressResolved, this,
              [this](const QString& address) {
                // 过滤完全相同的地址
                if (address != m_lastAddress) {
                  qDebug() << "记录轨迹点：" << address;

                  m_lastAddress = address;

                } else {
                  qDebug() << "位置未变化，跳过记录";
                }
                strMapKeyTestInfo = address;
                setMapKey();
                isShowRoute = true;
                mui->qwGpsList->rootContext()->setContextProperty("isShowRoute",
                                                                  isShowRoute);

                saveRoute(strJsonRouteFile, timeRoute, latRoute, lonRoute,
                          m_lastAddress);
              });
      connect(addressResolver, &GeoAddressResolver::resolveFailed, this,
              [this](const QString& error) {
                qDebug() << "地址解析失败：" << error;
                // 处理错误

                strMapKeyTestInfo = error;
                setMapKeyError();
                isShowRoute = false;
                mui->qwGpsList->rootContext()->setContextProperty("isShowRoute",
                                                                  isShowRoute);
              });

      isOne = true;

      // test
      getAddress(25.0217, 98.4464);
    }
  }
}

void Steps::keyReleaseEvent(QKeyEvent* event) { Q_UNUSED(event) }

bool Steps::eventFilter(QObject* obj, QEvent* event) {
  return QWidget::eventFilter(obj, event);
}

void Steps::closeSteps() {
  saveMovementType();
  mui->frameMain->show();
  mui->frameSteps->hide();
}

void Steps::on_btnReset_clicked() {
  CurrentSteps = 0;
  mui->lblSingle->setText("0");

  if (isHardStepSensor == 1) resetSteps = getAndroidSteps();

  QString date = QString::number(QDate::currentDate().month()) + "-" +
                 QString::number(QDate::currentDate().day());
  mui->lblCurrent->setText(date + " " + QTime::currentTime().toString());
  mui->lblNow->setText(date + " " + QTime::currentTime().toString());
  mui->lblKM->setText("0.00  " + tr("KM"));
}

void Steps::saveSteps() {
  QString strLength = m_StepsOptions->ui->editStepLength->text().trimmed();
  QString strThreshold =
      m_StepsOptions->ui->editStepsThreshold->text().trimmed();

  QJsonObject stepsObj = rootObj["Steps"].toObject();
  stepsObj["Length"] = strLength;
  stepsObj["Threshold"] = strThreshold;

  // 将Steps节点加入根对象
  rootObj["Steps"] = stepsObj;

  QFuture<void> future = QtConcurrent::run([=]() {
    // 写入JSON文件
    QString jsonPath = iniDir + "steps.json";
    // 读取现有JSON文件（如果存在）
    QFile file(jsonPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QJsonDocument doc(rootObj);
      file.write(doc.toJson(QJsonDocument::Indented));  // 格式化输出，便于阅读
      file.close();
      qDebug() << "JSON文件写入成功:" << jsonPath;
    } else {
      qDebug() << "JSON文件打开失败:" << file.errorString();
    }
  });

  // 使用 QFutureWatcher 监控进度
  QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
  connect(watcher, &QFutureWatcher<void>::finished, this, [=]() {
    qDebug() << "Steps save completed.";

    watcher->deleteLater();
  });
  watcher->setFuture(future);
}

void Steps::loadStepsToTable() {
  clearAll();

  // 获取Steps对象（对应原INI的/Steps节点）
  QJsonObject stepsObj = rootObj["Steps"].toObject();

  // 读取记录总数（对应原/Steps/Count）
  int count = stepsObj["Count"].toInt(0);
  int start = 0;
  if (count > maxCount) start = 1;

  // 获取Table数组（对应原/Steps/Table-i-*系列键）
  QJsonArray tableArray = stepsObj["Table"].toArray();

  // 循环加载记录
  for (int i = start; i < count; i++) {
    // 读取当前索引的记录（处理数组越界情况）
    QString str0 = "";
    qlonglong steps = 0;
    QString str2 = "";

    if (i >= 0 && i < tableArray.size()) {
      QJsonArray rowArray = tableArray.at(i).toArray();
      str0 = rowArray.size() > 0 ? rowArray[0].toString("") : "";
      steps = rowArray.size() > 1 ? rowArray[1].toVariant().toLongLong(0) : 0;
      str2 = rowArray.size() > 2 ? rowArray[2].toString("") : "";
    }

    // 处理公里数（若为空则计算）
    if (str2.isEmpty()) {
      double km =
          m_StepsOptions->ui->editStepLength->text().trimmed().toDouble() *
          steps / 100 / 1000;
      str2 = QString("%1").arg(km, 0, 'f', 2);
    }

    // 添加记录（保持原过滤逻辑）
    if (str0 != "" && steps >= 0 && !str2.isNull()) {
      addRecord(str0, steps, str2);
    }
  }

  m_Method->setCurrentIndexFromQW(mui->qwSteps, getCount() - 1);
  m_Method->setScrollBarPos(mui->qwSteps, 1.0);
}

void Steps::openStepsUI() {
  mui->frameMain->hide();
  mui->frameSteps->show();

  updateHardSensorSteps();

  QString date = QString::number(QDate::currentDate().month()) + "-" +
                 QString::number(QDate::currentDate().day());
  mui->lblNow->setText(date + " " + QTime::currentTime().toString());
  double d_km =
      m_StepsOptions->ui->editStepLength->text().trimmed().toDouble() *
      mui->lblSingle->text().toInt() / 100 / 1000;
  QString km = QString("%1").arg(d_km, 0, 'f', 2) + "  " + tr("KM");
  mui->lblKM->setText(km);

  if (mui->lblGpsInfo->text() == tr("GPS Info") ||
      mui->lblGpsInfo->text() == "GPS Info") {
    QSettings Reg(iniDir + "gpslist.ini", QSettings::IniFormat);

    double m_td = Reg.value("/GPS/TotalDistance", 0).toDouble();

    strTotalDistance = QString::number(m_td) + " km";

    strGpsInfoShow = QString(" \n") + " \n" + " \n" + " \n" + " \n" + " \n" +
                     " \n" + " \n" + " \n" + tr("Total Distance") + " : " +
                     strTotalDistance;
    mui->lblGpsInfo->setText(strGpsInfoShow);
  }

  if (getGpsListCount() == 0 && !timer->isActive()) {
    int nYear = QDate::currentDate().year();
    int nMonth = QDate::currentDate().month();
    loadGpsList(nYear, nMonth);
    allGpsTotal();
  }

  // Route
  if (!isChina) isChina = m_Method->isInChina();
  // 连接信号槽，获取结果
  setAddressResolverConnect();
}

void Steps::addRecord(QString date, qlonglong steps, QString km) {
  appendSteps(date, steps, km);
}

qlonglong Steps::getCurrentSteps() {
  int count = getCount();
  if (count == 0) return 0;

  QString str = getDate(count - 1);
  if (str == getCurrentDate()) return getSteps(count - 1);
  return 0;
}

QString Steps::getCurrentDate() {
  QString c_date;
  c_date = QDate::currentDate().toString("yyyy-M-d");

  return c_date;
}

QString Steps::getFullDate() {
  QString date;
  if (isZH_CN) {
    QLocale chineseLocale(QLocale::Chinese, QLocale::China);
    date = chineseLocale.toString(QDate::currentDate(), "yyyy-M-d ddd");
  } else
    date = QDate::currentDate().toString("yyyy-M-d ddd");

  return date;
}

qlonglong Steps::getTodaySteps() {
  QString jsonPath = iniDir + "steps.json";
  QJsonObject rootObj;
  QFile file(jsonPath);

  // 读取JSON文件内容
  if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    rootObj = QJsonDocument::fromJson(file.readAll()).object();
    file.close();
  }

  // 获取Steps对象
  QJsonObject stepsObj = rootObj["Steps"].toObject();

  // 读取记录总数（对应原/Steps/Count）
  int count = stepsObj["Count"].toInt(0);
  QString date, c_date = getCurrentDate();

  if (count > 0) {
    // 获取Table数组
    QJsonArray tableArray = stepsObj["Table"].toArray();
    int lastIndex = count - 1;

    // 检查索引有效性
    if (lastIndex >= 0 && lastIndex < tableArray.size()) {
      QJsonArray lastRow = tableArray.at(lastIndex).toArray();
      // 读取最后一条记录的日期（对应原/Steps/Table-(count-1)-0）
      date = lastRow.size() > 0 ? lastRow[0].toString("") : "";

      // 处理日期（截取日期部分，去掉时间）
      QString mdate = date.split(" ").at(0).trimmed();

      if (mdate == c_date) {
        // 返回最后一条记录的步数（对应原/Steps/Table-(count-1)-1）
        return lastRow.size() > 1 ? lastRow[1].toVariant().toLongLong(0) : 0;
      }
    }
  }

  return 0;
}

void Steps::setTableSteps(qlonglong steps) {
  QString jsonPath = iniDir + "steps.json";
  QFile file(jsonPath);

  // 读取JSON文件
  if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    rootObj = QJsonDocument::fromJson(file.readAll()).object();
    file.close();
  }

  // 读取Steps对象中的Count值（对应原INI的/Steps/Count），默认0
  QJsonObject stepsObj = rootObj["Steps"].toObject();  // 获取Steps节点
  int count = stepsObj["Count"].toInt(0);              // 读取Count，默认值0

  // 读取步长和阈值（对应原/Steps/Length和/Steps/Threshold）
  QString stepLength = stepsObj["Length"].toString("35");
  QString stepsThreshold = stepsObj["Threshold"].toString("10000");

  m_StepsOptions->ui->editStepLength->setText(stepLength);
  m_StepsOptions->ui->editStepsThreshold->setText(stepsThreshold);

  // 设置上下文属性
  mui->qwSteps->rootContext()->setContextProperty("nStepsThreshold",
                                                  stepsThreshold.toInt());

  double km = m_StepsOptions->ui->editStepLength->text().trimmed().toDouble() *
              steps / 100 / 1000;
  QString strKM = QString("%1").arg(km, 0, 'f', 2);

  QString date, c_date, full_date;
  full_date = getFullDate();
  c_date = getCurrentDate();
  if (count > 0) {
    // 从已获取的stepsObj中读取Table数组
    QJsonArray tableArray = stepsObj["Table"].toArray();

    // 读取倒数第一个Table元素的0索引值
    int index = count - 1;
    if (index >= 0 && index < tableArray.size()) {
      QJsonValue tableItem = tableArray.at(index);
      if (tableItem.isArray()) {
        // 取数组第0个元素作为日期，默认空字符串
        date = tableItem.toArray().at(0).toString("");
      } else {
        date = "";
      }
    } else {
      date = "";
    }

    QString mdate = "";
    if (!date.isEmpty()) {  // 只有date非空时，才拆分日期
      mdate = date.split(" ").at(0).trimmed();
    }

    // 表中已有今天的记录，则直接修改此记录
    if (mdate == c_date) {
      int index = count - 1;

      if (index >= 0 && index < tableArray.size()) {
        // 获取当前索引对应的数组项（[日期, 步数, 公里数]）
        QJsonArray rowArray = tableArray.at(index).toArray();

        // 更新数组中的三个值
        rowArray[0] = date;           // 对应-0（日期）
        rowArray[1] = (double)steps;  // 对应-1（步数）
        rowArray[2] = strKM;          // 对应-2（公里数）

        // 将更新后的行放回数组
        tableArray[index] = rowArray;

        // 同步更新stepsObj中的Table数组
        stepsObj["Table"] = tableArray;
      }

    } else {  // 在当前表中增加新数据
      count++;

      if (count > maxCount) {
        tableArray.removeAt(0);
        count = maxCount;
      }

      // 新建一行数据
      QJsonArray newRow;
      newRow.append(full_date);
      newRow.append(steps);
      newRow.append(strKM);
      tableArray.append(newRow);  // 新增到数组

      stepsObj["Table"] = tableArray;
      stepsObj["Count"] = count;
    }
  } else {              // count==0
    count = count + 1;  // 从0变成1，代表要新增第一条记录
    QJsonArray tableArray = stepsObj["Table"].toArray();

    // 1. 创建第一条记录的数组（日期、步数、公里数）
    QJsonArray newRow;
    newRow.append(full_date);
    newRow.append(steps);
    newRow.append(strKM);

    // 2. 用append新增到数组末尾（关键！空数组会变成size=1）
    tableArray.append(newRow);

    // 3. 同步更新stepsObj和count
    stepsObj["Table"] = tableArray;
    stepsObj["Count"] = count;
  }

  rootObj["Steps"] = stepsObj;

  qDebug() << "Set steps table completed.";

  saveSteps();

  loadStepsToTable();
}

void Steps::setMaxMark() {
  if (getCount() > 1) {
    QList<int> list;

    for (int i = 0; i < getCount(); i++) {
      list.append(getSteps(i));
    }

    int maxValue = *std::max_element(list.begin(), list.end());
    for (int i = 0; i < list.count(); i++) {
      if (maxValue == list.at(i)) {
        // max value
        break;
      }
    }
  }
}

void Steps::appendSteps(QString date, int steps, QString km) {
  QString strSteps = QString::number(steps);
  double dCalorie = steps * 0.04;
  QString strCalorie =
      QString("%1").arg(dCalorie, 0, 'f', 2) + "  " + tr("Calorie");

  double d_km =
      m_StepsOptions->ui->editStepLength->text().trimmed().toDouble() * steps /
      100 / 1000;
  km = QString("%1").arg(d_km, 0, 'f', 2) + "  " + tr("KM");

  QQuickItem* root = mui->qwSteps->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "addItem", Q_ARG(QVariant, date),
                            Q_ARG(QVariant, strSteps), Q_ARG(QVariant, km),
                            Q_ARG(QVariant, strCalorie), Q_ARG(QVariant, 0));
}

int Steps::getCount() {
  QQuickItem* root = mui->qwSteps->rootObject();
  QVariant itemCount;
  QMetaObject::invokeMethod((QObject*)root, "getItemCount",
                            Q_RETURN_ARG(QVariant, itemCount));
  return itemCount.toInt();
}

QString Steps::getDate(int row) {
  QQuickItem* root = mui->qwSteps->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject*)root, "getText0",
                            Q_RETURN_ARG(QVariant, item), Q_ARG(QVariant, row));
  return item.toString();
}

int Steps::getSteps(int row) {
  QQuickItem* root = mui->qwSteps->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject*)root, "getText1",
                            Q_RETURN_ARG(QVariant, item), Q_ARG(QVariant, row));
  return item.toInt();
}

QString Steps::getKM(int row) {
  QQuickItem* root = mui->qwSteps->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject*)root, "getText2",
                            Q_RETURN_ARG(QVariant, item), Q_ARG(QVariant, row));
  return item.toString();
}

void Steps::setTableData(int index, QString date, int steps, QString km) {
  QQuickItem* root = mui->qwSteps->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "modifyItem",
                            Q_ARG(QVariant, index), Q_ARG(QVariant, date),
                            Q_ARG(QVariant, QString::number(steps)),
                            Q_ARG(QVariant, km));
}

void Steps::clearAll() {
  int count = getCount();
  for (int i = 0; i < count; i++) m_Method->delItemFromQW(mui->qwSteps, 0);
}

void Steps::setScrollBarPos(double pos) {
  QQuickItem* root = mui->qwSteps->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "setScrollBarPos",
                            Q_ARG(QVariant, pos));
}

void Steps::startRecordMotion() {
  QSettings Reg(iniDir + "gpslist.ini", QSettings::IniFormat);

  m_TotalDistance = Reg.value("/GPS/TotalDistance", 0).toDouble();
  oldTotalDistance = m_TotalDistance;

#ifdef Q_OS_ANDROID
#else
  isGpsTest = true;

  m_positionSource = QGeoPositionInfoSource::createDefaultSource(this);
  if (m_positionSource) {
    connect(m_positionSource, &QGeoPositionInfoSource::positionUpdated, this,
            &Steps::positionUpdated);
    m_positionSource->setUpdateInterval(2000);
  } else {
    mui->lblGpsInfo->setText(tr("No GPS signal..."));
    mui->btnGPS->setText(tr("Start"));
    return;
  }
#endif

#ifdef Q_OS_ANDROID

  m_activity = QJniObject(QNativeInterface::QAndroidApplication::context());

  // 1. 定义Java端返回值的枚举（对应不同失败场景，便于维护）
  enum GpsStartResult {
    GPS_START_SUCCESS = 1,                 // 启动成功
    GPS_START_PERMISSION_DENIED = 0,       // 权限问题（未获取/用户拒绝）
    GPS_START_SERVICE_DISABLED = -1,       // 位置服务未开启
    GPS_START_LOCATION_MANAGER_NULL = -2,  // LocationManager初始化失败
    GPS_START_UNKNOWN_ERROR = -3           // 未知异常
  };

  // 2. 优化后的调用逻辑
  if (m_activity.isValid()) {
    jdouble result = 0;
    try {
      // 捕获JNI调用的异常（避免崩溃）
      result = m_activity.callMethod<jdouble>("startGpsUpdates", "()D");
    } catch (const std::exception& e) {
      qWarning() << "JNI call startGpsUpdates exception：" << e.what();
      result = GPS_START_UNKNOWN_ERROR;
    } catch (...) {
      qWarning() << "JNI call startGpsUpdates occurred unknown exception";
      result = GPS_START_UNKNOWN_ERROR;
    }

    // 3. 根据返回值区分失败场景，给出精准提示（英文基准）
    QString tipText;
    if (result == GPS_START_SUCCESS) {
      tipText = tr("GPS started successfully...");

    } else {
      switch (static_cast<GpsStartResult>(result)) {
        case GPS_START_PERMISSION_DENIED:
          tipText =
              tr("Location permission not obtained, please grant permission "
                 "and try again");
          qWarning() << "GPS start failed：Location permission denied";
          break;
        case GPS_START_SERVICE_DISABLED:
          tipText =
              tr("Location service is disabled, please enable it in settings");
          qWarning() << "GPS start failed：Location service disabled";
          break;
        case GPS_START_LOCATION_MANAGER_NULL:
          tipText = tr(
              "Location service initialization failed, please restart the app");
          qWarning() << "GPS start failed：LocationManager is null";
          break;
        case GPS_START_UNKNOWN_ERROR:
          tipText = tr("GPS start failed, unknown error");
          qWarning() << "GPS start failed：Unknown exception";
          break;
        default:  // 原返回0的兜底（兼容旧逻辑）
          tipText = tr(
              "GPS start failed, please check permission and location service");
          qWarning() << "GPS start failed：Unknown reason (return value="
                     << result << ")";
          break;
      }
      // ====== 仅失败场景执行：显示Toast + return ======
      m_Method->showToastMessage(tipText);
      return;
    }

    // ====== 成功场景执行：显示Toast，不return ======
    m_Method->showToastMessage(tipText);

  } else {
    qWarning() << "Android Activity is invalid, cannot start GPS";
    m_Method->showToastMessage(tr("Activity is invalid, GPS start failed"));
    // Activity无效的失败场景，保留return
    return;
  }

#else
  if (m_positionSource) {
    m_positionSource->startUpdates();
  }
#endif

  clearTrack();
  resetTerrainDistance();

  m_time.setHMS(0, 0, 0, 0);

  startDt = QDateTime::currentDateTime();
  strStartTime = startDt.time().toString();

  if (isZH_CN) {
    QLocale chineseLocale(QLocale::Chinese, QLocale::China);
    t0 = chineseLocale.toString(QDate::currentDate(), "ddd MM月 dd yyyy");
  } else
    t0 = QDate::currentDate().toString();

  mui->lblGpsDateTime->setText(t0 + " " + strStartTime);
  setDateLabelToAndroid(t0 + " " + strStartTime);

  startDT = QDateTime::currentDateTime();

  QString ss0 = t0;
  QString s0 = ss0.replace(" ", "");
  QString sst = strStartTime;
  QString s1 = sst.replace(":", "");
  QString csvPath =
      iniDir + "memo/gps/" + getCurrentYear() + "/" + getCurrentMonth() + "/";
  if (!QDir(csvPath).exists()) {
    QDir().mkpath(csvPath);
  }
  strCSVFile = csvPath + s0 + "-gps-" + s1 + ".csv";
  strJsonRouteFile = csvPath + s0 + "-gps-" + s1 + ".json";
  strJsonSpeedFile = csvPath + s0 + "-gps-" + s1 + "_Speed.json";

  timer->start(1000);
  routeMemoryCache = QJsonArray();
  m_lastAddress = "";
  isInitTime = false;
  m_distance = 0;
  m_speed = 0;
  oldAlt = NAN;  // 用NaN标识未初始化，而非0
  oldLat = 0;
  oldLon = 0;
  altitude = 0;

  mw_one->m_Reader->keepScreenOn();
  emit distanceChanged(m_distance);
  emit timeChanged();

  mui->btnGPS->setText(tr("Stop"));
  mui->tabMotion->setCurrentIndex(1);

  mui->lblGpsInfo->setStyleSheet(lblStartStyle);
  mui->btnGPS->setStyleSheet(btnRoundStyleRed);
  mui->gboxMotionType->setEnabled(false);
  mui->btnSelGpsDate->setEnabled(false);
}

void Steps::positionUpdated(const QGeoPositionInfo& info) {
  if (lastPosition.isValid()) {
    double b = 1000;
    // Convert to km
    m_distance += (double)lastPosition.distanceTo(info.coordinate()) / b;
    emit distanceChanged(m_distance);
  }
  lastPosition = info.coordinate();

  if (!isGpsTest) {
    latitude = lastPosition.latitude();
    longitude = lastPosition.longitude();
  }
}

void Steps::updateGetGps() {
  m_time = m_time.addSecs(1);
  totalSeconds = static_cast<qlonglong>(m_time.hour()) * 3600 +
                 static_cast<qlonglong>(m_time.minute()) * 60 + m_time.second();

#ifdef Q_OS_ANDROID

  // 获取当前运动距离（目前通过c++端计算）
  // jdouble distance;
  // distance = m_activity.callMethod<jdouble>("getTotalDistance", "()D");
  // QString str_distance = QString::number(distance, 'f', 2);
  // m_distance = str_distance.toDouble();

  if (!isGpsTest) {
    latitude = m_activity.callMethod<jdouble>("getLatitude", "()D");
    longitude = m_activity.callMethod<jdouble>("getLongitude", "()D");

    latitude = QString::number(latitude, 'f', 6).toDouble();
    longitude = QString::number(longitude, 'f', 6).toDouble();
  }

  QJniObject jstrGpsStatus;
  jstrGpsStatus =
      m_activity.callMethod<jstring>("getGpsStatus", "()Ljava/lang/String;");

  if (jstrGpsStatus.isValid()) {
    QString strStatus = jstrGpsStatus.toString();
    QStringList list = strStatus.split("\n");
    if (list.count() == 8) {
      // str1 = list.at(0);
      str2 = list.at(1);
      str3 = list.at(2);

      str4 = list.at(3);

      strAltitude = list.at(4);
      QStringList altiList = strAltitude.split(":");
      if (altiList.count() > 0) {
        dAltitude = altiList.at(1).trimmed();
        altitude = dAltitude.toDouble();
      }

      str6 = list.at(5);
      str7 = list.at(6);
      strGpsStatus = tr("Average Speed") + " : " + str3 + "\n" + str4 + "\n" +
                     strAltitude + " m" + "\n" + str6 + "\n" + str7;

      if (std::isnan(oldAlt)) oldAlt = altitude;
      if (oldLat == 0) oldLat = latitude;
      if (oldLon == 0) oldLon = longitude;
    }

    if (m_time.second() % 3 == 0) {
      if (!isGpsTest) {
        jdouble speed;

        speed = m_activity.callMethod<jdouble>("getMySpeed", "()D");
        maxSpeed = m_activity.callMethod<jdouble>("getMaxSpeed", "()D");

        mySpeed = speed;
        if (mySpeed > 0) {
          const double COORD_CHANGE_THRESHOLD = 1e-6;  // 约0.1米的变化阈值
          bool coordChanged =
              (qAbs(latitude - oldLat) > COORD_CHANGE_THRESHOLD) ||
              (qAbs(longitude - oldLon) > COORD_CHANGE_THRESHOLD);
          if (coordChanged) {
            appendTrack(latitude, longitude);

            QStringList data_list;
            data_list.append(QString::number(latitude, 'f', 6));
            data_list.append(QString::number(longitude, 'f', 6));
            data_list.append(dAltitude);
            appendToCSV(strCSVFile, data_list);

            // 计算地形距离：仅当old数据有效（避免初始值）
            getTerrain();

            bearing1 = calculateBearing(oldLat, oldLon, latitude, longitude);
            mui->lblDirection->setText(bearingToDirection(bearing1));
            compass->setBearing(bearing1);
          }

          oldLat = latitude;
          oldLon = longitude;
          oldAlt = altitude;
        }
      }
    }

    updateInfoText(str1, str3);
  }

#else
  //  test ///////////////////////////////////////////////////////////////////

  if (isGpsTest) {
    if (m_time.second() % 3 == 0) {
      appendTrack(latitude, longitude);

      // 生成有符号64位随机数（qint64），范围：-9999999999 ~ 9999999999
      qint64 randomInt1 =
          static_cast<qint64>(QRandomGenerator::global()->generate64()) %
          10000000000LL;
      qint64 randomInt2 =
          static_cast<qint64>(QRandomGenerator::global()->generate64()) %
          10000000000LL;
      // 转换为正负小数（范围：-0.009999999999 ~ 0.009999999999）
      double randomDouble1 = static_cast<double>(randomInt1) / 10000000000000.0;
      double randomDouble2 = static_cast<double>(randomInt2) / 10000000000000.0;
      latitude = latitude + randomDouble1;
      longitude = longitude + randomDouble2;
      qDebug() << randomDouble1 << randomDouble2;

      QStringList data_list;
      data_list.append(QString::number(latitude, 'f', 6));
      data_list.append(QString::number(longitude, 'f', 6));

      double baseRandom = QRandomGenerator::global()->generateDouble();
      double randomInRange = baseRandom * (50 - (-50)) + (-50);
      altitude = qRound(randomInRange * 100) / 100.0;
      dAltitude = QString::number(altitude, 'f', 2);
      data_list.append(dAltitude);
      appendToCSV(strCSVFile, data_list);

      unsigned seed =
          std::chrono::system_clock::now().time_since_epoch().count();
      std::default_random_engine generator(seed);
      std::uniform_real_distribution<double> distribution(0.0, 50.0);
      mySpeed = distribution(generator);
      if (mySpeed > maxSpeed) maxSpeed = mySpeed;

      strAltitude = "Altitude: " + QString::number(distribution(generator));

      getTerrain();

      bearing1 = calculateBearing(oldLat, oldLon, latitude, longitude);
      mui->lblDirection->setText(bearingToDirection(bearing1));
      compass->setBearing(bearing1);

      oldLat = latitude;
      oldLon = longitude;
      oldAlt = altitude;

      qDebug() << "m_time%3=" << m_time.second();
    }

    qDebug() << "m_time=" << m_time.second() << totalSeconds;
  }
/////////////////////////////////////////////////////////////////////////////
#endif

  if (mySpeed > 0) {
    // setCurrentGpsSpeed(mySpeed, maxSpeed);
    compass->setSpeed(mySpeed);
  }
  str1 = QString::number(m_distance, 'f', 2) + " km";
  strTotalDistance = QString::number(m_TotalDistance) + " km";

  endDT = QDateTime::currentDateTime();
  qint64 secondsDiff = startDT.secsTo(endDT);
  // 处理负数（可选）
  bool isNegative = secondsDiff < 0;
  secondsDiff = qAbs(secondsDiff);  // 若只需正值则取绝对值
  // 分解为时、分、秒
  qint64 hours = secondsDiff / 3600;
  qint64 remainingSeconds = secondsDiff % 3600;
  qint64 minutes = remainingSeconds / 60;
  qint64 seconds = remainingSeconds % 60;
  // 格式化为字符串（补零对齐）
  QString timeStr = QString("%1%2:%3:%4")
                        .arg(isNegative ? "-" : "")           // 保留负号
                        .arg(hours, 2, 10, QLatin1Char('0'))  // 2位数字，补零
                        .arg(minutes, 2, 10, QLatin1Char('0'))
                        .arg(seconds, 2, 10, QLatin1Char('0'));
  strDurationTime = tr("Duration") + " : " + timeStr;

  strGpsInfoShow = tr("Current Distance") + " : " + str1 + "\n" +
                   tr("Exercise Duration") + " : " + str2 + "\n" +
                   strDurationTime +
                   "\nLon.-Lat.: " + QString::number(longitude) + " - " +
                   QString::number(latitude) + "\n" + strGpsStatus + "\n" +
                   tr("Total Distance") + " : " + strTotalDistance;
  mui->lblGpsInfo->setText(strGpsInfoShow);
  emit timeChanged();

  if (m_time.second() % 6 == 0) {
    refreshMotionData();
  }

  if (m_time.second() % 12 == 0) {
    if (mui->chkPlayRunVoice->isChecked()) {
      if (mySpeed > 0) {
        if (mySpeed != oldMySpeed) {
          m_Method->stopPlayMyText();
          m_Method->playMyText(mui->lblDirection->text() + " " +
                               QString::number(mySpeed, 'f', 2));
          oldMySpeed = mySpeed;
        }
      }
    }
  }

  if (m_distance > 0 || isGpsTest) {
    // 先校验经纬度是否在有效范围（-90~90纬度，-180~180经度）
    bool latValid = (latitude >= -90 && latitude <= 90);
    bool lonValid = (longitude >= -180 && longitude <= 180);
    if (!latValid || !lonValid) {
      // 无效坐标，不请求

    } else {
      QDateTime currentTime = QDateTime::currentDateTime();

      if (!isInitTime) {
        m_lastGetAddressTime = currentTime;
        m_lastSaveSpeedTime = currentTime;
        m_lastFetchWeatherTime = currentTime;

        weatherFetcher->fetchWeather(latitude, longitude);

        refreshRoute();
        saveSpeedData(strJsonSpeedFile, mySpeed, altitude);

        isInitTime = true;
      }

      // Weather
      if (m_lastFetchWeatherTime.secsTo(currentTime) >=
          1800) {  // 30分钟=1800秒
        weatherFetcher->fetchWeather(latitude, longitude);
        m_lastFetchWeatherTime = currentTime;  // 更新上次请求时间
      }

      // Route
      int tr = 150;
      if (isGpsTest) tr = 5;
      if (isShowRoute) {
        if (m_lastGetAddressTime.secsTo(currentTime) >=
            tr) {  // 距离上次超过150秒
          refreshRoute();
          m_lastGetAddressTime = currentTime;  // 更新上次执行时间
        }
      }

      // Speed and Altitude
      int ts = 30;
      if (isGpsTest) ts = 3;
      if (m_lastSaveSpeedTime.secsTo(currentTime) >= ts) {  // 距离上次超过60秒
        saveSpeedData(strJsonSpeedFile, mySpeed, altitude);
        m_lastSaveSpeedTime = currentTime;  // 更新上次执行时间
      }
    }
  }
}

void Steps::stopRecordMotion() {
  refreshRoute();
  saveSpeedData(strJsonSpeedFile, mySpeed, altitude);

  QTimer::singleShot(2000, mw_one, [this]() {
    timer->stop();
    mw_one->m_Reader->cancelKeepScreenOn();

    int nYear = QDate::currentDate().year();
    int nMonth = QDate::currentDate().month();
    clearAllGpsList();
    loadGpsList(nYear, nMonth);
    m_Method->gotoBegin(mui->qwGpsList);
    m_Method->setCurrentIndexFromQW(mui->qwGpsList, 0);

    refreshMotionData();
  });

  mui->lblGpsInfo->setText(strGpsInfoShow);

  mui->lblGpsInfo->setStyleSheet(lblStyle);
  mui->btnGPS->setStyleSheet(btnRoundStyle);

#ifdef Q_OS_ANDROID
  // 返回值为总距离
  m_activity.callMethod<jdouble>("stopGpsUpdates", "()D");

#else
  if (m_positionSource) {
    m_positionSource->stopUpdates();
  }
  delete m_positionSource;
#endif

  mui->gboxMotionType->setEnabled(true);
  mui->btnSelGpsDate->setEnabled(true);
}

void Steps::refreshRoute() {
  if (isShowRoute) {
    latRoute = latitude;
    lonRoute = longitude;
    timeRoute = QDateTime::currentDateTime().time().toString();
    distanceRoute = str1;
    speedRoute = QString::number(mySpeed, 'f', 2) + " km/h";
    directionRoute = mui->lblDirection->text();
    getAddress(latitude, longitude);
  }
}

void Steps::refreshTotalDistance() {
  m_TotalDistance = oldTotalDistance + m_distance;
  strTotalDistance = QString::number(m_TotalDistance) + " km";

  QSettings Reg(iniDir + "gpslist.ini", QSettings::IniFormat);
  Reg.setValue("/GPS/TotalDistance", m_TotalDistance);
}

void Steps::refreshMotionData() {
  refreshTotalDistance();

  endDt = QDateTime::currentDateTime();
  strEndTime = endDt.time().toString();

  QString t00, t1, t2, t3, t4, t5, str_type;

  if (mui->rbCycling->isChecked()) str_type = tr("Cycling");
  if (mui->rbHiking->isChecked()) str_type = tr("Hiking");
  if (mui->rbRunning->isChecked()) str_type = tr("Running");
  t00 = str_type + " " + t0;

  t1 = tr("Time") + ": " + strStartTime + " - " + strEndTime + strCurrentTemp;

  t2 = tr("Distance") + ": " + str1;

  qint64 secondsDiff = startDt.secsTo(endDt);
  // 若 endDt 在 startDt 之后，返回正数；否则返回负数（取绝对值确保为正）
  if (secondsDiff < 0) secondsDiff = -secondsDiff;
  // 转换为“时:分:秒”格式
  int h = secondsDiff / 3600;
  int m = (secondsDiff % 3600) / 60;
  int s = secondsDiff % 60;
  QString my_duration = QString("%1:%2:%3")
                            .arg(h, 2, 10, QChar('0'))
                            .arg(m, 2, 10, QChar('0'))
                            .arg(s, 2, 10, QChar('0'));
  t3 = tr("Exercise Duration") + ": " + str2 + "\n" + tr("Duration") + ": " +
       my_duration;

  t4 = tr("Average Speed") + ": " + str3 + "\n" + tr("Max Speed") + ": " +
       QString::number(maxSpeed, 'f', 2) + " km/h";
  t5 = str6 + "\n" + str7;

  if (m_distance > 0 || isGpsTest) {
    int nYear = QDate::currentDate().year();
    int nMonth = QDate::currentDate().month();
    QString stry, strm;
    stry = QString::number(nYear);
    strm = QString::number(nMonth);
    QString strTitle = stry + " - " + strm;
    if (mui->btnSelGpsDate->text() != strTitle) {
      clearAllGpsList();
      loadGpsList(nYear, nMonth);
      mui->btnSelGpsDate->setText(strTitle);
    }

    QString text0, text1, startTime1, startTime2;
    int countList = m_Method->getCountFromQW(mui->qwGpsList);
    if (countList > 0) {
      text0 = m_Method->getText0(mui->qwGpsList, 0);
      text1 = m_Method->getText1(mui->qwGpsList, 0);
    }
    startTime1 = text1.split("-").at(0);
    startTime2 = t1.split("-").at(0);

    // if (text0 == t00 && startTime1 == startTime2) {
    //  updateGpsList(0, t00, t1, t2, t3, t4, t5, strCurrentWeatherIcon);
    //} else {
    // insertGpsList(0, t00, t1, t2, t3, t4, t5, strCurrentWeatherIcon);
    //}

    strGpsMapDateTime = t00 + " " + t1;
    setDateLabelToAndroid(strGpsMapDateTime);

    QSettings Reg1(iniDir + stry + "-gpslist.ini", QSettings::IniFormat);

    int count = getGpsListCount();
    if (timer->isActive()) count = getGpsListCount() + 1;

    QString strYearMonth = stry + "-" + strm;
    Reg1.setValue("/" + strYearMonth + "/Count", count);
    Reg1.setValue("/" + strYearMonth + "/" + QString::number(count),
                  t00 + "-=-" + t1 + "-=-" + t2 + "-=-" + t3 + "-=-" + t4 +
                      "-=-" + t5 + "-=-" + strCurrentWeatherIcon + "-=-" +
                      strGpsTerrain);

    if (timer->isActive()) return;

    double dMonthTotal = 0;  // 里程月总计
    double dCycling = 0;
    double dHiking = 0;
    double dRunning = 0;
    int countCycling = 0;
    int countHiking = 0;
    int countRunning = 0;
    for (int i = 0; i < count; i++) {
      QString strType = getGpsListText0(i).split(" ").at(0);
      QString str = getGpsListText2(i);
      double jl = 0.00;
      QStringList list = str.split(" ");
      if (list.count() == 3) {
        QString str1 = list.at(1);
        jl = str1.toDouble();
        dMonthTotal += jl;
      }

      if (strType == tr("Cycling")) {
        dCycling += jl;
        countCycling++;
      }

      if (strType == tr("Hiking")) {
        dHiking += jl;
        countHiking++;
      }

      if (strType == tr("Running")) {
        dRunning += jl;
        countRunning++;
      }
    }

    QString s1, s2, s3, s4;
    s1 = QString::number(dMonthTotal) + "-=-" + QString::number(count);
    s2 = QString::number(dCycling) + "-=-" + QString::number(countCycling);
    s3 = QString::number(dHiking) + "-=-" + QString::number(countHiking);
    s4 = QString::number(dRunning) + "-=-" + QString::number(countRunning);
    Reg1.setValue("/" + stry + "/" + strm,
                  s1 + "-=-" + s2 + "-=-" + s3 + "-=-" + s4);

    allGpsTotal();
  }
}

void Steps::insertGpsList(int curIndex, QString t0, QString t1, QString t2,
                          QString t3, QString t4, QString t5, QString t6,
                          QString t7, QVariantList speedData,
                          QVariantList altitudeData) {
  QQuickItem* root = mui->qwGpsList->rootObject();
  if (!root) {
    qWarning() << "rootObject is null!";
    return;
  }

  // 调用QML的insertItem，最后一个参数传入speedData
  QMetaObject::invokeMethod(
      root, "insertItem", Q_ARG(QVariant, curIndex),  // 参数1：索引
      Q_ARG(QVariant, t0),                            // 参数2：text0
      Q_ARG(QVariant, t1),                            // 参数3：text1
      Q_ARG(QVariant, t2),                            // 参数4：text2
      Q_ARG(QVariant, t3),                            // 参数5：text3
      Q_ARG(QVariant, t4),                            // 参数6：text4
      Q_ARG(QVariant, t5),                            // 参数7：text5
      Q_ARG(QVariant, t6),                            // 参数8：text6
      Q_ARG(QVariant, t7),                            // 参数9：text7
      Q_ARG(QVariant, speedData),    // 速度数据（QVariantList）
      Q_ARG(QVariant, altitudeData)  // 海拔数据
  );
}

void Steps::updateGpsList(int curIndex, QString t0, QString t1, QString t2,
                          QString t3, QString t4, QString t5, QString t6) {
  QQuickItem* root = mui->qwGpsList->rootObject();
  QMetaObject::invokeMethod(
      (QObject*)root, "updateItem",  // 调用QML的updateItem
      Q_ARG(QVariant, curIndex), Q_ARG(QVariant, t0), Q_ARG(QVariant, t1),
      Q_ARG(QVariant, t2), Q_ARG(QVariant, t3), Q_ARG(QVariant, t4),
      Q_ARG(QVariant, t5), Q_ARG(QVariant, t6), Q_ARG(QVariant, 0));
}

int Steps::getGpsListCount() {
  QQuickItem* root = mui->qwGpsList->rootObject();
  QVariant itemCount;
  QMetaObject::invokeMethod((QObject*)root, "getItemCount",
                            Q_RETURN_ARG(QVariant, itemCount));
  return itemCount.toInt();
}

void Steps::delGpsListItem(int index) {
  QQuickItem* root = mui->qwGpsList->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "delItem", Q_ARG(QVariant, index));
}

void Steps::clearAllGpsList() {
  int count = getGpsListCount();
  for (int i = 0; i < count; i++) {
    delGpsListItem(0);
  }
}

void Steps::loadGpsList(int nYear, int nMonth) {
  mui->btnSelGpsDate->setText(QString::number(nYear) + " - " +
                              QString::number(nMonth));

  QSettings Reg(iniDir + QString::number(nYear) + "-gpslist.ini",
                QSettings::IniFormat);

  QString strYearMonth = QString::number(nYear) + "-" + QString::number(nMonth);
  int count = Reg.value("/" + strYearMonth + "/Count", 0).toInt();
  for (int i = 0; i < count; i++) {
    QString str =
        Reg.value("/" + strYearMonth + "/" + QString::number(i + 1), "")
            .toString();
    QStringList list = str.split("-=-");
    QString t0, t1, t2, t3, t4, t5, t6, t7;
    if (list.count() > 0) {
      t0 = list.at(0);
      t1 = list.at(1);
      t2 = list.at(2);
      t3 = list.at(3);
      t4 = list.at(4);
      t5 = list.at(5);
      if (list.count() >= 7) t6 = list.at(6);  // 天气图标
      if (list.count() >= 8) t7 = list.at(7);  // 地形距离汇总
    }

    QString strGpsTime = t0 + "-=-" + t1 + "-=-" + t2 + "-=-" + t4;
    QString speedFile = getJsonRouteFile(strGpsTime);
    speedFile = speedFile.replace(".json", "_Speed.json");

    QVariantList speedData = getSpeedData(speedFile);
    QVariantList altitudeData = getAltitudeData(speedFile);

    /*speedData << QVariant(0.0) << QVariant(3.5) << QVariant(5.2)
              << QVariant(7.8) << QVariant(10.1) << QVariant(8.5)
              << QVariant(6.3) << QVariant(4.0);*/

    insertGpsList(0, t0, t1, t2, t3, t4, t5, t6, t7, speedData, altitudeData);
  }

  if (count > 0) {
    m_Method->gotoBegin(mui->qwGpsList);
    m_Method->setCurrentIndexFromQW(mui->qwGpsList, 0);
  }
}

void Steps::selGpsListYearMonth() {
  QStringList list = mui->btnSelGpsDate->text().split("-");
  int y = 2025;
  int m = 2;
  if (list.count() == 2) {
    y = list.at(0).toInt();
    m = list.at(1).toInt();
  }

  if (isAndroid) {
    m_Method->setDateTimePickerFlag("ym", y, m, 0, 0, 0, "gpslist");
    m_Method->openDateTimePicker();
    return;
  }

  ////////////////////////////////////////////////////////////

  QDate date(y, m, 1);

  mw_one->m_DateSelector->m_datePickerYM->setDate(date);

  mw_one->m_DateSelector->dateFlag = 1;
  mw_one->m_DateSelector->init();
}

void Steps::getGpsListDataFromYearMonth() {
  clearAllGpsList();

  QStringList list;

  if (isAndroid)
    list = m_Method->getDateTimePickerValue();
  else
    list = ymdList;

  int y = list.at(0).toInt();
  int m = list.at(1).toInt();

  loadGpsList(y, m);
  allGpsTotal();
}

QString Steps::getGpsListText0(int index) {
  QQuickItem* root = mui->qwGpsList->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject*)root, "getText0",
                            Q_RETURN_ARG(QVariant, item),
                            Q_ARG(QVariant, index));
  return item.toString();
}

QString Steps::getGpsListText2(int index) {
  QQuickItem* root = mui->qwGpsList->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject*)root, "getText2",
                            Q_RETURN_ARG(QVariant, item),
                            Q_ARG(QVariant, index));
  return item.toString();
}

void Steps::allGpsTotal() {
  QString title = mui->btnSelGpsDate->text();
  QStringList list = title.split("-");
  QString stry = list.at(0);
  stry = stry.trimmed();
  QString strm = list.at(1);
  strm = strm.trimmed();

  QSettings Reg(iniDir + stry + "-gpslist.ini", QSettings::IniFormat);

  double cmTotal, cmTotal_Cycling, cmTotal_Hiking, cmTotal_Running;
  int cmCount, cmCount_Cycling, cmCount_Hiking, cmCount_Running;

  double yt = 0;
  int ycount = 0;
  double monthCyclingKM = 0;
  double monthHikingKM = 0;
  double monthRunningKM = 0;
  double yearCyclingKM = 0;
  double yearHikingKM = 0;
  double yearRunningKM = 0;
  int monthCyclingCount = 0;
  int monthHikingCount = 0;
  int monthRunningCount = 0;
  int yearCyclingCount = 0;
  int yearHikingCount = 0;
  int yearRunningCount = 0;
  for (int i = 0; i < 12; i++) {
    double mt = 0;
    int mcount = 0;
    QString str_mt =
        Reg.value("/" + stry + "/" + QString::number(i + 1), 0).toString();
    QStringList list = str_mt.split("-=-");
    if (list.count() == 2) {
      mt = list.at(0).toDouble();
      mcount = list.at(1).toInt();
    } else if (list.count() == 8) {
      mt = list.at(0).toDouble();
      mcount = list.at(1).toInt();

      monthCyclingKM = list.at(2).toDouble();
      monthCyclingCount = list.at(3).toInt();

      monthHikingKM = list.at(4).toDouble();
      monthHikingCount = list.at(5).toInt();

      monthRunningKM = list.at(6).toDouble();
      monthRunningCount = list.at(7).toInt();
    } else {
      mt = 0;
      mcount = 0;

      monthCyclingKM = 0;
      monthCyclingCount = 0;

      monthHikingKM = 0;
      monthHikingCount = 0;

      monthRunningKM = 0;
      monthRunningCount = 0;
    }

    yt += mt;

    yearCyclingKM += monthCyclingKM;
    yearCyclingCount += monthCyclingCount;

    yearHikingKM += monthHikingKM;
    yearHikingCount += monthHikingCount;

    yearRunningKM += monthRunningKM;
    yearRunningCount += monthRunningCount;

    if (QString::number(i + 1) == strm) {
      cmTotal = mt;
      cmCount = mcount;

      cmTotal_Cycling = monthCyclingKM;
      cmCount_Cycling = monthCyclingCount;

      cmTotal_Hiking = monthHikingKM;
      cmCount_Hiking = monthHikingCount;

      cmTotal_Running = monthRunningKM;
      cmCount_Running = monthRunningCount;
    }
  }

  QSettings Reg1(iniDir + "gpslist.ini", QSettings::IniFormat);

  double m_td = Reg1.value("/GPS/TotalDistance", 0).toDouble();
  Q_UNUSED(m_td);

  QString s1_month, s2_month, s3_month, s4_month;
  s1_month = strm + " " + tr("Month") + ": \n" + QString::number(cmTotal) +
             " km  " + QString::number(cmCount) + "\n";
  s2_month = tr("Cycling") + ": " + QString::number(cmTotal_Cycling) + " km  " +
             QString::number(cmCount_Cycling) + "\n";
  s3_month = tr("Hiking") + ": " + QString::number(cmTotal_Hiking) + " km  " +
             QString::number(cmCount_Hiking) + "\n";
  s4_month = tr("Running") + ": " + QString::number(cmTotal_Running) + " km  " +
             QString::number(cmCount_Running);

  QString s1_year, s2_year, s3_year, s4_year;
  ycount = yearCyclingCount + yearHikingCount + yearRunningCount;
  s1_year = stry + " " + tr("Year") + ": \n" + QString::number(yt) + " km  " +
            QString::number(ycount) + "\n";
  s2_year = tr("Cycling") + ": " + QString::number(yearCyclingKM) + " km  " +
            QString::number(yearCyclingCount) + "\n";
  s3_year = tr("Hiking") + ": " + QString::number(yearHikingKM) + " km  " +
            QString::number(yearHikingCount) + "\n";
  s4_year = tr("Running") + ": " + QString::number(yearRunningKM) + " km  " +
            QString::number(yearRunningCount);

  m_monthlyStatsText = s1_month + s2_month + s3_month + s4_month;
  mui->lblMonthTotal->setText(m_monthlyStatsText);
  m_yearlyStatsText = s1_year + s2_year + s3_year + s4_year;
  mui->lblYearTotal->setText(m_yearlyStatsText);
}

void Steps::appendTrack(double lat, double lon) {
  addTrackDataToAndroid(lat, lon);
  appendTrackPointAndroid(lat, lon);

  return;

  QQuickItem* root = mui->qwMap->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "appendTrack", Q_ARG(QVariant, lat),
                            Q_ARG(QVariant, lon));
}

void Steps::updateInfoText(QString strDistance, QString strSpeed) {
  setInfoLabelToAndroid(strDistance + " | " + strSpeed + "\n" + strGpsTerrain);

  return;

  QQuickItem* root = mui->qwMap->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "updateInfoText",
                            Q_ARG(QVariant, strDistance),
                            Q_ARG(QVariant, strSpeed));
}

void Steps::updateTrackData(double lat, double lon) {
  addTrackDataToAndroid(lat, lon);

  return;

  QQuickItem* root = mui->qwMap->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "updateTrackData",
                            Q_ARG(QVariant, lat), Q_ARG(QVariant, lon));
}

void Steps::updateMapTrackUi(double lat, double lon) {
  QQuickItem* root = mui->qwMap->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "updateMapTrackUi",
                            Q_ARG(QVariant, lat), Q_ARG(QVariant, lon));
}

void Steps::clearTrack() {
  clearTrackAndroid();

  if (!timer->isActive()) clearTrackDataToAndroid();

  return;

  QQuickItem* root = mui->qwMap->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "clearTrack");
}

void Steps::writeGpsPos(double lat, double lon, int i, int count) {
  QString ss0 = t0;
  QString s0 = ss0.replace(" ", "");
  QString sst = strStartTime;
  QString s1 = sst.replace(":", "");

  lat = QString::number(lat, 'f', 6).toDouble();
  lon = QString::number(lon, 'f', 6).toDouble();

  QSettings Reg(iniDir + s0 + "-gps-" + s1 + ".ini", QSettings::IniFormat);

  Reg.setValue("/" + QString::number(i) + "/lat", lat);
  Reg.setValue("/" + QString::number(i) + "/lon", lon);
  Reg.setValue("/count", count);
}

void Steps::getGpsTrack() {
  if (timer->isActive()) {
    openMapWindow();
  } else {
    mw_one->showProgress();
    QQuickItem* root = mui->qwGpsList->rootObject();
    QVariant item;
    QMetaObject::invokeMethod((QObject*)root, "getGpsList",
                              Q_RETURN_ARG(QVariant, item));
    strGpsList = item.toString();
    mw_one->myUpdateGpsMapThread->start();
  }
}

void Steps::updateGpsTrack() {
  QStringList list = strGpsList.split("-=-");
  QString st1 = list.at(0);
  QString st2 = list.at(1);
  strGpsMapDateTime = st1 + " " + st2;

  strGpsTerrain = list.at(4);
  qDebug() << "Gps Terrain=" << strGpsTerrain << list.count();

  QString st3 = list.at(2);
  QString st4 = list.at(3);
  QString st1_0 = st1.split(" ").at(0);
  st1 = st1.replace(st1_0, "");
  st1 = st1.replace(" ", "");
  st2 = st2.split("-").at(0);
  QStringList list2 = st2.split(":");
  st2 = list2.at(1) + list2.at(2) + list2.at(3);
  st1 = st1.trimmed();
  st2 = st2.trimmed();
  st3 = st3.split(":").at(1);
  st3 = st3.trimmed();
  st4 = st4.split(":").at(1);
  st4 = st4.trimmed();
  strGpsMapDistnce = st3;
  strGpsMapSpeed = st4;

  QString str_year, str_month, str_gpsdate;
  str_gpsdate = mui->btnSelGpsDate->text();
  QStringList gpsdateList = str_gpsdate.split("-");
  if (gpsdateList.count() == 2) {
    str_year = gpsdateList.at(0).trimmed();
    str_month = gpsdateList.at(1).trimmed();
  }

  QString csvPath = iniDir + "memo/gps/" + str_year + "/" + str_month + "/";
  QString gpsFile = iniDir + "memo/gps/" + st1 + "-gps-" + st2 + ".csv";
  QString gpsOptimizedFile =
      iniDir + "memo/gps/" + st1 + "-gps-" + st2 + "-opt.csv";

  if (!QFile::exists(gpsOptimizedFile)) {
    gpsFile = csvPath + st1 + "-gps-" + st2 + ".csv";
    gpsOptimizedFile = csvPath + st1 + "-gps-" + st2 + "-opt.csv";
  }

  double lat = 0;
  double lon = 0;
  double alt = 0;
  QString strLat, strLon, strAlt;
  if (QFile::exists(gpsFile)) {
    QVector<GPSCoordinate> rawGPSData;

    // clearTrack();

    QFile file(gpsFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qWarning() << gpsFile;
      return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
      QString line = in.readLine();
      QStringList fields = line.split(",");

      strLat = fields.at(0);
      strLon = fields.at(1);

      lat = strLat.toDouble();
      lon = strLon.toDouble();

      if (fields.count() >= 3) {
        strAlt = fields.at(2);
        alt = strAlt.toDouble();
      }

      rawGPSData.append(GPSCoordinate(lat, lon, alt));
    }

    file.close();

    // 应用高斯滤波
    QVector<GPSCoordinate> filteredData =
        gaussianFilter(rawGPSData, gaussianWindowSize, gaussianSigma);

    // 检测并修正异常点
    QVector<GPSCoordinate> optimizedData = detectAndCorrectOutliers(
        filteredData, outlierThreshold, outlierAltThreshold);

    for (int i = 0; i < optimizedData.count(); i++) {
      lat = optimizedData.at(i).latitude;
      lon = optimizedData.at(i).longitude;
      alt = optimizedData.at(i).altitude;  // 获取优化后的海拔

      QStringList m_data;
      m_data.append(QString::number(lat, 'f', 6));
      m_data.append(QString::number(lon, 'f', 6));
      m_data.append(QString::number(alt, 'f', 2));  // 写入海拔（保留2位小数）

      appendToCSV(gpsOptimizedFile, m_data);
    }

    isGpsMapTrackFile = true;
    lastLat = lat;
    lastLon = lon;

    QFile gfile(gpsFile);
    gfile.remove();

  } else {
    isGpsMapTrackFile = false;
    qDebug() << "gps file=" + gpsFile + " no exists.";
  }

  if (QFile::exists(gpsOptimizedFile)) {
    clearTrack();

    QVector<GPSCoordinate> optimizedData;

    QFile file1(gpsOptimizedFile);
    if (!file1.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qWarning() << gpsOptimizedFile;
      return;
    }

    QTextStream in(&file1);
    while (!in.atEnd()) {
      QString line = in.readLine();
      QStringList fields = line.split(",");
      lat = fields.at(0).toDouble();
      lon = fields.at(1).toDouble();
      if (fields.count() >= 3) {
        alt = fields.at(2).toDouble();
        optimizedData.append(GPSCoordinate(lat, lon, alt));
      }
      updateTrackData(lat, lon);
    }

    file1.close();

    // 新增：计算地形距离（先重置，再计算）
    /*resetTerrainDistance();
    if (optimizedData.count() > 0) {
      calculateTerrainDistance(optimizedData);
    } else {
      updateTerrainUI();
    }*/

    isGpsMapTrackFile = true;
    lastLat = lat;
    lastLon = lon;
  } else {
    isGpsMapTrackFile = false;
    qDebug() << "gps opt file=" + gpsOptimizedFile + " no exists.";
  }
}

void Steps::updateGpsMapUi() {
  if (isGpsMapTrackFile) {
    appendTrackPointAndroid(lastLat, lastLon);
    setDateLabelToAndroid(strGpsMapDateTime);

    QStringList list = strGpsMapSpeed.split("\n");
    if (list.count() > 1) strGpsMapSpeed = list.at(0);
    setInfoLabelToAndroid(strGpsMapDistnce + " | " + strGpsMapSpeed + "\n" +
                          strGpsTerrain);

    openMapWindow();

    return;

    /////////////////////////////////////////////////////

    updateMapTrackUi(lastLat, lastLon);
    mui->lblGpsDateTime->setText(strGpsMapDateTime);
    updateInfoText(strGpsMapDistnce, strGpsMapSpeed);
  }
}

// 高斯滤波函数（支持海拔）
QVector<GPSCoordinate> gaussianFilter(const QVector<GPSCoordinate>& data,
                                      int windowSize, double sigma) {
  if (data.isEmpty() || windowSize % 2 == 0) return data;  // 窗口必须是奇数

  QVector<GPSCoordinate> filteredData;
  int halfWindow = windowSize / 2;
  int n = data.size();

  // 计算高斯核权重
  QVector<double> kernel(windowSize);
  double sum = 0.0;
  for (int i = 0; i < windowSize; ++i) {
    int x = i - halfWindow;
    kernel[i] = exp(-(x * x) / (2 * sigma * sigma));
    sum += kernel[i];
  }
  // 归一化权重
  for (int i = 0; i < windowSize; ++i) {
    kernel[i] /= sum;
  }

  // 对每个点进行滤波（经纬度+海拔分别滤波）
  for (int i = 0; i < n; ++i) {
    double latSum = 0.0, lonSum = 0.0, altSum = 0.0;
    for (int j = 0; j < windowSize; ++j) {
      int idx = i - halfWindow + j;
      if (idx < 0) idx = 0;
      if (idx >= n) idx = n - 1;
      latSum += data[idx].latitude * kernel[j];
      lonSum += data[idx].longitude * kernel[j];
      altSum += data[idx].altitude * kernel[j];  // 海拔也参与滤波
    }
    filteredData.append(GPSCoordinate(latSum, lonSum, altSum));
  }

  return filteredData;
}

double calculateHaversineDistance(double lat1, double lon1, double lat2,
                                  double lon2) {
  // 转换为弧度
  double radLat1 = lat1 * M_PI / 180.0;
  double radLon1 = lon1 * M_PI / 180.0;
  double radLat2 = lat2 * M_PI / 180.0;
  double radLon2 = lon2 * M_PI / 180.0;

  // 经纬度差值
  double dLat = radLat2 - radLat1;
  double dLon = radLon2 - radLon1;

  // Haversine公式核心计算
  double a = sin(dLat / 2) * sin(dLat / 2) +
             cos(radLat1) * cos(radLat2) * sin(dLon / 2) * sin(dLon / 2);
  double c = 2 * atan2(sqrt(a), sqrt(1 - a));

  // 返回距离（米）
  return EARTH_RADIUS_TENCENT * c;
}

// 异常点检测函数（支持海拔）
QVector<GPSCoordinate> detectAndCorrectOutliers(
    const QVector<GPSCoordinate>& data, double distThreshold,
    double altThreshold) {
  if (data.size() < 3) return data;

  QVector<GPSCoordinate> correctedData = data;

  for (int i = 1; i < data.size() - 1; ++i) {
    // 计算当前点与前后点的水平距离
    double distPrev =
        calculateHaversineDistance(data[i].latitude, data[i].longitude,
                                   data[i - 1].latitude, data[i - 1].longitude);
    double distNext =
        calculateHaversineDistance(data[i].latitude, data[i].longitude,
                                   data[i + 1].latitude, data[i + 1].longitude);

    // 计算海拔变化异常
    double altDiffPrev = qAbs(data[i].altitude - data[i - 1].altitude);
    double altDiffNext = qAbs(data[i].altitude - data[i + 1].altitude);

    // 如果距离或海拔变化超过阈值，视为异常点，用前后点均值修正
    if ((distPrev > distThreshold && distNext > distThreshold) ||
        (altDiffPrev > altThreshold && altDiffNext > altThreshold)) {
      double avgLat = (data[i - 1].latitude + data[i + 1].latitude) / 2;
      double avgLon = (data[i - 1].longitude + data[i + 1].longitude) / 2;
      double avgAlt =
          (data[i - 1].altitude + data[i + 1].altitude) / 2;  // 海拔均值修正
      correctedData[i] = GPSCoordinate(avgLat, avgLon, avgAlt);
    }
  }

  return correctedData;
}

void Steps::saveMovementType() {
  bool b1 = mui->rbCycling->isChecked();
  bool b2 = mui->rbHiking->isChecked();
  bool b3 = mui->rbRunning->isChecked();
  bool b4 = mui->chkPlayRunVoice->isChecked();

  QFuture<void> future = QtConcurrent::run([=]() {
    QSettings Reg(iniDir + "gpslist.ini", QSettings::IniFormat);
    Reg.setValue("/GPS/isCycling", b1);
    Reg.setValue("/GPS/isHiking", b2);
    Reg.setValue("/GPS/isRunning", b3);
    Reg.setValue("/GPS/isPlayRunVoice", b4);
    Reg.sync();
  });

  // 使用 QFutureWatcher 监控进度
  QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
  connect(watcher, &QFutureWatcher<void>::finished, this, [=]() {
    qDebug() << "MovementType save completed.";

    watcher->deleteLater();
  });
  watcher->setFuture(future);
}

void Steps::setVibrate() {
#ifdef Q_OS_ANDROID

  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  activity.callMethod<void>("com.x/MyActivity", "setVibrate", "()V");

#endif
}

void Steps::writeCSV(const QString& filePath, const QList<QStringList>& data) {
  QFile file(filePath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qWarning() << filePath;
    return;
  }

  QTextStream out(&file);
  for (const QStringList& row : data) {
    out << row.join(",") << "\n";
  }

  file.close();
}

void Steps::appendToCSV(const QString& filePath, const QStringList& data) {
  QFile file(filePath);
  if (!file.open(QIODevice::Append | QIODevice::Text))  // 以追加模式打开文件
  {
    return;
  }

  QTextStream out(&file);

  // 将数据拼接为CSV格式并写入文件
  out << data.join(",") << "\n";

  file.close();
}

void Steps::updateHardSensorSteps() {
  if (isAndroid) {
    if (isHardStepSensor != 1) return;
  }

  qDebug() << "Started updating the hardware sensor steps...";

  strDate = m_Method->setCurrentDateValue();

  initTodayInitSteps();

  qlonglong steps = 0;
  qlonglong ts = getAndroidSteps();
  steps = ts - initTodaySteps;
  if (steps < 0) return;
  if (steps > 100000000) return;
  CurrentSteps = ts - resetSteps + getOldSteps();
  mui->lcdNumber->display(QString::number(steps));
  mui->lblSingle->setText(QString::number(CurrentSteps));

  setTableSteps(steps);
}

qlonglong Steps::getAndroidSteps() {
  qlonglong a = 0;
#ifdef Q_OS_ANDROID

  a = QJniObject::callStaticMethod<float>("com.x/MyService", "getSteps", "()F");

#endif
  return a;
}

void Steps::initTodayInitSteps() {
  qlonglong a = getAndroidSteps();

  QString jsonPath = iniDir + "initsteps.json";
  QJsonObject jsonObj;
  QFile file(jsonPath);
  if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isObject()) {
      jsonObj = doc.object();
    }
  }

  QString date = jsonObj["Date"].toString("");  // 第二个参数为默认值

  // 获取当前日期（与原逻辑一致）
  QString c_date = getCurrentDate();

  if (date != c_date) {
    // 更新JSON对象（基于已有的jsonObj）
    jsonObj["Date"] = c_date;
    jsonObj["InitValue"] = (double)a;  // 处理长整数类型
    jsonObj["oldSteps"] = 0;

    // 写入文件（复用已定义的file对象）
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      file.write(QJsonDocument(jsonObj).toJson(QJsonDocument::Indented));
      file.close();
    }

    initTodaySteps = a;
  } else {
    //  从JSON中读取InitValue，默认值0，转换为qlonglong
    initTodaySteps = jsonObj["InitValue"].toVariant().toLongLong(0);

    if (a - initTodaySteps <= 0) {
      // 更新JSON键值
      jsonObj["InitValue"] = (double)a;               // 存储InitValue
      jsonObj["oldSteps"] = (double)getTodaySteps();  // 存储oldSteps

      // 写入文件（同步操作，对应Reg.sync()）
      if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(QJsonDocument(jsonObj).toJson(QJsonDocument::Indented));
        file.close();
      }

      initTodaySteps = a;
    }
  }
}

qlonglong Steps::getOldSteps() {
  QString jsonPath = iniDir + "initsteps.json";
  QJsonObject rootObj;
  QFile file(jsonPath);

  // 读取JSON文件内容
  if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    rootObj = QJsonDocument::fromJson(file.readAll()).object();
    file.close();
  }

  // 读取"oldSteps"值（对应原INI的"oldSteps"键），默认0，转换为qlonglong
  return rootObj["oldSteps"].toVariant().toLongLong(0);
}

void Steps::getHardStepSensor() {
#ifdef Q_OS_ANDROID

  isHardStepSensor = QJniObject::callStaticMethod<int>(
      "com.x/MyService", "getHardStepCounter", "()I");

  if (isHardStepSensor == 0) {
    mui->btnStepsOptions->setHidden(true);
    mui->btnReset->setHidden(true);
    mui->tabMotion->setTabEnabled(0, false);
    mui->tabMotion->setCurrentIndex(1);
  }
  if (isHardStepSensor == 1) {
    mui->lblSteps->hide();
    resetSteps = getAndroidSteps();
    initTodayInitSteps();
  }
#endif
}

void Steps::sendMsg(int CurTableCount) {
  Q_UNUSED(CurTableCount);
#ifdef Q_OS_ANDROID
  double sl = m_StepsOptions->ui->editStepLength->text().toDouble();
  double d0 = sl / 100;
  double x = CurTableCount * d0;
  double gl = x / 1000;
  QString strNotify = tr("Today") + " : " + QString::number(CurTableCount) +
                      "  ( " + QString::number(gl, 'f', 2) + " " + tr("km") +
                      " )";

  QJniObject javaNotification = QJniObject::fromString(strNotify);
  QJniObject::callStaticMethod<void>(
      "com/x/MyService", "notify",
      "(Landroid/content/Context;Ljava/lang/String;)V",
      QNativeInterface::QAndroidApplication::context(),
      javaNotification.object<jstring>());

#endif
}

void Steps::setCurrentGpsSpeed(double speed, double maxSpeed) {
  m_speedometer->setCurrentSpeed(speed);
  m_speedometer->setMaxSpeed(maxSpeed);
}

QString Steps::getCurrentYear() {
  return QString::number(QDate::currentDate().year());
}

QString Steps::getCurrentMonth() {
  return QString::number(QDate::currentDate().month());
}

void Steps::openMapWindow() {
#ifdef Q_OS_ANDROID
  setMapType();

  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  activity.callMethod<void>("openMapWindow", "()V");

#endif
}

void Steps::clearTrackAndroid() {
#ifdef Q_OS_ANDROID

  try {
    // 步骤1：获取主活动MyActivity实例（Qt默认获取的是主活动）
    QJniObject myActivity = QNativeInterface::QAndroidApplication::context();
    if (!myActivity.isValid()) {
      qWarning() << "Qt获取MyActivity实例失败，无法清除轨迹";
      return;
    }

    // 步骤2：调用MyActivity的forwardClearTrack()转发方法
    myActivity.callMethod<void>("forwardClearTrack", "()V");
    qDebug() << "Qt调用MyActivity.forwardClearTrack()成功，已转发清除轨迹请求";

  } catch (const std::exception& e) {
    qCritical() << "清除轨迹调用异常：" << e.what();
  } catch (...) {
    qCritical() << "清除轨迹发生未知错误";
  }

#endif
}

void Steps::appendTrackPointAndroid(double latitude, double longitude) {
#ifdef Q_OS_ANDROID

  try {
    if (latitude < -90.0 || latitude > 90.0 || longitude < -180.0 ||
        longitude > 180.0) {
      qWarning() << "无效经纬度：" << latitude << "," << longitude;
      return;
    }

    QJniObject myActivity = QNativeInterface::QAndroidApplication::context();
    if (!myActivity.isValid()) {
      qWarning() << "Qt获取MyActivity实例失败，无法追加轨迹点";
      return;
    }

    // 新增：调用MyActivity的isMapActivityInstance()判断地图实例是否存在
    jboolean isInstanceValid = myActivity.callMethod<jboolean>(
        "isMapActivityInstance",  // Java方法名
        "()Z"  // 方法签名：无参数，返回boolean（Z表示boolean）
    );
    if (!isInstanceValid) {
      qWarning()
          << "地图实例不存在（isMapActivityInstance返回false），跳过追加轨迹点";
      return;
    }

    myActivity.callMethod<void>("forwardAppendTrackPoint", "(DD)V", latitude,
                                longitude);
    qDebug() << "Qt调用MyActivity.forwardAppendTrackPoint()成功，轨迹点："
             << latitude << "," << longitude;

  } catch (const std::exception& e) {
    qCritical() << "追加轨迹点异常：" << e.what();
  } catch (...) {
    qCritical() << "追加轨迹点发生未知错误";
  }

#else
  Q_UNUSED(latitude);
  Q_UNUSED(longitude);
#endif
}

void Steps::addTrackDataToAndroid(double latitude, double longitude) {
  Q_UNUSED(latitude);
  Q_UNUSED(longitude);

#ifdef Q_OS_ANDROID

  try {
    QJniObject activity = QNativeInterface::QAndroidApplication::context();

    if (activity.isValid()) {
      activity.callMethod<void>("addTrackPoint",  // Java方法名
                                "(DD)V",   // 方法签名：两个double参数，返回void
                                latitude,  // 第一个参数：纬度
                                longitude  // 第二个参数：经度
      );
      qDebug() << "Qt调用addTrackPoint成功，纬度：" << latitude << "经度："
               << longitude;
    } else {
      qWarning() << "获取MapActivity实例失败，无法添加轨迹点";
    }
  } catch (const std::exception& e) {
    qCritical() << "调用addTrackPoint失败：" << e.what();
  }

#endif
}

void Steps::clearTrackDataToAndroid() {
#ifdef Q_OS_ANDROID

  try {
    QJniObject activity = QNativeInterface::QAndroidApplication::context();

    if (activity.isValid()) {
      activity.callMethod<void>("clearTrackPoints",  // Java方法名
                                "()V"  // 方法签名：无参数，返回void
      );
      qDebug() << "Qt调用clearTrackPoints成功";
    } else {
      qWarning() << "获取MapActivity实例失败，无法清除轨迹点";
    }
  } catch (const std::exception& e) {
    qCritical() << "调用clearTrackPoints失败：" << e.what();
  }

#endif
}

void Steps::setDateLabelToAndroid(const QString& str) {
  Q_UNUSED(str);
#ifdef Q_OS_ANDROID

  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  if (activity.isValid()) {
    // 将QString转换为Java的String
    QJniObject javaStr = QJniObject::fromString(str);
    // 调用Java的setDateTitle方法
    activity.callMethod<void>("setDateTitle", "(Ljava/lang/String;)V",
                              javaStr.object<jstring>());
  }

#endif
}

void Steps::setInfoLabelToAndroid(const QString& str) {
#ifdef Q_OS_ANDROID

  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  if (activity.isValid()) {
    QJniObject javaStr = QJniObject::fromString(str);
    activity.callMethod<void>("setInfoTitle", "(Ljava/lang/String;)V",
                              javaStr.object<jstring>());
  }

#else
  Q_UNUSED(str);
#endif
}

// 核心转换方法：1:1复刻Java wgs84ToGcj02_bak（运算顺序、变量名完全对齐）
QGeoCoordinate Steps::wgs84ToGcj02_cpp(double wgs84Lat, double wgs84Lon) {
  // 输入校验（保持原有）
  if (wgs84Lon < -180.0 || wgs84Lon > 180.0 || wgs84Lat < -90.0 ||
      wgs84Lat > 90.0) {
    qWarning() << "[WGS84ToGCJ02] 无效 WGS84 坐标：lat=" << wgs84Lat
               << ", lon=" << wgs84Lon;
    return QGeoCoordinate();
  }

  // 判断是否在中国境内（同步Java的经纬度范围，关键差异点）
  if (!isInChina(wgs84Lat, wgs84Lon)) {
    return QGeoCoordinate(wgs84Lat, wgs84Lon);
  }

  double x = wgs84Lon - 105.0;  // 对应Java的wgsLng - 105.0
  double y = wgs84Lat - 35.0;   // 对应Java的wgsLat - 35.0

  // 调用复刻的辅助函数，避免inline计算偏差
  double dLat = transformLat(x, y);
  double dLng = transformLng(x, y);

  // 完全同步Java的弧度计算逻辑
  double radLat = (wgs84Lat / 180.0) * PI;
  double sinRadLat = sin(radLat);
  double magic = 1 - AXIS_TENCENT * sinRadLat * sinRadLat;
  double sqrtMagic = sqrt(magic);

  // 偏移量计算：括号位置、运算顺序完全复刻Java
  dLat = (dLat * 180.0) /
         (((EARTH_RADIUS_TENCENT * (1 - AXIS_TENCENT)) / (magic * sqrtMagic)) *
          PI);
  dLng =
      (dLng * 180.0) / ((EARTH_RADIUS_TENCENT / sqrtMagic) * cos(radLat) * PI);

  // 精度补偿：完全同步Java的加法顺序和补偿值
  double gcjLat = wgs84Lat + dLat + PRECISION_COMPENSATION;
  double gcjLng = wgs84Lon + dLng + PRECISION_COMPENSATION;

  return QGeoCoordinate(gcjLat, gcjLng);
}

// 判断境内函数：同步Java的经纬度范围
bool Steps::isInChina(double lat, double lon) {
  return (lon > GCJ02_LON_MIN && lon < GCJ02_LON_MAX) &&
         (lat > GCJ02_LAT_MIN && lat < GCJ02_LAT_MAX);
}

void Steps::saveRoute(const QString& file, const QString& time, double lat,
                      double lon, const QString& address) {
  if (!timer->isActive()) return;

  bool latValid = (lat >= -90 && lat <= 90);
  bool lonValid = (lon >= -180 && lon <= 180);
  if (!latValid || !lonValid) return;

  if (address == "") return;

  // 初始化 JSON 数组（读取现有数据或创建新数组）
  QJsonArray routeArray;
  QFile jsonFile(file);

  // 构造新的路由对象（字段类型严格匹配：字符串/数字）
  QJsonObject newRoute;
  newRoute["time"] = time;  // 字符串类型
  newRoute["lat"] = lat;    // 数字类型（double）
  newRoute["lon"] = lon;    // 数字类型（double）
  newRoute["address"] = directionRoute + "\n" + distanceRoute + " | " +
                        speedRoute + "\n" + strAltitude + " m" + "\n\n" +
                        address;

  // 追加新对象到数组
  routeMemoryCache.append(newRoute);

  routeArray = routeMemoryCache;

  // 写入文件（覆盖原有内容，UTF-8 编码，格式化输出）
  if (!jsonFile.open(QIODevice::WriteOnly | QIODevice::Text |
                     QIODevice::Truncate)) {
    qWarning() << "[saveRoute] 无法打开文件（写入）：" << file
               << jsonFile.errorString();
    return;
  }

  // 生成格式化的 JSON 文档（缩进 4 空格，便于阅读）
  QJsonDocument outputDoc(routeArray);
  jsonFile.write(outputDoc.toJson(QJsonDocument::Indented));
  jsonFile.flush();
  jsonFile.close();

  qDebug() << "[saveRoute] 路由数据追加成功：" << file << "（累计"
           << routeArray.size() << "条）";
}

QStringList Steps::readRoute(const QString& file) {
  QJsonArray routeArray;
  QStringList routeList;

  if (!timer->isActive()) {
    QFile jsonFile(file);

    // 1. 基础文件检查（不存在/打开失败直接返回空列表）
    if (!jsonFile.exists() ||
        !jsonFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qWarning() << "[readRoute] 文件无法读取：" << file
                 << (jsonFile.exists() ? jsonFile.errorString() : "文件不存在");
      return routeList;
    }

    // 2. 解析 JSON 数组（仅校验是否为数组，不校验内部字段）
    routeArray = QJsonDocument::fromJson(jsonFile.readAll()).array();
    jsonFile.close();  // 立即关闭文件
  } else {
    routeArray = routeMemoryCache;
  }

  if (routeArray.isEmpty()) {
    qDebug() << "[readRoute] JSON 数组为空：" << file;
    return routeList;
  }

  // 3. 遍历解析：缺失字段直接置空/0，不跳过数据
  for (int i = 0; i < routeArray.size(); ++i) {
    QJsonObject obj = routeArray.at(i).toObject();

    // 读取字段：不存在则返回默认值（字符串空，数字0）
    QString time =
        QString::number(i + 1) + ". " + obj["time"].toString();  // 缺失 → ""
    double lat = obj["lat"].toDouble();                          // 缺失 → 0.0
    double lon = obj["lon"].toDouble();                          // 缺失 → 0.0
    QString address = obj["address"].toString();                 // 缺失 → ""

    // 格式化经纬度（保留6位小数，0值也会正常显示）
    QString latLonStr =
        QString("%1 - %2").arg(lat, 0, 'f', 6).arg(lon, 0, 'f', 6);
    // 拼接格式：time===lat lon===address（缺失字段显示空字符串）
    QString routeItem = QString("%1===%2===%3").arg(time, latLonStr, address);

    routeList.append(routeItem);
  }

  qDebug() << "[readRoute] 读取完成：" << file << "（共" << routeList.size()
           << "条数据）";
  return routeList;
}

void Steps::getAddress(double lat, double lon) {
  QGeoCoordinate gcj02Coord;
  if (isAndroid)
    gcj02Coord = wgs84ToGcj02(lat, lon);
  else
    gcj02Coord = wgs84ToGcj02_cpp(lat, lon);
  addressResolver->getAddressFromCoord(gcj02Coord.latitude(),
                                       gcj02Coord.longitude());
}

void Steps::setMapKey() {
  QSettings Reg(iniDir + "gpslist.ini", QSettings::IniFormat);
  QString arg1 = Reg.value("/Map/MapKey", "").toString();

  m_StepsOptions->ui->editMapKey->setPlainText(arg1);
  if (addressResolver) addressResolver->setTencentApiKey(arg1);

#ifdef Q_OS_ANDROID

  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  if (activity.isValid()) {
    QString mapKey = arg1;
    QJniObject jKey = QJniObject::fromString(mapKey);

    // 调用Java的setMapKey方法，参数为String，返回值为void
    activity.callMethod<void>(
        "setMapKey",
        "(Ljava/lang/String;)V",  // JNI签名：接收String参数，无返回值
        jKey.object<jstring>());
    qDebug() << "已调用Java层setMapKey方法";
  }

#endif
}

void Steps::setMapKeyError() {
#ifdef Q_OS_ANDROID

  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  if (activity.isValid()) {
    QString mapKey = "error";
    QJniObject jKey = QJniObject::fromString(mapKey);

    // 调用Java的setMapKey方法，参数为String，返回值为void
    activity.callMethod<void>(
        "setMapKey",
        "(Ljava/lang/String;)V",  // JNI签名：接收String参数，无返回值
        jKey.object<jstring>());
    qDebug() << "已调用Java层setMapKey方法";
  }

#endif
}

void Steps::setMapType() {
#ifdef Q_OS_ANDROID
  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  if (activity.isValid()) {
    int mapKey = 1;
    if (m_StepsOptions->ui->rbOsm->isChecked()) mapKey = 1;
    if (m_StepsOptions->ui->rbTencent->isChecked()) mapKey = 2;

    // JNI调用：传递int参数，签名改为"(I)V"
    activity.callMethod<void>("setMapType",  // Java层方法名
                              "(I)V",        // JNI签名：接收int参数，无返回值
                              mapKey         // 直接传递int值
    );
  }
#endif
}

QString Steps::getJsonRouteFile(const QString& strGpsList) {
  QStringList list = strGpsList.split("-=-");

  QString st1 = list.at(0);
  QString st2 = list.at(1);
  strGpsMapDateTime = st1 + " " + st2;

  QString st3 = list.at(2);
  QString st4 = list.at(3);
  QString st1_0 = st1.split(" ").at(0);
  st1 = st1.replace(st1_0, "");
  st1 = st1.replace(" ", "");
  st2 = st2.split("-").at(0);
  QStringList list2 = st2.split(":");
  st2 = list2.at(1) + list2.at(2) + list2.at(3);
  st1 = st1.trimmed();
  st2 = st2.trimmed();
  st3 = st3.split(":").at(1);
  st3 = st3.trimmed();
  st4 = st4.split(":").at(1);
  st4 = st4.trimmed();
  strGpsMapDistnce = st3;
  strGpsMapSpeed = st4;

  QString str_year, str_month, str_gpsdate;
  str_gpsdate = mui->btnSelGpsDate->text();
  QStringList gpsdateList = str_gpsdate.split("-");
  if (gpsdateList.count() == 2) {
    str_year = gpsdateList.at(0).trimmed();
    str_month = gpsdateList.at(1).trimmed();
  }

  QString csvPath = iniDir + "memo/gps/" + str_year + "/" + str_month + "/";
  QString routeFile = csvPath + st1 + "-gps-" + st2 + ".json";
  return routeFile;
}

void Steps::getRouteList(const QString& strGpsTime) {
  strGpsList = strGpsTime;

  QString routeFile = getJsonRouteFile(strGpsList);

  qDebug() << "routeFile=" << routeFile;
  if (!QFile::exists(routeFile)) return;

  // 获取弹出窗口对象（routeDialog）
  QQuickItem* root = mui->qwGpsList->rootObject();
  if (!root) {
    qWarning() << "[C++] 未找到QML根对象";
    return;
  }

  QObject* routeDialog = root->findChild<QObject*>("routeDialog");
  if (!routeDialog) {
    qWarning() << "[C++] 未找到 routeDialog 对象";
    return;
  }

  // 清空旧数据
  QMetaObject::invokeMethod(routeDialog, "clearRouteModel");

  QStringList routeList = readRoute(routeFile);
  for (int i = 0; i < routeList.count(); i++) {
    QString routeItem = routeList.at(i);
    // 按 "===" 拆分条目（对应 readRoute 中的拼接格式）
    QStringList parts = routeItem.split("===");
    if (parts.size() != 3) {
      qWarning() << "[C++] 无效路由数据（索引" << i << "）：" << routeItem;
      continue;
    }

    // parts[0] = 时间，parts[1] = 纬度 经度，parts[2] = 地址
    QString timeStr = parts[0];
    QString latLonStr = parts[1];
    QString addressStr = parts[2];

    // 调用 QML 的 addRouteItem 方法添加数据到弹出窗口
    // 参数顺序：timeStr, latLonStr, addressStr
    QMetaObject::invokeMethod(
        routeDialog, "addRouteItem", Q_ARG(QVariant, timeStr),
        Q_ARG(QVariant, latLonStr), Q_ARG(QVariant, addressStr));
  }

  // 显示弹出窗口
  QMetaObject::invokeMethod(routeDialog, "setVisible", Q_ARG(QVariant, true));
}

void Steps::closeRouteDialog() {
  QQuickItem* root = mui->qwGpsList->rootObject();
  if (!root) {
    qWarning() << "[C++] 未找到QML根对象";
    return;
  }

  QObject* routeDialog = root->findChild<QObject*>("routeDialog");
  if (!routeDialog) {
    qWarning() << "[C++] 未找到 routeDialog 对象";
    return;
  }
  QMetaObject::invokeMethod(routeDialog, "setVisible", Q_ARG(QVariant, false));
}

bool Steps::isRouteShow() {
  QQuickItem* root = mui->qwGpsList->rootObject();
  if (!root) {
    qWarning() << "[C++] 未找到QML根对象";
    return false;
  }

  QObject* routeDialog = root->findChild<QObject*>("routeDialog");
  if (!routeDialog) {
    qWarning() << "[C++] 未找到 routeDialog 对象";
    return false;
  }

  // 调用isVisible()并获取返回值
  QVariant result;
  bool invokeSuccess = QMetaObject::invokeMethod(
      routeDialog, "isVisible",
      Qt::DirectConnection,           // 关键：强制同步调用，允许获取返回值
      Q_RETURN_ARG(QVariant, result)  // 指定返回值的接收变量
  );

  if (!invokeSuccess) {
    qWarning() << "[C++] 调用isVisible()失败";
    return false;
  }

  // 将QVariant转换为bool并返回
  return result.toBool();
}

// 坐标转换：调用安卓端CoordinateConverterUtil工具类
QGeoCoordinate Steps::wgs84ToGcj02(double wgs84Lat, double wgs84Lon) {
  Q_UNUSED(wgs84Lat);
  Q_UNUSED(wgs84Lon);

#ifdef Q_OS_ANDROID

  // Qt6中使用QJniEnvironment管理JNI环境
  QJniEnvironment env;

  try {
    // 1. 调用Java工具类的静态方法：CoordinateConverterUtil.wgs84ToGcj02(double,
    // double) 类路径："com/x/CoordinateConverterUtil"（注意用/分隔包名）
    // 方法签名："(DD)Lcom/tencent/mapsdk/raster/model/LatLng;"（两个double参数，返回LatLng对象）
    QJniObject gcjLatLng = QJniObject::callStaticObjectMethod(
        "com/x/CoordinateConverterUtil",  // Java类全路径
        "wgs84ToGcj02",                   // 静态方法名
        "(DD)Lcom/tencent/tencentmap/mapsdk/maps/model/LatLng;",  // 方法签名
        wgs84Lat,  // 参数1：WGS84纬度
        wgs84Lon   // 参数2：WGS84经度
    );

    // 2. 检查返回的LatLng对象是否有效
    if (!gcjLatLng.isValid()) {
      qWarning() << "[Qt6 JNI] 坐标转换失败：返回无效LatLng对象";
      return QGeoCoordinate(wgs84Lat, wgs84Lon);  // 失败时返回原坐标
    }

    // 3. 解析LatLng对象的纬度和经度（调用其getLatitude()和getLongitude()方法）
    double gcjLat = gcjLatLng.callMethod<jdouble>("getLatitude", "()D");
    double gcjLon = gcjLatLng.callMethod<jdouble>("getLongitude", "()D");

    // 4. 检查JNI调用是否产生异常（Qt6需显式处理）
    if (env->ExceptionCheck()) {
      qWarning() << "[Qt6 JNI] 解析LatLng时发生异常";
      env->ExceptionClear();  // 清除异常，避免崩溃
      return QGeoCoordinate(wgs84Lat, wgs84Lon);
    }

    qDebug() << "[Qt6 JNI] 转换成功：WGS84(" << wgs84Lat << "," << wgs84Lon
             << ") -> GCJ02(" << gcjLat << "," << gcjLon << ")";

    return QGeoCoordinate(gcjLat, gcjLon);

  } catch (const std::exception& e) {
    qCritical() << "[Qt6 JNI] 转换过程异常：" << e.what();
    // 清除可能的JNI异常
    if (env->ExceptionCheck()) {
      env->ExceptionClear();
    }
  }

#endif

  // 所有异常情况均返回原坐标
  return QGeoCoordinate(wgs84Lat, wgs84Lon);
}

// 保存速度数据（追加模式，每次调用添加一条带时间戳的速度记录）
void Steps::saveSpeedData(const QString& jsonFile, double speed,
                          double altitude) {
  // 1. 读取已有数据（若文件存在）
  QJsonArray speedArray;
  QFile file(jsonFile);

  // 如果文件存在且可读取，解析现有数据
  if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isArray()) {
      speedArray = doc.array();  // 现有数据是数组，直接复用
    } else {
      qWarning() << "saveSpeedData: 文件" << jsonFile
                 << "格式错误，将创建新文件";
    }
  }

  // 2. 添加新的速度记录（包含时间戳，便于追溯采样时间）
  QJsonObject newRecord;
  // 替换原来的时间戳获取方式
  newRecord["timestamp"] =
      QDateTime::currentMSecsSinceEpoch();  // 直接调用静态方法，更高效
  newRecord["speed"] = speed;               // 速度值（单位根据业务定，如km/h）
  newRecord["altitude"] = altitude;
  speedArray.append(newRecord);

  // 3. 写入更新后的数据到文件
  if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
    QJsonDocument doc(speedArray);
    file.write(doc.toJson(QJsonDocument::Indented));  // 格式化输出，便于调试
    file.close();
  } else {
    qWarning() << "saveSpeedData: 无法打开文件" << jsonFile << "进行写入";
  }
}

// 读取所有速度数据，返回QVariantList（便于传递到QML）
QVariantList Steps::getSpeedData(const QString& jsonFile) {
  QVariantList speedList;
  QFile file(jsonFile);

  // 检查文件是否存在
  if (!file.exists()) {
    qWarning() << "getSpeedData: 文件" << jsonFile << "不存在";
    return speedList;  // 返回空列表
  }

  // 打开并读取文件
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning() << "getSpeedData: 无法打开文件" << jsonFile;
    return speedList;
  }

  QByteArray data = file.readAll();
  file.close();

  // 解析JSON
  QJsonDocument doc = QJsonDocument::fromJson(data);
  if (!doc.isArray()) {
    qWarning() << "getSpeedData: 文件" << jsonFile << "不是有效的JSON数组";
    return speedList;
  }

  QJsonArray speedArray = doc.array();
  // 使用const迭代器遍历，避免容器分离
  for (auto it = speedArray.constBegin(); it != speedArray.constEnd(); ++it) {
    const QJsonValue& val = *it;  // 通过迭代器获取元素
    if (val.isObject()) {
      QJsonObject record = val.toObject();
      if (record.contains("speed") && record["speed"].isDouble()) {
        speedList.append(record["speed"].toDouble());
      }
    }
  }

  return speedList;
}

// 读取所有海拔数据，返回QVariantList（便于传递到QML）
QVariantList Steps::getAltitudeData(const QString& jsonFile) {
  QVariantList altitudeList;
  QFile file(jsonFile);

  // 检查文件是否存在
  if (!file.exists()) {
    qWarning() << "getAltitudeData: 文件" << jsonFile << "不存在";
    return altitudeList;  // 返回空列表
  }

  // 打开并读取文件
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning() << "getSpeedData: 无法打开文件" << jsonFile;
    return altitudeList;
  }

  QByteArray data = file.readAll();
  file.close();

  // 解析JSON
  QJsonDocument doc = QJsonDocument::fromJson(data);
  if (!doc.isArray()) {
    qWarning() << "getAltitudeData: 文件" << jsonFile << "不是有效的JSON数组";
    return altitudeList;
  }

  QJsonArray altitudeArray = doc.array();
  // 使用const迭代器遍历，避免容器分离
  for (auto it = altitudeArray.constBegin(); it != altitudeArray.constEnd();
       ++it) {
    const QJsonValue& val = *it;  // 通过迭代器获取元素
    if (val.isObject()) {
      QJsonObject record = val.toObject();
      if (record.contains("altitude") && record["altitude"].isDouble()) {
        altitudeList.append(record["altitude"].toDouble());
      }
    }
  }

  return altitudeList;
}

void Steps::showSportsChart() {
  // 1. 关闭已有对话框
  if (statsDialog != nullptr) {
    delete statsDialog;
  }
  statsDialog = new QDialog(this);
  statsDialog->setWindowTitle(tr("Sports Statistics"));

  if (this->parentWidget()) {
    statsDialog->resize(this->parentWidget()->size());
  } else {
    statsDialog->resize(400, 710);  // 增加10px容纳频次曲线，保持紧凑
  }

  // 2. 读取 INI 文件（硬读1-12月，解决数据错乱）
  QString title = mui->btnSelGpsDate->text();
  QStringList list = title.split("-");
  if (list.size() < 2) {
    QLabel* errLabel = new QLabel(tr("Date format error!"), statsDialog);
    QVBoxLayout* errLayout = new QVBoxLayout(statsDialog);
    errLayout->setContentsMargins(15, 15, 15, 15);
    errLayout->addWidget(errLabel);
    statsDialog->setLayout(errLayout);
    statsDialog->exec();
    return;
  }
  QString stry = list.at(0).trimmed();
  QString yearGroup = stry;

  QSettings reg(iniDir + stry + "-gpslist.ini", QSettings::IniFormat);

  // 3. 数据处理（硬读1-12月，确保顺序和完整性）

  QVector<MonthData> monthDataList(12);  // 索引0-11对应1-12月

  // 硬读1-12月，不依赖childKeys，确保每个月都被处理
  reg.beginGroup(yearGroup);
  for (int month = 1; month <= 12; ++month) {  // 从1到12循环
    QString monthKey = QString::number(month);
    QString value =
        reg.value(monthKey).toString();  // 直接读取该月数据，空则说明无数据
    if (value.isEmpty()) {
      continue;  // 无数据则保持默认0
    }

    QStringList parts = value.split("-=-");
    if (parts.size() != 8) {
      qWarning() << "[INI Error] Month" << month
                 << "data format invalid:" << value;
      continue;
    }

    // 填充对应月份数据（索引=月份-1）
    MonthData& data = monthDataList[month - 1];
    data.cyclingDist = parts[2].toDouble();
    data.cyclingCount = parts[3].toInt();
    data.hikingDist = parts[4].toDouble();
    data.hikingCount = parts[5].toInt();
    data.runningDist = parts[6].toDouble();
    data.runningCount = parts[7].toInt();
  }
  reg.endGroup();

  // ------------------------------------------------------------------------------
  // 新增：自定义频次曲线Widget
  // 提取三种运动的频次数据（直接复用monthDataList）
  QVector<int> cyclingCounts, hikingCounts, runningCounts;
  for (const auto& data : std::as_const(monthDataList)) {
    cyclingCounts.append(data.cyclingCount);
    hikingCounts.append(data.hikingCount);
    runningCounts.append(data.runningCount);
  }
  // 创建自定义曲线Widget（传入数据、主题、父窗口）
  FrequencyCurveWidget* countCurveWidget = new FrequencyCurveWidget(
      cyclingCounts, hikingCounts, runningCounts, isDark, statsDialog);
  // --------------------------------------------------------------------------------

  // 4. 创建水平条形图（原有里程图表，保持不变）
  QChart* chart = new QChart();
  chart->setTitle(tr("%1 Monthly Sports Statistics").arg(stry));
  chart->legend()->setAlignment(Qt::AlignBottom);
  chart->setMargins(QMargins(0, 0, 0, 5));  // 彻底去除图表内边距，给Y轴腾空间
  if (isDark) {
    chart->setTheme(QChart::ChartThemeDark);
  } else {
    chart->setTheme(QChart::ChartThemeLight);
  }

  // 4.1 创建水平条形系列和集合
  QHorizontalBarSeries* barSeries = new QHorizontalBarSeries();
  QBarSet* cyclingSet = new QBarSet(tr("Cycling"));
  QBarSet* hikingSet = new QBarSet(tr("Hiking"));
  QBarSet* runningSet = new QBarSet(tr("Running"));

  // 主题适配颜色
  if (isDark) {
    cyclingSet->setBrush(QColor(90, 189, 94));
    hikingSet->setBrush(QColor(255, 171, 44));
    runningSet->setBrush(QColor(183, 70, 201));
  } else {
    cyclingSet->setBrush(QColor(76, 175, 80));
    hikingSet->setBrush(QColor(255, 152, 0));
    runningSet->setBrush(QColor(156, 39, 176));
  }

  // 4.2 填充数据
  for (int i = 0; i < 12; ++i) {
    const MonthData& data = monthDataList[i];
    *cyclingSet << data.cyclingDist;
    *hikingSet << data.hikingDist;
    *runningSet << data.runningDist;
  }

  barSeries->append(cyclingSet);
  barSeries->append(hikingSet);
  barSeries->append(runningSet);
  barSeries->setBarWidth(0.9);  // 最大化条形宽度，减少垂直方向空隙

  // 隐藏图表文字标签
  barSeries->setLabelsVisible(false);

  // 4.3 配置坐标轴（确保数字月份完整显示）
  QCategoryAxis* axisY = new QCategoryAxis();
  QStringList monthLabels;
  for (int i = 1; i <= 12; ++i) {
    monthLabels << QString::number(i);  // 数字标签
  }
  for (int i = 0; i < monthLabels.size(); ++i) {
    axisY->append(monthLabels[i], i + 1);
  }
  axisY->setLabelsAngle(0);
  axisY->setTitleText(tr("Month"));

  QValueAxis* axisX = new QValueAxis();
  axisX->setTitleText(tr("Distance (KM)"));
  axisX->setTickCount(5);

  // 4.4 添加系列和轴到图表
  chart->addSeries(barSeries);
  barSeries->attachAxis(axisY);  // 水平条形系列关联Y轴（月份）
  barSeries->attachAxis(axisX);  // 水平条形系列关联X轴（里程）
  chart->createDefaultAxes();

  // 4.5 图表视图配置（增加高度，去除边框）
  // QChartView* chartView = new QChartView(chart);
  CustomChartView* chartView = new CustomChartView();
  chartView->setChart(chart);
  chartView->setMonthData(monthDataList);

  chartView->setRenderHint(QPainter::Antialiasing);
  chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  int mh = 600;
  if (!isAndroid) mh = 450;
  chartView->setMinimumHeight(mh);  // 原有高度逻辑不变
  chartView->setMaximumHeight(mh);
  chartView->setFrameStyle(QFrame::NoFrame);
  chartView->setContentsMargins(0, 0, 0, 0);  // 去除视图边距

  // 5. 整合布局（用自定义曲线Widget替换原countChartView）
  QVBoxLayout* mainLayout = new QVBoxLayout(statsDialog);
  mainLayout->setContentsMargins(5, 5, 5, 5);  // 最小化整体边距
  mainLayout->setSpacing(5);

  QFont font = this->font();
  font.setBold(true);

  QLabel* totalLabel =
      new QLabel(tr("Total Distance") + " : " + strTotalDistance, statsDialog);
  totalLabel->setFrameShape(QFrame::Box);
  totalLabel->setFrameShadow(QFrame::Plain);
  totalLabel->setLineWidth(1);
  totalLabel->setContentsMargins(5, 5, 5, 5);
  totalLabel->setAlignment(Qt::AlignCenter);

  totalLabel->setFont(font);

  QLabel* monthlyLabel = new QLabel(m_monthlyStatsText, statsDialog);
  monthlyLabel->setAlignment(Qt::AlignLeft);
  monthlyLabel->setWordWrap(true);
  monthlyLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  QLabel* yearlyLabel = new QLabel(m_yearlyStatsText, statsDialog);
  yearlyLabel->setAlignment(Qt::AlignLeft);
  yearlyLabel->setWordWrap(true);
  yearlyLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  if (isAndroid)
    font.setPointSize(13);
  else
    font.setPointSize(9);
  monthlyLabel->setFont(font);
  yearlyLabel->setFont(font);

  monthlyLabel->setFrameShape(QFrame::Box);     // 边框形状：矩形框
  monthlyLabel->setFrameShadow(QFrame::Plain);  // 边框阴影：无阴影（简洁风格）
  monthlyLabel->setLineWidth(1);                // 边框线宽：1px
  monthlyLabel->setContentsMargins(5, 5, 5, 5);
  yearlyLabel->setFrameShape(QFrame::Box);
  yearlyLabel->setFrameShadow(QFrame::Plain);
  yearlyLabel->setLineWidth(1);
  yearlyLabel->setContentsMargins(5, 5, 5, 5);

  QHBoxLayout* hboxLayout = new QHBoxLayout();
  hboxLayout->addWidget(monthlyLabel);
  hboxLayout->addWidget(yearlyLabel);

  // 缩小的关闭按钮
  QPushButton* closeButton = new QPushButton(tr("Close"), statsDialog);
  QFont btnFont = closeButton->font();
  btnFont.setPointSize(10);
  closeButton->setFont(btnFont);
  closeButton->setStyleSheet(isDark ? R"(
        QPushButton {
            background-color: #66bb6a;
            color: white;
            border: none;
            padding: 5px 14px;
            border-radius: 4px;
        }
        QPushButton:hover { background-color: #4caf50; }
        QPushButton:pressed { background-color: #388e3c; }
    )"
                                    : R"(
        QPushButton {
            background-color: #4CAF50;
            color: white;
            border: none;
            padding: 5px 14px;
            border-radius: 4px;
        }
        QPushButton:hover { background-color: #45a049; }
        QPushButton:pressed { background-color: #3e8e41; }
    )");

  // 布局顺序：totalLabel → 标签横向布局 → 自定义频次曲线 → 里程图表 → 按钮
  mainLayout->addWidget(totalLabel);
  mainLayout->addLayout(hboxLayout);
  mainLayout->addWidget(countCurveWidget);  // 替换为自定义曲线Widget
  mainLayout->addWidget(chartView);         // 原有：里程图表
  mainLayout->addSpacing(3);
  mainLayout->addWidget(closeButton, 0, Qt::AlignCenter);
  closeButton->hide();

  // 6. 信号连接
  connect(closeButton, &QPushButton::clicked, statsDialog, &QDialog::close);
  connect(statsDialog, &QDialog::finished, this, [this, chartView, chart](int) {
    chartView->deleteLater();
    chart->deleteLater();
    delete this->statsDialog;
    this->statsDialog = nullptr;
  });

  statsDialog->setLayout(mainLayout);
  statsDialog->exec();
}

// 重置地形距离统计（开始新运动时调用）
void Steps::resetTerrainDistance() {
  m_uphillDistance = 0.0;
  m_flatDistance = 0.0;
  m_downhillDistance = 0.0;
}

// 从优化后的轨迹数据计算地形距离
void Steps::calculateTerrainDistance(
    const QVector<GPSCoordinate>& optimizedData) {
  if (optimizedData.size() < 2) return;  // 至少需要2个点才能计算

  for (int i = 1; i < optimizedData.size(); ++i) {
    const GPSCoordinate& prev = optimizedData[i - 1];
    const GPSCoordinate& curr = optimizedData[i];

    // 1. 计算水平距离（米），转换为公里
    double horizontalDistance =
        calculateHaversineDistance(prev.latitude, prev.longitude, curr.latitude,
                                   curr.longitude) /
        1000.0;  // 转换为公里

    // 2. 计算海拔差（米）
    double altitudeDiff = curr.altitude - prev.altitude;

    // 3. 根据海拔差判断地形类型，累加距离
    if (altitudeDiff > UPHILL_THRESHOLD) {
      m_uphillDistance += horizontalDistance;
    } else if (altitudeDiff < DOWNHILL_THRESHOLD) {
      m_downhillDistance += horizontalDistance;
    } else {
      m_flatDistance += horizontalDistance;
    }
  }

  // 触发UI更新
  updateTerrainUI();
}

void Steps::updateTerrainUI() {
  // 格式化距离为字符串（保留2位小数）
  QString uphillStr =
      tr("Uphill: ") + QString::number(m_uphillDistance, 'f', 2) + tr(" km");
  QString flatStr =
      tr("Flat: ") + QString::number(m_flatDistance, 'f', 2) + tr(" km");
  QString downhillStr = tr("Downhill: ") +
                        QString::number(m_downhillDistance, 'f', 2) + tr(" km");
  strGpsTerrain = uphillStr + " " + flatStr + " " + downhillStr;

  qDebug() << "Gps Terrain=" << strGpsTerrain;
}

void Steps::getTerrain() {
  if (oldLat != 0 || oldLon != 0) {
    double horizontalDistanceMeter =
        calculateHaversineDistance(oldLat, oldLon, latitude, longitude);
    double horizontalDistanceKm = horizontalDistanceMeter / 1000.0;

    // 优化1：增加距离有效性判断（避免极小值/负数）
    if (horizontalDistanceKm > EPS) {
      // 累加总距离
      m_distance += horizontalDistanceKm;

      // 优化2：计算海拔差并处理异常值（NaN/Inf）
      double altitudeDiff = altitude - oldAlt;
      // 兜底：若海拔差异常，归为平路
      if (std::isnan(altitudeDiff) || std::isinf(altitudeDiff)) {
        m_flatDistance += horizontalDistanceKm;
      }
      // 优化3：用容差判断，避免浮点数精度问题
      else if (altitudeDiff >
               UPHILL_THRESHOLD -
                   EPS) {  // 等价于 >= UPHILL_THRESHOLD（带容差）
        m_uphillDistance += horizontalDistanceKm;
      } else if (altitudeDiff <
                 DOWNHILL_THRESHOLD +
                     EPS) {  // 等价于 <= DOWNHILL_THRESHOLD（带容差）
        m_downhillDistance += horizontalDistanceKm;
      }
      // 优化4：else兜底，100%覆盖所有情况
      else {
        m_flatDistance += horizontalDistanceKm;
      }

      updateTerrainUI();
    }
  }
}

/**
 * @brief 计算两点GPS坐标的方位角（正北为0°，顺时针）
 * @param lat1 起点纬度（十进制度）
 * @param lon1 起点经度（十进制度）
 * @param lat2 终点纬度（十进制度）
 * @param lon2 终点经度（十进制度）
 * @return 方位角（0~360°），两点重合返回-1
 */
double Steps::calculateBearing(double lat1, double lon1, double lat2,
                               double lon2) {
  // 两点重合，无法计算方向
  if (qFuzzyCompare(lat1, lat2) && qFuzzyCompare(lon1, lon2)) {
    return -1.0;
  }

  // 转换为弧度
  const double radLat1 = lat1 * DEG_TO_RAD;
  const double radLon1 = lon1 * DEG_TO_RAD;
  const double radLat2 = lat2 * DEG_TO_RAD;
  const double radLon2 = lon2 * DEG_TO_RAD;

  // 球面几何方位角核心公式（WGS84坐标系适配）
  const double dLon = radLon2 - radLon1;
  const double y = sin(dLon) * cos(radLat2);
  const double x =
      cos(radLat1) * sin(radLat2) - sin(radLat1) * cos(radLat2) * cos(dLon);
  double bearingRad = atan2(y, x);
  double bearingDeg = bearingRad * RAD_TO_DEG;

  // 转换为0~360°范围
  bearingDeg = fmod(bearingDeg + 360.0, 360.0);
  return bearingDeg;
}

/**
 * @brief 将方位角转换为可读的方向描述（支持国际化）
 * @param bearing 方位角（0~360°）
 * @return 本地化的方向字符串（如"South by East 10 degrees"）
 */
QString Steps::bearingToDirection(double bearing) {
  // 1. 校验角度范围
  if (bearing < 0.0 || bearing > 360.0) {
    return tr("Invalid Direction");
  }

  // 规范化角度到[0, 360)
  bearing = fmod(bearing, 360.0);
  if (bearing < 0.0) bearing += 360.0;

  const double EPSILON = 0.001;

  // 2. 处理精准方向
  if (fabs(bearing) < EPSILON || fabs(bearing - 360.0) < EPSILON) {
    return tr("Due North");
  } else if (fabs(bearing - 90.0) < EPSILON) {
    return tr("Due East");
  } else if (fabs(bearing - 180.0) < EPSILON) {
    return tr("Due South");
  } else if (fabs(bearing - 270.0) < EPSILON) {
    return tr("Due West");
  }
  // 添加中间方向
  else if (fabs(bearing - 45.0) < EPSILON) {
    return tr("Northeast");
  } else if (fabs(bearing - 135.0) < EPSILON) {
    return tr("Southeast");
  } else if (fabs(bearing - 225.0) < EPSILON) {
    return tr("Southwest");
  } else if (fabs(bearing - 315.0) < EPSILON) {
    return tr("Northwest");
  }

  QString mainDir;
  double offset = 0.0;

  // 3. 象限划分
  if (bearing > 0 && bearing < 90) {  // 东北象限
    if (bearing <= 45) {
      mainDir = tr("North by East");
      offset = bearing;
    } else {
      mainDir = tr("East by North");
      offset = 90.0 - bearing;
    }
  } else if (bearing > 90 && bearing < 180) {  // 东南象限
    if (bearing <= 135) {
      mainDir = tr("East by South");
      offset = bearing - 90.0;
    } else {
      mainDir = tr("South by East");
      offset = 180.0 - bearing;
    }
  } else if (bearing > 180 && bearing < 270) {  // 西南象限
    if (bearing <= 225) {
      mainDir = tr("South by West");
      offset = bearing - 180.0;
    } else {
      mainDir = tr("West by South");
      offset = 270.0 - bearing;
    }
  } else {  // 西北象限 270~360
    if (bearing <= 315) {
      mainDir = tr("West by North");
      offset = bearing - 270.0;
    } else {
      mainDir = tr("North by West");
      offset = 360.0 - bearing;
    }
  }

  // 4. 格式化输出
  int roundedOffset = static_cast<int>(std::round(offset));

  // 处理偏移为0的情况（理论上不会发生，因为已处理了精准方向）
  if (roundedOffset == 0) {
    return mainDir;
  }

  return mainDir + " " + QString::number(roundedOffset) + " " + tr("degrees");
}
