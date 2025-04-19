#include "ReaderSet.h"

#include "src/MainWindow.h"
#include "ui_MainWindow.h"

ReaderSet *m_ReaderSet;

extern int fontSize, readerFontSize;
extern MainWindow *mw_one;
extern Method *m_Method;
extern QString iniDir, privateDir, defaultFontFamily, customFontFamily;
extern QSettings *iniPreferences;
extern QRegularExpression regxNumber;
extern bool isAndroid;

extern QFont::Weight readerFontWeight;

ReaderSet::ReaderSet(QWidget *parent) : QDialog(parent) {
  QPalette pal = palette();
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  pal.setColor(QPalette::Background, QColor(10, 10, 10, 200));
#else
  pal.setColor(QPalette::Window, QColor(10, 10, 10, 200));
#endif
  setPalette(pal);

  setModal(true);
  mw_one->ui->f_CustomColor->hide();

  this->installEventFilter(this);
  mw_one->ui->hSlider->installEventFilter(this);

  mw_one->ui->hSlider->setStyleSheet(mw_one->ui->hsH->styleSheet());
  mw_one->ui->btnFontLess->setStyleSheet("border:none");
  mw_one->ui->btnFontPlus->setStyleSheet("border:none");

  QFont f = m_Method->getNewFont(fontSize);
  mw_one->ui->btnStyle1->setFont(f);
  mw_one->ui->btnStyle2->setFont(f);
  mw_one->ui->btnStyle3->setFont(f);
  mw_one->ui->btnFont->setFont(f);
  mw_one->ui->lblProg->setFont(f);

  int nHeight = m_Method->getFontHeight();
  mw_one->ui->btnFont->setFixedHeight(nHeight * 3.5);

  f.setPointSize(12);
  mw_one->ui->lblInfo->setFont(f);
  mw_one->ui->lblInfo->adjustSize();
  mw_one->ui->lblInfo->setWordWrap(true);

  QValidator *validator =
      new QRegularExpressionValidator(regxNumber, mw_one->ui->editPage);
  mw_one->ui->editPage->setValidator(validator);

  f = m_Method->getNewFont(15);
  mw_one->ui->editBackgroundColor->setFont(f);
  mw_one->ui->editForegroundColor->setFont(f);
  QString color_0, color_1;
  QSettings Reg(privateDir + "reader.ini", QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  Reg.setIniCodec("utf-8");
#endif
  color_0 = Reg.value("/Reader/BackgroundColor", "#FFFFFF").toString();
  color_1 = Reg.value("/Reader/ForegroundColor", "#000000").toString();
  mw_one->ui->editBackgroundColor->setText(color_0);
  mw_one->ui->editForegroundColor->setText(color_1);

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
  y = mw_one->geometry().y() + mw_one->ui->qwReader->geometry().y() +
      (mw_one->ui->qwReader->height() - h) / 2;
  setFixedWidth(w);
  setGeometry(x, y, w, h);

  QStringList list = mw_one->ui->btnPages->text().split("\n");
  if (list.count() == 2) {
    mw_one->ui->hSlider->setValue(list.at(0).toInt());
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

  if (watch == mw_one->ui->hSlider) {
    if (event->type() == QEvent::MouseButtonRelease) {
      on_hSlider_sliderReleased();
      on_hSlider_sliderMoved(mw_one->ui->hSlider->value());
    }
  }

  return QWidget::eventFilter(watch, evn);
}

void ReaderSet::on_hSlider_sliderReleased() {
  mw_one->m_Reader->on_hSlider_sliderReleased(mw_one->ui->hSlider->value());
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
  QStringList list = mw_one->ui->btnPages->text().split("\n");
  if (list.count() == 2) {
    QString cur = list.at(0);
    QString total = list.at(1);
    mw_one->ui->lblProg->setText(tr("Reading Progress") + " : " + cur + " -> " +
                                 total);
  }
}

void ReaderSet::on_btnStyle1_clicked() {
  QSettings Reg(privateDir + "reader.ini", QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  Reg.setIniCodec("utf-8");
#endif
  Reg.setValue("/Reader/Style", "1");
  mw_one->m_Reader->readerStyle = "1";
  mw_one->m_Reader->setReaderStyle();
  mw_one->ui->f_CustomColor->hide();
}

void ReaderSet::on_btnStyle2_clicked() {
  QSettings Reg(privateDir + "reader.ini", QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  Reg.setIniCodec("utf-8");
#endif
  Reg.setValue("/Reader/Style", "2");
  mw_one->m_Reader->readerStyle = "2";
  mw_one->m_Reader->setReaderStyle();
  mw_one->ui->f_CustomColor->show();

  Reg.setValue("/Reader/BackgroundColor",
               mw_one->ui->editBackgroundColor->text());
  Reg.setValue("/Reader/ForegroundColor",
               mw_one->ui->editForegroundColor->text());
}

void ReaderSet::on_btnStyle3_clicked() {
  QSettings Reg(privateDir + "reader.ini", QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  Reg.setIniCodec("utf-8");
#endif
  Reg.setValue("/Reader/Style", "3");
  mw_one->m_Reader->readerStyle = "3";
  mw_one->m_Reader->setReaderStyle();
  mw_one->ui->f_CustomColor->hide();
}

void ReaderSet::on_btnFont_clicked() {
  QString fileName;
  fileName = QFileDialog::getOpenFileName(this, tr("Font"), "",
                                          tr("Font Files (*.*)"));
  if (fileName == "") return;

#ifdef Q_OS_ANDROID
  fileName = m_Method->getRealPathFile(fileName);
#endif

  QString readerFont = mw_one->m_Preferences->setFontDemo(
      fileName, mw_one->ui->btnFont, this->font().pointSize());
  iniPreferences->setValue("/Options/ReaderFont", fileName);

  mw_one->m_Reader->savePageVPos();
  mw_one->ui->qwReader->rootContext()->setContextProperty("FontName",
                                                          readerFont);
  mw_one->ui->qwReader->rootContext()->setContextProperty("FontWeight",
                                                          readerFontWeight);
  mw_one->m_Reader->setPageVPos();
}

void ReaderSet::on_hSlider_valueChanged(int value) { Q_UNUSED(value); }

void ReaderSet::on_btnGoPage_clicked() {
  if (mw_one->ui->editPage->text().trimmed() == "") return;

  int nPage = mw_one->ui->editPage->text().toInt();
  if (nPage <= 0) nPage = 1;
  if (nPage > mw_one->ui->hSlider->maximum())
    nPage = mw_one->ui->hSlider->maximum();
  mw_one->ui->hSlider->setValue(nPage);
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

  mw_one->ui->editBackgroundColor->setText(color_0);
  QString color_1 = mw_one->ui->editForegroundColor->text();

  QSettings Reg(privateDir + "reader.ini", QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  Reg.setIniCodec("utf-8");
#endif
  Reg.setValue("/Reader/BackgroundColor",
               mw_one->ui->editBackgroundColor->text());
  Reg.setValue("/Reader/ForegroundColor",
               mw_one->ui->editForegroundColor->text());

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

  mw_one->ui->editForegroundColor->setText(color_1);
  QString color_0 = mw_one->ui->editBackgroundColor->text();

  QSettings Reg(privateDir + "reader.ini", QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  Reg.setIniCodec("utf-8");
#endif
  Reg.setValue("/Reader/BackgroundColor",
               mw_one->ui->editBackgroundColor->text());
  Reg.setValue("/Reader/ForegroundColor",
               mw_one->ui->editForegroundColor->text());

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
  QString page = mw_one->ui->btnPages->text().split("\n").at(0);
  QString txt = "( " + page + " ) " + mw_one->m_Reader->getBookmarkText() +
                "\n" + QDateTime::currentDateTime().toString();
  mw_one->m_Reader->saveReader(txt, true);
  if (isAndroid) m_Method->showToastMessage(tr("Bookmark setup is complete."));
}

void ReaderSet::on_btnLessen_clicked() {
  mw_one->m_Reader->scrollValue = mw_one->m_Reader->scrollValue - 0.1;
  if (mw_one->m_Reader->scrollValue <= 0.1) mw_one->m_Reader->scrollValue = 0.1;

  QString value = QString::number(mw_one->m_Reader->scrollValue, 'f', 1);
  mw_one->ui->lblSpeed->setText(tr("Scroll Speed") + " : " + value);
}

void ReaderSet::on_btnDefault_clicked() {
  mw_one->m_Reader->scrollValue = 1.0;

  QString value = QString::number(mw_one->m_Reader->scrollValue, 'f', 1);
  mw_one->ui->lblSpeed->setText(tr("Scroll Speed") + " : " + value);
}

void ReaderSet::on_btnAdd_clicked() {
  mw_one->m_Reader->scrollValue = mw_one->m_Reader->scrollValue + 0.1;

  if (mw_one->m_Reader->scrollValue >= 2.0) mw_one->m_Reader->scrollValue = 2.0;

  QString value = QString::number(mw_one->m_Reader->scrollValue, 'f', 1);
  mw_one->ui->lblSpeed->setText(tr("Scroll Speed") + " : " + value);
}

void ReaderSet::saveScrollValue() {
  QSettings Reg(privateDir + "reader.ini", QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  Reg.setIniCodec("utf-8");
#endif
  Reg.setValue("/Reader/ScrollValue", mw_one->m_Reader->scrollValue);
}

void ReaderSet::on_btnClear_clicked() {
  iniPreferences->remove("/Options/ReaderFont");
  mw_one->ui->btnFont->setText(tr("Font"));
}
