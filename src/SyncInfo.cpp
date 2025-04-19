#include "SyncInfo.h"

#include "MainWindow.h"
#include "ui_SyncInfo.h"

extern MainWindow* mw_one;
extern Method* m_Method;
extern QString currentMDFile;

SyncInfo::SyncInfo(QWidget* parent) : QDialog(parent), ui(new Ui::SyncInfo) {
  ui->setupUi(this);
  this->installEventFilter(this);

  setWindowFlag(Qt::FramelessWindowHint);
  setAttribute(Qt::WA_TranslucentBackground);
  ui->frame->setStyleSheet(
      "#frame{background-color: rgb(255, 255, 255);border-radius:10px; "
      "border:1px solid gray;}");
  this->layout()->setContentsMargins(5, 5, 5, 5);

  setModal(true);
  QScroller::grabGesture(ui->textBrowser, QScroller::LeftMouseButtonGesture);
  ui->textBrowser->verticalScrollBar()->setStyleSheet(
      m_Method->vsbarStyleSmall);
  m_Method->setSCrollPro(ui->textBrowser);

  mw_one->set_ToolButtonStyle(this);
}

SyncInfo::~SyncInfo() { delete ui; }

void SyncInfo::on_btnClose_clicked() {
  mw_one->m_Notes->init_all_notes();

  close();
  ui->textBrowser->clear();
  infoList.clear();
}

void SyncInfo::init() {
  setGeometry(mw_one->geometry().x(), mw_one->geometry().y(), mw_one->width(),
              mw_one->height() / 2);
}

bool SyncInfo::eventFilter(QObject* watch, QEvent* evn) {
  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
      return true;
    }
  }

  return QWidget::eventFilter(watch, evn);
}

void SyncInfo::runSync(QString path) {}
