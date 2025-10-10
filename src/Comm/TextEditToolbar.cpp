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
  setFixedSize(24, 24);
  setAttribute(Qt::WA_TranslucentBackground);
  setMouseTracking(true);
  setCursor(Qt::OpenHandCursor);
}

void HandleWidget::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  QBrush brush(QColor(52, 152, 219));
  painter.setBrush(brush);
  painter.drawEllipse(rect().adjusted(2, 2, -2, -2));

  painter.setBrush(Qt::white);
  painter.drawEllipse(rect().adjusted(8, 8, -8, -8));
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
  setAttribute(Qt::WA_TranslucentBackground);
  installEventFilter(this);
  setFocusPolicy(Qt::StrongFocus);

  // 初始化控件
  initButtons();
  initHandles();

  // 布局设置
  QHBoxLayout *mainLayout = new QHBoxLayout(this);
  mainLayout->setSpacing(6);
  mainLayout->setContentsMargins(8, 8, 8, 8);
  mainLayout->addWidget(btnMinus);
  mainLayout->addWidget(btnPlus);
  mainLayout->addWidget(btnCopy);
  mainLayout->addWidget(btnCut);
  mainLayout->addWidget(btnPaste);
  mainLayout->addWidget(btnSelectAll);
  setLayout(mainLayout);

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
  btnSelectAll = new QPushButton(tr("SelAll"), this);

  // 绑定点击信号槽
  connect(btnMinus, &QPushButton::clicked, this,
          &TextEditToolbar::onMinusClicked);
  connect(btnPlus, &QPushButton::clicked, this,
          &TextEditToolbar::onPlusClicked);
  connect(btnCopy, &QPushButton::clicked, this,
          &TextEditToolbar::onCopyClicked);
  connect(btnCut, &QPushButton::clicked, this, &TextEditToolbar::onCutClicked);
  connect(btnPaste, &QPushButton::clicked, this,
          &TextEditToolbar::onPasteClicked);
  connect(btnSelectAll, &QPushButton::clicked, this,
          &TextEditToolbar::onSelectAllClicked);
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
  // 固定按钮尺寸：44x44px（触摸友好，刚好显示文字/图标）
  const int BUTTON_SIZE = 44;
  // 固定工具栏宽度：内边距(8 * 2) + 6个按钮(44 * 6) + 5个间距(5 * 5) =
  // 16+264+25=305 → 取整300px（足够容纳）
  const int TOOLBAR_WIDTH = 300;
  // 固定工具栏高度：按钮高度(44) + 上下内边距(8 * 2) = 60px
  const int TOOLBAR_HEIGHT = 60;

  // 1. 给所有按钮设固定大小
  QList<QPushButton *> buttons = {btnMinus, btnPlus,  btnCopy,
                                  btnCut,   btnPaste, btnSelectAll};
  for (auto btn : buttons) {
    btn->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
  }

  // 2. 给工具栏设固定大小
  setFixedSize(TOOLBAR_WIDTH, TOOLBAR_HEIGHT);

  // 3. 确保布局间距匹配（避免按钮挤压）
  QHBoxLayout *mainLayout = qobject_cast<QHBoxLayout *>(layout());
  if (mainLayout) {
    mainLayout->setSpacing(5);                   // 按钮间固定间距5px
    mainLayout->setContentsMargins(8, 8, 8, 8);  // 工具栏内边距固定8px
  }
}

