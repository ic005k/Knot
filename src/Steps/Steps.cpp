#include "Steps.h"

#include <QVector>
#include <cmath>

#include "src/MainWindow.h"
#include "ui_MainWindow.h"
#include "ui_StepsOptions.h"

extern QStringList ymdList;

extern MainWindow* mw_one;
extern Method* m_Method;
extern QRegularExpression regxNumber;
extern QList<float> rlistX, rlistY, rlistZ, glistX, glistY, glistZ;
extern unsigned int num_steps_walk, num_steps_run, num_steps_hop;
extern bool loading, isAndroid, zh_cn;
extern QString iniFile, iniDir, strDate;
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
QJniObject listenerWrapper;
QJniObject m_activity;
#endif

Steps::Steps(QWidget* parent) : QDialog(parent) {
  this->installEventFilter(this);

  mw_one->ui->lblSingle->adjustSize();
  QString date = QString::number(QDate::currentDate().month()) + "-" +
                 QString::number(QDate::currentDate().day());
  mw_one->ui->lblCurrent->setText(date + " " + QTime::currentTime().toString());

  QFont font0 = m_Method->getNewFont(15);
  mw_one->ui->lblSteps->setFont(font0);

  font0.setPointSize(13);
  mw_one->ui->lblGpsDateTime->setFont(font0);
  font0.setBold(true);
  mw_one->ui->lblCurrent->setFont(font0);
  mw_one->ui->lblToNow->setFont(font0);
  mw_one->ui->lblNow->setFont(font0);
  mw_one->ui->lblTitle1->setFont(font0);
  mw_one->ui->lblTitle2->setFont(font0);
  mw_one->ui->lblTitle3->setFont(font0);
  mw_one->ui->lblTitle4->setFont(font0);

  QFont font1 = m_Method->getNewFont(17);
  font1.setBold(true);
  mw_one->ui->lblKM->setFont(font1);
  mw_one->ui->lblSingle->setFont(font1);

  lblStyle = mw_one->ui->lblCurrentDistance->styleSheet();
  mw_one->ui->lblCurrentDistance->setStyleSheet(lblStyle);
  mw_one->ui->lblRunTime->setStyleSheet(lblStyle);
  mw_one->ui->lblAverageSpeed->setStyleSheet(lblStyle);
  mw_one->ui->lblGpsInfo->setStyleSheet(lblStyle);

  mw_one->ui->lblYearTotal->setStyleSheet(
      mw_one->ui->lblMonthTotal->styleSheet());
  mw_one->ui->btnGetGpsListData->hide();
  mw_one->ui->qwSpeed->setFixedHeight(90);

  timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &Steps::updateGetGps);

  QDir gpsdir;
  QString gpspath = iniDir + "/memo/gps/";
  if (!gpsdir.exists(gpspath)) gpsdir.mkpath(gpspath);

  getHardStepSensor();
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
  mw_one->ui->frameSteps->hide();
  mw_one->ui->frameMain->show();
}

void Steps::on_btnReset_clicked() {
  CurrentSteps = 0;
  mw_one->ui->lblSingle->setText("0");

  if (isHardStepSensor == 1) resetSteps = getAndroidSteps();

  QString date = QString::number(QDate::currentDate().month()) + "-" +
                 QString::number(QDate::currentDate().day());
  mw_one->ui->lblCurrent->setText(date + " " + QTime::currentTime().toString());
  mw_one->ui->lblNow->setText(date + " " + QTime::currentTime().toString());
  mw_one->ui->lblKM->setText("0.00  " + tr("KM"));
}

void Steps::saveSteps() {
  QSettings Reg(iniDir + "steps.ini", QSettings::IniFormat);

  if (getCount() > maxCount) {
    delItem(0);
  }
  int count = getCount();
  if (count > 0) {
    Reg.setValue("/Steps/Count", count);
  }
  for (int i = 0; i < count; i++) {
    Reg.setValue("/Steps/Table-" + QString::number(i) + "-0", getDate(i));
    Reg.setValue("/Steps/Table-" + QString::number(i) + "-1", getSteps(i));
    Reg.setValue("/Steps/Table-" + QString::number(i) + "-2", getKM(i));
  }
}

