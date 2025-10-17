#include "TextEditToolbar.h"

#include <QApplication>
#include <QDebug>
#include <QFontMetrics>
#include <QInputMethodEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QStyleOption>
#include <QTimer>
#include <QTouchEvent>

// ========================== HandleWidget 实现 ==========================
HandleWidget::HandleWidget(QWidget *parent)
    : QWidget(parent,
              Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool) {
#ifdef Q_OS_ANDROID
  setFixedSize(32, 32);
#else
  setFixedSize(16, 16);
#endif
  // setAttribute(Qt::WA_TranslucentBackground);
  setMouseTracking(true);
  setCursor(Qt::OpenHandCursor);
}

void HandleWidget::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);  // 抗锯齿保持，让矩形边角平滑

  QBrush brush(QColor(52, 152, 219));  // 外圈颜色
  painter.setBrush(brush);
  // 画外矩形（比原椭圆略小，留出边距）
  painter.drawRect(rect().adjusted(2, 2, -2, -2));

  painter.setBrush(Qt::white);  // 内矩形颜色
  // 画内矩形
  painter.drawRect(rect().adjusted(8, 8, -8, -8));
}

// ========================== HandleWidget 修复实现 ==========================

void HandleWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    m_dragOffset = event->pos();
    m_isDragging = true;
    setCursor(Qt::ClosedHandCursor);

    // 捕获鼠标，确保所有鼠标事件都发送到这个部件
    grabMouse();

    emit pressed();
    event->accept();
  } else {
    event->ignore();
  }
}

void HandleWidget::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    m_isDragging = false;
    setCursor(Qt::OpenHandCursor);

    // 释放鼠标捕获
    if (mouseGrabber() == this) {
      releaseMouse();
    }

    emit released();
    event->accept();
  } else {
    event->ignore();
  }
}

void HandleWidget::mouseMoveEvent(QMouseEvent *event) {
  if (m_isDragging) {
    // 确保只有左键拖动时才处理
    if (event->buttons() & Qt::LeftButton) {
      QPoint newGlobalPos = event->globalPosition().toPoint() - m_dragOffset;
      move(newGlobalPos);
      emit moved(newGlobalPos);
      event->accept();
    } else {
      // 如果没有左键按下，但m_isDragging为true，说明状态不一致，重置状态
      m_isDragging = false;
      setCursor(Qt::OpenHandCursor);
      if (mouseGrabber() == this) {
        releaseMouse();
      }
      event->ignore();
    }
  } else {
    event->ignore();
  }
}

// ========================== TextEditToolbar 实现 ==========================

TextEditToolbar::TextEditToolbar(QWidget *parent) : QWidget(parent) {
  // 窗口属性设置
  setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);
  // setAttribute(Qt::WA_TranslucentBackground);
  installEventFilter(this);
  setFocusPolicy(Qt::StrongFocus);

  // 初始化控件
  initButtons();
  initHandles();

  // ===================== 布局重构开始 =====================
  QVBoxLayout *mainVLayout = new QVBoxLayout(this);
  mainVLayout->setSpacing(5);                   // 两排垂直间距（不变）
  mainVLayout->setContentsMargins(8, 8, 8, 8);  // 主布局边距（不变）

  // 第一排：上排按钮 - 明确左对齐，间距5
  QHBoxLayout *topHLayout = new QHBoxLayout();
  topHLayout->setSpacing(5);  // 统一间距为5（与adjustButtonSizes一致）
  topHLayout->setAlignment(
      Qt::AlignLeft);  // 明确左对齐，确保第一个按钮起始位置统一
  topHLayout->addWidget(btnMinus);
  topHLayout->addWidget(btnPlus);
  topHLayout->addWidget(btnCopy);
  topHLayout->addWidget(btnCut);
  topHLayout->addWidget(btnPaste);

  // 第二排：下排按钮 - 同样左对齐，间距5（与上排完全一致）
  QHBoxLayout *bottomHLayout = new QHBoxLayout();
  bottomHLayout->setSpacing(5);  // 与上排统一间距
  bottomHLayout->setAlignment(
      Qt::AlignLeft);  // 明确左对齐，与上排第一个按钮对齐
  bottomHLayout->addWidget(btnCursorLeft);
  bottomHLayout->addWidget(btnCursorRight);
  bottomHLayout->addWidget(btnClose);
  bottomHLayout->addWidget(btnSelectAll);
  bottomHLayout->addWidget(btnDel);
  bottomHLayout->addSpacerItem(  // 保留弹簧，仅占用右侧空间，不影响左对齐
      new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

  mainVLayout->addLayout(topHLayout);
  mainVLayout->addLayout(bottomHLayout);
  setLayout(mainVLayout);
  // ===================== 布局重构结束 =====================

  // 样式设置
  setStyleSheet(R"(
        QWidget {
            background: rgba(255, 255, 255, 0.95);
            border: 1px solid #ccc;
            border-radius: 6px;
            font-size: 16px;
        }
        QPushButton {
            color: #333;
            background: #f0f0f0;
            border-radius: 4px;
            padding: 6px;
            font-weight: bold;
            text-align: center;
        }
        QPushButton:pressed {
            background: #d0d0d0;
        }
    )");

  // 调整按钮尺寸
  adjustButtonSizes();
  hide();

  connect(m_startHandle, &HandleWidget::pressed, this,
          &TextEditToolbar::onStartHandlePressed);
  connect(m_endHandle, &HandleWidget::pressed, this,
          &TextEditToolbar::onEndHandlePressed);
  connect(m_startHandle, &HandleWidget::released, this,
          &TextEditToolbar::onHandleReleased);
  connect(m_endHandle, &HandleWidget::released, this,
          &TextEditToolbar::onHandleReleased);
}

