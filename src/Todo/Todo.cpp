#include "Todo.h"

#include "src/MainWindow.h"
#include "ui_MainWindow.h"
#include "ui_Todo.h"

QString highLblStyle = "color:rgb(212,35,122)";
int highCount;
QString orgLblStyle;

extern MainWindow* mw_one;
extern TextSelector* m_TextSelector;
extern Method* m_Method;
extern QString iniFile, iniDir, privateDir, encPassword, errorInfo;
extern bool loading, isBreak, zh_cn, isDark, isAndroid, isPasswordError,
    isEncrypt;
extern int fontSize;

extern WebDavHelper* listWebDavFiles(const QString& url,
                                     const QString& username,
                                     const QString& password);
extern CloudBackup* m_CloudBackup;

bool isNeedSync = false;

Todo::Todo(QWidget* parent) : QDialog(parent), ui(new Ui::Todo) {
  ui->setupUi(this);

  this->installEventFilter(this);
  mw_one->ui->editTodo->viewport()->installEventFilter(mw_one);

  this->setModal(true);

  QString strTar = "/data/data/com.x/files/msg.mp3";
  QFile::copy(":/res/msg.mp3", strTar);

  mw_one->ui->editTodo->setContentsMargins(12, 0, 12, 0);

  QFont f = this->font();
  f.setPointSize(12);
  mw_one->ui->btnAddTodo->setFont(f);
  mw_one->ui->btnBackTodo->setFont(f);
  mw_one->ui->btnHigh->setFont(f);
  mw_one->ui->btnLow->setFont(f);
  mw_one->ui->btnModify->setFont(f);
  mw_one->ui->btnSetTime->setFont(f);
  mw_one->ui->btnRecycle->setFont(f);

  mw_one->ui->btnPasteTodo->hide();
  mw_one->ui->progAudioBar->hide();
  mw_one->ui->sliderPlayAudio->hide();

  mw_one->ui->editTodo->setFixedHeight(getEditTextHeight(mw_one->ui->editTodo) +
                                       4);

  tmeRecordTime = new QTimer(this);
  connect(tmeRecordTime, SIGNAL(timeout()), this, SLOT(on_ShowRecordTime()));

  tmePlayProgress = new QTimer(this);
  connect(tmePlayProgress, SIGNAL(timeout()), this,
          SLOT(on_ShowPlayProgress()));

  QScroller::grabGesture(mw_one->ui->editTodo,
                         QScroller::LeftMouseButtonGesture);
  m_Method->setSCrollPro(mw_one->ui->editTodo);
}

Todo::~Todo() { delete ui; }

void Todo::keyReleaseEvent(QKeyEvent* event) { Q_UNUSED(event); }

void Todo::saveTodo() {
  if (!isNeedSave) return;

  isNeedSave = false;

  mw_one->isNeedAutoBackup = true;
  mw_one->strLatestModify = tr("Modi Todo");

  highCount = 0;

  int count_items = getCount();

  QString todoFile = iniDir + "todo.ini";
  iniTodo = new QSettings(todoFile, QSettings::IniFormat, this);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  iniTodo->setIniCodec("utf-8");
#endif

  iniTodo->setValue("/Todo/Count", count_items);
  for (int i = 0; i < count_items; i++) {
    QString strText = getItemTodoText(i);
    QString strTime = getItemTime(i);
    int type = getItemType(i);
    iniTodo->setValue("/Todo/Item" + QString::number(i), strText);
    iniTodo->setValue("/Todo/Time" + QString::number(i), strTime);
    iniTodo->setValue("/Todo/Type" + QString::number(i), type);
  }

  int count1 = getCountRecycle();
  iniTodo->setValue("/Todo/Count1", count1);
  for (int i = 0; i < count1; i++) {
    QString doneTime = getItemTimeRecycle(i);
    QString str = getItemTodoTextRecycle(i);
    iniTodo->setValue("/Todo/ItemRecycle" + QString::number(i), str);
    iniTodo->setValue("/Todo/ItemRecycleDoneTime" + QString::number(i),
                      doneTime);
  }

  isNeedSync = true;
}

void Todo::init_Todo() {
  clearAll();

  iniTodo = new QSettings(iniDir + "todo.ini", QSettings::IniFormat, this);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  iniTodo->setIniCodec("utf-8");
#endif

  int count = iniTodo->value("/Todo/Count").toInt();
  for (int i = 0; i < count; i++) {
    QString str = iniTodo->value("/Todo/Item" + QString::number(i)).toString();
    QString strTime =
        iniTodo->value("/Todo/Time" + QString::number(i)).toString();
    int type = iniTodo->value("/Todo/Type" + QString::number(i)).toInt();

    addItem(strTime, type, str);
  }

  clearAllRecycle();
  int count1 = iniTodo->value("/Todo/Count1").toInt();
  for (int i = 0; i < count1; i++) {
    QString doneTime =
        iniTodo->value("/Todo/ItemRecycleDoneTime" + QString::number(i))
            .toString();
    QString str =
        iniTodo->value("/Todo/ItemRecycle" + QString::number(i)).toString();
    addItemRecycle(doneTime, 0, str);
  }

  refreshTableLists();
  refreshAlarm();
}

