#include "Todo.h"

#include "defines.h"
#include "src/MainWindow.h"
#include "ui_Todo.h"

QString highLblStyle = "color:rgb(212,35,122)";

QString orgLblStyle;

int highCount;

bool isNeedSync = false;

Todo::Todo(QWidget* parent) : QDialog(parent), ui(new Ui::Todo) {
  ui->setupUi(this);

  this->installEventFilter(this);

  this->setModal(true);

  QString strTar = "/data/data/com.x/files/msg.mp3";
  QFile::copy(":/res/msg.mp3", strTar);

  tmeRecordTime = new QTimer(this);
  connect(tmeRecordTime, SIGNAL(timeout()), this, SLOT(on_ShowRecordTime()));

  tmePlayProgress = new QTimer(this);
  connect(tmePlayProgress, SIGNAL(timeout()), this,
          SLOT(on_ShowPlayProgress()));

  // QScroller::grabGesture(mw_one->ui->editTodo,
  // QScroller::LeftMouseButtonGesture);
  // m_Method->setSCrollPro(mw_one->ui->editTodo);
}

Todo::~Todo() { delete ui; }

void Todo::keyReleaseEvent(QKeyEvent* event) { Q_UNUSED(event); }

void Todo::saveTodo() {
  mw_one->strLatestModify = tr("Modi Todo");

  highCount = 0;

  // 获取数据
  int count_items = listTodo.count();
  int count1 = listRecycle.count();

  // 构建 JSON 对象
  QJsonObject rootObj;

  // 待办事项数组
  QJsonArray todoArray;
  for (int i = 0; i < count_items; i++) {
    QJsonObject itemObj;
    itemObj["text"] = getItemTodoText(i);
    itemObj["time"] = getItemTime(i);
    itemObj["type"] = getItemType(i);
    todoArray.append(itemObj);
  }
  rootObj["todo"] = todoArray;

  // 回收站数组
  QJsonArray recycleArray;
  for (int i = 0; i < count1; i++) {
    QJsonObject recycleObj;
    recycleObj["text"] = getItemTodoTextRecycle(i);
    recycleObj["doneTime"] = getItemTimeRecycle(i);
    recycleArray.append(recycleObj);
  }
  rootObj["recycle"] = recycleArray;

  // 写入文件
  QString tempFile = iniDir + "todo.tmp";
  QString endFile = iniDir + "todo.json";  // 改成 json 后缀

  QFile file(tempFile);
  if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QJsonDocument doc(rootObj);
    file.write(doc.toJson(QJsonDocument::Indented));  // 缩进格式化
    file.close();
  }

  m_Method->upIniFile(tempFile, endFile);

  isNeedSync = true;
}

QString Todo::getItemTodoText(int index) {
  QString str = listTodo.at(index);
  QStringList list = str.split("|==|");
  if (list.count() == 3) return list.at(2);

  return "";
}

void Todo::init_Todo() {
  listTodo.clear();
  listRecycle.clear();

  QString filePath = iniDir + "todo.json";
  if (QFile::exists(filePath)) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qWarning("无法打开 JSON 文件: %s", qPrintable(filePath));
      return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
      qWarning("JSON 解析失败");
      return;
    }

    QJsonObject root = doc.object();

    // 读取待办事项
    QJsonArray todoArray = root["todo"].toArray();
    for (int i = 0; i < todoArray.size(); i++) {
      QJsonObject item = todoArray[i].toObject();
      QString strTime = item["time"].toString();
      int type = item["type"].toInt();
      QString strText = item["text"].toString();

      addItem(strTime, type, strText);
    }

    // 读取回收站
    clearAllRecycle();  // 清空现有回收站数据
    QJsonArray recycleArray = root["recycle"].toArray();
    for (int i = 0; i < recycleArray.size(); i++) {
      QJsonObject item = recycleArray[i].toObject();
      QString doneTime = item["doneTime"].toString();
      QString strText = item["text"].toString();

      addItemRecycle(doneTime, 0, strText);
    }
  }

  refreshTableLists();
  refreshAlarm();
}

void Todo::addToList(QString str, bool isInsert) {
  if (str == "") return;

  int count = getCount();
  for (int i = 0; i < count; i++) {
    QString strTodo = getItemTodoText(i);

    if (str == strTodo) {
      setCurrentIndex(i);
      return;
    }
  }

  QString strTime = m_Method->setCurrentDateTimeValue();

  if (isInsert)
    insertItem(strTime, 0, str, 0);
  else
    addItem(strTime, 0, str);

  setCurrentIndex(0);
  refreshTableLists();

  cppRefreshTodoCardList();

  saveTodo();
}

void Todo::AddTodoText() {
  QString str = "todo txt";
  if (str == "") return;
  addToList(str, true);
}

int Todo::getEditTextHeight(QTextEdit* edit) {
  QTextDocument* doc = edit->document();
  int height = doc->size().height() * 1.10;
  int width = doc->size().width();
  doc->setTextWidth(width);
  return height;
}

void Todo::closeEvent(QCloseEvent* event) { Q_UNUSED(event); }

void Todo::closeTodo() {
  mw_one->clearWidgetFocus();

  stopPlayVoice();
  saveTodo();

  if (isAndroid) {
    m_Method->openMainEntranceWindow();
  } else {
    mw_one->ui->frameMain->show();
  }

  refreshTableLists();
  refreshAlarm();

  if (isNeedSync && mw_one->ui->chkAutoSync->isChecked() &&
      mw_one->ui->chkWebDAV->isChecked()) {
    QString todoFile = iniDir + "todo.json";
    QString todoZipFile = privateDir + "KnotData/todo.json.zip";

    if (!m_Method->compressFileWithZlib(todoFile, todoZipFile,
                                        Z_DEFAULT_COMPRESSION)) {
      errorInfo = tr("An error occurred while compressing the file.");
      auto msg = std::make_unique<ShowMessage>(mw_one);
      msg->showMsg("Knot", errorInfo, 1);
      return;
    }

    QString enc_file = m_Method->useEnc(todoZipFile);
    if (enc_file != "") todoZipFile = enc_file;

    m_Notes->appendToSyncList(todoZipFile);
    mw_one->showProgress();
    m_CloudBackup->createRemoteWebDAVDir();
    m_CloudBackup->uploadFilesToWebDAV(m_Notes->notes_sync_files);
    isNeedSync = false;
  }
}

