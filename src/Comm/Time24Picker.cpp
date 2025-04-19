#include "Time24Picker.h"

#include <QHBoxLayout>

Time24Picker::Time24Picker(QWidget *parent) : QWidget(parent) {
  setupUI();
  // 启用循环滚动
  m_hourWheel->setCircular(true);
  m_minuteWheel->setCircular(true);
}

void Time24Picker::setupUI() {
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setSpacing(15);
  layout->setContentsMargins(5, 0, 5, 0);

  // 小时 (00-23)
  m_hourWheel = new WheelWidget;
  QStringList hours;
  for (int h = 0; h < 24; ++h) hours << QString::asprintf("%02d", h);
  m_hourWheel->setItems(hours);
  layout->addWidget(m_hourWheel);

  // 分钟 (00-59)
  m_minuteWheel = new WheelWidget;
  QStringList minutes;
  for (int m = 0; m < 60; ++m) minutes << QString::asprintf("%02d", m);
  m_minuteWheel->setItems(minutes);
  layout->addWidget(m_minuteWheel);

  // 连接信号
  connect(m_hourWheel, &WheelWidget::currentIndexChanged,
          [this] { emit timeChanged(time()); });
  connect(m_minuteWheel, &WheelWidget::currentIndexChanged,
          [this] { emit timeChanged(time()); });
}

QTime Time24Picker::time() const {
  return QTime(m_hourWheel->currentIndex(), m_minuteWheel->currentIndex());
}

void Time24Picker::setTime(const QTime &time) {
  if (!time.isValid()) return;

  m_hourWheel->setCurrentIndex(time.hour());
  m_minuteWheel->setCurrentIndex(time.minute());
}