void Todo::addToList(QString str) {
  if (str == "") return;

  int count = getCount();
  for (int i = 0; i < count; i++) {
    QString strTodo = getItemTodoText(i);

    if (str == strTodo) {
      setCurrentIndex(i);
      return;
    }
  }

  QString strTime = QDateTime::currentDateTime().toString();
  insertItem(strTime, 0, str, 0);

  setCurrentIndex(0);
  refreshTableLists();
  isNeedSave = true;
  saveTodo();
}

void Todo::on_btnAdd_clicked() {
  QString str = mw_one->ui->editTodo->toPlainText().trimmed();
  if (str == "") return;
  addToList(str);
  mw_one->ui->editTodo->setText("");
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
  m_Method->closeKeyboard();
  stopPlayVoice();
  saveTodo();
  mw_one->ui->frameTodo->hide();
  mw_one->ui->frameMain->show();

  refreshTableLists();
  refreshAlarm();
  mw_one->ui->qwTodo->rootContext()->setContextProperty("isBtnVisible", false);

  if (isNeedSync && mw_one->ui->chkAutoSync->isChecked() &&
      mw_one->ui->chkWebDAV->isChecked()) {
    QString todoFile = iniDir + "todo.ini";
    QString todoZipFile = privateDir + "KnotData/todo.ini.zip";

    // if (!m_Method->compressFile(todoZipFile, todoFile, encPassword)) {

    if (!m_Method->compressFileWithZlib(todoFile, todoZipFile,
                                        Z_DEFAULT_COMPRESSION)) {
      errorInfo = tr("An error occurred while compressing the file.");
      ShowMessage* msg = new ShowMessage(this);
      msg->showMsg("Knot", errorInfo, 1);
      return;
    }

    QString enc_file = m_Method->useEnc(todoZipFile);
    if (enc_file != "") todoZipFile = enc_file;

    QStringList files;
    files.append(todoZipFile);
    m_CloudBackup->uploadFilesToWebDAV(files);
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

void Todo::on_btnHigh_clicked() {
  int count = getCount();
  if (count == 0) return;

  int row = getCurrentIndex();

  QString strTime = getItemTime(row);

  if (getTimeStr(strTime) != "") return;

  QString strText = getItemTodoText(row);

  delItem(row);
  insertItem(strTime, 1, strText, 0);
  setCurrentIndex(0);

  refreshAlarm();
  isNeedSave = true;
  saveTodo();
}

void Todo::on_btnLow_clicked() {
  int count = getCount();
  if (count == 0) return;

  int row = getCurrentIndex();

  QString strTime = getItemTime(row);

  if (getTimeStr(strTime) != "") return;

  QString strTodoText = getItemTodoText(row);
  delItem(row);
  addItem(strTime, 0, strTodoText);

  refreshAlarm();

  setCurrentIndex(getCount() - 1);

  isNeedSave = true;
  saveTodo();
}

void Todo::on_SetAlarm() {
  int row = getCurrentIndex();
  if (row < 0) return;

  QString strTodoText = getItemTodoText(row);
  QString strTime;

  if (!mw_one->m_TodoAlarm->ui->chk1->isChecked() &&
      !mw_one->m_TodoAlarm->ui->chk2->isChecked() &&
      !mw_one->m_TodoAlarm->ui->chk3->isChecked() &&
      !mw_one->m_TodoAlarm->ui->chk4->isChecked() &&
      !mw_one->m_TodoAlarm->ui->chk5->isChecked() &&
      !mw_one->m_TodoAlarm->ui->chk6->isChecked() &&
      !mw_one->m_TodoAlarm->ui->chk7->isChecked()) {
    mw_one->m_TodoAlarm->ui->chkDaily->setChecked(false);
  }

  QString str;
  if (mw_one->m_TodoAlarm->ui->chk1->isChecked()) str = str + "1";
  if (mw_one->m_TodoAlarm->ui->chk2->isChecked()) str = str + "2";
  if (mw_one->m_TodoAlarm->ui->chk3->isChecked()) str = str + "3";
  if (mw_one->m_TodoAlarm->ui->chk4->isChecked()) str = str + "4";
  if (mw_one->m_TodoAlarm->ui->chk5->isChecked()) str = str + "5";
  if (mw_one->m_TodoAlarm->ui->chk6->isChecked()) str = str + "6";
  if (mw_one->m_TodoAlarm->ui->chk7->isChecked()) str = str + "7";
  if (str.length() > 0) {
    strTime = tr("Alarm") + "  " + str + "  " +
              mw_one->m_TodoAlarm->ui->dateTimeEdit->time().toString("HH:mm");

  } else {
    strTime =
        tr("Alarm") + "  " + mw_one->m_TodoAlarm->ui->dateTimeEdit->text();
  }

  delItem(row);
  insertItem(strTime, 0, strTodoText, row);
  setCurrentIndex(row);

  ui->frameSetTime->hide();
  mw_one->m_TodoAlarm->close();

  refreshTableLists();
  refreshAlarm();
  isNeedSave = true;
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

void Todo::on_btnSetTime_clicked() {
  int count = getCount();
  if (count == 0) return;

  int row = getCurrentIndex();

  delete mw_one->m_TodoAlarm;
  mw_one->m_TodoAlarm = new TodoAlarm(this);

  QString str = getItemTime(row);
  QDate date;
  QTime time;
  mw_one->m_TodoAlarm->ui->chk1->setChecked(false);
  mw_one->m_TodoAlarm->ui->chk2->setChecked(false);
  mw_one->m_TodoAlarm->ui->chk3->setChecked(false);
  mw_one->m_TodoAlarm->ui->chk4->setChecked(false);
  mw_one->m_TodoAlarm->ui->chk5->setChecked(false);
  mw_one->m_TodoAlarm->ui->chk6->setChecked(false);
  mw_one->m_TodoAlarm->ui->chk7->setChecked(false);
  mw_one->m_TodoAlarm->ui->chkDaily->setChecked(false);

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
        if (s2 == "1") mw_one->m_TodoAlarm->ui->chk1->setChecked(true);
        if (s2 == "2") mw_one->m_TodoAlarm->ui->chk2->setChecked(true);
        if (s2 == "3") mw_one->m_TodoAlarm->ui->chk3->setChecked(true);
        if (s2 == "4") mw_one->m_TodoAlarm->ui->chk4->setChecked(true);
        if (s2 == "5") mw_one->m_TodoAlarm->ui->chk5->setChecked(true);
        if (s2 == "6") mw_one->m_TodoAlarm->ui->chk6->setChecked(true);
        if (s2 == "7") mw_one->m_TodoAlarm->ui->chk7->setChecked(true);
      }
      date = QDate::currentDate();

      for (int i = 0; i < list.count(); i++) {
        if (list.at(i).contains(":")) {
          time = QTime::fromString(list.at(i), "HH:mm");
          break;
        }
      }

      mw_one->m_TodoAlarm->ui->chkDaily->setChecked(true);
    }

    mw_one->m_TodoAlarm->ui->dateTimeEdit->setDate(date);
    mw_one->m_TodoAlarm->ui->dateTimeEdit->setTime(time);

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
        if (s2 == "1") mw_one->m_TodoAlarm->ui->chk1->setChecked(true);
        if (s2 == "2") mw_one->m_TodoAlarm->ui->chk2->setChecked(true);
        if (s2 == "3") mw_one->m_TodoAlarm->ui->chk3->setChecked(true);
        if (s2 == "4") mw_one->m_TodoAlarm->ui->chk4->setChecked(true);
        if (s2 == "5") mw_one->m_TodoAlarm->ui->chk5->setChecked(true);
        if (s2 == "6") mw_one->m_TodoAlarm->ui->chk6->setChecked(true);
        if (s2 == "7") mw_one->m_TodoAlarm->ui->chk7->setChecked(true);
      }
      date = QDate::currentDate();
      for (int i = 0; i < list.count(); i++) {
        if (list.at(i).contains(":")) {
          time = QTime::fromString(list.at(i), "HH:mm");
          break;
        }
      }

      mw_one->m_TodoAlarm->ui->chkDaily->setChecked(true);
    }

    mw_one->m_TodoAlarm->ui->dateTimeEdit->setDate(date);
    mw_one->m_TodoAlarm->ui->dateTimeEdit->setTime(time);
  }

  mw_one->m_TodoAlarm->initDlg();
  currentTodoItem = getItemTodoText(row);
  QString txt = tr("Todo") + " : " + currentTodoItem;
  txt = txt.replace("\n", " ");
  QFontMetrics fm(this->font());
  QString qsLine = fm.elidedText(txt, Qt::ElideRight, mw_one->width() - 10);
  mw_one->m_TodoAlarm->ui->lblTodoText->setText(qsLine);

  mw_one->m_TodoAlarm->m_datePicker->setDate(date);
  mw_one->m_TodoAlarm->m_timePicker->setTime(time);
  mw_one->m_TodoAlarm->show();
}