void Steps::loadStepsToTable() {
  clearAll();

  QString ini_file;
  ini_file = iniDir + "steps.ini";
  QSettings Reg(ini_file, QSettings::IniFormat);

  mw_one->m_StepsOptions->ui->editStepLength->setText(
      Reg.value("/Steps/Length", "35").toString());
  mw_one->m_StepsOptions->ui->editStepsThreshold->setText(
      Reg.value("/Steps/Threshold", "10000").toString());

  mw_one->ui->qwSteps->rootContext()->setContextProperty(
      "nStepsThreshold",
      mw_one->m_StepsOptions->ui->editStepsThreshold->text().toInt());

  int count = Reg.value("/Steps/Count").toInt();
  int start = 0;
  if (count > maxCount) start = 1;

  for (int i = start; i < count; i++) {
    QString str0 =
        Reg.value("/Steps/Table-" + QString::number(i) + "-0", "").toString();
    qlonglong steps =
        Reg.value("/Steps/Table-" + QString::number(i) + "-1", 0).toLongLong();
    QString str2 =
        Reg.value("/Steps/Table-" + QString::number(i) + "-2", "").toString();
    if (str2 == "") {
      double km = mw_one->m_StepsOptions->ui->editStepLength->text()
                      .trimmed()
                      .toDouble() *
                  steps / 100 / 1000;
      str2 = QString("%1").arg(km, 0, 'f', 2);
    }

    if (str0 != "" && steps >= 0 && !str2.isNull())
      addRecord(str0, steps, str2);
  }
}

void Steps::openStepsUI() {
  mw_one->ui->frameMain->hide();
  mw_one->ui->frameSteps->show();

  updateHardSensorSteps();

  loadStepsToTable();

  m_Method->setCurrentIndexFromQW(mw_one->ui->qwSteps, getCount() - 1);
  m_Method->setScrollBarPos(mw_one->ui->qwSteps, 1.0);

  QString date = QString::number(QDate::currentDate().month()) + "-" +
                 QString::number(QDate::currentDate().day());
  mw_one->ui->lblNow->setText(date + " " + QTime::currentTime().toString());
  double d_km =
      mw_one->m_StepsOptions->ui->editStepLength->text().trimmed().toDouble() *
      mw_one->ui->lblSingle->text().toInt() / 100 / 1000;
  QString km = QString("%1").arg(d_km, 0, 'f', 2) + "  " + tr("KM");
  mw_one->ui->lblKM->setText(km);

  if (mw_one->ui->lblGpsInfo->text() == tr("GPS Info")) {
    QSettings Reg(iniDir + "gpslist.ini", QSettings::IniFormat);

    double m_td = Reg.value("/GPS/TotalDistance", 0).toDouble();
    mw_one->ui->lblTotalDistance->setText(QString::number(m_td) + " km");
  }

  if (getGpsListCount() == 0) {
    int nYear = QDate::currentDate().year();
    int nMonth = QDate::currentDate().month();
    loadGpsList(nYear, nMonth);
    allGpsTotal();
  }
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
  if (zh_cn) {
    QLocale chineseLocale(QLocale::Chinese, QLocale::China);
    date = chineseLocale.toString(QDate::currentDate(), "yyyy-M-d ddd");
  } else
    date = QDate::currentDate().toString("yyyy-M-d ddd");

  return date;
}

