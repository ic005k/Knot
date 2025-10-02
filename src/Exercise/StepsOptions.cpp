#include "StepsOptions.h"

#include "src/MainWindow.h"
#include "src/defines.h"
#include "ui_StepsOptions.h"

StepsOptions::StepsOptions(QWidget *parent)
    : QDialog(parent), ui(new Ui::StepsOptions) {
  ui->setupUi(this);
  this->installEventFilter(this);
  m_Method->set_ToolButtonStyle(this);
  setModal(true);

  QValidator *validator1 =
      new QRegularExpressionValidator(regxNumber, ui->editStepLength);
  ui->editStepLength->setValidator(validator1);

  QValidator *validator2 =
      new QRegularExpressionValidator(regxNumber, ui->editStepsThreshold);
  ui->editStepsThreshold->setValidator(validator2);
}

StepsOptions::~StepsOptions() { delete ui; }

void StepsOptions::closeEvent(QCloseEvent *event) {
  Q_UNUSED(event);
  m_Method->closeGrayWindows();
}

bool StepsOptions::eventFilter(QObject *obj, QEvent *evn) {
  if (evn->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
      on_btnBack_clicked();
      return true;
    }
  }

  return QWidget::eventFilter(obj, evn);
}

void StepsOptions::init() {
  int x, y, w, h;
  x = mw_one->geometry().x();
  y = 0;
  w = mw_one->width();
  h = this->height();
  setGeometry(x, y, w, h);

  m_Method->m_widget = new QWidget(mw_one);
  m_Method->showGrayWindows();
  show();
  isTextChange = false;
}

void StepsOptions::on_btnBack_clicked() {
  mw_one->clearWidgetFocus();
  if (isTextChange) {
    mw_one->m_Steps->saveSteps();
    mw_one->m_Steps->loadStepsToTable();
  }

  close();
}

void StepsOptions::on_editStepsThreshold_textChanged(const QString &arg1) {
  Q_UNUSED(arg1);
  isTextChange = true;
}

void StepsOptions::on_editStepLength_textChanged(const QString &arg1) {
  Q_UNUSED(arg1);
  isTextChange = true;
}