void TextEditToolbar::bindEditWidget(QWidget *editWidget) {
  // 1. 先解绑旧控件的信号（避免重复绑定）
  if (m_textEdit) {
    disconnect(m_textEdit, &QTextEdit::selectionChanged, this,
               &TextEditToolbar::onSelectionChanged);
  }
  if (m_lineEdit) {
    disconnect(m_lineEdit, &QLineEdit::cursorPositionChanged, this,
               &TextEditToolbar::onSelectionChanged);
    disconnect(m_lineEdit, &QLineEdit::textChanged, this,
               &TextEditToolbar::onSelectionChanged);
  }

  // 2. 解绑旧控件
  m_textEdit = nullptr;
  m_lineEdit = nullptr;

  // 3. 绑定新控件（QTextEdit 或 QLineEdit）
  m_textEdit = qobject_cast<QTextEdit *>(editWidget);
  m_lineEdit = qobject_cast<QLineEdit *>(editWidget);

  if (!m_textEdit && !m_lineEdit) {
    qWarning() << "TextEditToolbar: 绑定失败，目标不是QTextEdit或QLineEdit";
    return;
  }

  // 4. 绑定选择变化信号（关键：实时监听选择状态）
  if (m_textEdit) {
    // QTextEdit 有现成的 selectionChanged 信号
    connect(m_textEdit, &QTextEdit::selectionChanged, this,
            &TextEditToolbar::onSelectionChanged);
  } else if (m_lineEdit) {
    // QLineEdit 用 cursorPositionChanged + textChanged 覆盖选择变化场景
    connect(m_lineEdit, &QLineEdit::cursorPositionChanged, this,
            &TextEditToolbar::onSelectionChanged);
    connect(m_lineEdit, &QLineEdit::textChanged, this,
            &TextEditToolbar::onSelectionChanged);
  }
}

void TextEditToolbar::onSelectionChanged() {
  bool hasSelection = false;

  // 判断当前是否有选择
  if (m_textEdit) {
    hasSelection = m_textEdit->textCursor().hasSelection();
  } else if (m_lineEdit) {
    hasSelection = !m_lineEdit->selectedText().isEmpty();
  }

  // 无选择时隐藏手柄，有选择时更新手柄位置
  if (!hasSelection) {
    m_startHandle->hide();
    m_endHandle->hide();
    qDebug() << "[SelectionChanged] 无选择，隐藏手柄";
  } else {
    // 有选择时更新手柄位置（确保手柄跟选择范围同步）
    updateHandlesPosition();
    m_startHandle->show();
    m_endHandle->show();
    qDebug() << "[SelectionChanged] 有选择，显示并更新手柄";
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
  // activateWindow();
  // setFocus();

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
    int selectEndPos = qMin(currentPos + 2, textLength);

    // 【仅此处设置初始基准位置】：长按触发时的初始选择范围
    m_originalStartPos = currentPos;
    m_originalEndPos = selectEndPos;

    cursor.setPosition(currentPos, QTextCursor::MoveAnchor);
    cursor.setPosition(selectEndPos, QTextCursor::KeepAnchor);
    m_textEdit->setTextCursor(cursor);

    qDebug() << "[AutoSelect] QTextEdit: 设置选择范围:" << currentPos << "-"
             << selectEndPos;
  } else if (m_lineEdit) {
    // 1. 获取 QLineEdit 当前光标位置（已通过 EditEventFilter 定位到长按位置）
    currentPos = m_lineEdit->cursorPosition();
    textLength = m_lineEdit->text().length();
    qDebug() << "[AutoSelect] QLineEdit: currentPos=" << currentPos
             << ", textLength=" << textLength;

    // 2. 计算选择结束位置：至少选择 1 个字符（避免长度为 0）
    int selectEndPos = qMin(currentPos + 2, textLength);
    // 若 currentPos 已在文本末尾，向前选择 2 个字符
    if (selectEndPos == currentPos) {
      selectEndPos = qMax(currentPos - 2, 0);
      // 交换位置，确保 start <= end
      if (currentPos > selectEndPos) {
        std::swap(currentPos, selectEndPos);
      }
    }

    // 3. 保存初始基准位置
    m_originalStartPos = currentPos;
    m_originalEndPos = selectEndPos;

    // 4. 执行选择并验证结果
    int selectLength = selectEndPos - currentPos;
    m_lineEdit->setSelection(currentPos, selectLength);
    QString selectedText = m_lineEdit->selectedText();
    qDebug() << "[AutoSelect] QLineEdit 选择：" << currentPos << "-"
             << selectEndPos << "，长度：" << selectLength << "，内容："
             << selectedText;

    // 5. 更新选择位置变量
    m_startPos = currentPos;
    m_endPos = selectEndPos;
  }

  qDebug() << "[AutoSelect] 初始基准位置：" << m_originalStartPos << "-"
           << m_originalEndPos;
}