void TextEditToolbar::initButtons() {
  // 初始化按钮
  btnMinus = new QPushButton("<-", this);
  btnPlus = new QPushButton("->", this);
  btnCopy = new QPushButton(tr("Copy"), this);
  btnCut = new QPushButton(tr("Cut"), this);
  btnPaste = new QPushButton(tr("Paste"), this);
  btnSelectAll = new QPushButton(tr("All"), this);

  btnCursorLeft = new QPushButton("->", this);
  btnCursorRight = new QPushButton("<-", this);
  btnDel = new QPushButton(tr("Del"), this);
  btnClose = new QPushButton(tr("Close"), this);

  // 开启长按连续触发（使用默认值：延迟 500ms，间隔 100ms）
  btnMinus->setAutoRepeat(true);
  btnPlus->setAutoRepeat(true);
  btnCursorLeft->setAutoRepeat(true);
  btnCursorRight->setAutoRepeat(true);

  // 绑定点击信号槽
  connect(btnMinus, &QPushButton::clicked, this,
          &TextEditToolbar::onMinusClicked);

  // connect(btnMinus, &QPushButton::pressed, this,
  //         &TextEditToolbar::onMinusPressed);
  // connect(btnMinus, &QPushButton::released, this,
  //         &TextEditToolbar::onMinusReleased);

  connect(btnPlus, &QPushButton::clicked, this,
          &TextEditToolbar::onPlusClicked);
  connect(btnCopy, &QPushButton::clicked, this,
          &TextEditToolbar::onCopyClicked);
  connect(btnCut, &QPushButton::clicked, this, &TextEditToolbar::onCutClicked);
  connect(btnPaste, &QPushButton::clicked, this,
          &TextEditToolbar::onPasteClicked);
  connect(btnSelectAll, &QPushButton::clicked, this,
          &TextEditToolbar::onSelectAllClicked);

  connect(btnCursorLeft, &QPushButton::clicked, this,
          &TextEditToolbar::onCursorLeftClicked);
  connect(btnCursorRight, &QPushButton::clicked, this,
          &TextEditToolbar::onCursorRightClicked);
  connect(btnDel, &QPushButton::clicked, this, &TextEditToolbar::onDelClicked);

  connect(btnClose, &QPushButton::clicked, this,
          &TextEditToolbar::onCloseClicked);
}

void TextEditToolbar::initHandles() {
  // 初始化选择手柄
  m_startHandle = new HandleWidget(this);
  m_endHandle = new HandleWidget(this);
  m_startHandle->hide();
  m_endHandle->hide();

  // 绑定手柄移动信号槽
  connect(m_startHandle, &HandleWidget::moved, this,
          &TextEditToolbar::onStartHandleMoved);
  connect(m_endHandle, &HandleWidget::moved, this,
          &TextEditToolbar::onEndHandleMoved);
}

void TextEditToolbar::adjustButtonSizes() {
  const int BUTTON_HEIGHT = 42;
  const int BUTTON_WIDTH = 55;
  const int TOOLBAR_WIDTH = 312;
  const int TOOLBAR_HEIGHT = 108;

  // 统一设置按钮大小
  QList<QPushButton *> buttons = {
      btnMinus,     btnPlus,       btnCopy,        btnCut, btnPaste,
      btnSelectAll, btnCursorLeft, btnCursorRight, btnDel, btnClose};
  for (auto btn : buttons) {
    btn->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
  }

  // 更新工具栏固定尺寸
  setFixedSize(TOOLBAR_WIDTH, TOOLBAR_HEIGHT);

  // 仅保留「边距清零」逻辑，删除重复的spacing设置
  if (QVBoxLayout *mainVLayout = qobject_cast<QVBoxLayout *>(layout())) {
    // 第一排布局：仅清零边距
    if (QHBoxLayout *topHLayout =
            qobject_cast<QHBoxLayout *>(mainVLayout->itemAt(0)->layout())) {
      topHLayout->setContentsMargins(0, 0, 0, 0);
    }
    // 第二排布局：仅清零边距
    if (QHBoxLayout *bottomHLayout =
            qobject_cast<QHBoxLayout *>(mainVLayout->itemAt(1)->layout())) {
      bottomHLayout->setContentsMargins(0, 0, 0, 0);
    }
  }
}

void TextEditToolbar::bindEditWidget(QWidget *editWidget) {
  // 先解绑旧的
  if (m_textEdit) {
    disconnect(m_textEdit, &QTextEdit::selectionChanged, this,
               &TextEditToolbar::onSelectionChanged);
    m_textEdit = nullptr;
  }
  if (m_lineEdit) {
    disconnect(m_lineEdit, &QLineEdit::selectionChanged, this,
               &TextEditToolbar::onSelectionChanged);
    disconnect(m_lineEdit, &QLineEdit::cursorPositionChanged, this,
               &TextEditToolbar::onCursorPositionChanged);
    m_lineEdit = nullptr;
  }

  m_textEdit = qobject_cast<QTextEdit *>(editWidget);
  m_lineEdit = qobject_cast<QLineEdit *>(editWidget);

  if (!m_textEdit && !m_lineEdit) {
    qWarning() << "绑定失败，目标不是 QTextEdit 或 QLineEdit";
    return;
  }

  // 绑定选择变化
  if (m_textEdit) {
    connect(m_textEdit, &QTextEdit::selectionChanged, this,
            &TextEditToolbar::onSelectionChanged);

    connect(m_textEdit, &QTextEdit::cursorPositionChanged, this,
            &TextEditToolbar::onCursorPositionChanged);

    connect(m_textEdit->verticalScrollBar(), &QScrollBar::valueChanged, this,
            &TextEditToolbar::onScrollChanged);
    connect(m_textEdit->horizontalScrollBar(), &QScrollBar::valueChanged, this,
            &TextEditToolbar::onScrollChanged);
  } else {
    connect(m_lineEdit, &QLineEdit::selectionChanged, this,
            &TextEditToolbar::onSelectionChanged);
    connect(m_lineEdit, &QLineEdit::cursorPositionChanged, this,
            &TextEditToolbar::onCursorPositionChanged);
  }
}

void TextEditToolbar::onCursorPositionChanged() {
  if (m_textEdit) {
    int pos = m_textEdit->textCursor().position();
    m_startPos = pos;
    m_endPos = pos;
  } else if (m_lineEdit) {
    int pos = m_lineEdit->cursorPosition();
    m_startPos = pos;
    m_endPos = pos;
  }
  qDebug() << "[CursorPositionChanged] 同步到 m_startPos/m_endPos:"
           << m_startPos;
}

