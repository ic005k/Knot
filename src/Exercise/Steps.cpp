#include "Steps.h"

#include <QVector>
#include <cmath>

#include "src/MainWindow.h"
#include "src/defines.h"
#include "ui_MainWindow.h"
#include "ui_StepsOptions.h"

extern QStringList ymdList;

extern MainWindow* mw_one;
extern Ui::MainWindow* mui;
extern Method* m_Method;
extern QRegularExpression regxNumber;
extern QList<float> rlistX, rlistY, rlistZ, glistX, glistY, glistZ;
extern unsigned int num_steps_walk, num_steps_run, num_steps_hop;

extern void setTableNoItemFlags(QTableWidget* t, int row);

struct GPSCoordinate {
  double latitude;
  double longitude;
};
QVector<GPSCoordinate> gaussianFilter(const QVector<GPSCoordinate>& rawData,
                                      int windowSize, double sigma);
QVector<GPSCoordinate> detectAndCorrectOutliers(
    const QVector<GPSCoordinate>& data, double threshold);

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
  mui->lblTitle1->setFont(font0);
  mui->lblTitle2->setFont(font0);
  mui->lblTitle3->setFont(font0);
  mui->lblTitle4->setFont(font0);

  QFont font1 = m_Method->getNewFont(17);
  font1.setBold(true);
  mui->lblKM->setFont(font1);
  mui->lblSingle->setFont(font1);

  lblStyle = mui->lblCurrentDistance->styleSheet();
  mui->lblCurrentDistance->setStyleSheet(lblStyle);
  mui->lblRunTime->setStyleSheet(lblStyle);
  mui->lblAverageSpeed->setStyleSheet(lblStyle);
  mui->lblGpsInfo->setStyleSheet(lblStyle);

  mui->lblYearTotal->setStyleSheet(mui->lblMonthTotal->styleSheet());
  mui->btnGetGpsListData->hide();
  mui->qwSpeed->setFixedHeight(90);
  mui->qwSpeed->hide();

  timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &Steps::updateGetGps);

  QDir gpsdir;
  QString gpspath = iniDir + "/memo/gps/";
  if (!gpsdir.exists(gpspath)) gpsdir.mkpath(gpspath);

  getHardStepSensor();

  m_speedometer = new Speedometer(this);
  mui->f_speed->setFixedHeight(130);
  m_speedometer->setMaxSpeed(10.00);  // 最高时速(km/h)
  m_speedometer->setMinSpeed(0);      // 最低时速(km/h)
  m_speedometer->setCurrentSpeed(0.0);
  m_speedometer->setBackgroundColor(QColor(30, 30, 30));  // 背景色
  mui->f_speed->layout()->setSpacing(0);
  mui->f_speed->layout()->setContentsMargins(0, 0, 0, 0);
  mui->f_speed->layout()->addWidget(m_speedometer);

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
}

Steps::~Steps() {}

void Steps::keyReleaseEvent(QKeyEvent* event) { Q_UNUSED(event) }

bool Steps::eventFilter(QObject* watch, QEvent* evn) {
  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
      on_btnBack_clicked();
      return true;
    }
  }

  return QWidget::eventFilter(watch, evn);
}

void Steps::on_btnBack_clicked() {
  saveSteps();
  saveMovementType();
  mui->frameSteps->hide();
  mui->frameMain->show();
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
  if (getCount() > maxCount) {
    m_Method->delItemFromQW(mui->qwSteps, 0);
  }
  int count = getCount();

  QStringList listDate, listKm;
  QList<int> listSteps;
  for (int i = 0; i < count; i++) {
    listDate.append(getDate(i));
    listSteps.append(getSteps(i));
    listKm.append(getKM(i));
  }

  QString strLength =
      mw_one->m_StepsOptions->ui->editStepLength->text().trimmed();
  QString strThreshold =
      mw_one->m_StepsOptions->ui->editStepsThreshold->text().trimmed();

  QFuture<void> future = QtConcurrent::run([=]() {
    writeStepsToJson(iniDir, count, listDate, listSteps, listKm, strLength,
                     strThreshold);
  });

  // 使用 QFutureWatcher 监控进度
  QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
  connect(watcher, &QFutureWatcher<void>::finished, this, [=]() {
    qDebug() << "Steps save completed.";

    watcher->deleteLater();
  });
  watcher->setFuture(future);
}

