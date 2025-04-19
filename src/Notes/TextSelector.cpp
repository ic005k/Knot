#include "TextSelector.h"

#include "src/MainWindow.h"
#include "ui_MainWindow.h"
#include "ui_TextSelector.h"

extern MainWindow *mw_one;
extern Method *m_Method;

TextSelector::TextSelector(QWidget *parent)
    : QDialog(parent), ui(new Ui::TextSelector) {
  ui->setupUi(this);

  QFont font0 = m_Method->getNewFont(19);
  QObjectList btnList =
      mw_one->getAllToolButton(mw_one->getAllUIControls(ui->frame));
  for (int i = 0; i < btnList.count(); i++) {
    QToolButton *btn = (QToolButton *)btnList.at(i);
    btn->setFont(font0);
  }

  m_Method->set_ToolButtonStyle2(this);

  ui->lineEdit->setReadOnly(true);

  this->installEventFilter(this);
  ui->lineEdit->installEventFilter(this);

  int a = 500;
  int b = 25;
  ui->btnLeft0->setAutoRepeat(true);       // 启用长按
  ui->btnLeft0->setAutoRepeatDelay(a);     // 触发长按的时间
  ui->btnLeft0->setAutoRepeatInterval(b);  // 长按时click信号间隔

  ui->btnLeft1->setAutoRepeat(true);
  ui->btnLeft1->setAutoRepeatDelay(a);
  ui->btnLeft1->setAutoRepeatInterval(b);

  ui->btnRight0->setAutoRepeat(true);
  ui->btnRight0->setAutoRepeatDelay(a);
  ui->btnRight0->setAutoRepeatInterval(b);

  ui->btnRight1->setAutoRepeat(true);
  ui->btnRight1->setAutoRepeatDelay(a);
  ui->btnRight1->setAutoRepeatInterval(b);

  oriHeight = height();

#ifdef Q_OS_ANDROID
  ui->btnShareTxt->show();
#else
  ui->btnShareTxt->hide();
#endif
}

TextSelector::~TextSelector() { delete ui; }

void TextSelector::on_btnClose_clicked() {
  ui->lineEdit->clear();
  close();
}

void TextSelector::init(int y) {
  setGeometry(mw_one->geometry().x() + (mw_one->width() - this->width()) / 2, y,
              this->width(), this->height());
  setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);

  show();
}

bool TextSelector::eventFilter(QObject *watch, QEvent *evn) {
  QMouseEvent *event = static_cast<QMouseEvent *>(evn);
  if (event->type() == QEvent::MouseButtonPress) {
    isMousePress = true;
    isMouseRelease = false;
  }

  if (event->type() == QEvent::MouseButtonRelease) {
    isMousePress = false;
    isMouseMove = false;
    isMouseRelease = true;
  }

  if (event->type() == QEvent::MouseMove) {
    isMouseMove = true;

    if (isMousePress) {
      if (watch == this || watch == ui->lineEdit) {
        int y = event->globalY();
        if (y <= 0) y = 0;
        if (y >= mw_one->height() - height()) y = mw_one->height() - height();
        this->setGeometry(geometry().x(), y, width(), height());
      }
    }
  }

  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
      on_btnClose_clicked();
      return true;
    }
  }

  if (watch == ui->lineEdit) {
    if (event->type() == QEvent::MouseMove) {
      return true;
    }

    if (event->type() == QEvent::MouseButtonDblClick) {
      return true;
    }
  }

  return QWidget::eventFilter(watch, evn);
}

void TextSelector::on_btnCopy_clicked() {
  mw_one->m_Notes->byTextEdit->copy();

  ui->lineEdit->clear();

  close();
}

void TextSelector::on_btnCut_clicked() {
  mw_one->m_Notes->byTextEdit->cut();

  close();
}

void TextSelector::on_btnPaste_clicked() {
  strByCopyText = QApplication::clipboard()->text();

  QString str = strByCopyText;
  if (mw_one->m_Notes->byTextEdit == mw_one->ui->editTodo) {
    if (str.length() > 50) {
      str = str.mid(0, 50);
    }
  } else {
    if (str.length() > 200) {
      str = str.mid(0, 200);
    }
  }
  QApplication::clipboard()->setText(str);
  mw_one->m_Notes->byTextEdit->paste();
  QApplication::clipboard()->setText(strByCopyText);

  close();
}

void TextSelector::on_btnSetAll_clicked() {
  mw_one->m_Notes->byTextEdit->selectAll();
}

void TextSelector::on_btnLeft1_clicked() {
  mw_one->m_Notes->start--;
  if (mw_one->m_Notes->start < 0) mw_one->m_Notes->start = 0;

  mw_one->m_Notes->selectText(mw_one->m_Notes->start, mw_one->m_Notes->end);
  ui->lineEdit->setCursorPosition(0);
}

void TextSelector::on_btnLeft0_clicked() {
  mw_one->m_Notes->start++;
  if (mw_one->m_Notes->start >= mw_one->m_Notes->end)
    mw_one->m_Notes->start = mw_one->m_Notes->end - 1;

  mw_one->m_Notes->selectText(mw_one->m_Notes->start, mw_one->m_Notes->end);
  ui->lineEdit->setCursorPosition(0);
}

void TextSelector::on_btnRight1_clicked() {
  mw_one->m_Notes->end++;

  mw_one->m_Notes->selectText(mw_one->m_Notes->start, mw_one->m_Notes->end);

  if (ui->lineEdit->text().trimmed() == "") {
    mw_one->m_Notes->end--;
    mw_one->m_Notes->selectText(mw_one->m_Notes->start, mw_one->m_Notes->end);
  }
}

void TextSelector::on_btnRight0_clicked() {
  mw_one->m_Notes->end--;
  if (mw_one->m_Notes->end <= mw_one->m_Notes->start)
    mw_one->m_Notes->end = mw_one->m_Notes->start + 1;

  mw_one->m_Notes->selectText(mw_one->m_Notes->start, mw_one->m_Notes->end);
}

void TextSelector::on_btnBing_clicked() {
  QString str = ui->lineEdit->text().trimmed();
  if (str.length() > 0) {
    QString strurl = "https://bing.com/search?q=" + str;
    QUrl url(strurl);
    QDesktopServices::openUrl(url);
    on_btnClose_clicked();
  }
}

void TextSelector::on_btnDel_clicked() {
  mw_one->m_Notes->byTextEdit->textCursor().removeSelectedText();
  on_btnClose_clicked();
}

void TextSelector::on_btnShareTxt_clicked() {
  QString txt = ui->lineEdit->text();
  txt = txt.trimmed();
  if (txt != "") {
    m_Method->closeKeyboard();
    mw_one->m_ReceiveShare->shareString(tr("Share to"), txt);
  }
}