void TextEditToolbar::onScrollChanged() {
  if (isVisible() && (m_startHandle->isVisible() || m_endHandle->isVisible())) {
    updateHandlesPosition();
  }
}

void TextEditToolbar::onSelectionChanged() {
  if (m_dragging) {
    // 拖动过程中忽略选择变化事件
    return;
  }

  bool hasSelection = false;
  int currentCursorPos = 0;

  if (m_textEdit) {
    QTextCursor cursor = m_textEdit->textCursor();
    hasSelection = cursor.hasSelection();
    currentCursorPos = cursor.position();
    if (hasSelection) {
      m_startPos = cursor.selectionStart();
      m_endPos = cursor.selectionEnd();
    } else {
      m_startPos = currentCursorPos;
      m_endPos = currentCursorPos;
    }
  } else if (m_lineEdit) {
    hasSelection = !m_lineEdit->selectedText().isEmpty();
    currentCursorPos = m_lineEdit->cursorPosition();
    if (hasSelection) {
      m_startPos = m_lineEdit->selectionStart();
      m_endPos = currentCursorPos;  // QLineEdit 的光标在选择末尾
    } else {
      m_startPos = currentCursorPos;
      m_endPos = currentCursorPos;
    }
  }

  if (!hasSelection) {
    m_startHandle->hide();
    m_endHandle->hide();
    qDebug() << "[SelectionChanged] 无选择，同步光标位置:" << currentCursorPos;
  } else {
    updateHandlesPosition();
    qDebug() << "[SelectionChanged] 有选择，更新选择范围:" << m_startPos << "-"
             << m_endPos;
  }
}

void TextEditToolbar::showAtSelection() {
  if (!m_textEdit && !m_lineEdit) return;

  QWidget *editWidget = m_textEdit ? static_cast<QWidget *>(m_textEdit)
                                   : static_cast<QWidget *>(m_lineEdit);
  QRect screenRect = QApplication::primaryScreen()->availableGeometry();

  bool hasSelection = false;
  if (m_textEdit) {
    hasSelection = m_textEdit->textCursor().hasSelection();
  } else if (m_lineEdit) {
    hasSelection = !m_lineEdit->selectedText().isEmpty();
  }
  if (!hasSelection) {  // 无选择时才自动选2字符
    autoSelectTwoChars();
  }

  QRect editContentRect = editWidget->rect();
  QPoint editTopLeft = editWidget->mapToGlobal(editContentRect.topLeft());
  QPoint editBottomLeft = editWidget->mapToGlobal(editContentRect.bottomLeft());
  const int SPACING = 5;

  int targetX = editTopLeft.x();
  int targetY = (editTopLeft.y() >= height() + SPACING)
                    ? (editTopLeft.y() - height() - SPACING)
                    : (editBottomLeft.y() + SPACING);

  // 防止工具栏超出屏幕
  targetX = qMax(0, qMin(targetX, screenRect.right() - width()));
  targetY = qMax(0, qMin(targetY, screenRect.bottom() - height()));

  // 兜底：位置异常时居中
  if (targetX < 0 || targetX > screenRect.right() || targetY < 0 ||
      targetY > screenRect.bottom()) {
    targetX = (screenRect.width() - width()) / 2;
    targetY = (screenRect.height() - height()) / 2;
  }

  // 显示工具栏
  move(targetX, targetY);
  show();
  raise();

  // 显示手柄（此时已自动选择2字符，手柄会分开）
  showHandlesAtSelection();

  qDebug() << "[Toolbar Pos] X:" << targetX << "Y:" << targetY
           << "Screen:" << screenRect.size() << "ToolSize:" << size();
}

void TextEditToolbar::autoSelectTwoChars() {
  if (!m_textEdit && !m_lineEdit) return;

  int currentPos = 0;
  int textLength = 0;

  if (m_textEdit) {
    QTextCursor cursor = m_textEdit->textCursor();
    currentPos = cursor.position();
    textLength = m_textEdit->toPlainText().length();
  } else if (m_lineEdit) {
    currentPos = m_lineEdit->cursorPosition();
    textLength = m_lineEdit->text().length();
  }

  // 确保选择范围有效
  int selectStart = currentPos;
  int selectEnd = qMin(currentPos + 2, textLength);

  // 如果无法向右扩展，尝试向左扩展
  if (selectEnd == currentPos && currentPos > 0) {
    selectStart = qMax(0, currentPos - 2);
    selectEnd = currentPos;
  }

  // 确保选择至少一个字符
  if (selectStart == selectEnd && textLength > 0) {
    if (selectStart > 0) {
      selectStart = selectStart - 1;
    } else if (selectEnd < textLength) {
      selectEnd = selectEnd + 1;
    }
  }

  m_originalStartPos = selectStart;
  m_originalEndPos = selectEnd;

  // 调用统一的选择函数
  selectEditText(m_originalStartPos, m_originalEndPos);

  qDebug() << "[autoSelectTwoChars] 自动选择:" << selectStart << "-"
           << selectEnd << "当前光标:" << currentPos
           << "文本长度:" << textLength;
}

void TextEditToolbar::hide() {
  // 隐藏工具栏时同时隐藏手柄
  QWidget::hide();
  if (m_startHandle) m_startHandle->hide();
  if (m_endHandle) m_endHandle->hide();
}

// 统一选中文本的函数
void TextEditToolbar::selectEditText(int start, int end) {
  if (!m_textEdit && !m_lineEdit) return;

  if (start > end) std::swap(start, end);

  if (m_textEdit) {
    QTextCursor cursor = m_textEdit->textCursor();
    cursor.setPosition(start, QTextCursor::MoveAnchor);
    cursor.setPosition(end, QTextCursor::KeepAnchor);
    m_textEdit->setTextCursor(cursor);

    // 让起点可见（不改变选区）
    QTextCursor startCursor = m_textEdit->textCursor();
    startCursor.setPosition(start);
    m_textEdit->ensureCursorVisible();
  } else if (m_lineEdit) {
    // 临时移到起点触发滚动
    m_lineEdit->setCursorPosition(start);
    // 恢复选择
    m_lineEdit->setSelection(start, end - start);
  }

  m_startPos = start;
  m_endPos = end;
}