// 转换后的JSON写入函数
void Steps::writeStepsToJson(const QString& iniDir, int count,
                             const QList<QString>& listDate,
                             const QList<int>& listSteps,
                             const QList<QString>& listKm,
                             const QString& strLength,
                             const QString& strThreshold) {
  // 构建Steps节点（对应原来的/Steps节点）
  QJsonObject stepsObj;

  stepsObj["Length"] = strLength;
  stepsObj["Threshold"] = strThreshold;

  // 设置计数（对应原来的/Steps/Count）
  if (count > 0) {
    stepsObj["Count"] = count;
  }

  // 构建Table数组（对应原来的/Steps/Table-i-*）
  QJsonArray tableArray;
  for (int i = 0; i < count; i++) {
    // 每个元素是一个数组，对应[日期, 步数, 公里数]
    QJsonArray rowArray;
    rowArray.append(listDate.at(i));   // 对应Table-i-0
    rowArray.append(listSteps.at(i));  // 对应Table-i-1
    rowArray.append(listKm.at(i));     // 对应Table-i-2
    tableArray.append(rowArray);
  }
  stepsObj["Table"] = tableArray;  // 将数组存入Steps节点

  // 将Steps节点加入根对象
  rootObj["Steps"] = stepsObj;

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
}

void Steps::loadStepsToTable() {
  clearAll();

  // 获取Steps对象（对应原INI的/Steps节点）
  QJsonObject stepsObj = rootObj["Steps"].toObject();

  // 读取步长和阈值（对应原/Steps/Length和/Steps/Threshold）
  QString stepLength = stepsObj["Length"].toString("35");
  QString stepsThreshold = stepsObj["Threshold"].toString("10000");

  mw_one->m_StepsOptions->ui->editStepLength->setText(stepLength);
  mw_one->m_StepsOptions->ui->editStepsThreshold->setText(stepsThreshold);

  // 设置上下文属性
  mui->qwSteps->rootContext()->setContextProperty("nStepsThreshold",
                                                  stepsThreshold.toInt());

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
      double km = stepLength.trimmed().toDouble() * steps / 100 / 1000;
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
      mw_one->m_StepsOptions->ui->editStepLength->text().trimmed().toDouble() *
      mui->lblSingle->text().toInt() / 100 / 1000;
  QString km = QString("%1").arg(d_km, 0, 'f', 2) + "  " + tr("KM");
  mui->lblKM->setText(km);

  if (mui->lblGpsInfo->text() == tr("GPS Info")) {
    QSettings Reg(iniDir + "gpslist.ini", QSettings::IniFormat);

    double m_td = Reg.value("/GPS/TotalDistance", 0).toDouble();
    mui->lblTotalDistance->setText(QString::number(m_td) + " km");
  }

  if (getGpsListCount() == 0) {
    int nYear = QDate::currentDate().year();
    int nMonth = QDate::currentDate().month();
    loadGpsList(nYear, nMonth);
    allGpsTotal();
  }

  isNeedRestoreUI = false;
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
  double km =
      mw_one->m_StepsOptions->ui->editStepLength->text().trimmed().toDouble() *
      steps / 100 / 1000;

  QFuture<void> future = QtConcurrent::run([=]() {
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

    QString date, c_date, full_date;
    full_date = getFullDate();
    c_date = getCurrentDate();
    if (count > 0) {
      // 从已获取的stepsObj中读取Table数组（对应原INI的/Steps/Table-*）
      QJsonArray tableArray = stepsObj["Table"].toArray();

      // 读取倒数第一个Table元素的0索引值（对应原/Steps/Table-(count-1)-0）
      // 先判断索引是否有效，避免越界
      int index = count - 1;
      if (index >= 0 && index < tableArray.size()) {
        QJsonValue tableItem = tableArray.at(index);
        if (tableItem.isArray()) {
          // 取数组第0个元素作为日期，默认空字符串
          date = tableItem.toArray().at(0).toString("");
        } else {
          date = "";  // 格式异常时用默认值
        }
      } else {
        date = "";  // 索引无效时用默认值
      }

      QString mdate = "";
      if (!date.isEmpty()) {  // 只有date非空时，才拆分日期
        mdate = date.split(" ").at(0).trimmed();
      }

      if (mdate == c_date) {
        QString strKM = QString("%1").arg(km, 0, 'f', 2);

        // 计算目标索引（对应原count-1）
        int index = count - 1;

        // 确保索引有效（在数组范围内）
        if (index >= 0 && index < tableArray.size()) {
          // 获取当前索引对应的数组项（[日期, 步数, 公里数]）
          QJsonArray rowArray = tableArray.at(index).toArray();

          // 更新数组中的三个值（对应原Table-(count-1)-0/1/2）
          rowArray[0] = date;           // 对应-0（日期）
          rowArray[1] = (double)steps;  // 对应-1（步数）
          rowArray[2] = strKM;          // 对应-2（公里数）

          // 将更新后的行放回数组
          tableArray[index] = rowArray;

          // 同步更新stepsObj中的Table数组
          stepsObj["Table"] = tableArray;
        }

      } else {
        count++;

        // 新建一行数据
        QJsonArray newRow;
        newRow.append(full_date);
        newRow.append(0);
        newRow.append("0");
        tableArray.append(newRow);  // 新增到数组
        stepsObj["Table"] = tableArray;
        stepsObj["Count"] = count;
      }
    } else {              // count==0
      count = count + 1;  // 从0变成1，代表要新增第一条记录
      QJsonArray tableArray = stepsObj["Table"].toArray();

      // 1. 创建第一条记录的数组（日期、步数、公里数）
      QJsonArray newRow;
      newRow.append(full_date);  // 完整日期
      newRow.append(0);          // 初始步数0
      newRow.append("0");        // 初始公里数"0"

      // 2. 用append新增到数组末尾（关键！空数组会变成size=1）
      tableArray.append(newRow);

      // 3. 同步更新stepsObj和count
      stepsObj["Table"] = tableArray;
      stepsObj["Count"] = count;  // 此时count=1，与tableArray.size()=1一致
    }

    rootObj["Steps"] = stepsObj;
  });

  // 使用 QFutureWatcher 监控进度
  QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
  connect(watcher, &QFutureWatcher<void>::finished, this, [=]() {
    qDebug() << "Set steps table completed.";
    loadStepsToTable();

    watcher->deleteLater();
  });
  watcher->setFuture(future);
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
      mw_one->m_StepsOptions->ui->editStepLength->text().trimmed().toDouble() *
      steps / 100 / 1000;
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

  if (m_activity.isValid()) {
    if (m_activity.callMethod<jdouble>("startGpsUpdates", "()D") == 0) {
      qWarning() << "LocationManager is null";
      mui->lblGpsInfo->setText("LocationManager is null...");
      mui->btnGPS->setText(tr("Start"));
      return;
    }
  }

#else
  if (m_positionSource) {
    m_positionSource->startUpdates();
  }
#endif

  clearTrack();

  m_time.setHMS(0, 0, 0, 0);

  strStartTime = QTime::currentTime().toString();

  if (isZH_CN) {
    QLocale chineseLocale(QLocale::Chinese, QLocale::China);
    t0 = chineseLocale.toString(QDate::currentDate(), "ddd MM月 dd yyyy");
  } else
    t0 = QDate::currentDate().toString();

  mui->lblGpsDateTime->setText(t0 + " " + strStartTime);
  startDT = QDateTime::currentDateTime();

  QString ss0 = t0;
  QString s0 = ss0.replace(" ", "");
  QString sst = strStartTime;
  QString s1 = sst.replace(":", "");
  QString csvPath =
      iniDir + "/memo/gps/" + getCurrentYear() + "/" + getCurrentMonth() + "/";
  if (!QDir(csvPath).exists()) {
    QDir().mkpath(csvPath);
  }
  strCSVFile = csvPath + s0 + "-gps-" + s1 + ".csv";

  timer->start(1000);
  m_distance = 0;
  m_speed = 0;
  emit distanceChanged(m_distance);
  emit timeChanged();

  mui->btnGPS->setText(tr("Stop"));
  mui->tabMotion->setCurrentIndex(1);

  mui->lblRunTime->setStyleSheet(lblStartStyle);
  mui->lblAverageSpeed->setStyleSheet(lblStartStyle);
  mui->lblCurrentDistance->setStyleSheet(lblStartStyle);
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

  // 获取当前运动距离
  jdouble distance;

  distance = m_activity.callMethod<jdouble>("getTotalDistance", "()D");

  QString str_distance = QString::number(distance, 'f', 2);
  m_distance = str_distance.toDouble();

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
    strGpsStatus = jstrGpsStatus.toString();
    QStringList list = strGpsStatus.split("\n");
    if (list.count() > 2) {
      str1 = list.at(0);
      str2 = list.at(1);
      str3 = list.at(2);
      mui->lblCurrentDistance->setText(str1);
      mui->lblRunTime->setText(str2);
      mui->lblAverageSpeed->setText(str3);

      setCurrentGpsSpeed(mySpeed, maxSpeed);

      str4 = list.at(3);
      str5 = list.at(4);
      str6 = list.at(5);
      str7 = list.at(6);
      strGpsStatus = str4 + "\n" + str5 + "\n" + str6 + "\n" + str7;
    }

    if (m_time.second() % 3 == 0) {
      if (!isGpsTest) {
        jdouble m_speed;

        m_speed = m_activity.callMethod<jdouble>("getMySpeed", "()D");
        maxSpeed = m_activity.callMethod<jdouble>("getMaxSpeed", "()D");

        mySpeed = m_speed;
        if (m_distance > 0 & mySpeed > 0) {
          if (latitude + longitude != oldLat + oldLon) {
            appendTrack(latitude, longitude);

            QStringList data_list;
            data_list.append(QString::number(latitude, 'f', 6));
            data_list.append(QString::number(longitude, 'f', 6));
            appendToCSV(strCSVFile, data_list);

            oldLat = latitude;
            oldLon = longitude;
          }
        }
      } else {
        appendTrack(latitude, longitude);
        latitude = latitude + 0.001;
        longitude = longitude + 0.001;

        QStringList data_list;
        data_list.append(QString::number(latitude, 'f', 6));
        data_list.append(QString::number(longitude, 'f', 6));
        appendToCSV(strCSVFile, data_list);
      }

      updateInfoText(str1, str3);
    }
  }

