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
  setFixedSize(32, 32);
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

void HandleWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    m_dragOffset = event->pos();
    m_isDragging = true;
    setCursor(Qt::ClosedHandCursor);
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
    emit released();
    event->accept();
  } else {
    event->ignore();
  }
}

void HandleWidget::mouseMoveEvent(QMouseEvent *event) {
  if (m_isDragging) {
    QPoint newGlobalPos = event->globalPosition().toPoint() - m_dragOffset;
    move(newGlobalPos);
    emit moved(newGlobalPos);
    event->accept();
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
  btnClose = new QPushButton(tr("Cls"), this);

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
  const int BUTTON_SIZE = 44;      // 按钮大小不变（与原有一致）
  const int TOOLBAR_WIDTH = 258;   // 宽度不变
  const int TOOLBAR_HEIGHT = 110;  // 高度不变

  // 统一设置按钮大小（44×44）- 不变
  QList<QPushButton *> buttons = {
      btnMinus,     btnPlus,       btnCopy,        btnCut, btnPaste,
      btnSelectAll, btnCursorLeft, btnCursorRight, btnDel, btnClose};
  for (auto btn : buttons) {
    btn->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
  }

  // 更新工具栏固定尺寸 - 不变
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
    // QTextEdit 没有 cursorPositionChanged 信号，用 cursorRectChanged 代替
    connect(m_textEdit, &QTextEdit::cursorPositionChanged, this,
            &TextEditToolbar::onCursorPositionChanged);
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

void TextEditToolbar::onSelectionChanged() {
  bool hasSelection = false;
  int currentCursorPos = 0;  // 新增：记录当前光标位置

  // 判断是否有选择 + 获取当前光标位置
  if (m_textEdit) {
    QTextCursor cursor = m_textEdit->textCursor();
    hasSelection = cursor.hasSelection();
    currentCursorPos = cursor.position();  // 取当前光标位置
    // 有选择时同步选择范围，无选择时同步光标位置
    if (hasSelection) {
      m_startPos = cursor.selectionStart();
      m_endPos = cursor.selectionEnd();
    } else {
      m_startPos = currentCursorPos;
      m_endPos = currentCursorPos;
    }
  } else if (m_lineEdit) {
    hasSelection = !m_lineEdit->selectedText().isEmpty();
    currentCursorPos = m_lineEdit->cursorPosition();  // 取当前光标位置
    if (hasSelection) {
      m_startPos = m_lineEdit->selectionStart();
      m_endPos = m_lineEdit->cursorPosition();  // QLineEdit 光标在选择末尾
    } else {
      m_startPos = currentCursorPos;
      m_endPos = currentCursorPos;
    }
  }

  // 手柄显示逻辑不变
  if (!hasSelection) {
    m_startHandle->hide();
    m_endHandle->hide();
    qDebug() << "[SelectionChanged] 无选择，同步光标位置到 m_startPos/m_endPos:"
             << currentCursorPos;
  } else {
    updateHandlesPosition();

    if (m_lineEdit) {
      m_startHandle->hide();
      m_endHandle->hide();
      return;
    }

    m_startHandle->show();
    m_endHandle->show();
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

  int selectEndPos = qMin(currentPos + 2, textLength);
  if (selectEndPos == currentPos) {
    selectEndPos = qMax(currentPos - 2, 0);
  }

  m_originalStartPos = qMin(currentPos, selectEndPos);
  m_originalEndPos = qMax(currentPos, selectEndPos);

  // 调用统一的选择函数
  selectEditText(m_originalStartPos, m_originalEndPos);
}

void TextEditToolbar::showHandlesAtSelection() {
  if (!m_textEdit && !m_lineEdit) return;

  // 临时屏蔽 QLineEdit 手柄
  if (m_lineEdit) {
    m_startHandle->hide();
    m_endHandle->hide();
    return;
  }

  int startPos = 0, endPos = 0;

  qDebug() << "[showHandlesAtSelection] 开始显示手柄";

  // -------------------------- QTextEdit 处理 --------------------------
  if (m_textEdit) {
    QTextCursor cursor = m_textEdit->textCursor();
    startPos = cursor.selectionStart();
    endPos = cursor.selectionEnd();

    // 无选择时隐藏手柄
    if (startPos == endPos) {
      m_startHandle->hide();
      m_endHandle->hide();
      return;
    }

    // 起始手柄：获取startRect并计算位置（垂直居中+水平外移）
    QTextCursor startCursor = cursor;
    startCursor.setPosition(startPos);
    QRect startRect = m_textEdit->cursorRect(startCursor);
    QPoint startGlobal =
        m_textEdit->viewport()->mapToGlobal(
            QPoint(startRect.left(), startRect.center().y())) -
        QPoint(m_startHandle->width() / 2, m_startHandle->height() / 2);
    startGlobal.rx() -= m_startHandle->width() / 2;  // 水平左移，避免压字
    m_startHandle->move(startGlobal);
    m_startHandle->show();
    m_startHandle->raise();

    // 结束手柄
    QTextCursor endCursor = cursor;
    endCursor.setPosition(endPos);
    QRect endRect = m_textEdit->cursorRect(endCursor);
    QPoint endGlobal =
        m_textEdit->viewport()->mapToGlobal(
            QPoint(endRect.right(), endRect.center().y())) -
        QPoint(m_endHandle->width() / 2, m_endHandle->height() / 2);
    endGlobal.rx() += m_endHandle->width() / 2;  // 水平右移，避免压字
    m_endHandle->move(endGlobal);
    m_endHandle->show();
    m_endHandle->raise();
  } else if (m_lineEdit) {
    // 1. 利用 QLineEdit 选中特性：光标在末尾，计算起点
    int cursorEndPos = m_lineEdit->cursorPosition();         // 选择末尾（固定）
    int selectLength = m_lineEdit->selectedText().length();  // 选择长度
    int selectStartPos = cursorEndPos - selectLength;        // 选择起点（精准）
    if (selectLength <= 0 || selectStartPos < 0 ||
        cursorEndPos > m_lineEdit->text().length()) {
      m_startHandle->hide();
      m_endHandle->hide();
      return;
    }

    // 2. 关键：计算【动态滚动偏移】（获取可视区真实位置）
    QRect contentRect =
        m_lineEdit->contentsRect();  // 可视区本地矩形（左0，上0）
    QFontMetrics fm = m_lineEdit->fontMetrics();
    QString fullText = m_lineEdit->text();
    // 可视区左边界对应的光标位置（滚动后，这个光标是当前可见的最左侧字符）
    int visibleLeftCursor =
        m_lineEdit->cursorPositionAt(QPoint(0, contentRect.height() / 2));
    // 滚动偏移 = 可视区左边界光标对应的文本宽度（即：隐藏的文本宽度）
    int scrollOffset = fm.horizontalAdvance(fullText.left(visibleLeftCursor));

    // 3. 计算【可视区本地宽度】（去掉隐藏部分，仅保留可见的宽度）
    int startLocalWidth = fm.horizontalAdvance(fullText.left(selectStartPos)) -
                          scrollOffset;  // 选中起点的可视区宽度
    int endLocalWidth = fm.horizontalAdvance(fullText.left(cursorEndPos)) -
                        scrollOffset;  // 选中终点的可视区宽度

    // 4. 基础参数：手柄尺寸、屏幕范围、垂直居中
    QRect screenRect = QApplication::primaryScreen()->availableGeometry();
    int handleHalfW = m_startHandle->width() / 2;   // 手柄半宽（32/2=16px）
    int handleHalfH = m_startHandle->height() / 2;  // 手柄半高（32/2=16px）
    int fixedY = contentRect.top() + contentRect.height() / 2 -
                 handleHalfH;  // 垂直居中（不变）

    // 5. 【核心优化】手柄位置计算：让手柄完全在光标外侧，不压字
    // 起始手柄：向左偏移16px（手柄右边缘对齐起点光标，不压左侧文字）
    QPoint startLocal(startLocalWidth - handleHalfW - handleHalfW, fixedY);
    // 结束手柄：向右偏移16px（手柄左边缘对齐终点光标，不压右侧文字）
    QPoint endLocal(endLocalWidth - handleHalfW + handleHalfW, fixedY);

    // 6. 转换为全局坐标（加上 QLineEdit 自身的全局左边界）
    QPoint lineEditGlobalPos = m_lineEdit->mapToGlobal(QPoint(0, 0));
    QPoint startGlobal = lineEditGlobalPos + startLocal;
    QPoint endGlobal = lineEditGlobalPos + endLocal;

    // 7. 屏幕内兜底（避免超出屏幕，优先保证不压字）
    // 起始手柄：左边界不小于屏幕左边缘
    startGlobal.setX(qMax(screenRect.left(), startGlobal.x()));
    // 结束手柄：右边界不大于屏幕右边缘
    endGlobal.setX(
        qMax(screenRect.left(),
             qMin(endGlobal.x(), screenRect.right() - m_endHandle->width())));
    // 垂直方向兜底（不变）
    startGlobal.setY(qMax(
        screenRect.top(),
        qMin(startGlobal.y(), screenRect.bottom() - m_startHandle->height())));
    endGlobal.setY(
        qMax(screenRect.top(),
             qMin(endGlobal.y(), screenRect.bottom() - m_endHandle->height())));

    // 8. 显示手柄
    m_startHandle->move(startGlobal);
    m_endHandle->move(endGlobal);
    m_startHandle->show();
    m_endHandle->show();
    m_startHandle->raise();
    m_endHandle->raise();

    // 调试：验证参数（确保本地宽度在可视区内）
    qDebug() << "[QLineEdit 优化后] "
             << "滚动偏移：" << scrollOffset << "起点可视宽度："
             << startLocalWidth << "终点可视宽度：" << endLocalWidth
             << "起始手柄本地X：" << startLocal.x() << "结束手柄本地X："
             << endLocal.x() << "全局位置：Start(" << startGlobal.x() << ","
             << startGlobal.y() << "), End(" << endGlobal.x() << ","
             << endGlobal.y() << ")";
  }

  // 更新选择位置变量
  m_startPos = startPos;
  m_endPos = endPos;
}

void TextEditToolbar::hide() {
  // 隐藏工具栏时同时隐藏手柄
  QWidget::hide();
  if (m_startHandle) m_startHandle->hide();
  if (m_endHandle) m_endHandle->hide();
}

// 统一选中文本的函数
/*void TextEditToolbar::selectEditText(int start, int end) {
  if (!m_textEdit && !m_lineEdit) return;

  // 确保 start <= end
  if (start > end) std::swap(start, end);

  if (m_textEdit) {
    // QTextEdit 用 QTextCursor 选择
    QTextCursor cursor = m_textEdit->textCursor();
    cursor.setPosition(start, QTextCursor::MoveAnchor);
    cursor.setPosition(end, QTextCursor::KeepAnchor);
    m_textEdit->setTextCursor(cursor);

    // 确保光标位置可见（滚动到视图）
    m_textEdit->ensureCursorVisible();
  } else if (m_lineEdit) {
    m_lineEdit->setCursorPosition(start);
    m_lineEdit->setSelection(start, end - start);
  }

  // 保存最新选择位置
  m_startPos = start;
  m_endPos = end;
}*/

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

  // 计算鼠标对应的文本位置
  int newPos = 0;
  if (m_textEdit) {
    QPointF posInWidget = m_textEdit->viewport()->mapFromGlobal(globalPos);
    newPos = m_textEdit->cursorForPosition(posInWidget.toPoint()).position();
  } else {
    QPointF posInWidget = m_lineEdit->mapFromGlobal(globalPos);
    newPos = m_lineEdit->cursorPositionAt(posInWidget.toPoint());
  }

  // 关键：用拖动开始时固定的另一端计算，不依赖实时选择
  if (isStartHandle) {
    updateSelection(newPos, m_dragAnchorEnd);  // end 固定
  } else {
    updateSelection(m_dragAnchorEnd, newPos);  // start 固定
  }
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

  // 临时屏蔽 QLineEdit 手柄
  if (m_lineEdit) {
    m_startHandle->hide();
    m_endHandle->hide();
    return;
  }

  // QTextEdit 处理
  if (m_textEdit) {
    QTextCursor cursor = m_textEdit->textCursor();
    int startPos = cursor.selectionStart();
    int endPos = cursor.selectionEnd();

    QTextCursor startCursor = cursor;
    startCursor.setPosition(startPos);
    QRect startRect = m_textEdit->cursorRect(startCursor);
    QPoint startGlobal =
        m_textEdit->viewport()->mapToGlobal(
            QPoint(startRect.left(), startRect.center().y())) -
        QPoint(m_startHandle->width() / 2, m_startHandle->height() / 2);
    startGlobal.rx() -= m_startHandle->width() / 2;
    m_startHandle->move(startGlobal);

    QTextCursor endCursor = cursor;
    endCursor.setPosition(endPos);
    QRect endRect = m_textEdit->cursorRect(endCursor);
    QPoint endGlobal =
        m_textEdit->viewport()->mapToGlobal(
            QPoint(endRect.right(), endRect.center().y())) -
        QPoint(m_endHandle->width() / 2, m_endHandle->height() / 2);
    endGlobal.rx() += m_endHandle->width() / 2;
    m_endHandle->move(endGlobal);
  }
  // QLineEdit 处理
  else if (m_lineEdit) {
    // 1. 计算选择起点/终点（基于光标在末尾特性）
    int cursorEndPos = m_lineEdit->cursorPosition();
    int selectLength = m_lineEdit->selectedText().length();
    int selectStartPos = cursorEndPos - selectLength;
    if (selectLength <= 0 || selectStartPos < 0 ||
        cursorEndPos > m_lineEdit->text().length()) {
      m_startHandle->hide();
      m_endHandle->hide();
      return;
    }

    // 2. 计算动态滚动偏移
    QRect contentRect = m_lineEdit->contentsRect();
    QFontMetrics fm = m_lineEdit->fontMetrics();
    QString fullText = m_lineEdit->text();
    int visibleLeftCursor =
        m_lineEdit->cursorPositionAt(QPoint(0, contentRect.height() / 2));
    int scrollOffset = fm.horizontalAdvance(fullText.left(visibleLeftCursor));

    // 3. 全局宽度转本地宽度
    int startLocalWidth =
        fm.horizontalAdvance(fullText.left(selectStartPos)) - scrollOffset;
    int endLocalWidth =
        fm.horizontalAdvance(fullText.left(cursorEndPos)) - scrollOffset;

    // 4. 基础参数
    QRect screenRect = QApplication::primaryScreen()->availableGeometry();
    int handleHalfW = m_startHandle->width() / 2;
    int handleHalfH = m_startHandle->height() / 2;
    int fixedY = contentRect.top() + contentRect.height() / 2 - handleHalfH;

    // 5. 【核心优化】手柄位置计算（与showHandlesAtSelection同步）
    QPoint startLocal(startLocalWidth - handleHalfW - handleHalfW,
                      fixedY);  // 起始手柄左移16px
    QPoint endLocal(endLocalWidth - handleHalfW + handleHalfW,
                    fixedY);  // 结束手柄右移16px

    // 6. 转换为全局坐标
    QPoint lineEditGlobalPos = m_lineEdit->mapToGlobal(QPoint(0, 0));
    QPoint startGlobal = lineEditGlobalPos + startLocal;
    QPoint endGlobal = lineEditGlobalPos + endLocal;

    // 7. 屏幕兜底（优先保证不压字）
    startGlobal.setX(qMax(screenRect.left(), startGlobal.x()));
    endGlobal.setX(
        qMax(screenRect.left(),
             qMin(endGlobal.x(), screenRect.right() - m_endHandle->width())));
    startGlobal.setY(qMax(
        screenRect.top(),
        qMin(startGlobal.y(), screenRect.bottom() - m_startHandle->height())));
    endGlobal.setY(
        qMax(screenRect.top(),
             qMin(endGlobal.y(), screenRect.bottom() - m_endHandle->height())));

    // 8. 更新手柄
    m_startHandle->move(startGlobal);
    m_endHandle->move(endGlobal);
    m_startHandle->show();
    m_endHandle->show();
    m_startHandle->raise();
    m_endHandle->raise();
  }
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
  // 释放后更新基准（可选，下次拖动基于新位置）
  m_originalStartPos = m_startPos;
  m_originalEndPos = m_endPos;
  updateHandlesPosition();
  qDebug() << "[HandleReleased] 基准更新为：" << m_originalStartPos << "-"
           << m_originalEndPos;
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