// ========================== 手柄与选择同步实现 ==========================
void TextEditToolbar::onStartHandleMoved(const QPoint &globalPos) {
  updateSelectionFromHandle(globalPos, true);
}

void TextEditToolbar::onEndHandleMoved(const QPoint &globalPos) {
  updateSelectionFromHandle(globalPos, false);
}

void TextEditToolbar::updateSelectionFromHandle(const QPoint &globalPos,
                                                bool isStartHandle) {
  if (!m_textEdit && !m_lineEdit) return;

  int newPos = getTextPositionFromGlobal(globalPos);
  int textLength = getTextLength();

  // 确保新位置有效
  newPos = qMax(0, qMin(newPos, textLength));

  if (isStartHandle) {
    // 拖动起始手柄
    m_startPos = qMin(newPos, m_endPos);

    // 确保选择范围至少一个字符
    if (m_startPos == m_endPos) {
      if (m_endPos > 0) {
        m_startPos = m_endPos - 1;
      } else if (textLength > 1) {
        m_endPos = m_startPos + 1;
      }
    }
  } else {
    // 拖动结束手柄
    m_endPos = qMax(newPos, m_startPos);

    // 确保选择范围至少一个字符
    if (m_startPos == m_endPos) {
      if (m_endPos < textLength) {
        m_endPos = m_startPos + 1;
      } else if (m_startPos > 0) {
        m_startPos = m_endPos - 1;
      }
    }
  }

  selectEditText(m_startPos, m_endPos);

  qDebug() << "[updateSelectionFromHandle] 手柄拖动，新位置:" << newPos
           << "选择范围:" << m_startPos << "-" << m_endPos;
}

int TextEditToolbar::getTextLength() {
  if (m_textEdit) {
    return m_textEdit->toPlainText().length();
  } else if (m_lineEdit) {
    return m_lineEdit->text().length();
  }
  return 0;
}

int TextEditToolbar::getTextPositionFromGlobal(const QPoint &globalPos) {
  if (m_textEdit) {
    QPoint localPos = m_textEdit->viewport()->mapFromGlobal(globalPos);
    return m_textEdit->cursorForPosition(localPos).position();
  } else if (m_lineEdit) {
    // 将全局坐标转换为 QLineEdit 的本地坐标
    QPoint localPos = m_lineEdit->mapFromGlobal(globalPos);

    // 考虑 QLineEdit 的内容矩形和内边距
    QRect contentRect = m_lineEdit->contentsRect();

    // 计算相对于文本起始位置的坐标
    int textX = localPos.x() - contentRect.left();

    // 获取字体度量
    QFontMetrics fm(m_lineEdit->font());
    QString text = m_lineEdit->text();

    // 计算滚动偏移
    int totalTextWidth = fm.horizontalAdvance(text);
    int scrollOffset = 0;
    if (totalTextWidth > contentRect.width()) {
      int cursorPos = m_lineEdit->cursorPosition();
      int textBeforeCursor = fm.horizontalAdvance(text.left(cursorPos));
      if (textBeforeCursor > contentRect.width()) {
        scrollOffset =
            textBeforeCursor - contentRect.width() + fm.averageCharWidth();
      }
    }

    // 调整坐标以考虑滚动
    textX += scrollOffset;

    // 遍历文本找到对应的位置
    int pos = 0;
    int currentWidth = 0;
    for (; pos < text.length(); ++pos) {
      int charWidth = fm.horizontalAdvance(text.at(pos));
      if (currentWidth + charWidth / 2 > textX) {
        break;
      }
      currentWidth += charWidth;
    }

    return qMin(pos, text.length());
  }
  return 0;
}

void TextEditToolbar::updateSelection(int start, int end) {
  // 确保 start <= end（避免选择范围反转）
  if (start > end) std::swap(start, end);

  // 更新编辑框选择
  if (m_textEdit) {
    QTextCursor cursor = m_textEdit->textCursor();
    cursor.setPosition(start, QTextCursor::MoveAnchor);
    cursor.setPosition(end, QTextCursor::KeepAnchor);
    m_textEdit->setTextCursor(cursor);
  } else if (m_lineEdit) {
    m_lineEdit->setSelection(start, end - start);
  }

  // 保存最新选择位置
  m_startPos = start;
  m_endPos = end;

  qDebug() << "[UpdateSelection] 基准位置更新为：Start=" << m_originalStartPos
           << "End=" << m_originalEndPos;
}

void TextEditToolbar::updateHandlesPosition() {
  if (m_dragging) return;
  if (!m_textEdit && !m_lineEdit) return;

  int startPos = m_startPos;
  int endPos = m_endPos;

  if (startPos == endPos) {
    m_startHandle->hide();
    m_endHandle->hide();
    return;
  }

  if (m_textEdit) {
    updateTextEditHandlesPosition(startPos, endPos);
  } else if (m_lineEdit) {
    updateLineEditHandlesPosition(startPos, endPos);
  }
}