#else
  if (m_time.second() != 0) {
    m_speed = m_distance / m_time.second();
    emit speedChanged();
  }

  if (isGpsTest) {
    if (m_time.second() % 3 == 0) {
      appendTrack(latitude, longitude);

      quint64 randomInt =
          QRandomGenerator::global()->generate64() % 10000000000ULL;
      double randomDouble = static_cast<double>(randomInt) / 10000000000000.0;
      latitude = latitude + randomDouble;
      longitude = longitude + randomDouble;

      QStringList data_list;
      data_list.append(QString::number(latitude, 'f', 6));
      data_list.append(QString::number(longitude, 'f', 6));
      appendToCSV(strCSVFile, data_list);

      qDebug() << "m_time%3=" << m_time.second();
    }

    qDebug() << "m_time=" << m_time.second() << totalSeconds;
  }

#endif

  strTotalDistance = QString::number(m_TotalDistance) + " km";
  mui->lblTotalDistance->setText(strTotalDistance);

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

  strGpsInfoShow = strDurationTime +
                   "\nLon.-Lat.: " + QString::number(longitude) + " - " +
                   QString::number(latitude) + "\n" + strGpsStatus;
  mui->lblGpsInfo->setText(strGpsInfoShow);
  emit timeChanged();

  if (m_time.second() % 5 == 0) {
    refreshMotionData();
  }

  if (m_distance > 0 || isGpsTest) {
    // 先校验经纬度是否在有效范围（-90~90纬度，-180~180经度）
    bool latValid = (latitude >= -90 && latitude <= 90);
    bool lonValid = (longitude >= -180 && longitude <= 180);
    if (!latValid || !lonValid) {
      // 无效坐标，不请求

    } else {
      if (strCurrentTemp == "") {
        if (m_time.second() % 5 == 0) {
          weatherFetcher->fetchWeather(latitude, longitude);
        }
      } else {
        if (totalSeconds % 1800 == 0) {
          weatherFetcher->fetchWeather(latitude, longitude);
        }
      }
    }
  }
}