bool Todo::eventFilter(QObject* watch, QEvent* evn) {
  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
    }
  }

  return QWidget::eventFilter(watch, evn);
}

void Todo::on_btnHigh(int index) {
  int count = getCount();
  if (count == 0) return;

  int row = index;

  QString strTime = getItemTime(row);

  if (getTimeStr(strTime) != "") return;

  QString strText = getItemTodoText(row);

  delItem(row);
  insertItem(strTime, 1, strText, 0);
  setCurrentIndex(0);

  cppRefreshTodoCardList();

  refreshAlarm();

  saveTodo();
}

void Todo::delItem(int index) { listTodo.removeAt(index); }

void Todo::setCurrentIndex(int) {}

void Todo::setHighPriority(bool) {}

void Todo::addItem(QString strTime, int type, QString strText) {
  listTodo.append(strTime + "|==|" + QString::number(type) + "|==|" + strText);
}

void Todo::addItemRecycle(QString strTime, int type, QString strText) {
  Q_UNUSED(type);
  listRecycle.append(strTime + "|==|" + strText);
}

void Todo::delItemRecycle(int index) {
  listRecycle.remove(index);
  saveTodo();
}

QString Todo::getItemTodoTextRecycle(int index) {
  QString str = listRecycle.at(index);
  QStringList list = str.split("|==|");
  if (list.count() == 2) return list.at(1);

  return "";
}

void Todo::on_btnLow(int index) {
  int count = getCount();
  if (count == 0) return;

  int row = index;

  QString strTime = getItemTime(row);

  if (getTimeStr(strTime) != "") return;

  QString strTodoText = getItemTodoText(row);
  delItem(row);
  addItem(strTime, 0, strTodoText);

  cppRefreshTodoCardList();

  refreshAlarm();

  setCurrentIndex(getCount() - 1);

  saveTodo();
}

void Todo::on_SetAlarm(bool w1, bool w2, bool w3, bool w4, bool w5, bool w6,
                       bool w7, int y, int mon, int d, int h, int m) {
  int row = curTodoListIndex;
  if (row < 0) return;

  QString strTodoText = getItemTodoText(row);
  QString strTime;
  alarmTime.setHMS(h, m, 0, 0);
  alarmDate.setDate(y, mon, d);

  QString str;
  if (w1) str = str + "1";
  if (w2) str = str + "2";
  if (w3) str = str + "3";
  if (w4) str = str + "4";
  if (w5) str = str + "5";
  if (w6) str = str + "6";
  if (w7) str = str + "7";
  if (str.length() > 0) {
    strTime = tr("Alarm") + "  " + str + "  " + alarmTime.toString("HH:mm");

  } else {
    strTime = tr("Alarm") + "  " + alarmDate.toString("yyyy-M-d") + " " +
              alarmTime.toString("HH:mm");
  }

  delItem(row);
  insertItem(strTime, 0, strTodoText, row);
  setCurrentIndex(row);

  refreshTableLists();
  refreshAlarm();

  cppRefreshTodoCardList();

  saveTodo();

  goCurrentTodoItem(currentTodoItem);
}

bool Todo::isWeekValid(QString lblDateTime, QString strDate) {
  if (!lblDateTime.contains("-")) {
    int week = QDate::fromString(strDate, "yyyy-M-d").dayOfWeek();

    QStringList list = lblDateTime.split(" ");
    QString str = list.at(0);

    for (int i = 0; i < str.length(); i++) {
      if (str.mid(i, 1) == QString::number(week)) {
        return true;
      }
    }
  }
  return false;
}

qlonglong Todo::getSecond(QString strDateTime) {
  // 2022-8-22 18:18
  isTomorrow = false;
  QString strtime, sdt;
  sdt = strDateTime;
  if (!strDateTime.contains("-")) {
    int week = QDate::currentDate().dayOfWeek();
    QStringList list = strDateTime.split(" ");
    QString str = list.at(0);

    for (int i = 0; i < list.count(); i++) {
      QString st = list.at(i);
      if (st.contains(":")) {
        strtime = st;
        break;
      }
    }
    for (int i = 0; i < str.length(); i++) {
      if (str.mid(i, 1) == QString::number(week)) {
        strDateTime = QDate::currentDate().toString("yyyy-M-d") + " " + strtime;
      }
    }
  }

  strDateTime = strDateTime + ":00";
  QString strCur = QDateTime::currentDateTime().toString("yyyy-M-d HH:mm:ss");
  QDateTime timeCur = QDateTime::fromString(strCur, "yyyy-M-d HH:mm:ss");
  QDateTime timeAlarm = QDateTime::fromString(strDateTime, "yyyy-M-d HH:mm:ss");
  qlonglong seconds = timeCur.secsTo(timeAlarm);

  if (seconds <= 0) {
    if (!sdt.contains("-")) {
      QDateTime ctime = QDateTime::currentDateTime();
      QString strTmo = ctime.addDays(+1).toString("yyyy-M-d");

      if (isWeekValid(sdt.split(" ").at(0), strTmo)) {
        strDateTime = strTmo + " " + strtime;
        strDateTime = strDateTime + ":00";
        QString strCur =
            QDateTime::currentDateTime().toString("yyyy-M-d HH:mm:ss");
        QDateTime timeCur = QDateTime::fromString(strCur, "yyyy-M-d HH:mm:ss");
        QDateTime timeAlarm =
            QDateTime::fromString(strDateTime, "yyyy-M-d HH:mm:ss");
        seconds = timeCur.secsTo(timeAlarm);
        isTomorrow = true;
      }
    }
  }

  return seconds;
}

