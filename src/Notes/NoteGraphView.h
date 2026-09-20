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

// ★ 修复后的 ArrowEdge：解决不显示问题
class ArrowEdge : public QGraphicsPathItem {
 public:
  ArrowEdge(const QPointF& start, const QPointF& end, qreal curvature,
            const QPen& pen, qreal targetRadius = 0)
      : m_pen(pen) {
    setPen(pen);
    setBrush(pen.color());  // ★ 关键：设置 Brush 确保箭头实心填充
    setZValue(1);           // ★ 关键：提升 Z 值，防止被卫星节点(z=1)完全遮盖

    // ★ 关键：计算实际可见的终点，避免箭头画在目标圆圈内部被遮挡
    QPointF actualEnd = end;
    if (targetRadius > 0) {
      QPointF dir = end - start;
      qreal len = qSqrt(dir.x() * dir.x() + dir.y() * dir.y());
      if (len > 0) {
        // 将终点向起点方向回退 targetRadius + 箭头长度(10) + 余量(2)
        actualEnd = end - (dir / len) * (targetRadius + 12.0);
      }
    }

    // 计算贝塞尔曲线
    QPointF mid = (start + actualEnd) / 2.0;
    QPointF d = actualEnd - start;
    QPointF normal(-d.y(), d.x());
    qreal len = qSqrt(normal.x() * normal.x() + normal.y() * normal.y());
    if (len > 0) normal /= len;
    QPointF ctrl = mid + normal * curvature;

    QPainterPath path;
    path.moveTo(start);
    path.quadTo(ctrl, actualEnd);
    setPath(path);

    m_arrowAngle = qAtan2(actualEnd.y() - ctrl.y(), actualEnd.x() - ctrl.x());
    m_actualEnd = actualEnd;
  }

 protected:
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
             QWidget* widget) override {
    QGraphicsPathItem::paint(painter, option, widget);

    // 使用保存的 m_pen 绘制箭头
    painter->setPen(m_pen);
    painter->setBrush(m_pen.color());

    qreal arrowSize = 10;
    QPointF p1 =
        m_actualEnd - QPointF(qCos(m_arrowAngle - M_PI / 6) * arrowSize,
                              qSin(m_arrowAngle - M_PI / 6) * arrowSize);
    QPointF p2 =
        m_actualEnd - QPointF(qCos(m_arrowAngle + M_PI / 6) * arrowSize,
                              qSin(m_arrowAngle + M_PI / 6) * arrowSize);
    QPolygonF arrowHead;
    arrowHead << m_actualEnd << p1 << p2;
    painter->drawPolygon(arrowHead);
  }

 private:
  QPointF m_actualEnd;
  qreal m_arrowAngle;
  QPen m_pen;
};

// ★ GraphNodeItem 保持不变
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
    setFlag(QGraphicsItem::ItemIsFocusable, false);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setAcceptHoverEvents(false);
    setAcceptedMouseButtons(Qt::LeftButton);
    setZValue(type == Center ? 3 : 2);  // ★ 节点 Z 值高于连线(1)
  }

  QString filePath() const { return m_filePath; }
  NodeType nodeType() const { return m_type; }

  void setSelectedVisual(bool selected) {
    if (m_isSelected == selected) return;
    m_isSelected = selected;
    update();
  }

 signals:
  void nodeClicked(const QString& filePath);

 protected:
  QRectF boundingRect() const override {
    qreal penWidth = pen().widthF();
    qreal totalExtent = m_radius + penWidth / 2.0;
    qreal margin = 2.0;
    return QRectF(-totalExtent - margin, -totalExtent - margin,
                  (totalExtent + margin) * 2, (totalExtent + margin) * 2);
  }

  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
             QWidget* widget) override {
    QGraphicsEllipseItem::paint(painter, option, widget);
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

class NoteGraphView : public QDialog {
  Q_OBJECT

 public:
  explicit NoteGraphView(QWidget* parent = nullptr);
  ~NoteGraphView();
  void showNoteGraph();

 protected:
  // ★ 新增：鼠标拖拽画布事件
  bool eventFilter(QObject* obj, QEvent* event) override;

 private slots:
  void onNodeClicked(const QString& filePath);
  void on_btnView_clicked();
  void on_btnEdit_clicked();

 private:
  Ui::NoteGraphView* ui;
  GraphNodeItem* m_currentSelected = nullptr;

  // ★ 新增：拖拽状态
  bool m_isDragging = false;
  QPoint m_lastMousePos;
};

#endif  // NOTEGRAPHVIEW_H