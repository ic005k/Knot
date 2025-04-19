#include "SetReaderText.h"

#include "src/MainWindow.h"
#include "ui_SetReaderText.h"

extern MainWindow *mw_one;
extern Method *m_Method;

dlgSetText::dlgSetText(QWidget *parent)
    : QDialog(parent), ui(new Ui::dlgSetText) {
  ui->setupUi(this);

  mw_one->set_ToolButtonStyle(this);

  ui->lineEdit->setReadOnly(true);

#ifdef Q_OS_ANDROID
  ui->btnShare->show();
#else
  ui->btnShare->hide();
#endif
}

dlgSetText::~dlgSetText() { delete ui; }

void dlgSetText::init(int x, int y, int w, int h) {
  setGeometry(x, y, w, h);
  setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
  show();
}

bool dlgSetText::eventFilter(QObject *watch, QEvent *evn) {
  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
      close();
      return true;
    }
  }

  return QWidget::eventFilter(watch, evn);
}

void dlgSetText::on_btnBack_clicked() { mw_one->on_btnCancelSel_clicked(); }

void dlgSetText::on_btnCopy_clicked() { mw_one->on_btnCopy_clicked(); }

void dlgSetText::on_btnSearch_clicked() { mw_one->on_btnSearch_clicked(); }

void dlgSetText::on_lineEdit_textChanged(const QString &arg1) {
  Q_UNUSED(arg1);
}

void dlgSetText::on_btnShare_clicked() {
  QString txt = ui->lineEdit->text().trimmed();
  if (txt.length() > 0) {
    mw_one->m_ReceiveShare->shareString(tr("Share to"), txt);
    close();
  }
}