void Todo::on_btnSetTime(int index) {
  int count = getCount();
  if (count == 0) return;

  curTodoListIndex = index;

  QStringList list;
  QString str1, str2, str3, str4, str5, str6, str7;
  QString strDate, strTime;
  str1 = "0";
  str2 = "0";
  str3 = "0";
  str4 = "0";
  str5 = "0";
  str6 = "0";
  str7 = "0";

  int row = index;
  QString str = getItemTime(row);
  QDate date;
  QTime time;

  str = getTimeStr(str);

  if (str != "") {
    QStringList list = str.split(" ");
    if (str.contains("-")) {
      date = QDate::fromString(list.at(0), "yyyy-M-d");
      time = QTime::fromString(list.at(1), "HH:mm");
    } else {
      QString s1 = list.at(0);
      for (int i = 0; i < s1.length(); i++) {
        QString s2 = s1.mid(i, 1);
        if (s2 == "1") str1 = "1";
        if (s2 == "2") str2 = "1";
        if (s2 == "3") str3 = "1";
        if (s2 == "4") str4 = "1";
        if (s2 == "5") str5 = "1";
        if (s2 == "6") str6 = "1";
        if (s2 == "7") str7 = "1";
      }
      date = QDate::currentDate();

      for (int i = 0; i < list.count(); i++) {
        if (list.at(i).contains(":")) {
          time = QTime::fromString(list.at(i), "HH:mm");
          break;
        }
      }
    }

  } else {
    str = getItemTime(row);
    QStringList list = str.split(" ");
    if (str.mid(0, 2) == "20" && str.contains("-")) {
      date = QDate::fromString(list.at(0), "yyyy-M-d");
      time = QTime::fromString(list.at(1), "HH:mm");
    }

    if (list.count() > 2) {
      date = QDate::currentDate();
      time = QTime::currentTime();
    }

    if ((str.mid(0, 1) == "1" || str.mid(0, 1) == "2" || str.mid(0, 1) == "3" ||
         str.mid(0, 1) == "4" || str.mid(0, 1) == "5" || str.mid(0, 1) == "6" ||
         str.mid(0, 1) == "7") &&
        !str.contains("-")) {
      QString s1 = list.at(0);
      for (int i = 0; i < s1.length(); i++) {
        QString s2 = s1.mid(i, 1);
        if (s2 == "1") str1 = "1";
        if (s2 == "2") str2 = "1";
        if (s2 == "3") str3 = "1";
        if (s2 == "4") str4 = "1";
        if (s2 == "5") str5 = "1";
        if (s2 == "6") str6 = "1";
        if (s2 == "7") str7 = "1";
      }
      date = QDate::currentDate();
      for (int i = 0; i < list.count(); i++) {
        if (list.at(i).contains(":")) {
          time = QTime::fromString(list.at(i), "HH:mm");
          break;
        }
      }
    }
  }

  strDate = date.toString("yyyy-M-d");
  strTime = time.toString("HH:mm");
  list.append(str1);
  list.append(str2);
  list.append(str3);
  list.append(str4);
  list.append(str5);
  list.append(str6);
  list.append(str7);
  list.append(strDate);
  list.append(strTime);

  openTodoAlarmWindow(list);
}

void Todo::on_DelAlarm() {
  int row = curTodoListIndex;
  if (row < 0) return;

  QString str = getItemTime(row);
  QString str1 = str;
  str = getTimeStr(str);
  if (str != "") str1 = str;
  modifyTime(row, str1);
  modifyType(row, 0);

  refreshTableLists();
  refreshAlarm();

  cppRefreshTodoCardList();

  saveTodo();

  goCurrentTodoItem(currentTodoItem);
}

void Todo::startTimerAlarm(QString text, qlonglong minValue) {
  if (minValue < 5) return;

  Q_UNUSED(text);
#ifdef Q_OS_ANDROID
  QJniObject javaText = QJniObject::fromString(text);
  QJniObject jo = QNativeInterface::QAndroidApplication::context();

  jo.callStaticMethod<int>("com.x/MyService", "startPreciseAlarmInMyService",
                           "(Ljava/lang/String;)I", javaText.object<jstring>());

#endif
}

void Todo::stopTimerAlarm() {
#ifdef Q_OS_ANDROID

  QJniObject activity = QNativeInterface::QAndroidApplication::context();

  activity.callStaticMethod<int>("com.x/MyService", "stopAlarm", "()I");

#endif
}

void Todo::sendMsgAlarm(QString text) {
  Q_UNUSED(text);
#ifdef Q_OS_ANDROID
  QString strNotify = tr("Todo") + " : " + text;

  QJniObject javaNotification = QJniObject::fromString(strNotify);
  QJniObject::callStaticMethod<void>(
      "com/x/MyService", "notifyTodoAlarm",
      "(Landroid/content/Context;Ljava/lang/String;)V",
      QNativeInterface::QAndroidApplication::context(),
      javaNotification.object<jstring>());

#endif
}

void Todo::on_btnRecycle() {
  if (isAndroid) {
    openTodoRecycleWindow(listRecycle);
  } else {
  }
}

void Todo::on_btnReturn_clicked() {}

void Todo::on_btnClear_clicked() {
  clearAllRecycle();

  saveTodo();
}

