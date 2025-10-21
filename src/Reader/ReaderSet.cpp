#include "ReaderSet.h"

#include "src/MainWindow.h"
#include "src/defines.h"
#include "ui_MainWindow.h"

extern QFont::Weight readerFontWeight;

ReaderSet::ReaderSet(QWidget *parent) : QDialog(parent) {
  QPalette pal = palette();

  pal.setColor(QPalette::Window, QColor(10, 10, 10, 200));

  setPalette(pal);

  setModal(true);
  mui->f_CustomColor->hide();

  this->installEventFilter(this);
  mui->hSlider->installEventFilter(this);

  mui->hSlider->setStyleSheet(mui->hsH->styleSheet());
  mui->btnFontLess->setStyleSheet("border:none");
  mui->btnFontPlus->setStyleSheet("border:none");

  QFont f = m_Method->getNewFont(fontSize);
  mui->btnStyle1->setFont(f);
  mui->btnStyle2->setFont(f);
  mui->btnStyle3->setFont(f);
  mui->btnFont->setFont(f);
  mui->lblProg->setFont(f);

  int nHeight = m_Method->getFontHeight();
  mui->btnFont->setFixedHeight(nHeight * 3.5);

  f.setPointSize(12);
  mui->lblInfo->setFont(f);
  mui->lblInfo->adjustSize();
  mui->lblInfo->setWordWrap(true);

  QValidator *validator =
      new QRegularExpressionValidator(regxNumber, mui->editPage);
  mui->editPage->setValidator(validator);

  f = m_Method->getNewFont(15);
  mui->editBackgroundColor->setFont(f);
  mui->editForegroundColor->setFont(f);
  QString color_0, color_1;
  QSettings Reg(privateDir + "reader.ini", QSettings::IniFormat);

  color_0 = Reg.value("/Reader/BackgroundColor", "#FFFFFF").toString();
  color_1 = Reg.value("/Reader/ForegroundColor", "#000000").toString();
  mui->editBackgroundColor->setText(color_0);
  mui->editForegroundColor->setText(color_1);

  mw_one->m_Reader->strStyle2_0 = "color:" + color_1 +
                                  ";background-color:" + color_0 +
                                  ";border: 2px "
                                  "solid "
                                  "rgb(0,0,255);border-radius: 4px;";
  mw_one->m_Reader->strStyle2_1 = "color:" + color_1 +
                                  ";background-color:" + color_0 +
                                  ";border: 2px "
                                  "solid "
                                  "rgb(255,0,0);border-radius: 4px;";
}

ReaderSet::~ReaderSet() {}

void ReaderSet::init() {
  int x, y, w, h;
  w = mw_one->width();
  h = this->height();
  x = mw_one->geometry().x();
  y = mw_one->geometry().y() + mui->qwReader->geometry().y() +
      (mui->qwReader->height() - h) / 2;
  setFixedWidth(w);
  setGeometry(x, y, w, h);

  QStringList list = mui->btnPages->text().split("\n");
  if (list.count() == 2) {
    mui->hSlider->setValue(list.at(0).toInt());
  }

  show();
}

bool ReaderSet::eventFilter(QObject *watch, QEvent *evn) {
  QMouseEvent *event = static_cast<QMouseEvent *>(evn);
  if (evn->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
      on_btnBack_clicked();
      return true;
    }
  }

  if (watch == mui->hSlider) {
    if (event->type() == QEvent::MouseButtonRelease) {
      on_hSlider_sliderReleased();
      on_hSlider_sliderMoved(mui->hSlider->value());
    }
  }

  return QWidget::eventFilter(watch, evn);
}

void ReaderSet::on_hSlider_sliderReleased() {
  mw_one->m_Reader->on_hSlider_sliderReleased(mui->hSlider->value());
}

void ReaderSet::on_btnFontPlus_clicked() {
  readerFontSize++;
  mw_one->m_Reader->setFontSize(readerFontSize);
}

void ReaderSet::on_btnFontLess_clicked() {
  if (readerFontSize <= 8) return;
  readerFontSize--;
  mw_one->m_Reader->setFontSize(readerFontSize);
}

