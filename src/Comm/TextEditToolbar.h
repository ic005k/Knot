#ifndef TEXTEDITTOOLBAR_H
#define TEXTEDITTOOLBAR_H

#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QHBoxLayout>
#include <QInputMethodEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPoint>
#include <QPushButton>
#include <QScrollArea>
#include <QTextEdit>
#include <QTimer>
#include <QTimerEvent>
#include <QWidget>
#include <QtGui/QTouchEvent>

// 顶层操作工具栏：包含选择调整和编辑功能
class TextEditToolbar : public QWidget {
  Q_OBJECT
 public:
  explicit TextEditToolbar(QWidget *parent = nullptr) : QWidget(parent) {
    // 设置窗口属性
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    installEventFilter(this);

    // 设置为接收焦点的策略
    setFocusPolicy(Qt::StrongFocus);

    // 初始化按钮
    initButtons();

    // 1. 水平布局（无滚动盒）
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(6);                   // 按钮间距（紧凑但不拥挤）
    mainLayout->setContentsMargins(8, 8, 8, 8);  // 窗口内边距

    // 添加按钮
    mainLayout->addWidget(btnMinus);
    mainLayout->addWidget(btnPlus);
    mainLayout->addWidget(btnCopy);
    mainLayout->addWidget(btnCut);
    mainLayout->addWidget(btnPaste);
    mainLayout->addWidget(btnSelectAll);

    setLayout(mainLayout);

    // 2. 样式优化（适配紧凑布局）
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
                /* 按钮文字居中，避免挤压 */
                text-align: center;
            }
            QPushButton:pressed {
                background: #d0d0d0;
            }
        )");

    // 3. 动态计算按钮尺寸（核心优化）
    adjustButtonSizes();

    hide();
  }

  // 绑定目标编辑控件（QTextEdit或QLineEdit）
  void bindEditWidget(QWidget *editWidget) {
    m_textEdit = qobject_cast<QTextEdit *>(editWidget);
    m_lineEdit = qobject_cast<QLineEdit *>(editWidget);
    if (!m_textEdit && !m_lineEdit) {
      qWarning() << "TextEditToolbar: 绑定失败，目标不是QTextEdit或QLineEdit";
    }
  }

  // 显示窗口并定位到选择范围附近
  void showAtSelection() {
    if (!m_textEdit && !m_lineEdit) return;

    // 获取选择范围在屏幕上的位置（用于定位窗口）
    QRect selectionRect;
    if (m_textEdit) {
      // 1. 获取控件局部坐标系中的选择矩形
      QRect localRect = m_textEdit->cursorRect(m_textEdit->textCursor());
      // 2. 将局部矩形转换为全局坐标系矩形（关键修复）
      QPoint globalTopLeft = m_textEdit->mapToGlobal(localRect.topLeft());
      selectionRect =
          QRect(globalTopLeft, localRect.size());  // 用点和大小构造矩形
    } else if (m_lineEdit) {
      // 1. 获取控件局部坐标系中的矩形
      QRect localRect = m_lineEdit->rect();
      // 2. 转换为全局坐标系矩形（关键修复）
      QPoint globalTopLeft = m_lineEdit->mapToGlobal(localRect.topLeft());
      selectionRect =
          QRect(globalTopLeft, localRect.size());  // 用点和大小构造矩形
    }

    // 窗口定位到选择范围上方（避免遮挡）
    int targetX = selectionRect.x();
    int targetY = selectionRect.y() - height() - 10;

    // 确保窗口不超出屏幕边界
    QRect screenRect = QApplication::primaryScreen()->geometry();
    if (targetX + width() > screenRect.right()) {
      targetX = screenRect.right() - width();
    }
    if (targetY < 0) {
      targetY = selectionRect.bottom() + 10;  // 显示在下方
    }

    move(targetX, targetY);
    show();
  }

 private slots:
  // 缩小选择范围（-按钮）
  void onMinusClicked() {
    if (m_textEdit) adjustTextEditSelection(-1);  // 向左/上缩小
    if (m_lineEdit) adjustLineEditSelection(-1);
  }

  // 扩展选择范围（+按钮）
  void onPlusClicked() {
    if (m_textEdit) adjustTextEditSelection(1);  // 向右/下扩展
    if (m_lineEdit) adjustLineEditSelection(1);
  }

  // 复制
  void onCopyClicked() {
    if (m_textEdit) m_textEdit->copy();
    if (m_lineEdit) m_lineEdit->copy();
    hide();  // 操作后隐藏工具栏
  }

  // 剪切
  void onCutClicked() {
    if (m_textEdit) m_textEdit->cut();
    if (m_lineEdit) m_lineEdit->cut();
    hide();
  }

  // 粘贴
  void onPasteClicked() {
    if (m_textEdit) m_textEdit->paste();
    if (m_lineEdit) m_lineEdit->paste();
    hide();
  }

  // 全选
  void onSelectAllClicked() {
    if (m_textEdit) m_textEdit->selectAll();
    if (m_lineEdit) m_lineEdit->selectAll();
  }

 protected:
  // 重写按键事件处理函数
  void keyPressEvent(QKeyEvent *event) override {
    if (event->key() == Qt::Key_Back) {
      // 处理返回键
      hide();           // 隐藏工具栏
      event->accept();  // 标记事件已处理
    } else {
      // 其他按键交给基类处理
      QWidget::keyPressEvent(event);
    }
  }

  // 确保工具栏显示时获取焦点
  void showEvent(QShowEvent *event) override {
    QWidget::showEvent(event);
    setFocus();  // 获取焦点以便接收按键事件
  }

  // 重写事件过滤器，强制接收返回键事件
  bool eventFilter(QObject *obj, QEvent *event) override {
    // 仅处理自身的事件
    if (obj == this) {
      // 捕获返回键（KeyPress 和 KeyRelease 都处理，确保不遗漏）
      if (event->type() == QEvent::KeyPress ||
          event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Back) {
          hide();       // 关闭工具栏
          return true;  // 拦截事件，避免传递
        }
      }
    }
    // 其他事件交给基类处理
    return QWidget::eventFilter(obj, event);
  }

 private:
  // 调整QTextEdit的选择范围（step：-1向左扩展，1向右扩展）
  void adjustTextEditSelection(int step) {
    QTextCursor cursor = m_textEdit->textCursor();
    int cursorPos = cursor.position();  // 当前光标位置

    if (!cursor.hasSelection()) {
      // 无选择时，先选中光标所在字符
      if (step < 0) {
        // 向左扩展初始化：选中光标左侧字符
        cursor.setPosition(cursorPos - 1, QTextCursor::MoveAnchor);
        cursor.setPosition(cursorPos, QTextCursor::KeepAnchor);
      } else {
        // 向右扩展初始化：选中光标右侧字符
        cursor.setPosition(cursorPos, QTextCursor::MoveAnchor);
        cursor.setPosition(cursorPos + 1, QTextCursor::KeepAnchor);
      }
    } else {
      // 有选择时，向指定方向扩展
      int start = cursor.selectionStart();
      int end = cursor.selectionEnd();

      if (step < 0) {
        // 向左扩展：起始位置左移1
        start = qMax(0, start - 1);
        cursor.setPosition(start, QTextCursor::MoveAnchor);
        cursor.setPosition(end, QTextCursor::KeepAnchor);
      } else {
        // 向右扩展：结束位置右移1
        end = qMin(m_textEdit->toPlainText().length(), end + 1);
        cursor.setPosition(start, QTextCursor::MoveAnchor);
        cursor.setPosition(end, QTextCursor::KeepAnchor);
      }
    }

    m_textEdit->setTextCursor(cursor);
  }

  // 调整QLineEdit的选择范围（step：-1向左扩展，1向右扩展）
  void adjustLineEditSelection(int step) {
    int cursorPos = m_lineEdit->cursorPosition();
    int start = m_lineEdit->selectionStart();
    int length = m_lineEdit->selectedText().length();
    QString text = m_lineEdit->text();

    if (length <= 0) {
      // 无选择时，先选中光标所在字符
      if (step < 0) {
        // 向左扩展初始化
        start = qMax(0, cursorPos - 1);
        length = 1;
      } else {
        // 向右扩展初始化
        start = cursorPos;
        length = 1;
      }
    } else {
      // 有选择时，向指定方向扩展
      if (step < 0) {
        // 向左扩展：起始位置左移1，长度加1
        start = qMax(0, start - 1);
        length = qMin(text.length() - start, length + 1);
      } else {
        // 向右扩展：长度加1
        length = qMin(text.length() - start, length + 1);
      }
    }

    m_lineEdit->setSelection(start, length);
  }

  void initButtons() {
    // 初始化按钮（设置文本、绑定槽函数）
    btnMinus = new QPushButton("<-", this);
    btnPlus = new QPushButton("->", this);
    btnCopy = new QPushButton(tr("Copy"), this);
    btnCut = new QPushButton(tr("Cut"), this);
    btnPaste = new QPushButton(tr("Paste"), this);
    btnSelectAll = new QPushButton(tr("SelAll"), this);

    // 绑定点击事件
    connect(btnMinus, &QPushButton::clicked, this,
            &TextEditToolbar::onMinusClicked);
    connect(btnPlus, &QPushButton::clicked, this,
            &TextEditToolbar::onPlusClicked);
    connect(btnCopy, &QPushButton::clicked, this,
            &TextEditToolbar::onCopyClicked);
    connect(btnCut, &QPushButton::clicked, this,
            &TextEditToolbar::onCutClicked);
    connect(btnPaste, &QPushButton::clicked, this,
            &TextEditToolbar::onPasteClicked);
    connect(btnSelectAll, &QPushButton::clicked, this,
            &TextEditToolbar::onSelectAllClicked);
  }

  void adjustButtonSizes() {
    int screenWidth = QApplication::primaryScreen()->geometry().width();
    int windowWidth = static_cast<int>(screenWidth * 0.9);
    int buttonWidth = (windowWidth - 25) / 6;  // 5个间距，每个10px

    // 确保按钮不小于最小触摸尺寸
    int buttonSize = qMax(44, buttonWidth);

    // 应用尺寸
    QList<QPushButton *> buttons = {btnMinus, btnPlus,  btnCopy,
                                    btnCut,   btnPaste, btnSelectAll};
    for (auto btn : buttons) {
      btn->setFixedSize(buttonSize, buttonSize);
    }

    // 设置窗口尺寸
    setFixedWidth(windowWidth);
    setFixedHeight(buttonSize + 16);  // 按钮高度 + 上下内边距
  }

 private:
  // 按钮
  QPushButton *btnMinus = nullptr;
  QPushButton *btnPlus = nullptr;
  QPushButton *btnCopy = nullptr;
  QPushButton *btnCut = nullptr;
  QPushButton *btnPaste = nullptr;
  QPushButton *btnSelectAll = nullptr;

  // 绑定的编辑控件
  QTextEdit *m_textEdit = nullptr;
  QLineEdit *m_lineEdit = nullptr;

  void setupMinimumSize() {
    // 获取屏幕尺寸
    QRect screenRect = QApplication::primaryScreen()->geometry();
    int screenWidth = screenRect.width();
    int screenHeight = screenRect.height();

    // 计算最小尺寸（屏幕宽度的60%，高度的15%，确保足够大）
    int minWidth = qMax(300, static_cast<int>(screenWidth * 0.6));
    int minHeight = qMax(150, static_cast<int>(screenHeight * 0.15));

    // 应用最小尺寸
    setMinimumSize(minWidth, minHeight);
  }
};

