#include "TodoAlarm.h"

#include "MainWindow.h"
#include "defines.h"
#include "ui_TodoAlarm.h"

TodoAlarm::TodoAlarm(QWidget* parent) : QDialog(parent), ui(new Ui::TodoAlarm) {
  ui->setupUi(this);

  QFont font1 = m_Method->getNewFont(19);

  ui->btnBack->setFont(font1);
  ui->btnDelDT->setFont(font1);
  ui->btnSetDT->setFont(font1);
  ui->btnToday->setFont(font1);
  ui->btnTomorrow->setFont(font1);
  ui->btnNextWeek->setFont(font1);
  ui->btnTestSpeech->setFont(font1);

  font0 = m_Method->getNewFont(16);

  this->setFont(font0);
  ui->chk1->setFont(font0);
  ui->chk2->setFont(font0);
  ui->chk3->setFont(font0);
  ui->chk4->setFont(font0);
  ui->chk5->setFont(font0);
  ui->chk6->setFont(font0);
  ui->chk7->setFont(font0);
  ui->chkDaily->setFont(font0);
  ui->chkSpeech->setFont(font0);

  initDlg();

  ui->lblTodoText->setStyleSheet(mw_one->labelNormalStyleSheet);
  QFont font = this->font();
  font.setBold(true);
  ui->btnYear->setFont(font);
  ui->btnMonth->setFont(font);
  ui->btnDay->setFont(font);
  ui->btnHour->setFont(font);
  ui->btnMinute->setFont(font);

  ui->frameDT->hide();
  ui->btnToday->hide();
  ui->btnTomorrow->hide();
  ui->btnNextWeek->hide();
  ui->lblTodoText->hide();

  QString strStyleChk = mw_one->m_Preferences->chkStyle;
  ui->chk1->setStyleSheet(strStyleChk);
  ui->chk2->setStyleSheet(strStyleChk);
  ui->chk3->setStyleSheet(strStyleChk);
  ui->chk4->setStyleSheet(strStyleChk);
  ui->chk5->setStyleSheet(strStyleChk);
  ui->chk6->setStyleSheet(strStyleChk);
  ui->chk7->setStyleSheet(strStyleChk);
  ui->chkDaily->setStyleSheet(strStyleChk);
  ui->chkSpeech->setStyleSheet(strStyleChk);
  ui->frameDaily->setContentsMargins(10, 1, 10, 1);
  ui->lblSelByWeek->setStyleSheet(ui->lblTodoText->styleSheet());

  m_Method->set_ToolButtonStyle(this);

  if (!isAndroid) {
    ui->chkSpeech->hide();
    ui->btnTestSpeech->hide();

  } else {
    getChkVoice();
  }

  ui->dateTimeEdit->setFixedHeight(45);
}

TodoAlarm::~TodoAlarm() { delete ui; }

void TodoAlarm::initDlg() {
  QString str = ui->dateTimeEdit->text();
  QStringList list = str.split(" ");
  QString strdate = list.at(0);
  QString strTime = list.at(1);
  QStringList list1, list2;
  list1 = strdate.split("-");
  y = list1.at(0);
  m = list1.at(1);
  d = list1.at(2);
  list2 = strTime.split(":");
  h = list2.at(0);
  mm = list2.at(1);

  m_Method->setWinPos(this, 350);

  this->setModal(true);
  this->installEventFilter(this);
  if (!isAndroid) on_btnYear_clicked();
  ui->btnSetDT->setFocus();
}

void TodoAlarm::setBtnTitle() {
  QString strDT;
  strDT = y + "-" + m + "-" + d + " " + h + ":" + mm;
  ui->dateTimeEdit->setDateTime(QDateTime::fromString(strDT, "yyyy-M-d HH:mm"));

  ui->btnYear->setText(y + "\n" + tr("Year"));
  ui->btnMonth->setText(m + "\n" + tr("Month"));
  ui->btnDay->setText(d + "\n" + tr("Day"));
  ui->btnHour->setText(h + "\n" + tr("Hour"));
  ui->btnMinute->setText(mm + "\n" + tr("Minute"));
}

bool TodoAlarm::eventFilter(QObject* obj, QEvent* evn) {
  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
      on_btnBack_clicked();
      return true;
    }
  }

  return QWidget::eventFilter(obj, evn);
}

void TodoAlarm::on_btnBack_clicked() {
  m_Method->stopPlayMyText();
  m_Method->stopPlayRecord();
  close();
}

void TodoAlarm::on_btnYear_clicked() {
  showDatePicker();
  addBtn(QDate::currentDate().year(), 9, 3, tr("Year"), false);
}

void TodoAlarm::on_btnMonth_clicked() {
  showDatePicker();
  addBtn(1, 12, 3, tr("Month"), false);
}

void TodoAlarm::on_btnDay_clicked() {
  showDatePicker();

  int maxDay = 0;
  QString sy = ui->btnYear->text().split("\n").at(0);
  QString sm = ui->btnMonth->text().split("\n").at(0);
  maxDay = mw_one->getMaxDay(sy, sm);

  addBtn(1, maxDay, 6, tr("Day"), true);
}

void TodoAlarm::on_btnHour_clicked() {
  showTimePicker();
  addDial(0, 23, tr("Hour"));
}

void TodoAlarm::on_btnMinute_clicked() {
  showTimePicker();
  addDial(0, 59, tr("Minute"));
}

void TodoAlarm::addBtn(int start, int total, int col, QString flag, bool week) {

}

void TodoAlarm::onBtnClick(QToolButton* btn, QString flag) {}

void TodoAlarm::on_btnDelDT_clicked() {
  mw_one->m_Todo->on_DelAlarm();
  close();
}