void Steps::stopRecordMotion() {
  timer->stop();

  mui->lblGpsInfo->setText(strGpsInfoShow);

  mui->lblRunTime->setStyleSheet(lblStyle);
  mui->lblAverageSpeed->setStyleSheet(lblStyle);
  mui->lblCurrentDistance->setStyleSheet(lblStyle);
  mui->lblGpsInfo->setStyleSheet(lblStyle);
  mui->btnGPS->setStyleSheet(btnRoundStyle);

  refreshMotionData();

#ifdef Q_OS_ANDROID

  m_distance = m_activity.callMethod<jdouble>("stopGpsUpdates", "()D");

#else
  if (m_positionSource) {
    m_positionSource->stopUpdates();
  }
  delete m_positionSource;
#endif

  mui->gboxMotionType->setEnabled(true);
  mui->btnSelGpsDate->setEnabled(true);
  strCurrentTemp = "";
}

void Steps::refreshTotalDistance() {
  m_TotalDistance = oldTotalDistance + m_distance;
  strTotalDistance = QString::number(m_TotalDistance) + " km";
  mui->lblTotalDistance->setText(strTotalDistance);
  QSettings Reg(iniDir + "gpslist.ini", QSettings::IniFormat);
  Reg.setValue("/GPS/TotalDistance", m_TotalDistance);
}

