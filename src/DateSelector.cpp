#include "DateSelector.h"

#include "MainWindow.h"
#include "ui_DateSelector.h"
#include "ui_MainWindow.h"

extern MainWindow *mw_one;
extern Method *m_Method;
extern bool isAndroid;

DateSelector::DateSelector(QWidget *parent)
    : QDialog(parent), ui(new Ui::DateSelector) {
  ui->setupUi(this);
  this->installEventFilter(this);

  setModal(true);

  m_datePickerYM = new DatePicker(false, ui->gboxYear);
  m_datePickerYMD = new DatePicker(true, ui->gboxMonth);

  ui->gboxYear->layout()->addWidget(m_datePickerYM);
  ui->gboxMonth->layout()->addWidget(m_datePickerYMD);
}

DateSelector::~DateSelector() { delete ui; }

bool DateSelector::eventFilter(QObject *watch, QEvent *evn) {
  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
      close();
      return true;
    }
  }

  return QWidget::eventFilter(watch, evn);
}

void DateSelector::closeEvent(QCloseEvent *event) {
  Q_UNUSED(event)
  m_Method->closeGrayWindows();
}

void DateSelector::init() {
  if (dateFlag == 1) {
    ui->gboxMonth->hide();

    ui->gboxYear->setHidden(false);
    ui->gboxMonth->hide();

    ui->lblYear->setHidden(false);

    ui->lblFlag->hide();

    setMinimumHeight(300);
  }

  if (dateFlag == 2) {
    ui->gboxYear->hide();

    ui->gboxMonth->setHidden(false);

    ui->lblYear->hide();

    ui->lblFlag->hide();

    setMinimumHeight(300);
  }

  if (dateFlag == 3 || dateFlag == 4) {
    ui->lblYear->setHidden(false);

    if (dateFlag == 3) ui->lblFlag->setText(tr("Start Date"));
    if (dateFlag == 4) ui->lblFlag->setText(tr("End Date"));
    ui->lblFlag->setHidden(false);

    setMinimumHeight(300);
  }

  setFixedWidth(mw_one->width() - 10);
  int x, y, w, h;
  x = mw_one->geometry().x() + 5;
  y = mw_one->geometry().y() + (mw_one->height() - height()) / 2;
  w = width();
  h = height();
  setGeometry(x, y, w, h);

  m_Method->showGrayWindows();
  show();
}

void DateSelector::on_btnOk_clicked() {
  QString y, m, d;

  // if (isAndroid) {
  //   QStringList list = m_Method->getDateTimePickerValue();
  //   y = list.at(0);
  //   m = list.at(1);
  //   d = list.at(2);
  // }

  if (!ui->gboxYear->isHidden()) {
    QDate date = m_datePickerYM->date();
    y = QString::number(date.year());
    m = QString::number(date.month());
    d = QString::number(date.day());
  }

  if (!ui->gboxMonth->isHidden()) {
    QDate date = m_datePickerYMD->date();
    y = QString::number(date.year());
    m = QString::number(date.month());
    d = QString::number(date.day());
  }

  if (m.length() == 1) m = "0" + m;
  if (d.length() == 1) d = "0" + d;

  if (dateFlag == 1 || dateFlag == 2) {
    mw_one->ui->btnYear->setText(y);

    int value = m.toInt();
    if (value == 13)
      mw_one->ui->btnMonth->setText(tr("Year-Round"));
    else {
      mw_one->ui->btnMonth->setText(m);
    }
  }

  QString strYear, strMonth;
  strYear = mw_one->ui->btnYear->text();
  strMonth = mw_one->ui->btnMonth->text();
  if (dateFlag == 1 || dateFlag == 2)
    mw_one->m_Report->startReport1(strYear, strMonth);

  if (dateFlag == 3) {
    mw_one->ui->btnStartDate->setText(y + "  " + m + "  " + d);
  }

  if (dateFlag == 4) {
    mw_one->ui->btnEndDate->setText(y + "  " + m + "  " + d);
  }

  if (dateFlag == 3 || dateFlag == 4) mw_one->m_Report->startReport2();

  mw_one->m_Report->saveYMD();
  close();
}

void DateSelector::initStartEndDate(QString flag) {
  QString str;
  if (flag == "start") {
    str = mw_one->ui->btnStartDate->text();
    dateFlag = 3;
  }

  if (flag == "end") {
    str = mw_one->ui->btnEndDate->text();
    dateFlag = 4;
  }

  QStringList list = str.split("  ");
  int y, m, d;
  y = list.at(0).toInt();
  m = list.at(1).toInt();
  d = list.at(2).toInt();

  // if (isAndroid) {
  //   m_Method->setDateTimePickerFlag("ymd", y, m, d, 0, 0, flag);
  //   m_Method->openDateTimePicker();
  //   return;
  // }

  ui->gboxYear->hide();
  ui->gboxMonth->setHidden(false);

  m_datePickerYMD->setDate(QDate(y, m, d));

  init();
}
