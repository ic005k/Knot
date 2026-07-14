#include "ShowMessage.h"

#include "src/MainWindow.h"
#include "src/defines.h"
#include "ui_MainWindow.h"
#include "ui_ShowMessage.h"

ShowMessage::ShowMessage(QWidget* parent)
    : QDialog(parent), ui(new Ui::ShowMessage) {
  ui->setupUi(this);

  m_MsgBox = this;

  setWindowFlag(Qt::FramelessWindowHint);

#ifndef Q_OS_ANDROID
  setWindowFlag(Qt::WindowStaysOnTopHint);
#else
  setWindowFlag(Qt::WindowStaysOnTopHint, false);
#endif
  setAttribute(Qt::WA_MacAlwaysShowToolWindow, true);
  setWindowModality(Qt::WindowModal);

  if (ui->qwShowMsg->source().isEmpty()) {
    ui->qwShowMsg->rootContext()->setContextProperty("isDark", isDark);
    ui->qwShowMsg->rootContext()->setContextProperty("m_Method", m_Method);
    ui->qwShowMsg->rootContext()->setContextProperty("mw_one", mw_one);
    ui->qwShowMsg->rootContext()->setContextProperty("textContent", "");
    ui->qwShowMsg->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/showmsg.qml")));
  }

  auto* quickWindow = ui->qwShowMsg->quickWindow();
  QObject::connect(
      quickWindow, &QQuickWindow::afterRendering, this,
      []() { mw_one->safeCloseProgress(); },
      Qt::SingleShotConnection  // 自动断开
  );

  // 文本控件设置
  QFont font = this->font();
  font.setBold(true);
  ui->lblTitle->setFont(font);
  ui->lblTitle->setWordWrap(true);

  ui->editMsg->setReadOnly(true);
  ui->editMsg->setLineWrapMode(QTextEdit::WidgetWidth);
  ui->editMsg->verticalScrollBar()->setStyleSheet(m_Method->vsbarStyleSmall);
  QScroller::grabGesture(ui->editMsg, QScroller::LeftMouseButtonGesture);

  // 移除标题栏下的分割线（彻底隐藏）
  ui->hframe->hide();
  ui->hframe->setVisible(false);

  // 按钮样式（保留原逻辑）
  QString btnStyle = ui->btnOk->styleSheet();
  ui->btnCancel->setStyleSheet(btnStyle);
  ui->btnCopy->setStyleSheet(btnStyle);
  ui->btnDel->setStyleSheet(btnStyle);

  // 给消息框自身安装事件过滤器
  this->installEventFilter(this);
  // 给文本控件安装事件过滤器（
  ui->editMsg->viewport()->installEventFilter(this);
  ui->editMsg->hide();

  // 初始隐藏
  this->hide();
}

ShowMessage::~ShowMessage() {
  delete ui;
  if (m_MsgBox != nullptr) {
    m_MsgBox = nullptr;
  }
}

void ShowMessage::init(int btnCount, int adaptiveH) {
  isValue = false;
  btn_count = btnCount;

  int mainW = 0, mainH = 0;
  if (mw_one) {
    mainW = mw_one->geometry().width();
    mainH = mw_one->geometry().height();
  }

  // 宽度逻辑
  int dlgW = 380;
#ifdef Q_OS_ANDROID
  dlgW = mainW;
#else

#endif

  dlgW -= 4;

  // 高度逻辑
  int dlgH = adaptiveH;
  setMinimumSize(dlgW, 200);
  resize(dlgW, dlgH);

  // 居中计算
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
  m_Method->Sleep(100);

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
  QString showText = msgtxt + "\n";
  ui->editMsg->setText(showText);

  ui->lblTitle->ensurePolished();
  ui->lblTitle->updateGeometry();
  ui->editMsg->document()->adjustSize();
  ui->editMsg->updateGeometry();

  showText = markdownToHtmlWithMath(showText);
  ui->qwShowMsg->rootContext()->setContextProperty("textContent", showText);
  // ui->editMsg->setHtml(showText);

  // 计算高度、初始化弹窗尺寸（窗口隐藏状态下计算，无GL surface争夺）
  int textH = getTextEditContentHeight(ui->editMsg);
  int adaptiveDlgH = calcDialogTotalHeight(textH);
  init(btnCount, adaptiveDlgH);

  this->exec();

  return isValue;
}

// 按钮点击逻辑
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
  if (clipboard) {
    clipboard->setText(copyText);
  }
  isValue = false;
  close();
}

void ShowMessage::on_btnDel_clicked() {
  close();
  auto delMsg = std::make_unique<ShowMessage>(mw_one);
  if (delMsg->showMsg("Knot", tr("Delete this link?"), 2) && m_Notes) {
    m_Notes->delLink(copyText);
  }
}

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

bool ShowMessage::eventFilter(QObject* watch, QEvent* evn) {
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

  // 鼠标点击事件处理
  if (evn->type() == QEvent::MouseButtonPress) {
    if (btn_count == 0) {
      // on_btnCancel_clicked();
      return true;
    }
  }

  // 传递未处理的事件
  return QWidget::eventFilter(watch, evn);
}

// 空实现
void ShowMessage::on_editMsg_textChanged() {}

int ShowMessage::getTextEditContentHeight(QTextEdit* edit) {
  QTextDocument* doc = edit->document();
  // 强制文档重新布局，保证高度计算准确
  doc->adjustSize();
  // 文档完整高度 + 编辑框上下内边距
  int docH = doc->size().height();
  QMargins margins = edit->contentsMargins();
  return docH + margins.top() + margins.bottom();
}

int ShowMessage::calcDialogTotalHeight(int textH) {
  // 固定UI高度常量，根据你的ui布局微调数值
  const int titleHeight = ui->lblTitle->sizeHint().height();  // 标题高度
  const int titleMarginTop = 12;                              // 标题上边距
  const int titleEditSpace = 10;  // 标题与文本框间距
  const int editBtnSpace = 15;    // 文本框与按钮区间距
  const int btnAreaHeight = 45;   // 按钮区域固定高度
  const int bottomMargin = 12;    // 对话框底部边距
  const int topMargin = 8;        // 对话框顶部边距

  // 所有固定部件高度总和
  int fixedPartH = topMargin + titleMarginTop + titleHeight + titleEditSpace +
                   editBtnSpace + btnAreaHeight + bottomMargin;

  // 文本高度 + 固定区域 = 对话框总高度
  int totalH = fixedPartH + textH;

  // 高度上下限约束
  int minH = 200;
  int screenMaxH;
  if (mw_one)
    screenMaxH = mw_one->height() * 3 / 4;
  else
    screenMaxH =
        QGuiApplication::primaryScreen()->availableGeometry().height() * 3 / 4;

  // 低于最小值取最小高度，高于屏幕上限则用上限（开启滚动）
  if (totalH < minH) totalH = minH;
  if (totalH > screenMaxH) totalH = screenMaxH;

  return totalH;
}

void ShowMessage::closeEvent(QCloseEvent* event) { Q_UNUSED(event); }