void Steps::setTableSteps(qlonglong steps) {
  QSettings Reg(iniDir + "steps.ini", QSettings::IniFormat);

  int count = Reg.value("/Steps/Count", 0).toInt();
  QString date, c_date, full_date;
  full_date = getFullDate();
  c_date = getCurrentDate();
  if (count > 0) {
    date = Reg.value("/Steps/Table-" + QString::number(count - 1) + "-0", "")
               .toString();
    QString mdate = date.split(" ").at(0).trimmed();

    if (mdate == c_date) {
      double km = mw_one->m_StepsOptions->ui->editStepLength->text()
                      .trimmed()
                      .toDouble() *
                  steps / 100 / 1000;
      QString strKM = QString("%1").arg(km, 0, 'f', 2);

      Reg.setValue("/Steps/Table-" + QString::number(count - 1) + "-0", date);
      Reg.setValue("/Steps/Table-" + QString::number(count - 1) + "-1", steps);
      Reg.setValue("/Steps/Table-" + QString::number(count - 1) + "-2", strKM);

    } else {
      count = count + 1;
      Reg.setValue("/Steps/Table-" + QString::number(count - 1) + "-0",
                   full_date);
      Reg.setValue("/Steps/Table-" + QString::number(count - 1) + "-1", 0);
      Reg.setValue("/Steps/Table-" + QString::number(count - 1) + "-2", "0");

      Reg.setValue("/Steps/Count", count);
    }
  } else {  // count==0
    count = count + 1;
    Reg.setValue("/Steps/Table-" + QString::number(count - 1) + "-0",
                 full_date);
    Reg.setValue("/Steps/Table-" + QString::number(count - 1) + "-1", 0);
    Reg.setValue("/Steps/Table-" + QString::number(count - 1) + "-2", "0");

    Reg.setValue("/Steps/Count", count);
  }
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

  QQuickItem* root = mw_one->ui->qwSteps->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "addItem", Q_ARG(QVariant, date),
                            Q_ARG(QVariant, strSteps), Q_ARG(QVariant, km),
                            Q_ARG(QVariant, strCalorie), Q_ARG(QVariant, 0));
}

int Steps::getCount() {
  QQuickItem* root = mw_one->ui->qwSteps->rootObject();
  QVariant itemCount;
  QMetaObject::invokeMethod((QObject*)root, "getItemCount",
                            Q_RETURN_ARG(QVariant, itemCount));
  return itemCount.toInt();
}

QString Steps::getDate(int row) {
  QQuickItem* root = mw_one->ui->qwSteps->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject*)root, "getText0",
                            Q_RETURN_ARG(QVariant, item), Q_ARG(QVariant, row));
  return item.toString();
}

int Steps::getSteps(int row) {
  QQuickItem* root = mw_one->ui->qwSteps->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject*)root, "getText1",
                            Q_RETURN_ARG(QVariant, item), Q_ARG(QVariant, row));
  return item.toInt();
}

QString Steps::getKM(int row) {
  QQuickItem* root = mw_one->ui->qwSteps->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject*)root, "getText2",
                            Q_RETURN_ARG(QVariant, item), Q_ARG(QVariant, row));
  return item.toString();
}

void Steps::delItem(int index) {
  QQuickItem* root = mw_one->ui->qwSteps->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "delItem", Q_ARG(QVariant, index));
}

void Steps::setTableData(int index, QString date, int steps, QString km) {
  QQuickItem* root = mw_one->ui->qwSteps->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "modifyItem",
                            Q_ARG(QVariant, index), Q_ARG(QVariant, date),
                            Q_ARG(QVariant, QString::number(steps)),
                            Q_ARG(QVariant, km));
}

void Steps::clearAll() {
  int count = getCount();
  for (int i = 0; i < count; i++) delItem(0);
}

void Steps::setScrollBarPos(double pos) {
  QQuickItem* root = mw_one->ui->qwSteps->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "setScrollBarPos",
                            Q_ARG(QVariant, pos));
}

