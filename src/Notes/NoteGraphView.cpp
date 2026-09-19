#include "NoteGraphView.h"

#include "Comm/Method.h"
#include "MainWindow.h"
#include "Notes/NotesList.h"
#include "defines.h"
#include "ui_NoteGraphView.h"

NoteGraphView::NoteGraphView(QWidget* parent)
    : QDialog(parent), ui(new Ui::NoteGraphView) {
  ui->setupUi(this);
  setModal(true);
  setWindowTitle(tr("Note Graph"));
}

NoteGraphView::~NoteGraphView() { delete ui; }

void NoteGraphView::showNoteGraph() {
  m_Method->setWinPos(this, 500, 3);

  QGraphicsScene* scene = new QGraphicsScene(this);
  scene->setBackgroundBrush(QBrush(Qt::white));

  QString mainTitle =
      listNoteGraph.size() > 0 ? listNoteGraph.at(0) : "Unknown";
  QString jsonData = listNoteGraph.size() > 1 ? listNoteGraph.at(1) : "{}";

  // ★ 中心节点（保持不变）
  const qreal centerRadius = 40;
  auto* centerNode = scene->addEllipse(-centerRadius, -centerRadius,
                                       centerRadius * 2, centerRadius * 2);
  centerNode->setBrush(QColor(70, 130, 180));
  centerNode->setPen(Qt::NoPen);
  centerNode->setZValue(2);

  auto* centerText = scene->addText(mainTitle);
  centerText->setDefaultTextColor(Qt::white);
  centerText->setPos(-centerText->boundingRect().width() / 2,
                     -centerText->boundingRect().height() / 2);
  centerText->setZValue(3);

  // ★ 解析并绘制
  QJsonArray links =
      QJsonDocument::fromJson(jsonData.toUtf8()).object()["links"].toArray();
  int count = links.size();
  qreal orbitRadius = 180;

  for (int i = 0; i < count; ++i) {
    QJsonObject obj = links[i].toObject();
    QString title = obj["title"].toString();
    QString dir = obj["dir"].toString();

    qreal angle = (2 * M_PI * i) / qMax(count, 1);
    qreal x = orbitRadius * qCos(angle);
    qreal y = orbitRadius * qSin(angle);
    QPointF satPos(x, y);

    // ★ 根据方向定义颜色和曲率
    QColor color;
    qreal curvature = 40;  // 基础弯曲度
    bool reverseArrow = false;

    if (dir == "in") {
      color = QColor(46, 139, 87);  // 入链：绿色
      curvature = -40;              // 反向弯曲避免与出链重叠
      reverseArrow = true;
    } else if (dir == "both") {
      color = QColor(148, 103, 189);  // 双向：紫色
      curvature = 50;
    } else {
      color = QColor(100, 149, 237);  // 出链：矢车菊蓝
      curvature = 40;
    }

    QPen pen(color, 2);

    // ★ 绘制带箭头的曲线
    QPointF start = reverseArrow ? satPos : QPointF(0, 0);
    QPointF end = reverseArrow ? QPointF(0, 0) : satPos;
    auto* edge = new ArrowEdge(start, end, curvature, pen);
    scene->addItem(edge);

    // ★ 卫星节点：用颜色块填充区分类型
    qreal satRadius = 25;
    auto* satNode = scene->addEllipse(x - satRadius, y - satRadius,
                                      satRadius * 2, satRadius * 2);
    satNode->setBrush(color.lighter(160));  // 浅色填充
    satNode->setPen(QPen(color, 2));
    satNode->setZValue(1);

    // ★ 标题
    auto* satText = scene->addText(title);
    satText->setTextWidth(120);
    satText->setPos(x - 60, y + satRadius + 5);
    satText->setDefaultTextColor(Qt::black);
    satText->setZValue(1);
  }

  // ★ View 绑定
  ui->graphicsView->setScene(scene);
  ui->graphicsView->setRenderHint(QPainter::Antialiasing);
  scene->setSceneRect(-300, -300, 600, 600);
  ui->graphicsView->centerOn(0, 0);
  ui->graphicsView->scale(1.0, 1.0);

  show();
}