// DatePicker.cpp
#include "DatePicker.h"

#include <QDate>
#include <QHBoxLayout>

DatePicker::DatePicker(bool showDay, QWidget *parent)
    : QWidget(parent), m_showDay(showDay) {
  setupUI();

  m_yearWheel->setCircular(true);
  m_monthWheel->setCircular(true);
  m_dayWheel->setCircular(true);

  updateDayWheelVisibility();
}

void DatePicker::setShowDay(bool show) {
  if (m_showDay != show) {
    m_showDay = show;
    updateDayWheelVisibility();
    emit dateChanged(date());
  }
}

void DatePicker::setupUI() {
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setSpacing(15);

  // 年
  m_yearWheel = new WheelWidget;
  initYears();
  layout->addWidget(m_yearWheel);

  // 月
  m_monthWheel = new WheelWidget;
  QStringList months;
  for (int m = 1; m <= 12; ++m)
    months << QLocale().monthName(m, QLocale::ShortFormat);
  m_monthWheel->setItems(months);
  layout->addWidget(m_monthWheel);

  // 日
  m_dayWheel = new WheelWidget;
  updateDays(QDate::currentDate().year(), QDate::currentDate().month());
  layout->addWidget(m_dayWheel);

  // 信号连接（直接发射信号）
  connect(m_yearWheel, &WheelWidget::currentIndexChanged, this, [this] {
    updateDays(m_yearWheel->currentText().toInt(),
               m_monthWheel->currentIndex() + 1);
    emit dateChanged(date());
  });

  connect(m_monthWheel, &WheelWidget::currentIndexChanged, this, [this] {
    updateDays(m_yearWheel->currentText().toInt(),
               m_monthWheel->currentIndex() + 1);
    emit dateChanged(date());
  });

  connect(m_dayWheel, &WheelWidget::currentIndexChanged, this,
          [this] { emit dateChanged(date()); });
}

void DatePicker::updateDayWheelVisibility() {
  m_dayWheel->setVisible(m_showDay);
  adjustSize();
}

QDate DatePicker::date() const {
  int year = m_yearWheel->currentText().toInt();
  int month = m_monthWheel->currentIndex() + 1;
  int day = m_showDay ? (m_dayWheel->currentIndex() + 1) : 1;
  return QDate(year, month, day);
}

void DatePicker::setDate(const QDate &date) {
  if (!date.isValid()) return;

  m_yearWheel->setCurrentIndex(date.year() - 2022);
  m_monthWheel->setCurrentIndex(date.month() - 1);
  if (m_showDay) {
    updateDays(date.year(), date.month());
    m_dayWheel->setCurrentIndex(date.day() - 1);
  }
}

void DatePicker::updateDays(int year, int month) {
  int days = QDate(year, month, 1).daysInMonth();
  QStringList daysList;
  for (int d = 1; d <= days; ++d) daysList << QString::number(d);
  m_dayWheel->setItems(daysList);
}

void DatePicker::initYears() {
  QStringList years;
  int endYear = QDate::currentDate().year() + 10;
  for (int y = 2022; y <= endYear; ++y) years << QString::number(y);
  m_yearWheel->setItems(years);
}
