// DatePicker.h
#pragma once
#include <QLocale>
#include <QWidget>

#include "WheelWidget.h"

class DatePicker : public QWidget {
  Q_OBJECT
 public:
  explicit DatePicker(bool showDay = true, QWidget *parent = nullptr);
  void setShowDay(bool show);
  QDate date() const;
  void setDate(const QDate &date);

 signals:
  void dateChanged(const QDate &date);

 private:
  bool m_showDay;
  WheelWidget *m_yearWheel;
  WheelWidget *m_monthWheel;
  WheelWidget *m_dayWheel;

  void setupUI();
  void updateDayWheelVisibility();
  void updateDays(int year, int month);
  void initYears();
};