// 改进的 QTextEdit 手柄定位函数
void TextEditToolbar::updateTextEditHandlesPosition(int startPos, int endPos) {
  if (!m_textEdit) return;

  // 获取视口的可见区域
  QRect viewportRect = m_textEdit->viewport()->rect();
  QPoint viewportTopLeft = m_textEdit->viewport()->mapToGlobal(QPoint(0, 0));

  // 计算起始手柄位置
  QTextCursor startCursor = m_textEdit->textCursor();
  startCursor.setPosition(startPos);
  QRect startRect = m_textEdit->cursorRect(startCursor);

  // 确保光标矩形在视口内
  if (!viewportRect.contains(startRect.center())) {
    m_textEdit->ensureCursorVisible();
    startRect = m_textEdit->cursorRect(startCursor);
  }

  // 起始手柄：左边缘紧贴选区左边界
  // 手柄右边缘对齐选区左边界，不压住文字
  QPoint startGlobal =
      viewportTopLeft + QPoint(startRect.left(), startRect.center().y());
  startGlobal -= QPoint(m_startHandle->width(), m_startHandle->height() / 2);

  // 计算结束手柄位置
  QTextCursor endCursor = m_textEdit->textCursor();
  endCursor.setPosition(endPos);
  QRect endRect = m_textEdit->cursorRect(endCursor);

  if (!viewportRect.contains(endRect.center())) {
    m_textEdit->ensureCursorVisible();
    endRect = m_textEdit->cursorRect(endCursor);
  }

  // 结束手柄：右边缘紧贴选区右边界
  // 手柄左边缘对齐选区右边界，不压住文字
  QPoint endGlobal =
      viewportTopLeft + QPoint(endRect.right(), endRect.center().y());
  endGlobal -= QPoint(0, m_endHandle->height() / 2);

  // 屏幕边界检查
  QRect screenRect = QApplication::primaryScreen()->availableGeometry();

  // 起始手柄边界检查
  if (startGlobal.x() < screenRect.left()) {
    startGlobal.setX(screenRect.left());
  }
  if (startGlobal.y() < screenRect.top()) {
    startGlobal.setY(screenRect.top());
  }
  if (startGlobal.y() + m_startHandle->height() > screenRect.bottom()) {
    startGlobal.setY(screenRect.bottom() - m_startHandle->height());
  }

  // 结束手柄边界检查
  if (endGlobal.x() + m_endHandle->width() > screenRect.right()) {
    endGlobal.setX(screenRect.right() - m_endHandle->width());
  }
  if (endGlobal.y() < screenRect.top()) {
    endGlobal.setY(screenRect.top());
  }
  if (endGlobal.y() + m_endHandle->height() > screenRect.bottom()) {
    endGlobal.setY(screenRect.bottom() - m_endHandle->height());
  }

  m_startHandle->move(startGlobal);
  m_endHandle->move(endGlobal);

  qDebug() << "[QTextEdit手柄精确定位] StartPos:" << startPos
           << "EndPos:" << endPos << "StartRect:" << startRect
           << "EndRect:" << endRect << "StartGlobal:" << startGlobal
           << "EndGlobal:" << endGlobal;
}

void TextEditToolbar::updateLineEditHandlesPosition(int startPos, int endPos) {
  if (!m_lineEdit) return;

  QString text = m_lineEdit->text();
  if (text.isEmpty()) {
    m_startHandle->hide();
    m_endHandle->hide();
    return;
  }

  // 检查是否是密码模式
  bool isPasswordMode = (m_lineEdit->echoMode() != QLineEdit::Normal);

  // 对于密码模式，使用更简单但更可靠的计算方法
  if (isPasswordMode) {
    updateLineEditHandlesPositionForPasswordMode(startPos, endPos);
    return;
  }

  // 普通模式使用原来的计算方法
  updateLineEditHandlesPositionForNormalMode(startPos, endPos);
}

void TextEditToolbar::updateLineEditHandlesPositionForPasswordMode(int startPos,
                                                                   int endPos) {
  return;

  if (!m_lineEdit) return;

  QString text = m_lineEdit->text();
  if (text.isEmpty()) return;

  QFontMetrics fm(m_lineEdit->font());
  QRect contentRect = m_lineEdit->contentsRect();
  QPoint lineEditGlobalPos = m_lineEdit->mapToGlobal(QPoint(0, 0));

  // 密码模式：使用平均字符宽度计算
  int averageCharWidth = fm.averageCharWidth();
  int totalTextWidth = text.length() * averageCharWidth;

  // 计算字符位置
  int startTextPos = startPos * averageCharWidth;
  int endTextPos = endPos * averageCharWidth;

  // 计算滚动偏移（简化版本）
  int scrollOffset = 0;
  if (totalTextWidth > contentRect.width()) {
    int cursorPos = m_lineEdit->cursorPosition();
    int textBeforeCursor = cursorPos * averageCharWidth;
    if (textBeforeCursor > contentRect.width()) {
      scrollOffset = textBeforeCursor - contentRect.width() + averageCharWidth;
    }
    scrollOffset =
        qMax(0, qMin(scrollOffset, totalTextWidth - contentRect.width()));
  }

  // 转换为可视区域内的坐标
  int startVisibleX = contentRect.left() + startTextPos - scrollOffset;
  int endVisibleX = contentRect.left() + endTextPos - scrollOffset;

  // 确保坐标在可见区域内
  startVisibleX =
      qMax(contentRect.left(), qMin(startVisibleX, contentRect.right()));
  endVisibleX =
      qMax(contentRect.left(), qMin(endVisibleX, contentRect.right()));

  // 计算手柄垂直位置（居中）
  int handleY = contentRect.center().y() - m_startHandle->height() / 2;

  // 计算全局坐标
  QPoint startGlobal = lineEditGlobalPos + QPoint(startVisibleX, handleY);
  QPoint endGlobal = lineEditGlobalPos + QPoint(endVisibleX, handleY);

  // 调整手柄位置，使其边缘紧贴选区边界
  startGlobal.rx() -= m_startHandle->width();  // 起始手柄右边缘对齐选区起始位置

  // 屏幕边界检查
  QRect screenRect = QApplication::primaryScreen()->availableGeometry();

  // 起始手柄边界检查
  startGlobal.setX(qMax(screenRect.left(), startGlobal.x()));
  startGlobal.setY(qMax(
      screenRect.top(),
      qMin(startGlobal.y(), screenRect.bottom() - m_startHandle->height())));

  // 结束手柄边界检查
  endGlobal.setX(
      qMin(endGlobal.x(), screenRect.right() - m_endHandle->width()));
  endGlobal.setY(
      qMax(screenRect.top(),
           qMin(endGlobal.y(), screenRect.bottom() - m_endHandle->height())));

  m_startHandle->move(startGlobal);
  m_endHandle->move(endGlobal);

  qDebug() << "[QLineEdit密码模式定位] StartPos:" << startPos
           << "EndPos:" << endPos << "StartTextPos:" << startTextPos
           << "EndTextPos:" << endTextPos << "StartVisibleX:" << startVisibleX
           << "EndVisibleX:" << endVisibleX << "ScrollOffset:" << scrollOffset
           << "TextLength:" << text.length()
           << "AverageCharWidth:" << averageCharWidth
           << "StartGlobal:" << startGlobal << "EndGlobal:" << endGlobal;
}