void ReaderSet::on_hSlider_sliderMoved(int position) {
  mw_one->on_hSlider_sliderMoved(position);
}

void ReaderSet::updateProgress() {
  QStringList list = mui->btnPages->text().split("\n");
  if (list.count() == 2) {
    QString cur = list.at(0);
    QString total = list.at(1);
    mui->lblProg->setText(tr("Reading Progress") + " : " + cur + " -> " +
                          total);
  }
}

void ReaderSet::on_btnStyle1_clicked() {
  QSettings Reg(privateDir + "reader.ini", QSettings::IniFormat);

  Reg.setValue("/Reader/Style", "1");
  mw_one->m_Reader->readerStyle = "1";
  mw_one->m_Reader->setReaderStyle();
  mui->f_CustomColor->hide();
}

void ReaderSet::on_btnStyle2_clicked() {
  QSettings Reg(privateDir + "reader.ini", QSettings::IniFormat);

  Reg.setValue("/Reader/Style", "2");
  mw_one->m_Reader->readerStyle = "2";
  mw_one->m_Reader->setReaderStyle();
  mui->f_CustomColor->show();

  Reg.setValue("/Reader/BackgroundColor", mui->editBackgroundColor->text());
  Reg.setValue("/Reader/ForegroundColor", mui->editForegroundColor->text());
}

void ReaderSet::on_btnStyle3_clicked() {
  QSettings Reg(privateDir + "reader.ini", QSettings::IniFormat);

  Reg.setValue("/Reader/Style", "3");
  mw_one->m_Reader->readerStyle = "3";
  mw_one->m_Reader->setReaderStyle();
  mui->f_CustomColor->hide();
}

void ReaderSet::on_btnFont_clicked() {
  QString fileName;
  fileName = QFileDialog::getOpenFileName(this, tr("Font"), "",
                                          tr("Font Files (*.*)"));
  if (fileName == "") return;

  QString readerFont = mw_one->m_Preferences->setFontDemoUI(
      fileName, mui->btnFont, this->font().pointSize());
  iniPreferences->setValue("/Options/ReaderFont", fileName);

  mw_one->m_Reader->savePageVPos();
  mui->qwReader->rootContext()->setContextProperty("FontName", readerFont);
  mui->qwReader->rootContext()->setContextProperty("FontWeight",
                                                   readerFontWeight);
  mw_one->m_Reader->setPageVPos();

  mw_one->m_Reader->readReadNote(mw_one->m_Reader->cPage);
}

void ReaderSet::on_hSlider_valueChanged(int value) { Q_UNUSED(value); }

void ReaderSet::on_btnGoPage_clicked() {
  if (mui->editPage->text().trimmed() == "") return;

  int nPage = mui->editPage->text().toInt();
  if (nPage <= 0) nPage = 1;
  if (nPage > mui->hSlider->maximum()) nPage = mui->hSlider->maximum();
  mui->hSlider->setValue(nPage);
  on_hSlider_sliderMoved(nPage);
  on_hSlider_sliderReleased();
}

void ReaderSet::on_btnBack_clicked() {
  saveScrollValue();
  this->close();
}

void ReaderSet::on_btnBackgroundColor_clicked() {
  QString color_0;
  color_0 = m_Method->getCustomColor();
  if (color_0.isNull()) return;

  mui->editBackgroundColor->setText(color_0);
  QString color_1 = mui->editForegroundColor->text();

  QSettings Reg(privateDir + "reader.ini", QSettings::IniFormat);

  Reg.setValue("/Reader/BackgroundColor", mui->editBackgroundColor->text());
  Reg.setValue("/Reader/ForegroundColor", mui->editForegroundColor->text());

  mw_one->m_Reader->strStyle2_0 = "color:" + color_1 +
                                  ";background-color:" + color_0 +
                                  ";border: 2px "
                                  "solid "
                                  "rgb(0,0,255);border-radius: 4px;";
  mw_one->m_Reader->strStyle2_1 = "color:" + color_1 +
                                  ";background-color:" + color_0 +
                                  ";border: 2px "
                                  "solid "
                                  "rgb(255,0,0);border-radius: 4px;";
  mw_one->m_Reader->setReaderStyle();
}