void Todo::on_DelAlarm() {
  int row = getCurrentIndex();
  if (row < 0) return;

  QString str = getItemTime(row);
  QString str1 = str;
  str = getTimeStr(str);
  if (str != "") str1 = str;
  modifyTime(row, str1);
  modifyType(row, 0);
  ui->frameSetTime->hide();
  mw_one->m_TodoAlarm->close();

  refreshTableLists();
  refreshAlarm();
  isNeedSave = true;
  saveTodo();

  goCurrentTodoItem(currentTodoItem);
}

void Todo::startTimerAlarm(QString text) {
  Q_UNUSED(text);
#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QAndroidJniObject jo = QAndroidJniObject::fromString(text);
  // jo.callStaticMethod<int>("com.x/MyService", "startTimerAlarm", "()I");

  jo.callStaticMethod<int>("com.x/MyActivity", "startAlarm",
                           "(Ljava/lang/String;)I", jo.object<jstring>());

  jo.callStaticMethod<int>("com.x/ClockActivity", "setInfoText",
                           "(Ljava/lang/String;)I", jo.object<jstring>());
#else
  QJniObject jo = QJniObject::fromString(text);

  jo.callStaticMethod<int>("com.x/MyActivity", "startAlarm",
                           "(Ljava/lang/String;)I", jo.object<jstring>());

  jo.callStaticMethod<int>("com.x/ClockActivity", "setInfoText",
                           "(Ljava/lang/String;)I", jo.object<jstring>());
#endif

#endif
}