// 事件过滤器：监听长按事件，触发工具栏显示
class EditEventFilter : public QObject {
  Q_OBJECT
 public:
  explicit EditEventFilter(TextEditToolbar *toolbar, QObject *parent = nullptr)
      : QObject(parent), m_toolbar(toolbar) {
    // 确认定时器连接
    bool isConnected =
        connect(&m_timer, &QTimer::timeout, this, &EditEventFilter::onTimeout);
    qDebug() << "Timer connected:" << isConnected;
    m_timer.setSingleShot(true);
  }

 protected:
  bool eventFilter(QObject *obj, QEvent *event) {
    // 识别目标控件（QTextEdit的viewport、QTextEdit、QLineEdit）
    QTextEdit *textEdit = nullptr;
    QLineEdit *lineEdit = nullptr;

    // 检查是否是QTextEdit的viewport
    if (QWidget *viewport = qobject_cast<QWidget *>(obj)) {
      textEdit = qobject_cast<QTextEdit *>(viewport->parent());
    }
    // 检查是否是QTextEdit本身
    if (!textEdit) {
      textEdit = qobject_cast<QTextEdit *>(obj);
    }
    // 检查是否是QLineEdit
    if (!textEdit) {
      lineEdit = qobject_cast<QLineEdit *>(obj);
    }

    // 跳过Paint事件的日志输出（关键）
    if (event->type() == QEvent::Paint) {
      return false;  // 不拦截，也不输出日志
    }

    // 其他事件的日志和处理逻辑...
    qDebug() << "Event type:" << event->type() << "obj:" << obj
             << "textEdit:" << textEdit << "lineEdit:" << lineEdit;

    // 非目标控件，不处理
    if (!textEdit && !lineEdit) {
      return false;  // 不拦截事件
    }

    // 放行输入法相关事件
    if (event->type() == QInputMethodEvent::InputMethodQuery) {
      return false;  // 放行输入法事件
    }

    // 处理返回键（无论目标控件是谁，只要工具栏可见就关闭）
    if (event->type() == QEvent::KeyPress ||
        event->type() == QEvent::KeyRelease) {
      QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
      if (keyEvent->key() == Qt::Key_Back) {
        if (m_toolbar && m_toolbar->isVisible()) {
          m_toolbar->hide();
          return true;  // 拦截事件，避免重复处理
        }
      }
    }

    // 鼠标长按（修改返回值为false）
    if (event->type() == QEvent::MouseButtonPress) {
      QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
      if (mouseEvent->button() == Qt::LeftButton) {
        m_pressPos = mouseEvent->pos();
        m_timer.start(500);
        m_target = textEdit ? static_cast<QObject *>(textEdit)
                            : static_cast<QObject *>(lineEdit);
        qDebug() << "Mouse press - timer started";
        return false;  // 放行事件
      }
    }
    // 触摸长按（修改返回值为false）
    else if (event->type() == QEvent::TouchBegin) {
      QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
      const auto &touchPoints = touchEvent->points();
      if (!touchPoints.isEmpty()) {
        m_pressPos = touchPoints.first().position().toPoint();
        m_timer.start(500);
        m_target = textEdit ? static_cast<QObject *>(textEdit)
                            : static_cast<QObject *>(lineEdit);
        qDebug() << "Touch begin - timer started";
        return false;  // 放行事件
      }
    }
    // 处理定时器超时事件（新增）
    else if (event->type() == QEvent::Timer) {
      if (m_timer.isActive()) {
        m_timer.stop();
        onTimeout();  // 触发长按逻辑
        return true;  // 拦截定时器事件
      }
    }
    // 移动超过阈值
    else if (event->type() == QEvent::MouseMove ||
             event->type() == QEvent::TouchUpdate) {
      if (m_timer.isActive()) {
        qDebug() << "Move detected - check threshold";
        // 鼠标移动
        if (event->type() == QEvent::MouseMove) {
          QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
          if ((mouseEvent->pos() - m_pressPos).manhattanLength() > 10) {
            m_timer.stop();
            qDebug() << "Move exceeds threshold - timer stopped";
          }
        }
        // 触摸移动
        else if (event->type() == QEvent::TouchUpdate) {
          QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
          const auto &touchPoints = touchEvent->points();
          if (!touchPoints.isEmpty()) {
            const QTouchEvent::TouchPoint &point = touchPoints.first();
            if ((point.position() - QPointF(m_pressPos)).manhattanLength() >
                10) {
              m_timer.stop();
              qDebug() << "Touch move exceeds threshold - timer stopped";
            }
          }
        }
      }
    }
    // 释放事件
    else if (event->type() == QEvent::MouseButtonRelease ||
             event->type() == QEvent::TouchEnd) {
      if (m_timer.isActive()) {
        m_timer.stop();
        qDebug() << "Release detected - timer stopped";
      }
    }

    // 允许事件继续传递
    return false;
  }
 private slots:
  void onTimeout() {
    qDebug() << "Long press detected - show toolbar";
    if (m_target && m_toolbar) {
      m_toolbar->bindEditWidget(static_cast<QWidget *>(m_target));
      m_toolbar->showAtSelection();
    }
  }

 private:
  TextEditToolbar *m_toolbar = nullptr;
  QTimer m_timer;
  QPoint m_pressPos;
  QObject *m_target = nullptr;
};
#endif  // TEXTEDITTOOLBAR_H
