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

  // ★ 关键：对 viewport 安装事件过滤器，拦截鼠标事件
  ui->graphicsView->viewport()->installEventFilter(this);
  // 确保 viewport 能接收鼠标追踪
  ui->graphicsView->viewport()->setMouseTracking(true);
}

NoteGraphView::~NoteGraphView() { delete ui; }

bool NoteGraphView::eventFilter(QObject* obj, QEvent* event) {
  if (obj == ui->graphicsView->viewport()) {
    switch (event->type()) {
      case QEvent::MouseButtonPress: {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton ||
            mouseEvent->button() == Qt::MiddleButton) {
          QGraphicsItem* itemAtPos =
              ui->graphicsView->itemAt(mouseEvent->pos());

          // ★ 【核心】使用 qgraphicsitem_cast 准确识别节点
          // 替代原来无效的 itemAtPos->type() != GraphNodeItem::Type
          GraphNodeItem* nodeItem =
              itemAtPos ? qgraphicsitem_cast<GraphNodeItem*>(itemAtPos)
                        : nullptr;

          // 只有点击在【非节点】区域时，才启动画布拖拽
          if (!nodeItem) {
            m_isDragging = true;
            m_lastMousePos = mouseEvent->pos();
            ui->graphicsView->viewport()->setCursor(Qt::ClosedHandCursor);
            return true;
          }
          // ★ 如果点中了节点，不 return true，让事件继续传递给节点的
          // mousePressEvent
        }
        break;
      }

      case QEvent::MouseMove: {
        if (m_isDragging) {
          auto* mouseEvent = static_cast<QMouseEvent*>(event);
          QPoint delta = mouseEvent->pos() - m_lastMousePos;
          m_lastMousePos = mouseEvent->pos();
          QScrollBar* hBar = ui->graphicsView->horizontalScrollBar();
          QScrollBar* vBar = ui->graphicsView->verticalScrollBar();
          hBar->setValue(hBar->value() - delta.x());
          vBar->setValue(vBar->value() - delta.y());
          return true;
        }
        break;
      }

      case QEvent::MouseButtonRelease: {
        if (m_isDragging) {
          m_isDragging = false;
          ui->graphicsView->viewport()->setCursor(Qt::ArrowCursor);
          return true;
        }
        break;
      }

      default:
        break;
    }
  }
  return QDialog::eventFilter(obj, event);
}

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

  ui->btnEdit->setEnabled(true);
  ui->btnView->setEnabled(true);
}

void NoteGraphView::on_btnView_clicked() { m_Notes->previewNote(); }

void NoteGraphView::on_btnEdit_clicked() { m_Notes->openEditUI(); }