void Todo::stopTimerAlarm() {
#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QAndroidJniObject jo = QAndroidJniObject::fromString("stopTimerAlarm");
  // jo.callStaticMethod<int>("com.x/MyService", "stopTimerAlarm", "()I");

  jo.callStaticMethod<int>("com.x/MyActivity", "stopAlarm", "()I");
#else
  QJniObject jo = QJniObject::fromString("stopTimerAlarm");

  jo.callStaticMethod<int>("com.x/MyActivity", "stopAlarm", "()I");
#endif

#endif
}

void Todo::sendMsgAlarm(QString text) {
  Q_UNUSED(text);
#ifdef Q_OS_ANDROID
  QString strNotify = tr("Todo") + " : " + text;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QAndroidJniObject javaNotification = QAndroidJniObject::fromString(strNotify);
  QAndroidJniObject::callStaticMethod<void>(
      "com/x/MyService", "notifyTodoAlarm",
      "(Landroid/content/Context;Ljava/lang/String;)V",
      QtAndroid::androidContext().object(), javaNotification.object<jstring>());
#else
  QJniObject javaNotification = QJniObject::fromString(strNotify);
  QJniObject::callStaticMethod<void>(
      "com/x/MyService", "notifyTodoAlarm",
      "(Landroid/content/Context;Ljava/lang/String;)V",
      QNativeInterface::QAndroidApplication::context(),
      javaNotification.object<jstring>());
#endif

#endif
}

void Todo::on_btnRecycle_clicked() {
  mw_one->ui->frameTodo->hide();
  mw_one->ui->frameTodoRecycle->show();
}

void Todo::on_btnReturn_clicked() {
  mw_one->ui->frameTodoRecycle->hide();
  mw_one->ui->frameTodo->show();
}

void Todo::on_btnClear_clicked() {
  clearAllRecycle();

  isNeedSave = true;
  saveTodo();
}