void Todo::on_btnRestore_clicked() {
  if (getCountRecycle() == 0) return;

  int row = getCurrentIndexRecycle();
  QString strTime = m_Method->setCurrentDateTimeValue();
  QString strText = getItemTodoTextRecycle(row);
  addItem(strTime, 0, strText);

  isRestore = true;
  on_btnDel_clicked();

  setCurrentIndex(getCount() - 1);

  saveTodo();
}

void Todo::on_btnDel_clicked() {
  int row = getCurrentIndexRecycle();
  if (row < 0) return;

  if (!isRestore)
    delVoiceFile(row);
  else
    isRestore = false;

  delItemRecycle(row);

  saveTodo();
}

void Todo::refreshTableLists() {
  tableLists.clear();
  int count_items = getCount();

  for (int i = 0; i < count_items; i++) {
    QString strTime = getItemTime(i);
    QString strText = getItemTodoText(i);

    tableLists.append(strTime + "|=|" + strText);
  }
}

void Todo::refreshTableListsFromFile() {
  tableLists.clear();

  QString filePath = iniDir + "todo.json";

  if (QFile::exists(filePath)) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qWarning("无法打开 JSON 文件: %s", qPrintable(filePath));
      return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
      qWarning("JSON 解析失败");
      return;
    }

    QJsonObject root = doc.object();

    // 读取待办事项数组
    QJsonArray todoArray = root["todo"].toArray();
    for (int i = 0; i < todoArray.size(); i++) {
      QJsonObject item = todoArray[i].toObject();
      QString strTime = item["time"].toString();
      QString strText = item["text"].toString();

      // 保持原格式拼接
      tableLists.append(strTime + "|=|" + strText);
    }
  } else {
    qWarning("JSON 文件不存在: %s", qPrintable(filePath));

    iniTodo = new QSettings(iniDir + "todo.ini", QSettings::IniFormat, this);
    int count_items = iniTodo->value("/Todo/Count", 0).toInt();
    for (int i = 0; i < count_items; i++) {
      QString strTime =
          iniTodo->value("/Todo/Time" + QString::number(i)).toString();
      QString strText =
          iniTodo->value("/Todo/Item" + QString::number(i)).toString();

      tableLists.append(strTime + "|=|" + strText);
    }
  }
}

QString Todo::getTimeStr(QString str) {
  bool isTime = false;
  if (str.contains("定时提醒")) {
    str = str.replace("定时提醒", "").trimmed();
    isTime = true;
  }

  if (str.contains("Alarm")) {
    str = str.replace("Alarm", "").trimmed();
    isTime = true;
  }

  if (isTime) return str;

  return "";
}

void Todo::refreshAlarm() {
  stopTimerAlarm();
  int count = 0;
  isToday = false;
  QString str;

  QStringList listAlarm;
  QList<qlonglong> listTotalS;

  int count_items = getCount();

  if (count_items > 0) {
    for (int i = 0; i < count_items; i++) {
      str = getItemTime(i);

      str = getTimeStr(str);

      if (str != "") {
        modifyType(i, 3);

        qlonglong totals = getSecond(str);

        if (totals > 0) {
          count++;

          QString todo_text = getItemTodoText(i);
          QString str1 = str + "|" + todo_text + "|" + QString::number(totals) +
                         "|" + tr("Close");

          listAlarm.append(str1);
          listTotalS.append(totals);

          // set time marks
          QString strdate = str.split(" ").at(0);
          QString strToday = QDate::currentDate().toString("yyyy-M-d");
          QDateTime ctime = QDateTime::currentDateTime();
          QString strTmo = ctime.addDays(+1).toString("yyyy-M-d");
          if (strdate.contains("-")) {
            if (strdate == strToday) {
              modifyType(i, 1);
              isToday = true;
            }

            if (strTmo == strdate) {
              modifyType(i, 2);
            }
          } else {
            if (isWeekValid(str, strToday) && !isTomorrow) {
              modifyType(i, 1);
              isToday = true;
            }

            if (isWeekValid(str, strTmo) && isTomorrow) {
              modifyType(i, 2);
            }
          }

        } else {
          if (str.contains("-")) {
            modifyTime(i, str);
            modifyType(i, 0);
          }

          if (!str.contains("-")) {
            modifyTime(i, tr("Alarm") + "  " + str);
            modifyType(i, 3);

            QDateTime ctime = QDateTime::currentDateTime();
            QString strTmo = ctime.addDays(+1).toString("yyyy-M-d");
            if (isWeekValid(str, strTmo)) {
              modifyType(i, 2);
            }
          }
        }
      }
    }
  } else {
    count_items = tableLists.count();

    for (int i = 0; i < count_items; i++) {
      QString strList = tableLists.at(i);
      QStringList list = strList.split("|=|");
      QString strTime = list.at(0);

      strTime = getTimeStr(strTime);

      if (strTime != "") {
        qlonglong totals = getSecond(strTime);

        if (totals > 0) {
          count++;

          QString todo_text = list.at(1);
          QString str1 = strTime + "|" + todo_text + "|" +
                         QString::number(totals) + "|" + tr("Close");

          listAlarm.append(str1);
          listTotalS.append(totals);
        }
      }
    }
  }

  qlonglong minValue = 0;

  if (count > 0) {
    minValue = *std::min_element(listTotalS.begin(), listTotalS.end());
    for (int i = 0; i < listTotalS.count(); i++) {
      if (minValue == listTotalS.at(i)) {
        QString str1 = listAlarm.at(i);
        startTimerAlarm(str1, minValue);

        // isToDay?
        QDate todayDate = QDate::currentDate();
        QDate tomoDate = todayDate.addDays(1);
        QDateTime tomoDateTime =
            QDateTime::fromString(tomoDate.toString() + " 00:00:00");

        qint64 current_s = QDateTime::currentSecsSinceEpoch();

        qint64 tomo_s = tomoDateTime.toSecsSinceEpoch();
        if (minValue + current_s < tomo_s)
          isToday = true;
        else
          isToday = false;

        qDebug() << "current_s=" << current_s << "tomo_s=" << tomo_s
                 << "tomoDateTime=" << tomoDateTime.toString()
                 << "tomoDate=" << tomoDate.toString();

        qDebug() << "Min Time: " << listTotalS << minValue << str1
                 << "curVol: ";

        QStringList t_list = str1.split("|");
        if (t_list.count() >= 3) {
          strAlarmTime = t_list.at(0);
          strAlarmText = t_list.at(1);
        }

        // to top
        int listcount = getCount();
        for (int m = 0; m < listcount; m++) {
          QString date = getItemTime(m);
          int type = getItemType(m);
          QString text = getItemTodoText(m);
          if (str1.contains(text)) {
            delItem(m);
            insertItem(date, type, text, 0);

            break;
          }
        }

        break;
      }
    }
  }

  changeTodoIcon(isToday);
}