void Steps::startRecordMotion() {
  QSettings Reg(iniDir + "gpslist.ini", QSettings::IniFormat);

  m_TotalDistance = Reg.value("/GPS/TotalDistance", 0).toDouble();

#ifdef Q_OS_ANDROID
#else
  isGpsTest = true;

  m_positionSource = QGeoPositionInfoSource::createDefaultSource(this);
  if (m_positionSource) {
    connect(m_positionSource, &QGeoPositionInfoSource::positionUpdated, this,
            &Steps::positionUpdated);
    m_positionSource->setUpdateInterval(2000);
  } else {
    mw_one->ui->lblGpsInfo->setText(tr("No GPS signal..."));
    mw_one->ui->btnGPS->setText(tr("Start"));
    return;
  }
#endif

#ifdef Q_OS_ANDROID

  m_activity = QJniObject(QNativeInterface::QAndroidApplication::context());

  if (m_activity.isValid()) {
    if (nGpsMethod == 1) {
      if (m_activity.callMethod<jdouble>("startGpsUpdates", "()D") == 0) {
        qWarning() << "LocationManager is null";
        mw_one->ui->lblGpsInfo->setText("LocationManager is null...");
        mw_one->ui->btnGPS->setText(tr("Start"));
        return;
      }
    }

    if (nGpsMethod == 2) {
      listenerWrapper = QJniObject("com/x/LocationListenerWrapper",
                                   "(Landroid/content/Context;)V",
                                   m_activity.object<jobject>());
      if (listenerWrapper.isValid()) {
        if (listenerWrapper.callMethod<jdouble>("startGpsUpdates", "()D") ==
            0) {
          qWarning() << "LocationManager is null";
          mw_one->ui->lblGpsInfo->setText("LocationManager is null...");
          mw_one->ui->btnGPS->setText(tr("Start"));
          return;
        }
      }
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

  if (zh_cn) {
    QLocale chineseLocale(QLocale::Chinese, QLocale::China);
    t0 = chineseLocale.toString(QDate::currentDate(), "ddd MM月 dd yyyy");
  } else
    t0 = QDate::currentDate().toString();

  mw_one->ui->lblGpsDateTime->setText(t0 + " " + strStartTime);
  startDT = QDateTime::currentDateTime();

  QString ss0 = t0;
  QString s0 = ss0.replace(" ", "");
  QString sst = strStartTime;
  QString s1 = sst.replace(":", "");
  strCSVFile = iniDir + "/memo/gps/" + s0 + "-gps-" + s1 + ".csv";

  timer->start(1000);
  m_distance = 0;
  m_speed = 0;
  emit distanceChanged(m_distance);
  emit timeChanged();

  mw_one->ui->btnGPS->setText(tr("Stop"));
  mw_one->ui->tabMotion->setCurrentIndex(1);

  mw_one->ui->lblRunTime->setStyleSheet(lblStartStyle);
  mw_one->ui->lblAverageSpeed->setStyleSheet(lblStartStyle);
  mw_one->ui->lblCurrentDistance->setStyleSheet(lblStartStyle);
  mw_one->ui->lblGpsInfo->setStyleSheet(lblStartStyle);
  mw_one->ui->btnGPS->setStyleSheet(btnRoundStyleRed);
  mw_one->ui->gboxMotionType->setEnabled(false);
  mw_one->ui->btnSelGpsDate->setEnabled(false);
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

#ifdef Q_OS_ANDROID

  // 获取当前运动距离
  jdouble distance;
  if (nGpsMethod == 1)
    distance = m_activity.callMethod<jdouble>("getTotalDistance", "()D");
  else
    distance = listenerWrapper.callMethod<jdouble>("getTotalDistance", "()D");

  QString str_distance = QString::number(distance, 'f', 2);
  m_distance = str_distance.toDouble();

  if (!isGpsTest) {
    if (nGpsMethod == 1) {
      latitude = m_activity.callMethod<jdouble>("getLatitude", "()D");
      longitude = m_activity.callMethod<jdouble>("getLongitude", "()D");
    } else {
      latitude = listenerWrapper.callMethod<jdouble>("getLatitude", "()D");
      longitude = listenerWrapper.callMethod<jdouble>("getLongitude", "()D");
    }

    latitude = QString::number(latitude, 'f', 6).toDouble();
    longitude = QString::number(longitude, 'f', 6).toDouble();
  }

  QJniObject jstrGpsStatus;
  if (nGpsMethod == 1)
    jstrGpsStatus = m_activity.callObjectMethod<jstring>("getGpsStatus");
  else
    jstrGpsStatus = listenerWrapper.callObjectMethod<jstring>("getGpsStatus");

  if (jstrGpsStatus.isValid()) {
    strGpsStatus = jstrGpsStatus.toString();
    QStringList list = strGpsStatus.split("\n");
    if (list.count() > 2) {
      str1 = list.at(0);
      str2 = list.at(1);
      str3 = list.at(2);
      mw_one->ui->lblCurrentDistance->setText(str1);
      mw_one->ui->lblRunTime->setText(str2);
      mw_one->ui->lblAverageSpeed->setText(str3);

      setCurrentGpsSpeed(mySpeed);

      str4 = list.at(3);
      str5 = list.at(4);
      str6 = list.at(5);
      str7 = list.at(6);
      strGpsStatus = str4 + "\n" + str5 + "\n" + str6 + "\n" + str7;
    }

    if (m_time.second() % 3 == 0) {
      if (!isGpsTest) {
        jdouble m_speed;
        if (nGpsMethod == 1)
          m_speed = m_activity.callMethod<jdouble>("getMySpeed", "()D");
        else
          m_speed = listenerWrapper.callMethod<jdouble>("getMySpeed", "()D");

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

    qDebug() << "m_time=" << m_time.second();
  }

#endif

  strTotalDistance = QString::number(m_TotalDistance) + " km";
  mw_one->ui->lblTotalDistance->setText(strTotalDistance);

  // strDurationTime = tr("Duration") + " : " + m_time.toString("hh:mm:ss");

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
  mw_one->ui->lblGpsInfo->setText(strGpsInfoShow);
  emit timeChanged();

  if (m_time.second() % 5 == 0) {
    refreshMotionData();
  }
}

void Steps::stopRecordMotion() {
  timer->stop();

  m_TotalDistance = m_TotalDistance + m_distance;
  strTotalDistance = QString::number(m_TotalDistance) + " km";
  mw_one->ui->lblTotalDistance->setText(strTotalDistance);
  mw_one->ui->lblGpsInfo->setText(strGpsInfoShow);

  mw_one->ui->lblRunTime->setStyleSheet(lblStyle);
  mw_one->ui->lblAverageSpeed->setStyleSheet(lblStyle);
  mw_one->ui->lblCurrentDistance->setStyleSheet(lblStyle);
  mw_one->ui->lblGpsInfo->setStyleSheet(lblStyle);
  mw_one->ui->btnGPS->setStyleSheet(btnRoundStyle);

  refreshMotionData();

#ifdef Q_OS_ANDROID
  if (nGpsMethod == 1)
    m_distance = m_activity.callMethod<jdouble>("stopGpsUpdates", "()D");
  else
    m_distance = listenerWrapper.callMethod<jdouble>("stopGpsUpdates", "()D");
#else
  if (m_positionSource) {
    m_positionSource->stopUpdates();
  }
  delete m_positionSource;
#endif

  mw_one->ui->gboxMotionType->setEnabled(true);
  mw_one->ui->btnSelGpsDate->setEnabled(true);
}

void Steps::refreshMotionData() {
  QSettings Reg(iniDir + "gpslist.ini", QSettings::IniFormat);
  Reg.setValue("/GPS/TotalDistance", m_TotalDistance);

  strEndTime = QTime::currentTime().toString();

  QString t00, t1, t2, t3, t4, t5, str_type;

  if (mw_one->ui->rbCycling->isChecked()) str_type = tr("Cycling");
  if (mw_one->ui->rbHiking->isChecked()) str_type = tr("Hiking");
  if (mw_one->ui->rbRunning->isChecked()) str_type = tr("Running");
  t00 = str_type + " " + t0;

  t1 = tr("Time") + ": " + strStartTime + " - " + strEndTime;
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
    if (mw_one->ui->btnSelGpsDate->text() != strTitle) {
      clearAllGpsList();
      loadGpsList(nYear, nMonth);
      mw_one->ui->btnSelGpsDate->setText(strTitle);
    }

    QString text0, text1, startTime1, startTime2;
    text0 = m_Method->getText0(mw_one->ui->qwGpsList, 0);
    text1 = m_Method->getText1(mw_one->ui->qwGpsList, 0);
    startTime1 = text1.split("-").at(0);
    startTime2 = t1.split("-").at(0);

    if (text0 == t00 && startTime1 == startTime2) {
      m_Method->delItemFromQW(mw_one->ui->qwGpsList, 0);
    }

    insertGpsList(0, t00, t1, t2, t3, t4, t5);

    QSettings Reg1(iniDir + stry + "-gpslist.ini", QSettings::IniFormat);

    int count = getGpsListCount();
    QString strYearMonth = stry + "-" + strm;
    Reg1.setValue("/" + strYearMonth + "/Count", count);
    Reg1.setValue(
        "/" + strYearMonth + "/" + QString::number(count),
        t00 + "-=-" + t1 + "-=-" + t2 + "-=-" + t3 + "-=-" + t4 + "-=-" + t5);

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
                          QString t3, QString t4, QString t5) {
  QQuickItem* root = mw_one->ui->qwGpsList->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "insertItem",
                            Q_ARG(QVariant, curIndex), Q_ARG(QVariant, t0),
                            Q_ARG(QVariant, t1), Q_ARG(QVariant, t2),
                            Q_ARG(QVariant, t3), Q_ARG(QVariant, t4),
                            Q_ARG(QVariant, t5), Q_ARG(QVariant, 0));
}

int Steps::getGpsListCount() {
  QQuickItem* root = mw_one->ui->qwGpsList->rootObject();
  QVariant itemCount;
  QMetaObject::invokeMethod((QObject*)root, "getItemCount",
                            Q_RETURN_ARG(QVariant, itemCount));
  return itemCount.toInt();
}

void Steps::delGpsListItem(int index) {
  QQuickItem* root = mw_one->ui->qwGpsList->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "delItem", Q_ARG(QVariant, index));
}

void Steps::clearAllGpsList() {
  int count = getGpsListCount();
  for (int i = 0; i < count; i++) {
    delGpsListItem(0);
  }
}

void Steps::loadGpsList(int nYear, int nMonth) {
  mw_one->ui->btnSelGpsDate->setText(QString::number(nYear) + " - " +
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
    QString t0, t1, t2, t3, t4, t5;
    if (list.count() > 0) {
      t0 = list.at(0);
      t1 = list.at(1);
      t2 = list.at(2);
      t3 = list.at(3);
      t4 = list.at(4);
      t5 = list.at(5);
    }

    insertGpsList(0, t0, t1, t2, t3, t4, t5);
  }
}

void Steps::selGpsListYearMonth() {
  // if (isAndroid) {
  // int y, m;
  // QString str = mw_one->ui->btnSelGpsDate->text();
  //  QStringList list = str.split("-");
  //  y = list.at(0).toInt();
  //  m = list.at(1).toInt();
  //  m_Method->setDateTimePickerFlag("ym", y, m, 0, 0, 0, "gpslist");
  //  m_Method->openDateTimePicker();
  //  return;
  //}

  QStringList list = mw_one->ui->btnSelGpsDate->text().split("-");
  int y = 2025;
  int m = 2;
  if (list.count() == 2) {
    y = list.at(0).toInt();
    m = list.at(1).toInt();
  }
  QDate date(y, m, 1);
  mw_one->m_Report->m_DateSelector->m_datePickerYM->setDate(date);

  mw_one->m_Report->m_DateSelector->dateFlag = 1;
  mw_one->m_Report->m_DateSelector->init();
}

void Steps::getGpsListDataFromYearMonth() {
  clearAllGpsList();

  QStringList list;
  // list = m_Method->getDateTimePickerValue();
  list = ymdList;
  int y = list.at(0).toInt();
  int m = list.at(1).toInt();

  loadGpsList(y, m);
  allGpsTotal();
}

QString Steps::getGpsListText0(int index) {
  QQuickItem* root = mw_one->ui->qwGpsList->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject*)root, "getText0",
                            Q_RETURN_ARG(QVariant, item),
                            Q_ARG(QVariant, index));
  return item.toString();
}

