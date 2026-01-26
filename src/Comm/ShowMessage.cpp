#include "ShowMessage.h"

#include "src/MainWindow.h"
#include "src/defines.h"
#include "ui_ShowMessage.h"

ShowMessage::ShowMessage(QWidget* parent)
    : QDialog(parent), ui(new Ui::ShowMessage) {
  ui->setupUi(this);

  // 基础设置：无边框+原生模态
  setWindowFlag(Qt::FramelessWindowHint);
  setModal(true);
  setWindowModality(Qt::ApplicationModal);

  // 文本控件设置
  QFont font = this->font();
  font.setBold(true);
  ui->lblTitle->setFont(font);
  ui->lblTitle->setWordWrap(true);

  ui->editMsg->setReadOnly(true);
  ui->editMsg->setLineWrapMode(QTextEdit::WidgetWidth);
  ui->editMsg->verticalScrollBar()->setStyleSheet(m_Method->vsbarStyleSmall);
  QScroller::grabGesture(ui->editMsg, QScroller::LeftMouseButtonGesture);

  // 主窗口指针初始化
  mw_one = dynamic_cast<MainWindow*>(parent);
  if (mw_one == nullptr) {
    qWarning() << "MainWindow pointer is null!";
  }

  // 移除标题栏下的分割线（彻底隐藏）
  ui->hframe->hide();
  ui->hframe->setVisible(false);

  // 按钮样式（保留原逻辑）
  QString btnStyle = ui->btnOk->styleSheet();
  ui->btnCancel->setStyleSheet(btnStyle);
  ui->btnCopy->setStyleSheet(btnStyle);
  ui->btnDel->setStyleSheet(btnStyle);

  // ========== 核心：恢复最初的事件过滤（确保快捷键生效） ==========
  // 给消息框自身安装事件过滤器
  this->installEventFilter(this);
  // 给文本控件安装事件过滤器（最初的逻辑，确保按键能被捕获）
  ui->editMsg->viewport()->installEventFilter(this);

  // 初始隐藏
  this->hide();
}

ShowMessage::~ShowMessage() { delete ui; }

void ShowMessage::init(int btnCount) {
  isValue = false;
  btn_count = btnCount;

  // 空指针保护：主窗口为空则用默认尺寸
  int mainW = 360, mainH = 600;
  if (mw_one) {
    mainW = mw_one->geometry().width();
    mainH = mw_one->geometry().height();
  }

  // 1. 宽度逻辑（保留现有规则）
  int dlgW = 0;
#ifdef Q_OS_ANDROID
  dlgW = mainW;
#else
  dlgW = 360;
  if (dlgW >= mainW) dlgW = mainW;
#endif
  dlgW -= 20;

  // 2. 高度逻辑（固定为主窗口3/4）
  int dlgH;
  if (isAndroid)
    dlgH = mainH * 3 / 4;
  else
    dlgH = mainH * 2 / 3;
  if (dlgH < 200) dlgH = 200;

  // 3. 设置消息框尺寸
  setFixedSize(dlgW, dlgH);

  // 4. 居中计算
  int x = 0, y = 0;
  if (mw_one) {
    x = mw_one->geometry().x() + (mainW - dlgW) / 2;
    y = mw_one->geometry().y() + (mainH - dlgH) / 2;
  } else {
    QScreen* primaryScreen = QGuiApplication::primaryScreen();
    if (primaryScreen) {
      QRect screenRect = primaryScreen->availableGeometry();
      x = (screenRect.width() - dlgW) / 2;
      y = (screenRect.height() - dlgH) / 2;
    } else {
      x = 100;
      y = 100;
    }
  }
  setGeometry(x, y, dlgW, dlgH);

  // 滚动条设置
  ui->editMsg->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

bool ShowMessage::showMsg(QString title, QString msgtxt, int btnCount) {
  // 按钮数量处理
  if (btnCount == 2 || btnCount == 3) {
    msgtxt += "\n\n";
  }
  if (btnCount == 1) btnCount = 0;

  // 按钮显示控制
  ui->btnCancel->hide();
  ui->btnOk->hide();
  ui->btnCopy->hide();
  ui->btnDel->hide();

  switch (btnCount) {
    case 0:
      ui->btnOk->show();
      break;
    case 2:
      ui->btnOk->show();
      ui->btnCancel->show();
      break;
    case 3:
      ui->btnOk->show();
      ui->btnCancel->show();
      ui->btnCopy->show();
      break;
    case 4:
      ui->btnOk->show();
      ui->btnCancel->show();
      ui->btnCopy->show();
      ui->btnDel->show();
      break;
    default:
      ui->btnOk->show();
      break;
  }

  // 设置标题和文本
  ui->lblTitle->setText(title);
  ui->editMsg->setText(msgtxt);

  // 初始化尺寸+居中
  init(btnCount);

  // 全程用exec()
  this->exec();

  return isValue;
}

// 按钮点击逻辑
void ShowMessage::on_btnCancel_clicked() {
  isValue = false;
  close();  // 回归最初的close()，而非reject()，确保逻辑一致
}

void ShowMessage::on_btnOk_clicked() {
  isValue = true;
  close();  // 回归最初的close()
}

void ShowMessage::on_btnCopy_clicked() {
  QClipboard* clipboard = QApplication::clipboard();
  if (clipboard) {
    clipboard->setText(copyText);
  }
  isValue = false;
  close();  // 回归最初的close()
}

void ShowMessage::on_btnDel_clicked() {
  close();
  auto delMsg = std::make_unique<ShowMessage>(mw_one);
  if (delMsg->showMsg("Knot", tr("Delete this link?"), 2) && m_Notes) {
    m_Notes->delLink(copyText);
  }
}

// 保留AutoFeed方法
QString ShowMessage::AutoFeed(QString text, int nCharCount) {
  QString strText = text;
  int AntoIndex = 1;
  if (!strText.isEmpty()) {
    for (int i = 1; i < strText.size() + 1; i++) {
      if (i == nCharCount * AntoIndex + AntoIndex - 1) {
        strText.insert(i, "\n");
        AntoIndex++;
      }
    }
  }
  return strText;
}

// ========== 完全回归最初的eventFilter逻辑（快捷键核心） ==========
bool ShowMessage::eventFilter(QObject* watch, QEvent* evn) {
  // 最初的按键释放事件处理（100%复刻）
  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evn);
    // Y键 → 确认
    if (keyEvent->key() == Qt::Key_Y) {
      on_btnOk_clicked();
      return true;  // 拦截事件，防止穿透
    }
    // N键 → 取消
    if (keyEvent->key() == Qt::Key_N) {
      on_btnCancel_clicked();
      return true;
    }
    // Back键 → 取消
    if (keyEvent->key() == Qt::Key_Back) {
      on_btnCancel_clicked();
      return true;
    }
  }

  // 最初的鼠标点击事件处理（保留）
  if (evn->type() == QEvent::MouseButtonPress) {
    if (btn_count == 0) {
      on_btnCancel_clicked();
      return true;
    }
  }

  // 传递未处理的事件
  return QWidget::eventFilter(watch, evn);
}

// 空实现
void ShowMessage::on_editMsg_textChanged() {}