void Todo::on_btnRestore_clicked() {
  if (getCountRecycle() == 0) return;

  int row = getCurrentIndexRecycle();
  QString strTime = QDate::currentDate().toString("ddd MM dd yyyy") + "  " +
                    QTime::currentTime().toString();
  QString strText = getItemTodoTextRecycle(row);
  addItem(strTime, 0, strText);

  isRestore = true;
  on_btnDel_clicked();

  setCurrentIndex(getCount() - 1);

  isNeedSave = true;
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

  isNeedSave = true;
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

void Todo::refreshTableListsFromIni() {
  tableLists.clear();

  iniTodo = new QSettings(iniDir + "todo.ini", QSettings::IniFormat, this);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  iniTodo->setIniCodec("utf-8");
#endif

  int count_items = iniTodo->value("/Todo/Count", 0).toInt();

  for (int i = 0; i < count_items; i++) {
    QString strTime =
        iniTodo->value("/Todo/Time" + QString::number(i)).toString();
    QString strText =
        iniTodo->value("/Todo/Item" + QString::number(i)).toString();

    tableLists.append(strTime + "|=|" + strText);
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

  QString ini_file = "/data/data/com.x/files/msg.ini";
  QSettings Reg(ini_file, QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  Reg.setIniCodec("utf-8");
#endif

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
          QString strDate = str.split(" ").at(0);
          QString strToday = QDate::currentDate().toString("yyyy-M-d");
          QDateTime ctime = QDateTime::currentDateTime();
          QString strTmo = ctime.addDays(+1).toString("yyyy-M-d");
          if (strDate.contains("-")) {
            if (strDate == strToday) {
              modifyType(i, 1);
              isToday = true;
            }

            if (strTmo == strDate) {
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
        startTimerAlarm(str1);
        Reg.setValue("msg", str1);

        // isToDay?
        QDate todayDate = QDate::currentDate();
        QDate tomoDate = todayDate.addDays(1);
        QDateTime tomoDateTime =
            QDateTime::fromString(tomoDate.toString() + " 00:00:00");
        qint64 current_s = QDateTime::currentDateTimeUtc().toSecsSinceEpoch();
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

  Reg.setValue("count", count);
  QString strMute = "true";
  Reg.setValue("mute", strMute);

  if (!QFile(ini_file).exists())
    qDebug() << "ini no exists";
  else
    qDebug() << "ini ok";
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

void Todo::on_editTodo_textChanged() {
  int h = getEditTextHeight(mw_one->ui->editTodo) + 4;
  int ui_h = mw_one->ui->frameTodo->height();
  if (h > ui_h / 2) h = ui_h / 2;
  mw_one->ui->editTodo->setFixedHeight(h);
}

void Todo::insertItem(QString strTime, int type, QString strText,
                      int curIndex) {
  int itemheight = setItemHeight(strText);

  QQuickItem* root = mw_one->ui->qwTodo->rootObject();
  QMetaObject::invokeMethod(
      (QObject*)root, "insertItem", Q_ARG(QVariant, strTime),
      Q_ARG(QVariant, type), Q_ARG(QVariant, strText),
      Q_ARG(QVariant, itemheight), Q_ARG(QVariant, curIndex));
}

void Todo::insertRecycle(QString strTime, int type, QString strText,
                         int curIndex) {
  int itemheight = setItemHeight(strText);

  QQuickItem* root = mw_one->ui->qwRecycle->rootObject();
  QMetaObject::invokeMethod(
      (QObject*)root, "insertRecycle", Q_ARG(QVariant, strTime),
      Q_ARG(QVariant, type), Q_ARG(QVariant, strText),
      Q_ARG(QVariant, itemheight), Q_ARG(QVariant, curIndex));
}

int Todo::getCurrentIndex() {
  QQuickItem* root = mw_one->ui->qwTodo->rootObject();
  QVariant itemIndex;
  QMetaObject::invokeMethod((QObject*)root, "getCurrentIndex",
                            Q_RETURN_ARG(QVariant, itemIndex));
  return itemIndex.toInt();
}

int Todo::getCurrentIndexRecycle() {
  QQuickItem* root = mw_one->ui->qwRecycle->rootObject();
  QVariant itemIndex;
  QMetaObject::invokeMethod((QObject*)root, "getCurrentIndex",
                            Q_RETURN_ARG(QVariant, itemIndex));
  return itemIndex.toInt();
}

QString Todo::getItemTime(int index) {
  QQuickItem* root = mw_one->ui->qwTodo->rootObject();
  QVariant itemTime;
  QMetaObject::invokeMethod((QObject*)root, "getTime",
                            Q_RETURN_ARG(QVariant, itemTime),
                            Q_ARG(QVariant, index));
  return itemTime.toString();
}

QString Todo::getItemTimeRecycle(int index) {
  QQuickItem* root = mw_one->ui->qwRecycle->rootObject();
  QVariant itemTime;
  QMetaObject::invokeMethod((QObject*)root, "getTime",
                            Q_RETURN_ARG(QVariant, itemTime),
                            Q_ARG(QVariant, index));
  return itemTime.toString();
}

int Todo::getItemType(int index) {
  QQuickItem* root = mw_one->ui->qwTodo->rootObject();
  QVariant itemType;
  QMetaObject::invokeMethod((QObject*)root, "getType",
                            Q_RETURN_ARG(QVariant, itemType),
                            Q_ARG(QVariant, index));
  return itemType.toInt();
}

QString Todo::getItemTodoText(int index) {
  QQuickItem* root = mw_one->ui->qwTodo->rootObject();
  QVariant itemTodoText;
  QMetaObject::invokeMethod((QObject*)root, "getTodoText",
                            Q_RETURN_ARG(QVariant, itemTodoText),
                            Q_ARG(QVariant, index));
  return itemTodoText.toString();
}

QString Todo::getItemTodoTextRecycle(int index) {
  QQuickItem* root = mw_one->ui->qwRecycle->rootObject();
  QVariant itemTodoText;
  QMetaObject::invokeMethod((QObject*)root, "getTodoText",
                            Q_RETURN_ARG(QVariant, itemTodoText),
                            Q_ARG(QVariant, index));
  return itemTodoText.toString();
}

void Todo::delItem(int index) {
  QQuickItem* root = mw_one->ui->qwTodo->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "delItem", Q_ARG(QVariant, index));
}

void Todo::delItemRecycle(int index) {
  QQuickItem* root = mw_one->ui->qwRecycle->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "delItem", Q_ARG(QVariant, index));
}

void Todo::addItem(QString strTime, int type, QString strText) {
  int itemheight = setItemHeight(strText);

  QQuickItem* root = mw_one->ui->qwTodo->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "addItem", Q_ARG(QVariant, strTime),
                            Q_ARG(QVariant, type), Q_ARG(QVariant, strText),
                            Q_ARG(QVariant, itemheight));
}

void Todo::addItemRecycle(QString strTime, int type, QString strText) {
  int itemheight = setItemHeight(strText);

  QQuickItem* root = mw_one->ui->qwRecycle->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "addItem", Q_ARG(QVariant, strTime),
                            Q_ARG(QVariant, type), Q_ARG(QVariant, strText),
                            Q_ARG(QVariant, itemheight));
}

void Todo::setCurrentIndex(int index) {
  QQuickItem* root = mw_one->ui->qwTodo->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "setCurrentItem",
                            Q_ARG(QVariant, index));
}

void Todo::setHighPriority(bool isBool) {
  QQuickItem* root = mw_one->ui->qwTodo->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "setHighPriority",
                            Q_ARG(QVariant, isBool));
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

int Todo::getCount() {
  QQuickItem* root = mw_one->ui->qwTodo->rootObject();
  QVariant itemCount;
  QMetaObject::invokeMethod((QObject*)root, "getItemCount",
                            Q_RETURN_ARG(QVariant, itemCount));
  return itemCount.toInt();
}

int Todo::getCountRecycle() {
  QQuickItem* root = mw_one->ui->qwRecycle->rootObject();
  QVariant itemCount;
  QMetaObject::invokeMethod((QObject*)root, "getItemCount",
                            Q_RETURN_ARG(QVariant, itemCount));
  return itemCount.toInt();
}

void Todo::modifyTime(int index, QString strTime) {
  QQuickItem* root = mw_one->ui->qwTodo->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "modifyItemTime",
                            Q_ARG(QVariant, index), Q_ARG(QVariant, strTime));
}