void Steps::refreshMotionData() {
  refreshTotalDistance();

  strEndTime = QTime::currentTime().toString();

  QString t00, t1, t2, t3, t4, t5, str_type;

  if (mui->rbCycling->isChecked()) str_type = tr("Cycling");
  if (mui->rbHiking->isChecked()) str_type = tr("Hiking");
  if (mui->rbRunning->isChecked()) str_type = tr("Running");
  t00 = str_type + " " + t0;

  t1 = tr("Time") + ": " + strStartTime + " - " + strEndTime + strCurrentTemp;
  t2 = tr("Distance") + ": " + str1;
  t3 = tr("Exercise Duration") + ": " + str2;
  t4 = tr("Average Speed") + ": " + str3;
  t5 = str6;

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
    text0 = m_Method->getText0(mui->qwGpsList, 0);
    text1 = m_Method->getText1(mui->qwGpsList, 0);
    startTime1 = text1.split("-").at(0);
    startTime2 = t1.split("-").at(0);

    if (text0 == t00 && startTime1 == startTime2) {
      m_Method->delItemFromQW(mui->qwGpsList, 0);
    }

    insertGpsList(0, t00, t1, t2, t3, t4, t5, strCurrentWeatherIcon);

    QSettings Reg1(iniDir + stry + "-gpslist.ini", QSettings::IniFormat);

    int count = getGpsListCount();
    QString strYearMonth = stry + "-" + strm;
    Reg1.setValue("/" + strYearMonth + "/Count", count);
    Reg1.setValue("/" + strYearMonth + "/" + QString::number(count),
                  t00 + "-=-" + t1 + "-=-" + t2 + "-=-" + t3 + "-=-" + t4 +
                      "-=-" + t5 + "-=-" + strCurrentWeatherIcon);

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
                          QString t3, QString t4, QString t5, QString t6) {
  QQuickItem* root = mui->qwGpsList->rootObject();
  QMetaObject::invokeMethod(
      (QObject*)root, "insertItem", Q_ARG(QVariant, curIndex),
      Q_ARG(QVariant, t0), Q_ARG(QVariant, t1), Q_ARG(QVariant, t2),
      Q_ARG(QVariant, t3), Q_ARG(QVariant, t4), Q_ARG(QVariant, t5),
      Q_ARG(QVariant, t6), Q_ARG(QVariant, 0));
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
    QString t0, t1, t2, t3, t4, t5, t6;
    if (list.count() > 0) {
      t0 = list.at(0);
      t1 = list.at(1);
      t2 = list.at(2);
      t3 = list.at(3);
      t4 = list.at(4);
      t5 = list.at(5);
      if (list.count() == 7) t6 = list.at(6);
    }

    insertGpsList(0, t0, t1, t2, t3, t4, t5, t6);
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
    ycount += mcount;

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
  s1_year = stry + " " + tr("Year") + ": \n" + QString::number(yt) + " km  " +
            QString::number(ycount) + "\n";
  s2_year = tr("Cycling") + ": " + QString::number(yearCyclingKM) + " km  " +
            QString::number(yearCyclingCount) + "\n";
  s3_year = tr("Hiking") + ": " + QString::number(yearHikingKM) + " km  " +
            QString::number(yearHikingCount) + "\n";
  s4_year = tr("Running") + ": " + QString::number(yearRunningKM) + " km  " +
            QString::number(yearRunningCount);

  mui->lblMonthTotal->setText(s1_month + s2_month + s3_month + s4_month);
  mui->lblYearTotal->setText(s1_year + s2_year + s3_year + s4_year);
}

