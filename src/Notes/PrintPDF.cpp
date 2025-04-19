#include "PrintPDF.h"

#include "src/MainWindow.h"
#include "ui_PrintPDF.h"

extern MainWindow* mw_one;
extern Method* m_Method;
extern int fontSize;
extern bool isAndroid;

PrintPDF::PrintPDF(QWidget* parent) : QDialog(parent), ui(new Ui::PrintPDF) {
  ui->setupUi(this);
  setWindowFlag(Qt::FramelessWindowHint);
  QString style = "QDialog{border-radius:0px;border:0px solid darkred;}";
  this->setStyleSheet(style);

  QFont font = this->font();
  font.setPointSize(fontSize);
  ui->listWidget->setFont(font);
  ui->listWidget->setFocus();
  QScroller::grabGesture(ui->listWidget, QScroller::LeftMouseButtonGesture);
  m_Method->setSCrollPro(ui->listWidget);

  mw_one->set_ToolButtonStyle(this);
}

PrintPDF::~PrintPDF() { delete ui; }

void PrintPDF::closeEvent(QCloseEvent* event) {
  Q_UNUSED(event)
  m_Method->closeGrayWindows();
}

bool PrintPDF::eventFilter(QObject* watch, QEvent* evn) {
  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
      on_btnCancel_clicked();
    }

    return true;
  }

  return QWidget::eventFilter(watch, evn);
}

void PrintPDF::on_btnCancel_clicked() {
  strValue = "";
  close();
}

QString PrintPDF::getItem(QString title, QString lblText, QStringList valueList,
                          int valueIndex) {
  this->setWindowTitle(title);
  ui->lblText->setText(lblText);
  ui->listWidget->clear();
  ui->listWidget->addItems(valueList);
  ui->listWidget->setCurrentRow(valueIndex);

  int x, y, w, h;
  if (isAndroid)
    w = mw_one->width() - 40;
  else
    w = 230;
  h = valueList.count() * m_Method->getFontHeight() * 1.8;
  if (h > mw_one->height()) h = mw_one->height() - 50;

  x = mw_one->geometry().x() + (mw_one->geometry().width() - w) / 2;
  y = mw_one->geometry().y() + (mw_one->geometry().height() - h) / 2;
  setGeometry(x, y, w, h);

  m_Method->m_widget = new QWidget(mw_one);
  m_Method->showGrayWindows();
  show();
  while (!isHidden()) QCoreApplication::processEvents();

  return strValue;
}

void PrintPDF::on_btnOk_clicked() {
  strValue = ui->listWidget->currentItem()->text();
  close();
}