QString Steps::getGpsListText2(int index) {
  QQuickItem* root = mw_one->ui->qwGpsList->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject*)root, "getText2",
                            Q_RETURN_ARG(QVariant, item),
                            Q_ARG(QVariant, index));
  return item.toString();
}

void Steps::allGpsTotal() {
  QString title = mw_one->ui->btnSelGpsDate->text();
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

  mw_one->ui->lblMonthTotal->setText(s1_month + s2_month + s3_month + s4_month);
  mw_one->ui->lblYearTotal->setText(s1_year + s2_year + s3_year + s4_year);
}

void Steps::appendTrack(double lat, double lon) {
  QQuickItem* root = mw_one->ui->qwMap->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "appendTrack", Q_ARG(QVariant, lat),
                            Q_ARG(QVariant, lon));
}

void Steps::updateInfoText(QString strDistance, QString strSpeed) {
  QQuickItem* root = mw_one->ui->qwMap->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "updateInfoText",
                            Q_ARG(QVariant, strDistance),
                            Q_ARG(QVariant, strSpeed));
}

void Steps::updateTrackData(double lat, double lon) {
  QQuickItem* root = mw_one->ui->qwMap->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "updateTrackData",
                            Q_ARG(QVariant, lat), Q_ARG(QVariant, lon));
}

