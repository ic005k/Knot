#ifndef TIME24PICKER_H
#define TIME24PICKER_H

#include <QTime>
#include <QWidget>

#include "WheelWidget.h"

class Time24Picker : public QWidget {
  Q_OBJECT
 public:
  explicit Time24Picker(QWidget *parent = nullptr);
  QTime time() const;
  void setTime(const QTime &time);

 signals:
  void timeChanged(const QTime &time);

 private:
  WheelWidget *m_hourWheel;
  WheelWidget *m_minuteWheel;

  void setupUI();
};

#endif  // TIME24PICKER_H
