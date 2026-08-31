#include "ReaderSet.h"

#include "src/MainWindow.h"

extern QFont::Weight readerFontWeight;

ReaderSet::ReaderSet(QWidget* parent) : QDialog(parent) {}

ReaderSet::~ReaderSet() {}

void ReaderSet::init() {}

bool ReaderSet::eventFilter(QObject* watch, QEvent* evn) {
  QMouseEvent* event = static_cast<QMouseEvent*>(evn);
  if (evn->type() == QEvent::KeyPress) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
      on_btnBack_clicked();
      return true;
    }
  }

  return QWidget::eventFilter(watch, evn);
}

void ReaderSet::on_hSlider_sliderReleased() {}

void ReaderSet::on_btnFontPlus_clicked() {
  readerFontSize++;
  m_Reader->setFontSize(readerFontSize);
}

void ReaderSet::on_btnFontLess_clicked() {
  if (readerFontSize <= 8) return;
  readerFontSize--;
  m_Reader->setFontSize(readerFontSize);
}

void ReaderSet::on_hSlider_sliderMoved(int position) {
  mw_one->on_hSlider_sliderMoved(position);
}

void ReaderSet::updateProgress() {}

void ReaderSet::on_btnStyle1_clicked() {}

void ReaderSet::on_btnStyle2_clicked() {}

void ReaderSet::on_btnStyle3_clicked() {}

void ReaderSet::on_btnFont_clicked() {}

void ReaderSet::on_hSlider_valueChanged(int value) { Q_UNUSED(value); }

void ReaderSet::on_btnGoPage_clicked() {}

void ReaderSet::on_btnBack_clicked() {
  saveScrollValue();
  this->close();
}

void ReaderSet::on_btnBackgroundColor_clicked() {}

void ReaderSet::on_btnForegroundColor_clicked() {}

void ReaderSet::on_editBackgroundColor_textChanged(const QString& arg1) {
  Q_UNUSED(arg1);
  if (!mw_one->initMain) on_btnStyle2_clicked();
}

void ReaderSet::on_editForegroundColor_textChanged(const QString& arg1) {
  Q_UNUSED(arg1);
  if (!mw_one->initMain) on_btnStyle2_clicked();
}

void ReaderSet::on_btnSetBookmark_clicked() {}

void ReaderSet::on_btnLessen_clicked() {}

void ReaderSet::on_btnDefault_clicked() {}

void ReaderSet::on_btnPlus_clicked() {}

void ReaderSet::saveScrollValue() {
  QSettings Reg(privateDir + "reader.ini", QSettings::IniFormat);

  Reg.setValue("/Reader/ScrollValue", m_Reader->scrollValue);
}

void ReaderSet::on_btnClear_clicked() {}

void ReaderSet::setScrollValue() {}