void Todo::modifyType(int index, int type) {
  QQuickItem* root = mw_one->ui->qwTodo->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "modifyItemType",
                            Q_ARG(QVariant, index), Q_ARG(QVariant, type));
}

void Todo::modifyTodoText(int index, QString strTodoText) {
  QQuickItem* root = mw_one->ui->qwTodo->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "modifyItemText",
                            Q_ARG(QVariant, index),
                            Q_ARG(QVariant, strTodoText));
}

void Todo::clearAll() {
  int count = getCount();
  for (int i = 0; i < count; i++) {
    delItem(0);
  }
}

void Todo::clearAllRecycle() {
  int count = getCountRecycle();
  for (int i = 0; i < count; i++) {
    delVoiceFile(0);
    delItemRecycle(0);
  }
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
      m_Method->playRecord(iniDir + "memo/voice/" + getNumber(strItem) +
                           ".aac");
      mw_one->ui->progAudioBar->setStyleSheet(
          "QProgressBar{background:white;} "
          "QProgressBar::chunk{background:#1E90FF}");

      mw_one->ui->sliderPlayAudio->setValue(0);
      mw_one->ui->sliderPlayAudio->setMaximum(m_Method->getPlayDuration());
      mw_one->ui->sliderPlayAudio->show();
      tmePlayProgress->start(nInterval);
      return;
    }
  }

  QDialog* dlg = new QDialog(this);
  QVBoxLayout* vbox0 = new QVBoxLayout;
  dlg->setLayout(vbox0);
  vbox0->setContentsMargins(5, 5, 5, 5);
  dlg->setModal(true);
  dlg->setWindowFlag(Qt::FramelessWindowHint);

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
  vbox->addWidget(edit);
  edit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  edit->setPlainText(strItem);

  if (isAndroid) {
    QScroller::grabGesture(edit, QScroller::LeftMouseButtonGesture);
    m_Method->setSCrollPro(edit);
  }

  edit->horizontalScrollBar()->setHidden(true);
  edit->verticalScrollBar()->setStyleSheet(
      mw_one->ui->editDetails->verticalScrollBar()->styleSheet());

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

  QSpacerItem* sparcer_item =
      new QSpacerItem(0, 2, QSizePolicy::Fixed, QSizePolicy::Fixed);
  vbox->addItem(sparcer_item);

  vbox->addLayout(hbox, 0);

#ifdef Q_OS_ANDROID
  btnShare->show();
#else
  btnShare->hide();