void TextEditToolbar::showHandlesAtSelection() {
  if (!m_textEdit && !m_lineEdit) return;

  QWidget *editWidget = m_textEdit ? static_cast<QWidget *>(m_textEdit)
                                   : static_cast<QWidget *>(m_lineEdit);
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
  }
  // -------------------------- QLineEdit 处理 --------------------------
  else if (m_lineEdit) {
    int startPos = m_lineEdit->selectionStart();
    int endPos =
        m_lineEdit->selectionStart() + m_lineEdit->selectedText().length();

    qDebug() << "[QLineEdit状态] 选择起始:" << startPos << "选择结束:" << endPos
             << "选择长度:" << m_lineEdit->selectedText().length()
             << "选中文本:" << m_lineEdit->selectedText()
             << "光标位置:" << m_lineEdit->cursorPosition();

    // 无选择时隐藏手柄
    if (startPos == endPos || startPos < 0) {
      m_startHandle->hide();
      m_endHandle->hide();
      qDebug() << "[showHandlesAtSelection] 无选择或无效位置，隐藏手柄";
      return;
    }

    // 计算起始和结束位置的光标矩形
    QRect startRect = getLineEditCursorRect(m_lineEdit);
    QRect endRect = getLineEditCursorRect(m_lineEdit);

    // 调整起始位置矩形
    QLineEdit tempStart;
    tempStart.setFont(m_lineEdit->font());
    tempStart.setText(m_lineEdit->text().left(startPos));
    QRect tempStartRect = getLineEditCursorRect(&tempStart);
    startRect.setLeft(tempStartRect.left());

    // 调整结束位置矩形
    QLineEdit tempEnd;
    tempEnd.setFont(m_lineEdit->font());
    tempEnd.setText(m_lineEdit->text().left(endPos));
    QRect tempEndRect = getLineEditCursorRect(&tempEnd);
    endRect.setLeft(tempEndRect.left());

    // 转换为全局坐标
    QPoint startGlobal = m_lineEdit->mapToGlobal(
        QPoint(startRect.left(), startRect.center().y()));
    QPoint endGlobal =
        m_lineEdit->mapToGlobal(QPoint(endRect.right(), endRect.center().y()));

    // 调整手柄位置（避免遮挡文本）
    startGlobal -=
        QPoint(m_startHandle->width() / 2, m_startHandle->height() / 2);
    startGlobal.rx() -= m_startHandle->width() / 4;  // 水平左移

    endGlobal -= QPoint(m_endHandle->width() / 2, m_endHandle->height() / 2);
    endGlobal.rx() += m_endHandle->width() / 4;  // 水平右移

    // 移动手柄
    m_startHandle->move(startGlobal);
    m_endHandle->move(endGlobal);

    // 确保手柄显示
    m_startHandle->show();
    m_endHandle->show();
    m_startHandle->raise();
    m_endHandle->raise();

    qDebug() << "[Handle Pos] Start:" << startGlobal << "End:" << endGlobal;

    // 调试信息
    qDebug() << "手柄显示状态: "
             << "StartHandle visible:" << m_startHandle->isVisible()
             << "EndHandle visible:" << m_endHandle->isVisible()
             << "StartHandle geometry:" << m_startHandle->geometry()
             << "EndHandle geometry:" << m_endHandle->geometry();
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

// ========================== 选择调整相关实现 ==========================
void TextEditToolbar::adjustTextEditSelection(int step) {
  QTextCursor cursor = m_textEdit->textCursor();
  int cursorPos = cursor.position();

  if (!cursor.hasSelection()) {
    // 无选择时，初始化选择范围
    if (step < 0) {
      cursor.setPosition(cursorPos - 1, QTextCursor::MoveAnchor);
      cursor.setPosition(cursorPos, QTextCursor::KeepAnchor);
    } else {
      cursor.setPosition(cursorPos, QTextCursor::MoveAnchor);
      cursor.setPosition(cursorPos + 1, QTextCursor::KeepAnchor);
    }
  } else {
    // 有选择时，扩展/缩小选择范围
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();
    if (step < 0) {
      start = qMax(0, start - 1);
    } else {
      end = qMin(m_textEdit->toPlainText().length(), end + 1);
    }
    cursor.setPosition(start, QTextCursor::MoveAnchor);
    cursor.setPosition(end, QTextCursor::KeepAnchor);
  }

  m_textEdit->setTextCursor(cursor);
}

void TextEditToolbar::adjustLineEditSelection(int step) {
  int cursorPos = m_lineEdit->cursorPosition();
  int start = m_lineEdit->selectionStart();
  int length = m_lineEdit->selectedText().length();
  QString text = m_lineEdit->text();

  if (length <= 0) {
    // 无选择时，初始化选择范围
    if (step < 0) {
      start = qMax(0, cursorPos - 1);
      length = 1;
    } else {
      start = cursorPos;
      length = 1;
    }
  } else {
    // 有选择时，扩展/缩小选择范围
    if (step < 0) {
      start = qMax(0, start - 1);
      length = qMin(text.length() - start, length + 1);
    } else {
      length = qMin(text.length() - start, length + 1);
    }
  }

  m_lineEdit->setSelection(start, length);
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
  // QLineEdit
  else if (m_lineEdit) {
    int startPos = m_lineEdit->selectionStart();
    int endPos =
        m_lineEdit->selectionStart() + m_lineEdit->selectedText().length();

    // 起始手柄
    QLineEdit tempStartEdit;
    tempStartEdit.setFont(m_lineEdit->font());
    tempStartEdit.setText(m_lineEdit->text().left(startPos));
    QRect startRect = getLineEditCursorRect(&tempStartEdit);
    QPoint startGlobal =
        m_lineEdit->mapToGlobal(
            QPoint(startRect.left(), startRect.center().y())) -
        QPoint(m_startHandle->width() / 2, m_startHandle->height() / 2);
    startGlobal.rx() -= m_startHandle->width() / 2;
    m_startHandle->move(startGlobal);
    m_startHandle->show();
    m_startHandle->raise();

    // 结束手柄
    QLineEdit tempEndEdit;
    tempEndEdit.setFont(m_lineEdit->font());
    tempEndEdit.setText(m_lineEdit->text().left(endPos));
    QRect endRect = getLineEditCursorRect(&tempEndEdit);
    QPoint endGlobal =
        m_lineEdit->mapToGlobal(QPoint(endRect.right(), endRect.center().y())) -
        QPoint(m_endHandle->width() / 2, m_endHandle->height() / 2);
    endGlobal.rx() += m_endHandle->width() / 2;
    m_endHandle->move(endGlobal);
    m_endHandle->show();
    m_endHandle->raise();
  }
}

QRect TextEditToolbar::getLineEditCursorRect(QLineEdit *lineEdit) {
  if (!lineEdit) return QRect();

  // 获取编辑框基础矩形
  QRect contentRect = lineEdit->rect();
  QMargins margins = lineEdit->contentsMargins();
  contentRect.adjust(margins.left(), margins.top(), -margins.right(),
                     -margins.bottom());

  // 计算光标前文本的宽度
  QFontMetrics fm(lineEdit->font());
  QString prefixText = lineEdit->text().left(lineEdit->cursorPosition());

  // 使用更高效的方式计算文本宽度
  int textWidth = 0;
  if (prefixText.length() > 1000) {
    // 对于超长文本，使用优化算法
    textWidth = fm.horizontalAdvance(prefixText, prefixText.length());
  } else {
    textWidth = fm.horizontalAdvance(prefixText);
  }

  // 考虑文本对齐方式
  int alignment = lineEdit->alignment();
  if (alignment & Qt::AlignRight) {
    textWidth = contentRect.width() - textWidth;
  } else if (alignment & Qt::AlignHCenter) {
    textWidth = (contentRect.width() - textWidth) / 2;
  }

  // 固定光标宽度
  const int cursorWidth = 2;

  return QRect(contentRect.left() + textWidth, contentRect.top(), cursorWidth,
               contentRect.height());
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
  // 缩小选择范围（向左/上缩小）
  if (m_textEdit) adjustTextEditSelection(-1);
  if (m_lineEdit) adjustLineEditSelection(-1);
  // 同步更新手柄位置
  updateHandlesPosition();
}

void TextEditToolbar::onPlusClicked() {
  // 扩展选择范围（向右/下扩展）
  if (m_textEdit) adjustTextEditSelection(1);
  if (m_lineEdit) adjustLineEditSelection(1);
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