void Todo::changeTodoIcon(bool isToday) {
  if (!isToday) {
    if (isDark)
      mw_one->ui->btnTodo->setIcon(QIcon(":/res/todo_l.svg"));
    else
      mw_one->ui->btnTodo->setIcon(QIcon(":/res/todo.svg"));
  } else {
    mw_one->ui->btnTodo->setIcon(QIcon(":/res/todo1.svg"));
  }
}

void Todo::on_editTodo_textChanged() {}

void Todo::insertItem(QString strTime, int type, QString strText,
                      int curIndex) {
  listTodo.insert(curIndex,
                  strTime + "|==|" + QString::number(type) + "|==|" + strText);
}

void Todo::insertRecycle(QString strTime, int type, QString strText,
                         int curIndex) {
  listRecycle.insert(curIndex, strTime + "|==|" + strText);
}

int Todo::getCurrentIndex() { return 0; }

int Todo::getCurrentIndexRecycle() { return 0; }

QString Todo::getItemTime(int index) {
  QString str = listTodo.at(index);
  QStringList list = str.split("|==|");
  if (list.count() == 3) return list.at(0);

  return "00:00:00";
}

QString Todo::getItemTimeRecycle(int index) {
  QString str = listRecycle.at(index);
  QStringList list = str.split("|==|");
  if (list.count() == 2) return list.at(0);

  return "11:11:11";
}

int Todo::getItemType(int index) {
  QString str = listTodo.at(index);
  QStringList list = str.split("|==|");
  if (list.count() == 3) return list.at(1).toInt();

  return 0;
}

int Todo::setItemHeight(QString strTodoText) {
  QFont font = this->font();
  font.setPointSize(fontSize - 2);

  QFontMetrics fm(font);
  int fontHeight = fm.height();

  return fontHeight;

  QTextEdit* edit = new QTextEdit;
  edit->append(strTodoText);
  int itemHeight = fontHeight * 2 + getEditTextHeight(edit);

  return itemHeight;
}

int Todo::getCount() { return listTodo.count(); }

int Todo::getCountRecycle() { return listRecycle.count(); }

void Todo::modifyTime(int index, QString strTime) {
  QString str = listTodo.at(index);
  QStringList list = str.split("|==|");
  if (list.count() == 3) {
    QString str1, str2, str3, str4;
    str1 = strTime;
    str2 = list.at(1);
    str3 = list.at(2);
    str4 = str1 + "|==|" + str2 + "|==|" + str3;
    listTodo.replace(index, str4);
  }
}

void Todo::modifyType(int index, int type) {
  QString str = listTodo.at(index);
  QStringList list = str.split("|==|");
  if (list.count() == 3) {
    QString str1, str2, str3, str4;
    str1 = list.at(0);
    str2 = QString::number(type);
    str3 = list.at(2);
    str4 = str1 + "|==|" + str2 + "|==|" + str3;
    listTodo.replace(index, str4);
  }
}

void Todo::modifyTodoText(int index, QString strTodoText) {
  QString str = listTodo.at(index);
  QStringList list = str.split("|==|");
  if (list.count() == 3) {
    QString str1, str2, str3, str4;
    str1 = list.at(0);
    str2 = list.at(1);
    str3 = strTodoText;
    str4 = str1 + "|==|" + str2 + "|==|" + str3;
    listTodo.replace(index, str4);

    cppRefreshTodoCardList();

    saveTodo();
  }
}

void Todo::clearAllRecycle() {
  int count = getCountRecycle();
  if (count == 0) return;
  for (int i = 0; i < count; i++) {
    delVoiceFile(0);
  }

  listRecycle.clear();

  saveTodo();
}

void Todo::isAlarm(int index) {
  bool a = false;
  QString strTime = getItemTime(index);

  strTime = getTimeStr(strTime);
  if (strTime != "") a = true;
  qDebug() << "aabb" << a;
  setHighPriority(a);
}

