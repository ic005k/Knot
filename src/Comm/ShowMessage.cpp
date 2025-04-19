#include "ShowMessage.h"

#include "src/MainWindow.h"
#include "ui_ShowMessage.h"

extern MainWindow* mw_one;
extern Method* m_Method;
extern QString copyText;
extern bool isDark, isAndroid;

ShowMessage::ShowMessage(QWidget* parent)
    : QDialog(parent), ui(new Ui::ShowMessage) {
  ui->setupUi(this);

  this->installEventFilter(this);
  ui->editMsg->viewport()->installEventFilter(this);

  this->layout()->setContentsMargins(0, 0, 0, 0);
  setWindowFlag(Qt::FramelessWindowHint);
  setAttribute(Qt::WA_TranslucentBackground);
  if (isAndroid)
    ui->widget->setStyleSheet("background-color:rgba(0, 0, 0, 35%);");
  else
    ui->widget->setStyleSheet("background-color:rgba(0, 0, 0, 0%);");

  if (isDark)
    ui->frame->setStyleSheet(
        "QFrame{background-color: #455364;color: #FFFFFF;border-radius:10px; "
        "border:0px solid gray;}");
  else
    ui->frame->setStyleSheet(
        "QFrame{background-color: #F5F5F5;border-radius:10px; "
        "border:0px solid gray;}");

  setModal(true);

  QFont font = this->font();
  font.setBold(true);
  ui->lblTitle->setFont(font);
  ui->lblTitle->adjustSize();
  ui->lblTitle->setWordWrap(true);

  ui->editMsg->adjustSize();
  ui->editMsg->setReadOnly(true);
  QScroller::grabGesture(ui->editMsg, QScroller::LeftMouseButtonGesture);
  m_Method->setSCrollPro(ui->editMsg);

  ui->hframe->setFrameShape(QFrame::HLine);
  if (isDark)
    ui->hframe->setStyleSheet(
        "QFrame{background:rgb(0,205,205);min-height:2px}");
  else
    ui->hframe->setStyleSheet("QFrame{background:red;min-height:2px}");

  // mw_one->set_ToolButtonStyle(this);

  QString strBtnStyle = ui->btnOk->styleSheet();
  ui->btnCancel->setStyleSheet(strBtnStyle);
  ui->btnCopy->setStyleSheet(strBtnStyle);
  ui->btnDel->setStyleSheet(strBtnStyle);
}

ShowMessage::~ShowMessage() { delete ui; }

void ShowMessage::closeEvent(QCloseEvent* event) {
  Q_UNUSED(event)
  m_Method->closeGrayWindows();
}

bool ShowMessage::eventFilter(QObject* watch, QEvent* evn) {
  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
      on_btnCancel_clicked();
    }

    return true;
  }

  if (evn->type() == QEvent::MouseButtonPress) {
    if (btn_count == 0) {
      on_btnCancel_clicked();
    }

    return true;
  }

  return QWidget::eventFilter(watch, evn);
}

void ShowMessage::init() {
  isValue = false;
  setFixedHeight(mw_one->height());
  setFixedWidth(mw_one->width());
  setGeometry(mw_one->geometry().x(), mw_one->geometry().y(), width(),
              height());

  if (!m_Method->m_widget->isHidden()) {
    m_Method->m_widget->close();
  }

  if (!isAndroid) show();

  int x, y, w, h;

#ifdef Q_OS_ANDROID
  w = this->width();

#else
  w = 360;
  if (w >= mw_one->geometry().width()) {
    w = mw_one->geometry().width();
  }

#endif

  ui->frame->setFixedWidth(w - 20);

  int nEditH = mw_one->m_Todo->getEditTextHeight(ui->editMsg);
  int nH = 0;
  nH = nEditH + ui->btnCancel->height() + ui->lblTitle->height() +
       ui->hframe->height() + 70;

  if (nH > mw_one->height()) nH = mw_one->height() - 10;
  ui->frame->setFixedHeight(nH);
  h = ui->widget->height();
  x = mw_one->geometry().x() + (mw_one->width() - w) / 2;
  y = mw_one->geometry().y() + (mw_one->height() - h) / 2;
  ui->frame->setGeometry(x, y, w, h);
  if (isAndroid) show();
}

bool ShowMessage::showMsg(QString title, QString msgtxt, int btnCount) {
  // btnCount 1:Ok 2:Cancel + Ok

  if (btnCount == 2 || btnCount == 3) {
    msgtxt = msgtxt + "\n\n";
  }

  if (btnCount == 1) btnCount = 0;

  if (btnCount == 0) {
    ui->btnCancel->hide();
    ui->btnCopy->hide();
    ui->btnOk->show();
    ui->btnDel->hide();
  }
  if (btnCount == 1) {
    ui->btnCancel->hide();
    ui->btnOk->show();
    ui->btnCopy->hide();
    ui->btnDel->hide();
  }
  if (btnCount == 2) {
    ui->btnCancel->show();
    ui->btnOk->show();
    ui->btnCopy->hide();
    ui->btnDel->hide();
  }

  if (btnCount == 3) {
    ui->btnCancel->show();
    ui->btnOk->show();
    ui->btnCopy->show();
    ui->btnDel->hide();
  }

  if (btnCount == 4) {
    ui->btnCancel->show();
    ui->btnOk->show();
    ui->btnCopy->show();
    ui->btnDel->show();
  }

  btn_count = btnCount;

  ui->lblTitle->setText(title);
  ui->editMsg->setText(msgtxt);

  show();
  on_btnCancel_clicked();
  init();

  while (!isHidden()) QCoreApplication::processEvents();

  return isValue;
}

void ShowMessage::on_btnCancel_clicked() {
  isValue = false;
  close();
}

void ShowMessage::on_btnOk_clicked() {
  isValue = true;
  close();
}

void ShowMessage::on_btnCopy_clicked() {
  QClipboard* clipboard = QApplication::clipboard();
  clipboard->setText(copyText);
  isValue = false;
  close();
}

void ShowMessage::on_btnDel_clicked() {
  close();
  ShowMessage* msg = new ShowMessage(mw_one);
  if (msg->showMsg("Knot", tr("Delete this link?"), 2)) {
    mw_one->m_Notes->delLink(copyText);
  }
}

QString ShowMessage::AutoFeed(QString text, int nCharCount) {
  QString strText = text;
  int AntoIndex = 1;
  if (!strText.isEmpty()) {
    for (int i = 1; i < strText.size() + 1; i++)  // 25个字符换一行
    {
      if (i == nCharCount * AntoIndex + AntoIndex - 1) {
        strText.insert(i, "\n");
        AntoIndex++;
      }
    }
  }
  return strText;
}

void ShowMessage::on_editMsg_textChanged() {}