#endif

  connect(btnCancel, &QToolButton::clicked, [=]() mutable { dlg->close(); });
  connect(dlg, &QDialog::rejected,
          [=]() mutable { m_Method->closeGrayWindows(); });
  connect(dlg, &QDialog::accepted,
          [=]() mutable { m_Method->closeGrayWindows(); });
  connect(btnCopy, &QToolButton::clicked, [=]() mutable {
    edit->selectAll();
    edit->copy();
    dlg->close();
  });
  connect(btnShare, &QToolButton::clicked, [=]() mutable {
    QString txt = edit->toPlainText().trimmed();
    if (txt.length() > 0) {
      mw_one->m_ReceiveShare->shareString(tr("Share to"), txt);
    }
  });
  connect(btnOk, &QToolButton::clicked, [=]() mutable {
    QString strTime = getItemTime(row);
    int type = getItemType(row);
    delItem(row);
    insertItem(strTime, type, edit->toPlainText().trimmed(), row);
    setCurrentIndex(row);
    isNeedSave = true;
    saveTodo();
    dlg->close();
  });

  int x, y, w, h;
  h = mw_one->height() / 2;
  if (isAndroid) {
    w = mw_one->width() - 2;
    y = mw_one->geometry().y();
  } else {
    w = mw_one->width() / 2;
    if (w < dlg->width()) w = dlg->width();

    y = mw_one->geometry().y() + (mw_one->height() - h) / 2;
  }
  x = mw_one->geometry().x() + (mw_one->width() - w) / 2;

  dlg->setGeometry(x, y, w, h);
  dlg->setModal(true);
  m_Method->showGrayWindows();
  mw_one->set_ToolButtonStyle(dlg);
  dlg->show();
}

void Todo::addToRecycle() {
  int row = getCurrentIndex();
  QString strTodoText = getItemTodoText(row);
  QString doneTime =
      QDateTime::currentDateTime().toString() + "  " + tr("Done");

  insertRecycle(doneTime, 0, strTodoText, 0);

  isNeedSave = true;
}

void Todo::NewTodo() { mw_one->ui->btnTodo->click(); }

void Todo::startRecordVoice() {
  if (mw_one->ui->editTodo->toPlainText().trimmed().length() == 0) {
    if (isAudioRecordOne) return;
    isAudioRecordOne = true;
    stopPlayVoice();
    QString dir = iniDir + "memo/voice/";
    QDir mdir;
    mdir.mkpath(dir);
    audioFileName = tr("Voice") + " " + QDateTime::currentDateTime().toString();
    QString str = audioFileName;
    audioFilePath = dir + getNumber(str) + ".aac";
    m_Method->startRecord(audioFilePath);

    editStyle = mw_one->ui->editTodo->styleSheet();
    mw_one->ui->editTodo->setStyleSheet(
        "QTextEdit{background-color: #FF0000; color: white; border:1px solid "
        "#FFFFFF;}");

    mw_one->ui->editTodo->setText(tr("Recording audio in progress..."));
    nRecordSec = 0;
    tmeRecordTime->start(250);
    mw_one->ui->progAudioBar->setStyleSheet(
        "QProgressBar{background:white;} "
        "QProgressBar::chunk{background:#00FF7F}");
    mw_one->ui->progAudioBar->setMaximum(100);
    mw_one->ui->progAudioBar->show();
    isRecordVoice = true;
  }
}

void Todo::on_ShowRecordTime() {
  double db = m_Method->updateMicStatus();
  // qDebug() << "db=" << db;
  mw_one->ui->progAudioBar->setValue(db);

  nMSec = nMSec + 1;
  if (nMSec == 4) {
    nRecordSec = nRecordSec + 1;
    nMSec = 0;
  }
  mw_one->ui->editTodo->setText(tr("Recording audio in progress...") + " " +
                                m_Method->FormatHHMMSS(nRecordSec));
}

void Todo::stopRecordVoice() {
  if (isRecordVoice) {
    mw_one->ui->editTodo->setText("");
    tmeRecordTime->stop();
    nRecordSec = 0;

    m_Method->stopRecord();
    QFile file(audioFilePath);
    if (file.exists()) {
      if (file.size() > 0) {
        mw_one->ui->editTodo->setText(audioFileName);
        mw_one->on_btnAddTodo_clicked();
      } else {
        file.remove();
      }
    }
    isAudioRecordOne = false;
    mw_one->ui->editTodo->setStyleSheet(editStyle);
  }

  mw_one->ui->progAudioBar->hide();
}

void Todo::stopPlayVoice() {
  m_Method->stopPlayRecord();
  tmePlayProgress->stop();
  mw_one->ui->sliderPlayAudio->hide();

  int row = getCurrentIndex();
  if (row >= 0) {
    if (isVoice(row))
      mw_one->ui->btnModify->setIcon(QIcon(":/res/voice_l.svg"));
    else
      mw_one->ui->btnModify->setIcon(QIcon(":/res/edit_l.svg"));
  }
}

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