void TodoAlarm::on_btnSetDT_clicked() {
  bool w1 = ui->chk1->isChecked();
  bool w2 = ui->chk2->isChecked();
  bool w3 = ui->chk3->isChecked();
  bool w4 = ui->chk4->isChecked();
  bool w5 = ui->chk5->isChecked();
  bool w6 = ui->chk6->isChecked();
  bool w7 = ui->chk7->isChecked();

  int y = ui->dateTimeEdit->date().year();
  int mon = ui->dateTimeEdit->date().month();
  int d = ui->dateTimeEdit->date().day();
  int h = ui->dateTimeEdit->time().hour();
  int m = ui->dateTimeEdit->time().minute();
  mw_one->m_Todo->on_SetAlarm(w1, w2, w3, w4, w5, w6, w7, y, mon, d, h, m);
  close();
}

void TodoAlarm::addDial(int min, int max, QString flag) {}

void TodoAlarm::onDial(QDial* btn, QString flag) {
  if (flag == tr("Hour")) {
    h = QString::number(btn->sliderPosition());
    if (h.length() == 1) h = "0" + h;
  }

  if (flag == tr("Minute")) {
    mm = QString::number(btn->sliderPosition());
    if (mm.length() == 1) mm = "0" + mm;
  }
  setBtnTitle();
}

void TodoAlarm::on_btnToday_clicked() {
  int day = QDate::currentDate().day();
  y = QString::number(QDate::currentDate().year());
  m = QString::number(QDate::currentDate().month());
  d = QString::number(day);

  setBtnTitle();
  if (!isAndroid) on_btnDay_clicked();
}

void TodoAlarm::on_btnTomorrow_clicked() {
  QDateTime time = QDateTime::currentDateTime();
  QString str = time.addDays(+1).toString("yyyy-M-d");
  y = str.split("-").at(0);
  m = str.split("-").at(1);
  d = str.split("-").at(2);

  setBtnTitle();
  if (!isAndroid) on_btnDay_clicked();
}

void TodoAlarm::on_btnNextWeek_clicked() {
  int week = QDate::currentDate().dayOfWeek();
  int x = 8 - week;

  QDateTime time = QDateTime::currentDateTime();
  QString str = time.addDays(+x).toString("yyyy-M-d");

  y = str.split("-").at(0);
  m = str.split("-").at(1);
  d = str.split("-").at(2);

  setBtnTitle();
  if (!isAndroid) on_btnDay_clicked();
}

void TodoAlarm::on_chkDaily_clicked() {
  bool chk = ui->chkDaily->isChecked();
  ui->chk1->setChecked(chk);
  ui->chk2->setChecked(chk);
  ui->chk3->setChecked(chk);
  ui->chk4->setChecked(chk);
  ui->chk5->setChecked(chk);
  ui->chk6->setChecked(chk);
  ui->chk7->setChecked(chk);
}

void TodoAlarm::on_btnTestSpeech_clicked() {
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

void TodoAlarm::on_chkSpeech_clicked() {
  QString ini_file = privateDir + "msg.ini";
  QSettings Reg(ini_file, QSettings::IniFormat);

  if (ui->chkSpeech->isChecked())
    Reg.setValue("voice", "true");
  else
    Reg.setValue("voice", "false");
}

void TodoAlarm::getChkVoice() {
  QString ini_file = privateDir + "msg.ini";
  QSettings Reg(ini_file, QSettings::IniFormat);

  QString strVoice = Reg.value("voice").toString();
  qDebug() << "strVoice" << strVoice;
  if (strVoice == "true")
    ui->chkSpeech->setChecked(true);
  else
    ui->chkSpeech->setChecked(false);
}

void TodoAlarm::setDateTime() {
  QStringList list = m_Method->getDateTimePickerValue();
  y = list.at(0);
  m = list.at(1);
  d = list.at(2);
  h = list.at(3);
  mm = list.at(4);

  if (h.length() == 1) h = "0" + h;
  int n_mm = mm.toInt();
  if (n_mm < 10) mm = "0" + QString::number(n_mm);
  setBtnTitle();
}

void TodoAlarm::showTimePicker() {
  if (isAndroid && isVisible()) {
    m_Method->setDateTimePickerFlag("hm", y.toInt(), m.toInt(), d.toInt(),
                                    h.toInt(), mm.toInt(), "todo");
    m_Method->openDateTimePicker();

    return;
  }
}

void TodoAlarm::showDatePicker() {
  if (isAndroid && isVisible()) {
    m_Method->setDateTimePickerFlag("ymd", y.toInt(), m.toInt(), d.toInt(),
                                    h.toInt(), mm.toInt(), "todo");
    m_Method->openDateTimePicker();

    return;
  }
}

void TodoAlarm::showAlarmWin(QStringList list) {
  ui->chk1->setChecked(list.at(0).toInt());
  ui->chk2->setChecked(list.at(1).toInt());
  ui->chk3->setChecked(list.at(2).toInt());
  ui->chk4->setChecked(list.at(3).toInt());
  ui->chk5->setChecked(list.at(4).toInt());
  ui->chk6->setChecked(list.at(5).toInt());
  ui->chk7->setChecked(list.at(6).toInt());
  QString strDate = list.at(7);
  QString strTime = list.at(8);
  qInfo() << strDate << strTime;
  QDate date = QDate::fromString(strDate, "yyyy-M-d");
  QTime time = QTime::fromString(strTime, "HH:mm");
  ui->dateTimeEdit->setDate(date);
  ui->dateTimeEdit->setTime(time);

  this->setFixedHeight(300);
  m_Method->setWinPos(this, 350);
  show();
}
