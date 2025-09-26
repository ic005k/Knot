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

  ui->lblPageNumber->hide();
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

  int w = 260;
  QFontMetrics fontMetrics(ui->lblPageNumber->font());
  int nFontHeight = fontMetrics.height();
  int h = nFontHeight + 10;

  if (mw_one->m_Reader->isLandscape) {
    setFixedWidth(h);
    setFixedHeight(w);

  } else {
    setFixedWidth(w);
    setFixedHeight(h);
  }

  move(mw_one->frameGeometry().center() - this->rect().center());
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

void PageIndicator::drawSN(const QString& sn) {
  QPainter painter(this);
  painter.setPen(QColor(255, 102, 0));  // #ff6600
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setRenderHint(QPainter::TextAntialiasing, true);

  QFont font("Arial,Helvetica,sans-serif", 60);
  font.setStyleStrategy(QFont::PreferAntialias);  // 抗锯齿
  painter.setFont(font);

  if (mw_one->m_Reader->isLandscape) {
    // 横屏：顺时针旋转90度
    painter.save();
    painter.translate(width(), 0);  // 将原点移到右上角
    painter.rotate(90);             // 顺时针旋转90度

    // 绘制文本（此时Y轴方向变为向右）
    painter.drawText(QRect(0, 0, height(), width()), Qt::AlignCenter, sn);

    painter.restore();
  } else {
    // 正常竖屏绘制
    painter.drawText(rect(), Qt::AlignCenter, sn);
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
  drawSN(QString::number(sn));
  QWidget::paintEvent(event);
}

PageIndicator::~PageIndicator() { delete ui; }
