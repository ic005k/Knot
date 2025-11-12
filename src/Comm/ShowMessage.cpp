#include "ShowMessage.h"

#include "src/MainWindow.h"
#include "src/defines.h"
#include "ui_ShowMessage.h"

ShowMessage::ShowMessage(QWidget* parent)
    : QDialog(parent), ui(new Ui::ShowMessage) {
  ui->setupUi(this);

  m_ShowMessage = this;

  this->installEventFilter(this);
  ui->editMsg->viewport()->installEventFilter(this);
  ui->hframe->hide();

  setWindowFlag(Qt::FramelessWindowHint);

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

    if (keyEvent->key() == Qt::Key_Y) {
      on_btnOk_clicked();
    }

    if (keyEvent->key() == Qt::Key_N) {
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

  if (!m_Method->m_widget->isHidden()) {
    m_Method->m_widget->close();
  }

  m_Method->showGrayWindows();

  if (!isAndroid) show();

  int x, y, w, h;

#ifdef Q_OS_ANDROID
  w = mw_one->geometry().width();

#else
  w = 360;
  if (w >= mw_one->geometry().width()) {
    w = mw_one->geometry().width();
  }

#endif

  int nEditH = mw_one->m_Todo->getEditTextHeight(ui->editMsg);

  // 彻底重置布局
  setFixedSize(0, 0);
  ui->verticalLayout->invalidate();
  ui->verticalLayout->activate();
  ui->horizontalLayout->invalidate();
  ui->horizontalLayout->activate();

  int nH = 0;
  nH = nEditH + ui->btnCancel->height() + ui->lblTitle->height() +
       ui->hframe->height() + 70;

  if (nH > mw_one->height()) nH = mw_one->height() - 10;

  setFixedHeight(nH);
  setFixedWidth(w - 20);

  w = width();
  h = nH;

  x = mw_one->geometry().x() + (mw_one->geometry().width() - w) / 2;
  y = mw_one->geometry().y() + (mw_one->geometry().height() - h) / 2;
  setGeometry(x, y, w, h);

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
  auto msg = std::make_unique<ShowMessage>(this);
  if (msg->showMsg("Knot", tr("Delete this link?"), 2)) {
    m_Notes->delLink(copyText);
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
