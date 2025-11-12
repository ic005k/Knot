#include "StepsOptions.h"

#include "src/MainWindow.h"
#include "src/defines.h"
#include "ui_MainWindow.h"
#include "ui_StepsOptions.h"

StepsOptions::StepsOptions(QWidget* parent)
    : QDialog(parent), ui(new Ui::StepsOptions) {
  ui->setupUi(this);
  this->installEventFilter(this);
  m_Method->set_ToolButtonStyle(this);
  setModal(true);

  QValidator* validator1 =
      new QRegularExpressionValidator(regxNumber, ui->editStepLength);
  ui->editStepLength->setValidator(validator1);

  QValidator* validator2 =
      new QRegularExpressionValidator(regxNumber, ui->editStepsThreshold);
  ui->editStepsThreshold->setValidator(validator2);
}

StepsOptions::~StepsOptions() { delete ui; }

void StepsOptions::closeEvent(QCloseEvent* event) {
  Q_UNUSED(event);

  QSettings Reg(iniDir + "gpslist.ini", QSettings::IniFormat);
  Reg.setValue("/Map/MapKey", ui->editMapKey->toPlainText().trimmed());
  Reg.sync();

  m_Steps->setMapKey();
  m_Steps->getAddress(25.0217, 98.4464);
  m_Method->closeGrayWindows();
}

bool StepsOptions::eventFilter(QObject* obj, QEvent* evn) {
  if (evn->type() == QEvent::KeyPress) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
      on_btnBack_clicked();
      return true;
    }
  }

  return QWidget::eventFilter(obj, evn);
}

void StepsOptions::init() {
  if (m_Steps->timer->isActive()) return;

  int x, y, w, h;
  x = mw_one->geometry().x();
  y = 0;
  w = mw_one->width();
  h = 400;
  setGeometry(x, y, w, h);

  m_Method->m_widget = new QWidget(mw_one);
  m_Method->showGrayWindows();
  show();
  isTextChange = false;

  ui->btnWeb->setFixedHeight(ui->btnTestKey->height());

  // init edit toolbar
  initTextToolbarDynamic(this);
  if (editFilter != nullptr) {
    ui->editMapKey->removeEventFilter(editFilter);
    delete editFilter;
    editFilter = nullptr;
  }
  editFilter = new EditEventFilter(textToolbarDynamic, this);
  ui->editMapKey->installEventFilter(editFilter);
  ui->editMapKey->viewport()->installEventFilter(editFilter);
}

void StepsOptions::on_btnBack_clicked() {
  mw_one->clearWidgetFocus();
  if (isTextChange) {
    m_Steps->saveSteps();
    m_Steps->loadStepsToTable();
  }

  close();
}

void StepsOptions::on_editStepsThreshold_textChanged(const QString& arg1) {
  Q_UNUSED(arg1);
  isTextChange = true;
}

void StepsOptions::on_editStepLength_textChanged(const QString& arg1) {
  Q_UNUSED(arg1);
  isTextChange = true;
}

void StepsOptions::on_btnWeb_clicked() {
  QDesktopServices::openUrl(QUrl("https://lbs.qq.com/"));
}

void StepsOptions::on_editMapKey_textChanged() {}

void StepsOptions::on_btnTestKey_clicked() {
  QString arg1 = ui->editMapKey->toPlainText().trimmed();
  if (m_Steps->addressResolver) {
    m_Steps->strMapKeyTestInfo = "";
    m_Steps->addressResolver->setTencentApiKey(arg1);
    m_Steps->getAddress(22.529800, 113.934533);

    while (m_Steps->strMapKeyTestInfo == "")
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    auto msg = std::make_unique<ShowMessage>(this);
    msg->showMsg(appName, m_Steps->strMapKeyTestInfo, 1);

    // 确保编辑框内容不丢失
    ui->editMapKey->setPlainText(arg1);
  }
}

void StepsOptions::on_rbOsm_clicked(bool checked) {
  ui->rbTencent->setChecked(!checked);
  QSettings Reg(iniDir + "gpslist.ini", QSettings::IniFormat);
  Reg.setValue("/Map/MapType1", checked);
  Reg.sync();
}

void StepsOptions::on_rbTencent_clicked(bool checked) {
  ui->rbOsm->setChecked(!checked);
  QSettings Reg(iniDir + "gpslist.ini", QSettings::IniFormat);
  Reg.setValue("/Map/MapType2", checked);
  Reg.sync();
}
