#include "PageIndicator.h"

#include "src/MainWindow.h"
#include "src/defines.h"
#include "ui_MainWindow.h"
#include "ui_PageIndicator.h"

PageIndicator::PageIndicator(QWidget* parent)
    : QDialog(parent), ui(new Ui::PageIndicator) {
  ui->setupUi(this);

#ifdef Q_OS_ANDROID
#else
  setAttribute(Qt::WA_TranslucentBackground);
#endif

  this->setStyleSheet("background-color:rgba(0, 0, 0,25%);");
  setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint |
                 Qt::WindowDoesNotAcceptFocus);

  setGeometry(0, 0, 1, 1);

  ui->lblPageNumber->setStyleSheet("color:#ff6600;");
  ui->lblPageNumber->adjustSize();

  this->setContentsMargins(1, 1, 1, 1);
  this->layout()->setContentsMargins(1, 1, 1, 1);
}

void PageIndicator::setPicLeft() {
  showPageNumber("left");

  init();
}

void PageIndicator::setPicRight() {
  showPageNumber("right");

  init();
}

void PageIndicator::init() {
  if (mui->frameReader->isHidden()) return;

  int w = mw_one->width() - 20;
  QFontMetrics fontMetrics(ui->lblPageNumber->font());
  int nFontHeight = fontMetrics.height();
  setFixedWidth(w);
  setFixedHeight(nFontHeight + 10);
  int y;
  if (mui->f_ReaderFun->isVisible())
    y = mw_one->geometry().y() + mui->f_ReaderFun->height() + 5;
  else
    y = mw_one->geometry().y() + 5;
  this->setGeometry(mw_one->geometry().x() + (mw_one->width() - w) / 2, y,
                    this->width(), this->height());

  this->show();
}

void PageIndicator::showPageNumber(QString page) {
  sn = 0;
  cn = mui->btnPages->text().split("\n").at(0).toInt();
  tn = mui->btnPages->text().split("\n").at(1).toInt();
  if (page == "left") {
    if (cn + 1 < tn) {
      sn = cn + 1;

    } else {
      sn = tn;
    }
    ui->lblPageNumber->setText(QString::number(sn));
  }

  if (page == "right") {
    if (cn - 1 > 0) {
      sn = cn - 1;

    } else {
      sn = 1;
    }
    ui->lblPageNumber->setText(QString::number(sn));
  }
}

void PageIndicator::setY(int y) {
  this->setGeometry(
      mw_one->geometry().x() + mw_one->width() - this->width() - 10, y,
      this->width(), this->height());
}

bool PageIndicator::eventFilter(QObject* watch, QEvent* evn) {
  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
      return true;
    }
  }

  return QWidget::eventFilter(watch, evn);
}

void PageIndicator::closeEvent(QCloseEvent* event) { Q_UNUSED(event); }

void PageIndicator::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);
  return;

  QPainter painter(this);
  painter.fillRect(this->rect(), QColor(0, 0, 0, 0));
  QWidget::paintEvent(event);
}

PageIndicator::~PageIndicator() { delete ui; }
