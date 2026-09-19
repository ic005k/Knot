#ifndef NOTEGRAPHVIEW_H
#define NOTEGRAPHVIEW_H

#include <QBrush>
#include <QDialog>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsTextItem>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QtMath>

namespace Ui {
class NoteGraphView;
}

// ★ 1. 先定义 ArrowEdge（保持不变）
class ArrowEdge : public QGraphicsPathItem {
 public:
  ArrowEdge(const QPointF& start, const QPointF& end, qreal curvature,
            const QPen& pen)
      : m_start(start), m_end(end) {
    setPen(pen);
    setZValue(0);
    QPointF mid = (start + end) / 2.0;
    QPointF dir = end - start;
    QPointF normal(-dir.y(), dir.x());
    qreal len = qSqrt(normal.x() * normal.x() + normal.y() * normal.y());
    if (len > 0) normal /= len;
    QPointF ctrl = mid + normal * curvature;
    QPainterPath path;
    path.moveTo(start);
    path.quadTo(ctrl, end);
    setPath(path);
    m_arrowAngle = qAtan2(end.y() - ctrl.y(), end.x() - ctrl.x());
  }

 protected:
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
             QWidget* widget) override {
    QGraphicsPathItem::paint(painter, option, widget);
    painter->setPen(pen());
    painter->setBrush(pen().color());
    qreal arrowSize = 10;
    QPointF p1 = m_end - QPointF(qCos(m_arrowAngle - M_PI / 6) * arrowSize,
                                 qSin(m_arrowAngle - M_PI / 6) * arrowSize);
    QPointF p2 = m_end - QPointF(qCos(m_arrowAngle + M_PI / 6) * arrowSize,
                                 qSin(m_arrowAngle + M_PI / 6) * arrowSize);
    QPolygonF arrowHead;
    arrowHead << m_end << p1 << p2;
    painter->drawPolygon(arrowHead);
  }

 private:
  QPointF m_start, m_end;
  qreal m_arrowAngle;
};

// ★ 2. 再定义 GraphNodeItem（在 NoteGraphView 之前）
// ★ 简化版 GraphNodeItem：选中时在圆内画一个实心点，不改变边界
class GraphNodeItem : public QObject, public QGraphicsEllipseItem {
  Q_OBJECT
 public:
  enum NodeType { Center, Satellite };

  GraphNodeItem(qreal radius, NodeType type, const QString& filePath,
                QGraphicsItem* parent = nullptr)
      : QObject(nullptr),
        QGraphicsEllipseItem(-radius, -radius, radius * 2, radius * 2, parent),
        m_radius(radius),
        m_filePath(filePath),
        m_type(type),
        m_isSelected(false) {
    // ★ 1. 禁用默认的焦点框
    setFlag(QGraphicsItem::ItemIsFocusable, false);
    // ★ 2. 禁用默认的选中虚线框
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    // ★ 3. 【核心】禁用 Qt 默认的鼠标悬停高亮行为
    // 阻止 Qt
    // 在鼠标移入时自动叠加半透明高亮层，彻底消除鼠标划过圆圈时的蓝色残影
    setAcceptHoverEvents(false);

    setAcceptedMouseButtons(Qt::LeftButton);
    setZValue(type == Center ? 2 : 1);
  }

  QString filePath() const { return m_filePath; }
  NodeType nodeType() const { return m_type; }

  void setSelectedVisual(bool selected) {
    if (m_isSelected == selected) return;
    m_isSelected = selected;
    // ★ 不需要 prepareGeometryChange()，因为边界没变，直接 update 即可
    update();
  }

 signals:
  void nodeClicked(const QString& filePath);

 protected:
  // ★ 边界固定不变，永远只包含椭圆本身 + 画笔宽度
  QRectF boundingRect() const override {
    qreal penWidth = pen().widthF();
    qreal totalExtent = m_radius + penWidth / 2.0;
    // ★ 关键：向外扩展 2 像素余量，覆盖抗锯齿边缘，杜绝残影
    qreal margin = 2.0;
    return QRectF(-totalExtent - margin, -totalExtent - margin,
                  (totalExtent + margin) * 2, (totalExtent + margin) * 2);
  }

  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
             QWidget* widget) override {
    // ★ 【关键 1】不管当前是不是选中状态，每次都无条件重绘完整的椭圆。
    // 这能强制 Qt 覆盖掉上一次残留在底层的任何颜色或脏像素，彻底杜绝残影。
    QGraphicsEllipseItem::paint(painter, option, widget);

    // ★ 【关键 2】如果处于选中状态，再绘制橙色实心点
    if (m_isSelected) {
      painter->setPen(Qt::NoPen);
      painter->setBrush(QColor(255, 165, 0));

      qreal dotRadius = m_radius * 0.35;
      painter->drawEllipse(QPointF(0, 0), dotRadius, dotRadius);
    }
  }

  void mousePressEvent(QGraphicsSceneMouseEvent* event) override {
    if (event->button() == Qt::LeftButton) {
      emit nodeClicked(m_filePath);
      event->accept();
    }
  }

 private:
  qreal m_radius;
  QString m_filePath;
  NodeType m_type;
  bool m_isSelected;
};

// ★ 3. 最后定义 NoteGraphView（此时 GraphNodeItem 已声明）
class NoteGraphView : public QDialog {
  Q_OBJECT

 public:
  explicit NoteGraphView(QWidget* parent = nullptr);
  ~NoteGraphView();

  void showNoteGraph();

 private slots:
  void onNodeClicked(const QString& filePath);

  void on_btnView_clicked();

  void on_btnEdit_clicked();

 private:
  Ui::NoteGraphView* ui;
  GraphNodeItem* m_currentSelected = nullptr;
};

#endif  // NOTEGRAPHVIEW_H