void TextEditToolbar::updateLineEditHandlesPositionForNormalMode(int startPos,
                                                                 int endPos) {
  if (!m_lineEdit) return;

  QString text = m_lineEdit->text();
  if (text.isEmpty()) return;

  // 普通模式使用原来的计算方法
  QFontMetrics fm(m_lineEdit->font());
  QRect contentRect = m_lineEdit->contentsRect();
  QPoint lineEditGlobalPos = m_lineEdit->mapToGlobal(QPoint(0, 0));

  // 计算文本的总宽度
  int totalTextWidth = fm.horizontalAdvance(text);

  // 计算滚动偏移
  int scrollOffset = 0;
  if (totalTextWidth > contentRect.width()) {
    int cursorPos = m_lineEdit->cursorPosition();
    int textBeforeCursor = fm.horizontalAdvance(text.left(cursorPos));
    if (textBeforeCursor > contentRect.width()) {
      scrollOffset =
          textBeforeCursor - contentRect.width() + fm.averageCharWidth();
    }
    scrollOffset =
        qMax(0, qMin(scrollOffset, totalTextWidth - contentRect.width()));
  }

  // 计算选择起点和终点的文本位置
  int startTextPos = fm.horizontalAdvance(text.left(startPos));
  int endTextPos = fm.horizontalAdvance(text.left(endPos));

  // 转换为可视区域内的坐标
  int startVisibleX = contentRect.left() + startTextPos - scrollOffset;
  int endVisibleX = contentRect.left() + endTextPos - scrollOffset;

  // 确保坐标在可见区域内
  startVisibleX =
      qMax(contentRect.left(), qMin(startVisibleX, contentRect.right()));
  endVisibleX =
      qMax(contentRect.left(), qMin(endVisibleX, contentRect.right()));

  // 计算手柄垂直位置（居中）
  int handleY = contentRect.center().y() - m_startHandle->height() / 2;

  // 计算全局坐标
  QPoint startGlobal = lineEditGlobalPos + QPoint(startVisibleX, handleY);
  QPoint endGlobal = lineEditGlobalPos + QPoint(endVisibleX, handleY);

  // 调整手柄位置，使其边缘紧贴选区边界
  startGlobal.rx() -= m_startHandle->width();  // 起始手柄右边缘对齐选区起始位置

  // 屏幕边界检查
  QRect screenRect = QApplication::primaryScreen()->availableGeometry();

  // 起始手柄边界检查
  startGlobal.setX(qMax(screenRect.left(), startGlobal.x()));
  startGlobal.setY(qMax(
      screenRect.top(),
      qMin(startGlobal.y(), screenRect.bottom() - m_startHandle->height())));

  // 结束手柄边界检查
  endGlobal.setX(
      qMin(endGlobal.x(), screenRect.right() - m_endHandle->width()));
  endGlobal.setY(
      qMax(screenRect.top(),
           qMin(endGlobal.y(), screenRect.bottom() - m_endHandle->height())));

  m_startHandle->move(startGlobal);
  m_endHandle->move(endGlobal);

  qDebug() << "[QLineEdit普通模式定位] StartPos:" << startPos
           << "EndPos:" << endPos << "StartTextPos:" << startTextPos
           << "EndTextPos:" << endTextPos << "StartVisibleX:" << startVisibleX
           << "EndVisibleX:" << endVisibleX << "ScrollOffset:" << scrollOffset
           << "TextLength:" << text.length() << "StartGlobal:" << startGlobal
           << "EndGlobal:" << endGlobal;
}

// 统一的手柄显示函数
void TextEditToolbar::showHandlesAtSelection() {
  // 如果是 QLineEdit，隐藏手柄并返回
  if (m_lineEdit) {
    m_startHandle->hide();
    m_endHandle->hide();
    qDebug() << "[showHandlesAtSelection] QLineEdit 模式，隐藏手柄";
    return;
  }

  if (!m_textEdit && !m_lineEdit) return;

  qDebug() << "[showHandlesAtSelection] 开始显示手柄";

  int startPos = 0, endPos = 0;
  bool hasSelection = false;

  if (m_textEdit) {
    QTextCursor cursor = m_textEdit->textCursor();
    startPos = cursor.selectionStart();
    endPos = cursor.selectionEnd();
    hasSelection = cursor.hasSelection();
  } else if (m_lineEdit) {
    QString selectedText = m_lineEdit->selectedText();
    hasSelection = !selectedText.isEmpty();
    if (hasSelection) {
      startPos = m_lineEdit->selectionStart();
      endPos = startPos + selectedText.length();
    } else {
      // 如果没有选择，使用自动选择的位置
      startPos = m_startPos;
      endPos = m_endPos;
    }
  }

  if (!hasSelection && startPos == endPos) {
    m_startHandle->hide();
    m_endHandle->hide();
    qDebug() << "[showHandlesAtSelection] 无选择，隐藏手柄";
    return;
  }

  // 确保选择范围有效
  if (startPos > endPos) {
    std::swap(startPos, endPos);
  }

  // 确保选择范围至少一个字符
  if (startPos == endPos) {
    int textLength = getTextLength();
    if (textLength > 0) {
      if (endPos < textLength) {
        endPos = startPos + 1;
      } else if (startPos > 0) {
        startPos = endPos - 1;
      }
    }
  }

  if (m_textEdit) {
    updateTextEditHandlesPosition(startPos, endPos);
  } else if (m_lineEdit) {
    updateLineEditHandlesPosition(startPos, endPos);
  }

  // 更新选择位置变量
  m_startPos = startPos;
  m_endPos = endPos;

  m_startHandle->show();
  m_endHandle->show();
  m_startHandle->raise();
  m_endHandle->raise();

  qDebug() << "[showHandlesAtSelection] 手柄显示完成，选择范围:" << startPos
           << "-" << endPos;
}