void ReaderSet::on_btnForegroundColor_clicked() {
  QString color_1 = m_Method->getCustomColor();
  if (color_1.isNull()) return;

  mui->editForegroundColor->setText(color_1);
  QString color_0 = mui->editBackgroundColor->text();

  QSettings Reg(privateDir + "reader.ini", QSettings::IniFormat);

  Reg.setValue("/Reader/BackgroundColor", mui->editBackgroundColor->text());
  Reg.setValue("/Reader/ForegroundColor", mui->editForegroundColor->text());

  mw_one->m_Reader->strStyle2_0 = "color:" + color_1 +
                                  ";background-color:" + color_0 +
                                  ";border: 2px "
                                  "solid "
                                  "rgb(0,0,255);border-radius: 4px;";
  mw_one->m_Reader->strStyle2_1 = "color:" + color_1 +
                                  ";background-color:" + color_0 +
                                  ";border: 2px "
                                  "solid "
                                  "rgb(255,0,0);border-radius: 4px;";
  mw_one->m_Reader->setReaderStyle();
}

void ReaderSet::on_editBackgroundColor_textChanged(const QString &arg1) {
  Q_UNUSED(arg1);
  if (!mw_one->initMain) on_btnStyle2_clicked();
}

void ReaderSet::on_editForegroundColor_textChanged(const QString &arg1) {
  Q_UNUSED(arg1);
  if (!mw_one->initMain) on_btnStyle2_clicked();
}

void ReaderSet::on_btnSetBookmark_clicked() {
  QString page = mui->btnPages->text().split("\n").at(0);
  QString txt =
      "( " + page + " ) " + mw_one->m_Reader->getBookmarkTextFromQML() + "\n" +
      mw_one->m_Steps->getFullDate() + "  " + QTime::currentTime().toString();
  mw_one->m_Reader->saveReader(txt, true);
  if (isAndroid) m_Method->showToastMessage(tr("Bookmark setup is complete."));
}

void ReaderSet::on_btnLessen_clicked() {
  mw_one->m_Reader->scrollValue = mw_one->m_Reader->scrollValue - 0.05;
  if (mw_one->m_Reader->scrollValue <= 0.1) mw_one->m_Reader->scrollValue = 0.1;

  setScrollValue();

  QString value = QString::number(mw_one->m_Reader->scrollValue, 'f', 2);
  mui->lblSpeed->setText(tr("Scroll Speed") + " : " + value);
}

void ReaderSet::on_btnDefault_clicked() {
  mw_one->m_Reader->scrollValue = 1.0;

  setScrollValue();

  QString value = QString::number(mw_one->m_Reader->scrollValue, 'f', 2);
  mui->lblSpeed->setText(tr("Scroll Speed") + " : " + value);
}

void ReaderSet::on_btnAdd_clicked() {
  mw_one->m_Reader->scrollValue = mw_one->m_Reader->scrollValue + 0.05;

  if (mw_one->m_Reader->scrollValue >= 2.0) mw_one->m_Reader->scrollValue = 2.0;

  setScrollValue();

  QString value = QString::number(mw_one->m_Reader->scrollValue, 'f', 2);
  mui->lblSpeed->setText(tr("Scroll Speed") + " : " + value);
}

void ReaderSet::saveScrollValue() {
  QSettings Reg(privateDir + "reader.ini", QSettings::IniFormat);

  Reg.setValue("/Reader/ScrollValue", mw_one->m_Reader->scrollValue);
}

void ReaderSet::on_btnClear_clicked() {
  iniPreferences->remove("/Options/ReaderFont");
  mui->btnFont->setText(tr("Font"));
}

void ReaderSet::setScrollValue() {
  mui->qwReader->rootContext()->setContextProperty(
      "scrollValue", mw_one->m_Reader->scrollValue);
}