QString Todo::getVoiceFile(int row) {
  QString strItem = getItemTodoText(row).trimmed();
  QStringList list0 = strItem.split(" ");
  if (list0.count() > 0) {
    QString str = list0.at(0);
    if (str == tr("Voice")) {
      QString voiceFile = iniDir + "memo/voice/" + getNumber(strItem) + ".aac";
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
      QString voiceFile = iniDir + "memo/voice/" + getNumber(strItem) + ".aac";
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

void Todo::on_ShowPlayProgress() {
  int prog = m_Method->getPlayPosition();
  mw_one->ui->sliderPlayAudio->setValue(prog);

  if (!m_Method->getPlaying()) {
    stopPlayVoice();
  }
}

void Todo::openTodoUI() {
  mw_one->ui->qwTodo->rootContext()->setContextProperty("m_width",
                                                        mw_one->width());
  mw_one->ui->frameMain->hide();
  mw_one->ui->frameTodo->show();
  init_Todo();

  refreshAlarm();
  setCurrentIndex(0);
  stopPlayVoice();

  mw_one->closeProgress();
}

void Todo::openTodo() {
  m_TextSelector->close();
  m_TextSelector = new TextSelector(mw_one);
  isPasswordError = false;

  if (mw_one->ui->chkAutoSync->isChecked() &&
      mw_one->ui->chkWebDAV->isChecked()) {
    mw_one->showProgress();

    m_CloudBackup->createRemoteWebDAVDir();

    QString url = m_CloudBackup->getWebDAVArgument();

    WebDavHelper* helper =
        listWebDavFiles(url + "KnotData/", m_CloudBackup->USERNAME,
                        m_CloudBackup->APP_PASSWORD);

    // 连接信号
    QObject::connect(
        helper, &WebDavHelper::listCompleted,
        [](const QList<QPair<QString, QDateTime>>& files) {
          qDebug() << "获取到文件列表:";
          qDebug() << "共找到" << files.size() << "个文件:";

          if (files.size() == 0) {
            mw_one->m_Todo->openTodoUI();
            return;
          }

          bool isTodoFile = false;
          for (const auto& [path, mtime] : files) {
            qDebug() << "路径:" << path
                     << "修改时间:" << mtime.toString("yyyy-MM-dd hh:mm:ss");
            QString remoteFile = path;
            remoteFile = remoteFile.replace("/dav/", "");  // 此处需注意
            if (remoteFile.contains("todo.ini.zip")) {
              isTodoFile = true;
              QString localFile = privateDir + "KnotData/todo.ini.zip";
              QDateTime localModi = QFileInfo(localFile).lastModified();
              if (mtime > localModi || !QFile::exists(localFile)) {
                // 初始化下载器
                WebDavDownloader* downloader = new WebDavDownloader(
                    m_CloudBackup->USERNAME, m_CloudBackup->APP_PASSWORD);

                // 连接信号
                QObject::connect(downloader, &WebDavDownloader::progressChanged,
                                 [](int current, int total, QString file) {
                                   qDebug()
                                       << QString("进度: %1/%2  当前文件: %3")
                                              .arg(current)
                                              .arg(total)
                                              .arg(file);
                                 });

                QObject::connect(
                    downloader, &WebDavDownloader::downloadFinished,
                    [](bool success, QString error) {
                      qDebug() << (success ? "下载成功" : "下载失败: " + error);
                      QString zFile = privateDir + "KnotData/todo.ini.zip";

                      QString dec_file = m_Method->useDec(zFile);
                      if (dec_file != "") zFile = dec_file;

                      errorInfo = "";
                      if (!m_Method->decompressFileWithZlib(
                              zFile, privateDir + "KnotData/todo.ini")) {
                        mw_one->closeProgress();
                        errorInfo =
                            tr("Decompression failed. Please check in "
                               "Preferences that the passwords are consistent "
                               "across all platforms.");

                        ShowMessage* msg = new ShowMessage();
                        msg->showMsg("Knot", errorInfo, 1);
                        isPasswordError = true;
                        QFile::remove(zFile);
                        return;
                      }

                      //  m_Method->decompressWithPassword(
                      //     zFile, privateDir + "KnotData", encPassword);

                      QString zipToto = privateDir + "KnotData/todo.ini";
                      QString localTodo = iniDir + "todo.ini";

                      if (isPasswordError == false) {
                        if (QFileInfo(zipToto).lastModified() >
                            QFileInfo(localTodo).lastModified()) {
                          QFile::remove(localTodo);
                          QFile::copy(zipToto, localTodo);
                        }
                      } else {
                        QFile::remove(zFile);
                      }

                      mw_one->m_Todo->openTodoUI();
                    });

                // 需要下载的文件列表
                QList<QString> remoteFiles = {remoteFile};

                // 开始下载（1并发,根据文件的下载个数）
                QString lf = privateDir;
                qDebug() << "lf=" << lf;
                downloader->downloadFiles(remoteFiles, lf, 1);
              }

              if (mtime <= localModi) mw_one->m_Todo->openTodoUI();
              break;
            }
          }

          if (isTodoFile == false) mw_one->m_Todo->openTodoUI();
        });

    QObject::connect(helper, &WebDavHelper::errorOccurred,
                     [](const QString& error) {
                       qDebug() << "操作失败:" << error;
                       mw_one->m_Todo->openTodoUI();
                     });

  } else
    openTodoUI();
}