void Steps::updateMapTrackUi(double lat, double lon) {
  QQuickItem* root = mw_one->ui->qwMap->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "updateMapTrackUi",
                            Q_ARG(QVariant, lat), Q_ARG(QVariant, lon));
}

void Steps::clearTrack() {
  QQuickItem* root = mw_one->ui->qwMap->rootObject();
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
  QQuickItem* root = mw_one->ui->qwGpsList->rootObject();
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

  QString gpsFile = iniDir + "memo/gps/" + st1 + "-gps-" + st2 + ".csv";
  QString gpsOptimizedFile =
      iniDir + "memo/gps/" + st1 + "-gps-" + st2 + "-opt.csv";

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
    mw_one->ui->lblGpsDateTime->setText(strGpsMapDateTime);

    updateInfoText(strGpsMapDistnce, strGpsMapSpeed);
    mw_one->ui->tabMotion->setCurrentIndex(3);
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
  QSettings Reg(iniDir + "gpslist.ini", QSettings::IniFormat);

  Reg.setValue("/GPS/isCycling", isCycling);
  Reg.setValue("/GPS/isHiking", isHiking);
  Reg.setValue("/GPS/isRunning", isRunning);
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
  if (isHardStepSensor != 1) return;

  qDebug() << "Started updating the hardware sensor steps...";

  strDate = m_Method->setCurrentDateValue();
  initTodayInitSteps();
  qlonglong steps = 0;
  qlonglong ts = getAndroidSteps();
  steps = ts - initTodaySteps;
  if (steps < 0) return;
  if (steps > 100000000) return;
  CurrentSteps = ts - resetSteps;
  mw_one->ui->lcdNumber->display(QString::number(steps));
  mw_one->ui->lblSingle->setText(QString::number(CurrentSteps));

  setTableSteps(steps);

  sendMsg(steps);
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

  QSettings Reg(iniDir + "initsteps.ini", QSettings::IniFormat);
  QString date = Reg.value("Date", "").toString();
  QString c_date = getCurrentDate();

  if (date != c_date) {
    Reg.setValue("Date", c_date);
    Reg.setValue("InitValue", a);
    initTodaySteps = a;
  } else {
    initTodaySteps = Reg.value("InitValue", 0).toLongLong();
  }
}