// ========================== 事件处理实现 ==========================
void TextEditToolbar::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Back) {
    hide();
    event->accept();
  } else {
    QWidget::keyPressEvent(event);
  }
}

void TextEditToolbar::showEvent(QShowEvent *event) {
  QWidget::showEvent(event);
  setFocus();  // 显示时获取焦点，确保能接收返回键
}

bool TextEditToolbar::eventFilter(QObject *obj, QEvent *event) {
  if (obj == this) {
    // 拦截返回键（KeyPress/KeyRelease 都处理）
    if (event->type() == QEvent::KeyPress ||
        event->type() == QEvent::KeyRelease) {
      QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
      if (keyEvent->key() == Qt::Key_Back) {
        hide();
        return true;
      }
    }
  }
  return QWidget::eventFilter(obj, event);
}

// ========================== 槽函数实现 ==========================
void TextEditToolbar::onMinusClicked() {
  int start = m_startPos;
  int end = m_endPos;

  if (start == end) {
    // 无选择时，从当前光标位置向左扩展
    start = qMax(0, start - 1);
  } else {
    start = qMax(0, start - 1);
  }

  selectEditText(start, end);
  updateHandlesPosition();
}

// 按下按钮时模拟键盘按下
void TextEditToolbar::onMinusPressed() {
  QWidget *editor = m_textEdit ? static_cast<QWidget *>(m_textEdit)
                               : static_cast<QWidget *>(m_lineEdit);
  if (!editor) return;

  // 保存当前终点
  if (!isOne) {
    isOne = true;
    m_dragAnchorEnd3 = m_endPos;
  }

  int start = m_startPos;
  if (m_lineEdit) m_lineEdit->setCursorPosition(start);

  if (m_textEdit) {
    QTextCursor cursor = m_textEdit->textCursor();
    cursor.setPosition(start, QTextCursor::MoveAnchor);  // 设置光标起点
  }

  // 发送向左方向键按下事件（滚到可视区）
  QApplication::postEvent(
      editor, new QKeyEvent(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier));
}

// 松开按钮时用保存的终点重新选中
void TextEditToolbar::onMinusReleased() {
  QWidget *editor = m_textEdit ? static_cast<QWidget *>(m_textEdit)
                               : static_cast<QWidget *>(m_lineEdit);
  if (!editor) return;

  isOne = false;

  // 发送键盘抬起事件
  QApplication::postEvent(
      editor, new QKeyEvent(QEvent::KeyRelease, Qt::Key_Left, Qt::NoModifier));

  // 用保存的终点计算新范围
  QTimer::singleShot(0, this, [this]() {
    int start = m_startPos;      // 这个是被键盘事件更新后的起点
    int end = m_dragAnchorEnd3;  // 用按下时保存的终点

    if (start > end) std::swap(start, end);

    selectEditText(start, end);
    updateHandlesPosition();
  });
}

void TextEditToolbar::onPlusClicked() {
  // 扩展选择范围（向右/下扩展）

  int start = m_startPos;
  int end = m_endPos;
  int textLength = (m_textEdit) ? m_textEdit->toPlainText().length()
                                : m_lineEdit->text().length();

  end = qMin(textLength, end + 1);
  selectEditText(start, end);

  // 同步更新手柄位置
  updateHandlesPosition();
}

void TextEditToolbar::onCopyClicked() {
  // 复制选中内容
  if (m_textEdit) m_textEdit->copy();
  if (m_lineEdit) m_lineEdit->copy();
  // 操作后隐藏工具栏和手柄
  hide();
}

void TextEditToolbar::onCutClicked() {
  // 剪切选中内容
  if (m_textEdit) m_textEdit->cut();
  if (m_lineEdit) m_lineEdit->cut();
  // 操作后隐藏工具栏和手柄
  hide();
}

void TextEditToolbar::onPasteClicked() {
  // 粘贴内容
  if (m_textEdit) m_textEdit->paste();
  if (m_lineEdit) m_lineEdit->paste();
  // 操作后隐藏工具栏和手柄
  hide();
}

void TextEditToolbar::onSelectAllClicked() {
  // 全选内容
  if (m_textEdit) m_textEdit->selectAll();
  if (m_lineEdit) m_lineEdit->selectAll();
  // 同步更新手柄位置
  updateHandlesPosition();
}

void TextEditToolbar::onCursorLeftClicked() {
  // 向左收缩

  int start = m_startPos;
  int end = m_endPos;

  if (end > start) {
    start += 1;
    selectEditText(start, end);
  }

  updateHandlesPosition();
}

void TextEditToolbar::onCursorRightClicked() {
  // 向右收缩

  int start = m_startPos;
  int end = m_endPos;

  if (end > start) {
    end -= 1;
    selectEditText(start, end);
  }

  updateHandlesPosition();
}

void TextEditToolbar::onDelClicked() {
  // 功能：删除选中文字（无选中则不处理，符合用户需求）
  if (!m_textEdit && !m_lineEdit) return;

  bool hasSelection = false;
  // 处理QTextEdit
  if (m_textEdit) {
    QTextCursor cursor = m_textEdit->textCursor();
    hasSelection = cursor.hasSelection();
    if (hasSelection) {
      cursor.removeSelectedText();  // 仅删除，不放入剪贴板
      m_textEdit->setTextCursor(cursor);
    }
  }
  // 处理QLineEdit
  else if (m_lineEdit) {
    hasSelection = !m_lineEdit->selectedText().isEmpty();
    if (hasSelection) {
      int start = m_lineEdit->selectionStart();
      int len = m_lineEdit->selectedText().length();
      QString text = m_lineEdit->text();
      text.remove(start, len);  // 删除选中部分
      m_lineEdit->setText(text);
      m_lineEdit->setCursorPosition(start);  // 光标定位到删除位置
    }
  }

  // 选中内容删除后，隐藏手柄（与原有选择逻辑一致）
  if (hasSelection) {
    m_startHandle->hide();
    m_endHandle->hide();
  }
}

