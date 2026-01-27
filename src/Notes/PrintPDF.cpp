#include "PrintPDF.h"

#include "src/MainWindow.h"
#include "src/defines.h"
#include "ui_PrintPDF.h"

// Qt 6 头文件适配：包含QScreen
#include <QApplication>
#include <QScreen>

PrintPDF* PrintPDF::m_PrintPDF = nullptr;

PrintPDF::PrintPDF(QWidget* parent) : QDialog(parent), ui(new Ui::PrintPDF) {
  ui->setupUi(this);
  setWindowFlag(Qt::FramelessWindowHint);
  setModal(true);  // 设置模态对话框
  QString style = "QDialog{border-radius:0px;border:0px solid darkred;}";
  this->setStyleSheet(style);

  m_PrintPDF = this;

  // 修复：Qt 6 获取屏幕可用尺寸（替代QApplication::desktop()）
  QScreen* screen = this->screen();  // 获取当前窗口所在屏幕
  if (!screen) {
    screen = QGuiApplication::primaryScreen();  // 兜底：使用主屏
  }
  QRect screenRect = screen->availableGeometry();  // 可用区域（排除状态栏）

  int fixedHeight = screenRect.height() * 0.8;
  setFixedHeight(fixedHeight);

  QFont font = this->font();
  font.setPointSize(fontSize);
  ui->listWidget->setFont(font);
  ui->listWidget->setFocus();
  QScroller::grabGesture(ui->listWidget, QScroller::LeftMouseButtonGesture);

  if (m_Method) {
    m_Method->setSCrollPro(ui->listWidget);
    m_Method->set_ToolButtonStyle(this);
  }

  this->installEventFilter(this);
}

PrintPDF::~PrintPDF() {
  delete ui;
  // 析构时强制清理遮罩层
  if (m_Method) {
    m_Method->closeGrayWindows();
  }
}

// 重写hideEvent：确保隐藏时清理遮罩层
void PrintPDF::hideEvent(QHideEvent* event) {
  Q_UNUSED(event);
  if (m_Method) {
    m_Method->closeGrayWindows();
  }
  // 调用父类实现（必须）
  QDialog::hideEvent(event);
}

void PrintPDF::closeEvent(QCloseEvent* event) {
  Q_UNUSED(event)
  // 关闭时清理遮罩层
  if (m_Method) {
    m_Method->closeGrayWindows();
  }
  QDialog::closeEvent(event);
}

bool PrintPDF::eventFilter(QObject* watch, QEvent* evn) {
  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
      on_btnCancel_clicked();
      return true;
    }
  }
  return QWidget::eventFilter(watch, evn);
}

void PrintPDF::on_btnCancel_clicked() {
  strValue = "";
  this->reject();
  // 主动清理遮罩层
  if (m_Method) {
    m_Method->closeGrayWindows();
  }
  close();
}

void PrintPDF::on_btnOk_clicked() {
  if (ui->listWidget->currentItem()) {
    strValue = ui->listWidget->currentItem()->text();
  } else {
    strValue = "";
  }
  this->accept();
  // 主动清理遮罩层
  if (m_Method) {
    m_Method->closeGrayWindows();
  }
  close();
}

QString PrintPDF::getItem(QString title, QString lblText, QStringList valueList,
                          int valueIndex) {
  this->setWindowTitle(title);
  ui->lblText->setText(lblText);
  ui->listWidget->clear();
  ui->listWidget->addItems(valueList);
  ui->listWidget->setCurrentRow(valueIndex);

  // 修复：Qt 6 跨平台居中逻辑
  QScreen* screen = this->screen();
  if (!screen) {
    screen = QGuiApplication::primaryScreen();
  }
  QRect screenRect = screen->availableGeometry();

  int w = 230;
  if (isAndroid) {
    w = screenRect.width() - 40;  // 安卓端适配屏幕宽度
  }
  int h = this->height();

  // 限制最大高度
  if (h > screenRect.height() - 50) {
    h = screenRect.height() - 50;
  }

  // 计算居中坐标（确保在屏幕内）
  int x = (screenRect.width() - w) / 2 + screenRect.x();
  int y = (screenRect.height() - h) / 2 + screenRect.y();
  x = qMax(x, screenRect.x());
  y = qMax(y, screenRect.y());

  this->setGeometry(x, y, w, h);

  // 显示遮罩层
  if (m_Method) {
    m_Method->showGrayWindows();
  }

  // 执行模态对话框
  int result = this->exec();

  // 最终兜底清理遮罩层
  if (m_Method) {
    m_Method->closeGrayWindows();
  }

  return (result == QDialog::Accepted) ? strValue : "";
}