void NoteGraphView::showNoteGraph() {
  m_Method->setWinPos(this, 500, 3);
  ui->btnEdit->setEnabled(false);
  ui->btnView->setEnabled(false);

  QGraphicsScene* scene = new QGraphicsScene(this);
  scene->setBackgroundBrush(QBrush(Qt::white));

  QString mainTitle =
      listNoteGraph.size() > 0 ? listNoteGraph.at(0) : "Unknown";
  QString jsonData = listNoteGraph.size() > 1 ? listNoteGraph.at(1) : "{}";
  // 从jsonData中解析中心节点路径
  QString centerPath;
  QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8());
  if (!doc.isNull() && doc.isObject()) {
    centerPath = doc.object().value("current").toString();
  }

  // --- 1. 创建中心节点 ---
  const qreal centerRadius = 40;
  // ★ 确认 centerPath 有值
  qDebug() << "Center node path:" << centerPath;

  auto* centerNode =
      new GraphNodeItem(centerRadius, GraphNodeItem::Center, centerPath);
  centerNode->setBrush(QColor(70, 130, 180));
  centerNode->setPen(Qt::NoPen);
  scene->addItem(centerNode);
  connect(centerNode, &GraphNodeItem::nodeClicked, this,
          &NoteGraphView::onNodeClicked);

  auto* centerText = scene->addText(mainTitle);
  centerText->setDefaultTextColor(Qt::white);
  centerText->setPos(-centerText->boundingRect().width() / 2,
                     -centerText->boundingRect().height() / 2);
  centerText->setZValue(4);
  centerText->setAcceptedMouseButtons(Qt::NoButton);
  centerText->setAcceptHoverEvents(false);
  centerText->setTextInteractionFlags(Qt::NoTextInteraction);

  // --- 2. 【精确计算】最小安全轨道半径 ---
  QJsonArray links =
      QJsonDocument::fromJson(jsonData.toUtf8()).object()["links"].toArray();
  int count = links.size();

  const qreal satRadius = 25;
  const qreal textWidth = 120;  // setTextWidth(120)
  const qreal textHeight = 60;  // 预估文字最大高度
  const qreal gap = 15;         // 节点/文字之间的最小安全间隙

  // 每个卫星节点占据的有效宽度 = 圆直径 + 文字宽度(取较大者) + 间隙
  // 实际上文字在圆下方，所以水平方向主要看圆和文字的宽度包络
  qreal nodeEffectiveWidth = qMax(satRadius * 2, textWidth) + gap;

  // ★ 核心公式：根据弦长反推最小半径 R = (W/2) / sin(π/N)
  // 当 count <= 1 时不需要环形布局；count == 2 时特殊处理
  qreal orbitRadius = 200.0;  // 默认基础半径（保证离中心节点足够远）
  if (count >= 2) {
    qreal minRadiusForSpacing =
        (nodeEffectiveWidth / 2.0) / qSin(M_PI / qMax(count, 2));
    // 取"防拥挤半径"和"基础美观半径"的较大值
    orbitRadius = qMax(orbitRadius, minRadiusForSpacing);
  }

  // ★ 精确计算 SceneRect：轨道 + 圆半径 + 文字高度 + 小边距(20)
  qreal sceneHalfSize = orbitRadius + satRadius + textHeight + 20;

  // --- 3. 循环创建卫星节点和连线（精细化视觉）---
  for (int i = 0; i < count; ++i) {
    QJsonObject obj = links[i].toObject();
    QString title = obj["title"].toString();
    QString dir = obj["dir"].toString();
    QString filePath = obj["file"].toString();

    qreal angle = (2 * M_PI * i) / qMax(count, 1);
    qreal x = orbitRadius * qCos(angle);
    qreal y = orbitRadius * qSin(angle);
    QPointF satPos(x, y);

    QColor color;
    qreal curvature = 25;  // ★ 减小曲率，更优雅
    bool reverseArrow = false;

    if (dir == "in") {
      color = QColor(46, 139, 87);
      curvature = -25;
      reverseArrow = true;
    } else if (dir == "both") {
      color = QColor(148, 103, 189);
      curvature = 30;
    } else {
      color = QColor(100, 149, 237);
      curvature = 25;
    }

    // ★ 连线变细：1.5px
    QPen pen(color, 1.5);
    QPointF start = reverseArrow ? satPos : QPointF(0, 0);
    QPointF end = reverseArrow ? QPointF(0, 0) : satPos;
    qreal targetR = reverseArrow ? centerRadius : satRadius;

    auto* edge = new ArrowEdge(start, end, curvature, pen, targetR);
    scene->addItem(edge);

    // 创建卫星节点
    auto* satNode =
        new GraphNodeItem(satRadius, GraphNodeItem::Satellite, filePath);
    satNode->setPos(x, y);
    satNode->setBrush(color.lighter(160));
    satNode->setPen(QPen(color, 1.5));  // ★ 边框也变细
    scene->addItem(satNode);

    connect(satNode, &GraphNodeItem::nodeClicked, this,
            &NoteGraphView::onNodeClicked);

    // 创建标题文字
    auto* satText = scene->addText(title);
    satText->setTextWidth(textWidth);
    satText->setPos(x - textWidth / 2.0, y + satRadius + 5);
    satText->setDefaultTextColor(Qt::black);
    satText->setZValue(4);
    satText->setAcceptHoverEvents(false);
  }

  // --- 4. 配置 GraphicsView ---
  ui->graphicsView->setScene(scene);
  ui->graphicsView->setRenderHints(QPainter::Antialiasing |
                                   QPainter::TextAntialiasing);

  ui->graphicsView->viewport()->setUpdatesEnabled(false);
  ui->graphicsView->viewport()->update();
  ui->graphicsView->viewport()->setUpdatesEnabled(true);

  scene->setSceneRect(-sceneHalfSize, -sceneHalfSize, sceneHalfSize * 2,
                      sceneHalfSize * 2);

  ui->graphicsView->resetTransform();
  ui->graphicsView->centerOn(0, 0);

  show();
}