void TextEditToolbar::onCloseClicked() { hide(); }

// 按下起始手柄：记录固定的另一端（end）
void TextEditToolbar::onStartHandlePressed() {
  m_dragging = true;
  m_dragAnchorEnd = m_endPos;  // 拖动起始手柄时，end 固定
}

// 按下结束手柄：记录固定的另一端（start）
void TextEditToolbar::onEndHandlePressed() {
  m_dragging = true;
  m_dragAnchorEnd = m_startPos;  // 拖动结束手柄时，start 固定
}

// 释放手柄：重置拖动状态
void TextEditToolbar::onHandleReleased() {
  m_dragging = false;

  // 确保手柄的拖动状态被重置
  if (m_startHandle) {
    // 这里可以添加重置手柄状态的代码，如果需要的话
  }
  if (m_endHandle) {
    // 同上
  }

  // 释放后更新基准位置
  m_originalStartPos = m_startPos;
  m_originalEndPos = m_endPos;
  updateHandlesPosition();

  qDebug() << "[HandleReleased] 手柄释放，基准更新为：" << m_originalStartPos
           << "-" << m_originalEndPos;
}
// ========================== EditEventFilter 实现 ==========================
EditEventFilter::EditEventFilter(TextEditToolbar *toolbar, QObject *parent)
    : QObject(parent), m_toolbar(toolbar) {
  // 连接定时器信号槽（单次触发）
  bool isConnected =
      connect(&m_timer, &QTimer::timeout, this, &EditEventFilter::onTimeout);
  qDebug() << "EditEventFilter: Timer connected:" << isConnected;
  m_timer.setSingleShot(true);
}

bool EditEventFilter::eventFilter(QObject *obj, QEvent *event) {
  // 识别目标控件（QTextEdit/viewport 或 QLineEdit）
  QTextEdit *textEdit = nullptr;
  QLineEdit *lineEdit = nullptr;
  if (QWidget *viewport = qobject_cast<QWidget *>(obj)) {
    textEdit = qobject_cast<QTextEdit *>(viewport->parent());
  }
  if (!textEdit) textEdit = qobject_cast<QTextEdit *>(obj);
  if (!textEdit) lineEdit = qobject_cast<QLineEdit *>(obj);

  // 跳过 Paint 事件（减少日志冗余）
  if (event->type() == QEvent::Paint) return false;

  // 非目标控件，不处理
  if (!textEdit && !lineEdit) return false;

  // 放行输入法事件
  if (event->type() == QInputMethodEvent::InputMethodQuery) return false;

  // 处理返回键（隐藏工具栏）
  if (event->type() == QEvent::KeyPress ||
      event->type() == QEvent::KeyRelease) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    if (keyEvent->key() == Qt::Key_Back && m_toolbar &&
        m_toolbar->isVisible()) {
      m_toolbar->hide();
      return true;
    }
  }

  // 处理长按（鼠标/触摸）
  if (event->type() == QEvent::MouseButtonPress) {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    if (mouseEvent->button() == Qt::LeftButton) {
      m_pressPos = mouseEvent->pos();
      m_timer.start(500);  // 500ms 长按触发
      m_target = textEdit ? static_cast<QObject *>(textEdit)
                          : static_cast<QObject *>(lineEdit);
      qDebug() << "EditEventFilter: Mouse press - timer started";
      return false;
    }
  } else if (event->type() == QEvent::TouchBegin) {
    QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
    const auto &touchPoints = touchEvent->points();
    if (!touchPoints.isEmpty()) {
      m_pressPos = touchPoints.first().position().toPoint();
      m_timer.start(500);
      m_target = textEdit ? static_cast<QObject *>(textEdit)
                          : static_cast<QObject *>(lineEdit);
      qDebug() << "EditEventFilter: Touch begin - timer started";
      return false;
    }
  }

  // 处理移动（超过阈值则取消长按）
  else if (event->type() == QEvent::MouseMove ||
           event->type() == QEvent::TouchUpdate) {
    if (m_timer.isActive()) {
      qDebug() << "EditEventFilter: Move detected - check threshold";
      if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if ((mouseEvent->pos() - m_pressPos).manhattanLength() > 10) {
          m_timer.stop();
          qDebug() << "EditEventFilter: Move exceeds threshold - timer stopped";
        }
      } else if (event->type() == QEvent::TouchUpdate) {
        QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
        const auto &touchPoints = touchEvent->points();
        if (!touchPoints.isEmpty()) {
          if ((touchPoints.first().position() - QPointF(m_pressPos))
                  .manhattanLength() > 10) {
            m_timer.stop();
            qDebug() << "EditEventFilter: Touch move exceeds threshold - timer "
                        "stopped";
          }
        }
      }
    }
  }

  // 处理释放（取消长按）
  else if (event->type() == QEvent::MouseButtonRelease ||
           event->type() == QEvent::TouchEnd) {
    if (m_timer.isActive()) {
      m_timer.stop();
      qDebug() << "EditEventFilter: Release detected - timer stopped";
    }
  }

  return false;
}

void EditEventFilter::onTimeout() {
  qDebug() << "EditEventFilter: Long press detected - show toolbar";
  if (m_target && m_toolbar) {
    // 关键：将 QLineEdit 光标定位到长按的鼠标位置
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(m_target);
    if (lineEdit) {
      // 将鼠标按下位置（m_pressPos）转换为 QLineEdit 的光标位置
      int cursorPos = lineEdit->cursorPositionAt(m_pressPos);
      lineEdit->setCursorPosition(cursorPos);

      // 确保编辑框获得焦点
      if (!lineEdit->hasFocus()) {
        lineEdit->setFocus();
      }

      qDebug() << "[LongPress] QLineEdit 光标定位到：" << cursorPos << "位置："
               << m_pressPos << "文本长度：" << lineEdit->text().length();
    }

    // 绑定控件并显示工具栏（原有逻辑）
    m_toolbar->bindEditWidget(static_cast<QWidget *>(m_target));
    m_toolbar->showAtSelection();
  }
}