void Todo::reeditText() {
  int count = getCount();
  if (count == 0) return;

  int row = getCurrentIndex();
  QString strItem = getItemTodoText(row).trimmed();
  QStringList list0 = strItem.split(" ");
  if (list0.count() > 0) {
    QString str = list0.at(0);
    if (str == tr("Voice")) {
      tmePlayProgress->start(nInterval);
      return;
    }
  }

  if (m_ReeditTodo != nullptr) {
    m_ReeditTodo->close();
    m_ReeditTodo->deleteLater();
    m_ReeditTodo = nullptr;
  }

  m_ReeditTodo = new QDialog(this);
  QVBoxLayout* vbox0 = new QVBoxLayout;
  m_ReeditTodo->setLayout(vbox0);
  vbox0->setContentsMargins(5, 5, 5, 5);
  if (!isAndroid) m_ReeditTodo->setModal(true);
  m_ReeditTodo->setWindowFlag(Qt::FramelessWindowHint);

  QFrame* frame = new QFrame(this);
  vbox0->addWidget(frame);

  QVBoxLayout* vbox = new QVBoxLayout;

  frame->setLayout(vbox);
  vbox->setContentsMargins(6, 6, 6, 10);
  vbox->setSpacing(10);

  QLabel* lblTitle = new QLabel(this);
  lblTitle->adjustSize();
  lblTitle->setWordWrap(true);
  lblTitle->setText(tr("Editor"));
  vbox->addWidget(lblTitle);
  lblTitle->hide();

  QFrame* hframe = new QFrame(this);
  hframe->setFrameShape(QFrame::HLine);
  hframe->setStyleSheet("QFrame{background:red;min-height:2px}");
  vbox->addWidget(hframe);
  hframe->hide();

  QTextEdit* edit = new QTextEdit(this);
  edit->setAcceptRichText(false);

  initTextToolbarDynamic(m_ReeditTodo);
  EditEventFilter* editFilter =
      new EditEventFilter(textToolbarDynamic, m_ReeditTodo);
  editFilter->setParent(m_ReeditTodo);
  edit->installEventFilter(editFilter);
  edit->viewport()->installEventFilter(editFilter);

  vbox->addWidget(edit);
  edit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  edit->setPlainText(strItem);

  edit->horizontalScrollBar()->setHidden(true);
  // edit->verticalScrollBar()->setStyleSheet(
  //     mw_one->ui->editDetails->verticalScrollBar()->styleSheet());

  QToolButton* btnCancel = new QToolButton(this);
  QToolButton* btnCopy = new QToolButton(this);
  QToolButton* btnShare = new QToolButton(this);
  QToolButton* btnOk = new QToolButton(this);
  btnCancel->setText(tr("Cancel"));
  btnCopy->setText(tr("Copy"));
  btnShare->setText(tr("Share"));
  btnOk->setText(tr("OK"));

  QHBoxLayout* hbox = new QHBoxLayout;
  hbox->addWidget(btnCancel);
  hbox->addWidget(btnCopy);
  hbox->addWidget(btnShare);
  hbox->addWidget(btnOk);
  btnCancel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  btnCopy->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  btnShare->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  btnOk->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

  btnCancel->setFixedHeight(50);
  btnCopy->setFixedHeight(50);
  btnShare->setFixedHeight(50);
  btnOk->setFixedHeight(50);

  QSpacerItem* sparcer_item =
      new QSpacerItem(0, 2, QSizePolicy::Fixed, QSizePolicy::Fixed);
  vbox->addItem(sparcer_item);

  vbox->addLayout(hbox, 0);

#ifdef Q_OS_ANDROID
  btnShare->show();
#else
  btnShare->hide();
#endif

  connect(btnCancel, &QToolButton::clicked, m_ReeditTodo,
          [=]() mutable { m_ReeditTodo->close(); });

  connect(m_ReeditTodo, &QDialog::rejected, m_ReeditTodo,
          [=]() mutable { m_Method->closeGrayWindows(); });

  connect(m_ReeditTodo, &QDialog::accepted, m_ReeditTodo,
          [=]() mutable { m_Method->closeGrayWindows(); });

  connect(m_ReeditTodo, &QDialog::finished, this, [this](int result) {
    Q_UNUSED(result);
    closeTextToolBar();

    m_Method->closeGrayWindows();
  });

  connect(btnCopy, &QToolButton::clicked, m_ReeditTodo, [=]() mutable {
    edit->selectAll();
    edit->copy();
    m_ReeditTodo->close();
  });

  connect(btnShare, &QToolButton::clicked, m_ReeditTodo, [=]() mutable {
    QString txt = edit->toPlainText().trimmed();
    if (txt.length() > 0) {
      mw_one->m_ReceiveShare->shareString(tr("Share to"), txt);
    }
  });

  connect(btnOk, &QToolButton::clicked, m_ReeditTodo, [=]() mutable {
    QString strTime = getItemTime(row);
    int type = getItemType(row);
    delItem(row);
    insertItem(strTime, type, edit->toPlainText().trimmed(), row);
    setCurrentIndex(row);

    saveTodo();
    m_ReeditTodo->close();
  });

  edit->setFixedHeight(280);

  int x, y, w, h;
  h = 500;

  if (isAndroid) {
    w = mw_one->width() - 2;
    y = mw_one->geometry().y();
  } else {
    w = 300;
    y = mw_one->geometry().y() + (mw_one->height() - h) / 2;
  }
  x = mw_one->geometry().x() + (mw_one->width() - w) / 2;

  m_ReeditTodo->setGeometry(x, y, w, h);

  m_Method->showGrayWindows();
  m_Method->set_ToolButtonStyle(m_ReeditTodo);
  m_ReeditTodo->show();
}

void Todo::addToRecycle(int index) {
  QString strTodoText = getItemTodoText(index);
  QString doneTime = m_Method->setCurrentDateTimeValue() + "  " + tr("Done");

  insertRecycle(doneTime, 0, strTodoText, 0);
  delItem(index);
  cppRefreshTodoCardList();

  saveTodo();
}

void Todo::NewTodo() { mw_one->ui->btnTodo->click(); }

void Todo::startRecordVoice() {}

void Todo::on_ShowRecordTime() {}

void Todo::stopRecordVoice() {}

void Todo::stopPlayVoice() {}

bool Todo::isVoice(int row) {
  QString strItem = getItemTodoText(row).trimmed();
  QStringList list0 = strItem.split(" ");
  if (list0.count() > 0) {
    QString str = list0.at(0);
    if (str == tr("Voice")) {
      return true;
    }
  }
  return false;
}

