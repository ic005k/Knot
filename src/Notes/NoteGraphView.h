#ifndef NOTEGRAPHVIEW_H
#define NOTEGRAPHVIEW_H

#include <QBrush>
#include <QDialog>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QtMath>

namespace Ui {
class NoteGraphView;
}

class NoteGraphView : public QDialog {
  Q_OBJECT

 public:
  explicit NoteGraphView(QWidget* parent = nullptr);
  ~NoteGraphView();

  void showNoteGraph();

 private:
  Ui::NoteGraphView* ui;
};

// ★ 自定义带箭头的贝塞尔曲线边
class ArrowEdge : public QGraphicsPathItem {
 public:
  ArrowEdge(const QPointF& start, const QPointF& end, qreal curvature,
            const QPen& pen)
      : m_start(start), m_end(end) {
    setPen(pen);
    setZValue(0);

    // 计算控制点：垂直于连线方向偏移，实现曲线效果
    QPointF mid = (start + end) / 2.0;
    QPointF dir = end - start;
    QPointF normal(-dir.y(), dir.x());  // 法向量
    qreal len = qSqrt(normal.x() * normal.x() + normal.y() * normal.y());
    if (len > 0) normal /= len;

    QPointF ctrl = mid + normal * curvature;

    QPainterPath path;
    path.moveTo(start);
    path.quadTo(ctrl, end);
    setPath(path);

    // 保存终点切线方向用于绘制箭头
    m_arrowAngle = qAtan2(end.y() - ctrl.y(), end.x() - ctrl.x());
  }

 protected:
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
             QWidget* widget) override {
    QGraphicsPathItem::paint(painter, option, widget);

    // ★ 在曲线终点绘制箭头
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

#endif  // NOTEGRAPHVIEW_H