void Steps::appendTrack(double lat, double lon) {
  QQuickItem* root = mui->qwMap->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "appendTrack", Q_ARG(QVariant, lat),
                            Q_ARG(QVariant, lon));
}

void Steps::updateInfoText(QString strDistance, QString strSpeed) {
  QQuickItem* root = mui->qwMap->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "updateInfoText",
                            Q_ARG(QVariant, strDistance),
                            Q_ARG(QVariant, strSpeed));
}

void Steps::updateTrackData(double lat, double lon) {
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
  if (timer->isActive()) return;

  mw_one->showProgress();
  QQuickItem* root = mui->qwGpsList->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject*)root, "getGpsList",
                            Q_RETURN_ARG(QVariant, item));
  strGpsList = item.toString();
  mw_one->myUpdateGpsMapThread->start();
}

void Steps::updateGpsTrack() {
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

  QString csvPath = iniDir + "/memo/gps/" + str_year + "/" + str_month + "/";
  QString gpsFile = iniDir + "memo/gps/" + st1 + "-gps-" + st2 + ".csv";
  QString gpsOptimizedFile =
      iniDir + "memo/gps/" + st1 + "-gps-" + st2 + "-opt.csv";

  if (!QFile::exists(gpsOptimizedFile)) {
    gpsFile = csvPath + st1 + "-gps-" + st2 + ".csv";
    gpsOptimizedFile = csvPath + st1 + "-gps-" + st2 + "-opt.csv";
  }

  double lat = 0;
  double lon = 0;
  QString strLat, strLon;
  if (QFile::exists(gpsFile)) {
    QVector<GPSCoordinate> rawGPSData;

    clearTrack();

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

      rawGPSData.append({lat, lon});
    }

    file.close();

    int gaussianWindowSize = 3;
    double gaussianSigma = 1.0;
    double outlierThreshold = 0.001;  // 距离阈值，可根据实际情况调整

    // 应用高斯滤波
    QVector<GPSCoordinate> filteredData =
        gaussianFilter(rawGPSData, gaussianWindowSize, gaussianSigma);

    // 检测并修正异常点
    QVector<GPSCoordinate> optimizedData =
        detectAndCorrectOutliers(filteredData, outlierThreshold);

    for (int i = 0; i < optimizedData.count(); i++) {
      lat = optimizedData.at(i).latitude;
      lon = optimizedData.at(i).longitude;
      updateTrackData(lat, lon);

      QStringList m_data;
      m_data.append(QString::number(lat, 'f', 6));
      m_data.append(QString::number(lon, 'f', 6));
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
      updateTrackData(lat, lon);
    }

    file1.close();

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
    updateMapTrackUi(lastLat, lastLon);
    mui->lblGpsDateTime->setText(strGpsMapDateTime);

    updateInfoText(strGpsMapDistnce, strGpsMapSpeed);
    mui->tabMotion->setCurrentIndex(3);
  }
}

