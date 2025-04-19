// WheelWidget.h
#pragma once
#include <QElapsedTimer>
#include <QStringList>
#include <QTimer>
#include <QWidget>
#include <cmath>

class WheelWidget : public QWidget {
  Q_OBJECT
 public:
  explicit WheelWidget(QWidget *parent = nullptr);

  // 数据操作
  void setItems(const QStringList &items);
  QStringList items() const { return m_items; }
  int currentIndex() const { return m_currentIndex; }
  void setCurrentIndex(int index);

  // 显示设置
  void setCircular(bool circular) {
    m_circular = circular;
    update();
  }
  void setVisibleItems(int count);

  QString currentText() const;

 signals:
  void currentIndexChanged(int index);

 protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;

 private:
  void animateInertia();

  int m_dragStartY;    // 拖拽起始Y坐标
  qreal m_baseOffset;  // 拖拽基准偏移量

  QStringList m_items;
  int m_itemHeight = 40;
  int m_currentIndex;
  bool m_circular;
  int m_visibleItems;
  QColor m_highlightColor{"#007AFF"};
  QColor m_textColor;

  // 拖拽状态
  QPoint m_dragStartPos;
  int m_lastY;
  qreal m_offset;
  qreal m_velocity;
  QTimer m_inertiaTimer;
  QElapsedTimer m_dragTimer;
  void updateIndex();
  void updateIndex0();
  void wheelEvent0(QWheelEvent *event);
  qreal calculateOvershoot();
};