void Steps::getHardStepSensor() {
#ifdef Q_OS_ANDROID

  // QJniObject m_activity = QNativeInterface::QAndroidApplication::context();
  // m_activity.callMethod<void>("initStepSensor", "()V");

  isHardStepSensor = QJniObject::callStaticMethod<int>(
      "com.x/MyService", "getHardStepCounter", "()I");

  if (isHardStepSensor == 0) {
    mw_one->ui->btnStepsOptions->setHidden(true);
    mw_one->ui->btnReset->setHidden(true);
    mw_one->ui->tabMotion->setTabEnabled(0, false);
    mw_one->ui->tabMotion->setCurrentIndex(1);
  }
  if (isHardStepSensor == 1) {
    mw_one->ui->lblSteps->hide();
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
                      "  ( " + QString::number(gl) + " " + tr("km") + " )";

  QJniObject javaNotification = QJniObject::fromString(strNotify);
  QJniObject::callStaticMethod<void>(
      "com/x/MyService", "notify",
      "(Landroid/content/Context;Ljava/lang/String;)V",
      QNativeInterface::QAndroidApplication::context(),
      javaNotification.object<jstring>());

#endif
}

void Steps::setCurrentGpsSpeed(double speed) {
  QObject* rootObject = mw_one->ui->qwSpeed->rootObject();
  if (rootObject) {
    rootObject->setProperty("currentSpeed",
                            QString::number(speed, 'f', 2).toDouble());
  }
}