// 高斯滤波函数
QVector<GPSCoordinate> gaussianFilter(const QVector<GPSCoordinate>& rawData,
                                      int windowSize, double sigma) {
  QVector<GPSCoordinate> filteredData;
  int dataSize = rawData.size();

  for (int i = 0; i < dataSize; ++i) {
    double sumLatitude = 0.0;
    double sumLongitude = 0.0;
    double sumWeight = 0.0;

    for (int j = qMax(0, i - windowSize / 2);
         j < qMin(dataSize, i + windowSize / 2 + 1); ++j) {
      double distance = std::abs(i - j);
      double weight = std::exp(-(distance * distance) / (2 * sigma * sigma));
      sumLatitude += rawData[j].latitude * weight;
      sumLongitude += rawData[j].longitude * weight;
      sumWeight += weight;
    }

    GPSCoordinate filteredCoord;
    filteredCoord.latitude = sumLatitude / sumWeight;
    filteredCoord.longitude = sumLongitude / sumWeight;

    filteredData.append(filteredCoord);
  }

  return filteredData;
}

// 计算两点之间的距离（简化的平面距离计算，实际应用中可能需要更精确的地球表面距离计算）
double calculateDistance(const GPSCoordinate& p1, const GPSCoordinate& p2) {
  double dx = p2.longitude - p1.longitude;
  double dy = p2.latitude - p1.latitude;
  return std::sqrt(dx * dx + dy * dy);
}

// 异常点检测与修正函数
QVector<GPSCoordinate> detectAndCorrectOutliers(
    const QVector<GPSCoordinate>& data, double threshold) {
  QVector<GPSCoordinate> correctedData = data;
  int dataSize = data.size();

  for (int i = 1; i < dataSize - 1; ++i) {
    double dist1 = calculateDistance(data[i - 1], data[i]);
    double dist2 = calculateDistance(data[i], data[i + 1]);

    if (dist1 > threshold || dist2 > threshold) {
      // 进行线性插值修正
      double newLat = (data[i - 1].latitude + data[i + 1].latitude) / 2;
      double newLon = (data[i - 1].longitude + data[i + 1].longitude) / 2;
      correctedData[i].latitude = newLat;
      correctedData[i].longitude = newLon;
    }
  }

  return correctedData;
}

void Steps::saveMovementType() {
  bool b1 = mui->rbCycling->isChecked();
  bool b2 = mui->rbHiking->isChecked();
  bool b3 = mui->rbRunning->isChecked();

  QFuture<void> future = QtConcurrent::run([=]() {
    QSettings Reg(iniDir + "gpslist.ini", QSettings::IniFormat);
    Reg.setValue("/GPS/isCycling", b1);
    Reg.setValue("/GPS/isHiking", b2);
    Reg.setValue("/GPS/isRunning", b3);
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

  QFuture<void> future = QtConcurrent::run([=]() {
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

    // 5. 获取当前日期（与原逻辑一致）
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
  });

  // 使用 QFutureWatcher 监控进度
  QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
  connect(watcher, &QFutureWatcher<void>::finished, this, [=]() {
    qDebug() << "InitTodaySteps completed.";

    watcher->deleteLater();
  });
  watcher->setFuture(future);
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
  double sl = mw_one->m_StepsOptions->ui->editStepLength->text().toDouble();
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
  return;

  QObject* rootObject = mui->qwSpeed->rootObject();
  if (rootObject) {
    rootObject->setProperty("currentSpeed",
                            QString::number(speed, 'f', 2).toDouble());
    rootObject->setProperty("myMaxSpeed", maxSpeed);
  }
}

QString Steps::getCurrentYear() {
  return QString::number(QDate::currentDate().year());
}

QString Steps::getCurrentMonth() {
  return QString::number(QDate::currentDate().month());
}