bool Todo::isVoice(const QString& strTodoText) {
  QString strItem = strTodoText.trimmed();
  QStringList list0 = strItem.split(" ");
  if (list0.count() > 0) {
    QString str = list0.at(0);
    if (str == tr("Voice")) {
      return true;
    }
  }
  return false;
}

QString Todo::getVoiceFile(int row) {
  QString strItem = getItemTodoText(row).trimmed();
  QStringList list0 = strItem.split(" ");
  if (list0.count() > 0) {
    QString str = list0.at(0);
    if (str == tr("Voice")) {
      QString voiceFile = iniDir + "memo/voice/" + strItem.split("\n").at(1);
      if (QFile::exists(voiceFile)) return voiceFile;
    }
  }
  return "";
}

QString Todo::getVoiceFile(const QString& strTodoText) {
  QString strItem = strTodoText.trimmed();
  QStringList list0 = strItem.split(" ");
  if (list0.count() > 0) {
    QString str = list0.at(0);
    if (str == tr("Voice")) {
      QString voiceFile = iniDir + "memo/voice/" + strItem.split("\n").at(1);
      if (QFile::exists(voiceFile)) return voiceFile;
    }
  }
  return "";
}

void Todo::delVoiceFile(int row) {
  QString strItem = getItemTodoTextRecycle(row).trimmed();
  QStringList list0 = strItem.split(" ");
  if (list0.count() > 0) {
    QString str = list0.at(0);
    if (str == tr("Voice")) {
      QString voiceFile = iniDir + "memo/voice/" + strItem.split("\n").at(1);
      if (QFile::exists(voiceFile)) QFile::remove(voiceFile);
    }
  }
}

QString Todo::getNumber(QString str) {
  QString str0;
  for (int i = 0; i < str.length(); i++) {
    QString str1 = str.mid(i, 1);
    if (str1 != " ") {
      bool isOk;
      str1.toInt(&isOk, 10);
      if (isOk) str0 = str0 + str1;
    }
  }
  return str0;
}

void Todo::goCurrentTodoItem(QString curItem) {
  int count = getCount();
  if (count == 0) return;

  for (int i = 0; i < count; i++) {
    QString item = getItemTodoText(i);
    if (item == curItem) {
      setCurrentIndex(i);
      break;
    }
  }
}

void Todo::on_ShowPlayProgress() {}

void Todo::openTodoUI() {
  mw_one->execNeedSyncNotes();

  init_Todo();

  if (isAndroid) {
    openTodoListWindow(listTodo);
  } else {
    show();
  }

  refreshAlarm();
  setCurrentIndex(0);
  stopPlayVoice();

  mw_one->safeCloseProgress();

  if (isNeedAddToTodoList) {
    isNeedAddToTodoList = false;
  }
}

void Todo::openTodo() {
  // 延迟一小段时间再触发，避免模块快速切换时反复启停
  QTimer::singleShot(500, m_NotesList, &NotesList::rebuilderNotesVector);

  isPasswordError = false;
  isGetWebDavModiTime = true;

  mw_one->showProgress();

  if (mw_one->ui->chkAutoSync->isChecked() &&
      mw_one->ui->chkWebDAV->isChecked()) {
    if (!m_CloudBackup->checkWebDAVConnection()) {
      mw_one->safeCloseProgress();
      auto msg = std::make_unique<ShowMessage>(mw_one);
      msg->showMsg(appName,
                   tr("WebDAV connection failed. Please check the network, "
                      "website address or login information."),
                   1);

      QTimer::singleShot(100, mw_one, [this]() { openTodoUI(); });
      return;
    }

    QString url = m_CloudBackup->getWebDAVArgument();

    WebDavHelper* helper =
        listWebDavFiles(url + "KnotData/", m_CloudBackup->USERNAME,
                        m_CloudBackup->APP_PASSWORD);

    // 连接信号
    QObject::connect(
        helper, &WebDavHelper::listCompleted, this,
        [=](const QList<QPair<QString, QDateTime>>& files) {
          qDebug() << "获取到文件列表:" << url + "KnotData/";
          qDebug() << "共找到" << files.size() << "个文件:";

          if (files.size() == 0) {
            QTimer::singleShot(100, mw_one, [this]() { openTodoUI(); });
            return;
          }

          bool isTodoFile = false;
          for (const auto& [path, mtime] : files) {
            QDateTime remoteTime = mtime;  // 创建非const副本
            remoteTime.toTimeZone(QTimeZone::utc());
            remoteTime = remoteTime.toLocalTime();

            qDebug() << "路径:" << path << "修改时间:" << remoteTime
                     << remoteTime.toString("yyyy-MM-dd hh:mm:ss");

            QString remoteFile = path;

            remoteFile = m_CloudBackup->getRemoteKnotDataFullPath(remoteFile);

            qDebug() << "处理之后的远程文件：" << remoteFile;

            if (remoteFile.contains("todo.json.zip")) {
              isTodoFile = true;
              QString localFile = privateDir + "KnotData/todo.json.zip";
              QDateTime localModi = QFileInfo(localFile).lastModified();

              qDebug() << "localModi=" << localModi;

              if (remoteTime > localModi || !QFile::exists(localFile)) {
                // 初始化下载器
                WebDavDownloader* downloader = new WebDavDownloader(
                    m_CloudBackup->USERNAME, m_CloudBackup->APP_PASSWORD);

                // 连接信号
                QObject::connect(
                    downloader, &WebDavDownloader::progressChanged, this,
                    [](int current, int total, QString file) {
                      qDebug() << QString("进度: %1/%2  当前文件: %3")
                                      .arg(current)
                                      .arg(total)
                                      .arg(file);
                    });

                QObject::connect(
                    downloader, &WebDavDownloader::downloadFinished, this,
                    [this](bool success, QString error) {
                      qDebug() << (success ? "下载成功" : "下载失败: " + error);
                      QString zFile = privateDir + "KnotData/todo.json.zip";

                      QString dec_file = m_Method->useDec(zFile);
                      if (dec_file != "") zFile = dec_file;

                      errorInfo = "";
                      if (!m_Method->decompressFileWithZlib(
                              zFile, privateDir + "KnotData/todo.json")) {
                        mw_one->safeCloseProgress();
                        errorInfo =
                            tr("Decompression failed. Please check in "
                               "Preferences that the passwords are consistent "
                               "across all platforms.");

                        auto msg = std::make_unique<ShowMessage>(mw_one);
                        msg->showMsg("Knot", errorInfo, 1);
                        isPasswordError = true;
                        QFile::remove(zFile);

                        return;
                      }

                      QString zipToto = privateDir + "KnotData/todo.json";
                      QString localTodo = iniDir + "todo.json";

                      if (isPasswordError == false) {
                        if (QFileInfo(zipToto).lastModified() >
                            QFileInfo(localTodo).lastModified()) {
                          QString tempFile = iniDir + "temp_todo.tmp";
                          if (QFile::exists(tempFile)) QFile::remove(tempFile);
                          QFile::copy(zipToto, tempFile);
                          m_Method->upIniFile(tempFile, localTodo);
                        }
                      } else {
                        QFile::remove(zFile);
                      }

                      QTimer::singleShot(100, mw_one,
                                         [this]() { openTodoUI(); });
                    });

                // 安全通用清洗：只删除 dav 前缀，不删除完整路径结构
                remoteFile = remoteFile.replace("/dav/", "");

                // 需要下载的文件列表
                QList<QString> remoteFiles = {remoteFile};

                // 开始下载（1并发,根据文件的下载个数）
                QString lf = privateDir;
                qDebug() << "lf=" << lf;
                downloader->downloadFiles(remoteFiles, lf, 1);
              }

              if (mtime <= localModi)
                QTimer::singleShot(100, mw_one, [this]() { openTodoUI(); });
              break;
            }
          }

          if (isTodoFile == false)
            QTimer::singleShot(100, mw_one, [this]() { openTodoUI(); });
        });

    QObject::connect(helper, &WebDavHelper::errorOccurred, this,
                     [this](const QString& error) {
                       qDebug() << "操作失败:" << error;
                       QTimer::singleShot(100, [this]() { openTodoUI(); });
                     });

  } else
    QTimer::singleShot(100, mw_one, [this]() { openTodoUI(); });
}

