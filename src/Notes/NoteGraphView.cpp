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

void NoteGraphView::onNodeClicked(const QString& filePath) {
  // 1. 取消旧节点的选中高亮
  if (m_currentSelected) {
    m_currentSelected->setSelectedVisual(false);
  }

  // 2. 如果点击的是空白或无效路径，清空选中
  if (filePath.isEmpty()) {
    m_currentSelected = nullptr;
    // TODO: 通知底部按钮禁用
    return;
  }

  // 3. 找到被点击的节点并高亮
  //    因为信号是从节点发出的，sender() 就是被点击的节点
  auto* clickedNode = qobject_cast<GraphNodeItem*>(sender());
  if (clickedNode) {
    clickedNode->setSelectedVisual(true);
    m_currentSelected = clickedNode;
  }

  currentMDFile = iniDir + filePath;

  qDebug() << "Node clicked:" << filePath;

  ui->graphicsView->update();
  ui->graphicsView->repaint();
}

void NoteGraphView::showNoteGraph() {
  m_Method->setWinPos(this, 500, 3);

  QGraphicsScene* scene = new QGraphicsScene(this);
  scene->setBackgroundBrush(QBrush(Qt::white));

  QString mainTitle =
      listNoteGraph.size() > 0 ? listNoteGraph.at(0) : "Unknown";
  QString jsonData = listNoteGraph.size() > 1 ? listNoteGraph.at(1) : "{}";

  // ★ 关键：中心节点的路径必须从外部传入（你当前 listNoteGraph.at(0)
  // 只是标题！）
  //    建议 listNoteGraph.at(2) 存中心节点的完整 filePath
  QString centerPath = listNoteGraph.size() > 2 ? listNoteGraph.at(2) : "";

  const qreal centerRadius = 40;
  auto* centerNode =
      new GraphNodeItem(centerRadius, GraphNodeItem::Center, centerPath);
  centerNode->setBrush(QColor(70, 130, 180));
  centerNode->setPen(Qt::NoPen);
  scene->addItem(centerNode);

  // 中心文字（zValue=3 保证在节点上方）
  auto* centerText = scene->addText(mainTitle);
  centerText->setDefaultTextColor(Qt::white);
  centerText->setPos(-centerText->boundingRect().width() / 2,
                     -centerText->boundingRect().height() / 2);
  centerText->setZValue(3);
  centerText->setAcceptHoverEvents(false);

  // ★ 连接中心节点的点击信号
  connect(centerNode, &GraphNodeItem::nodeClicked, this,
          &NoteGraphView::onNodeClicked);

  QJsonArray links =
      QJsonDocument::fromJson(jsonData.toUtf8()).object()["links"].toArray();
  int count = links.size();
  qreal orbitRadius = 180;

  for (int i = 0; i < count; ++i) {
    QJsonObject obj = links[i].toObject();
    QString title = obj["title"].toString();
    QString dir = obj["dir"].toString();
    // ★ 确保 JSON 中有 "file" 字段（你的 generateGraphJson 应该已经生成了）
    QString filePath = obj["file"].toString();

    qreal angle = (2 * M_PI * i) / qMax(count, 1);
    qreal x = orbitRadius * qCos(angle);
    qreal y = orbitRadius * qSin(angle);
    QPointF satPos(x, y);

    QColor color;
    qreal curvature = 40;
    bool reverseArrow = false;

    if (dir == "in") {
      color = QColor(46, 139, 87);
      curvature = -40;
      reverseArrow = true;
    } else if (dir == "both") {
      color = QColor(148, 103, 189);
      curvature = 50;
    } else {
      color = QColor(100, 149, 237);
      curvature = 40;
    }

    QPen pen(color, 2);
    QPointF start = reverseArrow ? satPos : QPointF(0, 0);
    QPointF end = reverseArrow ? QPointF(0, 0) : satPos;
    auto* edge = new ArrowEdge(start, end, curvature, pen);
    scene->addItem(edge);

    // ★ 卫星节点替换为自定义类
    qreal satRadius = 25;
    auto* satNode =
        new GraphNodeItem(satRadius, GraphNodeItem::Satellite, filePath);
    satNode->setPos(x, y);
    satNode->setBrush(color.lighter(160));
    satNode->setPen(QPen(color, 2));
    scene->addItem(satNode);

    // ★ 连接卫星节点的点击信号
    connect(satNode, &GraphNodeItem::nodeClicked, this,
            &NoteGraphView::onNodeClicked);

    // 标题文字
    auto* satText = scene->addText(title);
    satText->setTextWidth(120);
    satText->setPos(x - 60, y + satRadius + 5);
    satText->setDefaultTextColor(Qt::black);
    satText->setZValue(1);
    satText->setAcceptHoverEvents(false);
  }

  ui->graphicsView->setScene(scene);

  // ★ 1. 开启抗锯齿（让文字和圆更平滑）
  ui->graphicsView->setRenderHints(QPainter::Antialiasing |
                                   QPainter::TextAntialiasing);

  // ★ 2. 【核心修复】关闭 viewport 的局部更新缓存，彻底消灭文本残影
  // 强制每次重绘都重新生成整个图谱的渲染缓存
  ui->graphicsView->viewport()->setUpdatesEnabled(false);
  ui->graphicsView->viewport()->update();
  ui->graphicsView->viewport()->setUpdatesEnabled(true);

  // ★ 3. 设置场景范围，防止背景出现黑边
  scene->setSceneRect(-300, -300, 600, 600);

  // ★ 4. 居中显示，保持正常 1:1 比例（删掉 fitInView）
  ui->graphicsView->centerOn(0, 0);
  ui->graphicsView->scale(1.0, 1.0);

  show();
}
void NoteGraphView::on_btnView_clicked() { m_Notes->previewNote(); }

void NoteGraphView::on_btnEdit_clicked() { m_Notes->openEditUI(); }
