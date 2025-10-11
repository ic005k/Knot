#ifndef TEXTEDITTOOLBAR_H
#define TEXTEDITTOOLBAR_H

#include <QHBoxLayout>
#include <QInputMethodEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPoint>
#include <QPushButton>
#include <QTextEdit>
#include <QTimer>
#include <QTouchEvent>
#include <QWidget>

// 自定义选择手柄控件（声明）
class HandleWidget : public QWidget {
  Q_OBJECT
 public:
  explicit HandleWidget(QWidget *parent = nullptr);

 signals:
  void moved(const QPoint &globalPos);
  void released();
  void pressed();

 protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

 private:
  QPoint m_dragStart;
  QPoint m_dragOffset;
  bool m_isDragging = false;
};

/////////////////////////////////////////////////////////////////////////////////

// 顶层操作工具栏（声明）
class TextEditToolbar : public QWidget {
  Q_OBJECT
 public:
  explicit TextEditToolbar(QWidget *parent = nullptr);
  void bindEditWidget(QWidget *editWidget);
  void showAtSelection();
  void hide();  // 重写隐藏逻辑（无override，因QWidget::hide非虚函数）

 protected:
  void keyPressEvent(QKeyEvent *event) override;
  void showEvent(QShowEvent *event) override;
  bool eventFilter(QObject *obj, QEvent *event) override;

 private slots:
  void onMinusClicked();
  void onPlusClicked();
  void onCopyClicked();
  void onCutClicked();
  void onPasteClicked();
  void onSelectAllClicked();
  void onStartHandleMoved(const QPoint &globalPos);
  void onEndHandleMoved(const QPoint &globalPos);
  void onSelectionChanged();

  void onHandleReleased();

  void onStartHandlePressed();
  void onEndHandlePressed();

 private:
  // 初始化函数声明
  void initButtons();
  void initHandles();
  void adjustButtonSizes();
  void showHandlesAtSelection();
  void setupMinimumSize();  // 原私有函数声明

  // 选择调整函数声明
  void adjustTextEditSelection(int step);
  void adjustLineEditSelection(int step);

  // 手柄与选择同步函数声明
  void updateSelectionFromHandle(const QPoint &globalPos, bool isStartHandle);
  void updateSelection(int start, int end);
  void updateHandlesPosition();

  // 成员变量声明
  // 按钮
  QPushButton *btnMinus = nullptr;
  QPushButton *btnPlus = nullptr;
  QPushButton *btnCopy = nullptr;
  QPushButton *btnCut = nullptr;
  QPushButton *btnPaste = nullptr;
  QPushButton *btnSelectAll = nullptr;

  // 选择手柄
  HandleWidget *m_startHandle = nullptr;
  HandleWidget *m_endHandle = nullptr;

  // 绑定的编辑控件
  QTextEdit *m_textEdit = nullptr;
  QLineEdit *m_lineEdit = nullptr;

  // 当前选择位置
  int m_startPos = 0;
  int m_endPos = 0;

  void autoSelectTwoChars();

  int m_originalStartPos = 0;
  int m_originalEndPos = 0;

  bool m_dragging = false;
  int m_dragAnchorStart = 0;
  int m_dragAnchorEnd = 0;
};

/////////////////////////////////////////////////////////////////////////////

// 事件过滤器（声明）
class EditEventFilter : public QObject {
  Q_OBJECT
 public:
  explicit EditEventFilter(TextEditToolbar *toolbar, QObject *parent = nullptr);

 protected:
  bool eventFilter(QObject *obj, QEvent *event) override;

 private slots:
  void onTimeout();

 private:
  TextEditToolbar *m_toolbar = nullptr;
  QTimer m_timer;
  QPoint m_pressPos;
  QObject *m_target = nullptr;
};

#endif  // TEXTEDITTOOLBAR_H