void Todo::closeTodoAlarm() {}

void Todo::showTodoAlarm() {
  int row = getCurrentIndex();
  QString str = getItemTime(row);
  QDate date;
  QTime time;

  str = getTimeStr(str);

  if (str != "") {
    QStringList list = str.split(" ");
    if (str.contains("-")) {
      date = QDate::fromString(list.at(0), "yyyy-M-d");
      time = QTime::fromString(list.at(1), "HH:mm");
    } else {
      QString s1 = list.at(0);
      for (int i = 0; i < s1.length(); i++) {
        QString s2 = s1.mid(i, 1);
      }
      date = QDate::currentDate();

      for (int i = 0; i < list.count(); i++) {
        if (list.at(i).contains(":")) {
          time = QTime::fromString(list.at(i), "HH:mm");
          break;
        }
      }
    }

    alarmDate = date;
    alarmTime = time;

  } else {
    str = getItemTime(row);
    QStringList list = str.split(" ");
    if (str.mid(0, 2) == "20" && str.contains("-")) {
      date = QDate::fromString(list.at(0), "yyyy-M-d");
      time = QTime::fromString(list.at(1), "HH:mm");
    }

    if (list.count() > 2) {
      date = QDate::currentDate();
      time = QTime::currentTime();
    }

    if ((str.mid(0, 1) == "1" || str.mid(0, 1) == "2" || str.mid(0, 1) == "3" ||
         str.mid(0, 1) == "4" || str.mid(0, 1) == "5" || str.mid(0, 1) == "6" ||
         str.mid(0, 1) == "7") &&
        !str.contains("-")) {
      QString s1 = list.at(0);
      for (int i = 0; i < s1.length(); i++) {
        QString s2 = s1.mid(i, 1);
      }
      date = QDate::currentDate();
      for (int i = 0; i < list.count(); i++) {
        if (list.at(i).contains(":")) {
          time = QTime::fromString(list.at(i), "HH:mm");
          break;
        }
      }
    }

    alarmDate = date;
    alarmTime = time;
  }

  // --------------------------

  isTodoAlarmShow = true;
}

bool Todo::getChkVoice() {
  QString ini_file = privateDir + "msg.ini";
  QSettings Reg(ini_file, QSettings::IniFormat);

  return Reg.value("voice", false).toBool();
}

void Todo::setChkVoice(bool value) {
  QString ini_file = privateDir + "msg.ini";
  QSettings Reg(ini_file, QSettings::IniFormat);

  Reg.setValue("voice", value);
}

void Todo::on_btnTestSpeech() {
  int count = mw_one->m_Todo->getCount();
  if (count == 0) return;
  int row = mw_one->m_Todo->getCurrentIndex();
  if (row < 0) return;

  bool isVoice = mw_one->m_Todo->isVoice(row);

  if (isVoice) {
    QString voiceFile = mw_one->m_Todo->getVoiceFile(row);
    m_Method->playRecord(voiceFile);
  } else {
    QString txt = mw_one->m_Todo->getItemTodoText(row);
    isPlayBook = false;
    m_Method->stopPlayMyText();
    m_Method->playMyText(txt);
  }
}

void Todo::setAlarmShowValue(bool value) { isTodoAlarmShow = value; }

void Todo::showInputPanel() {}
