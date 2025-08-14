import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import NoteGraph 1.0

Item {
    id: noteGraphView
    anchors.fill: parent

    // 控制器单例引用
    property NoteGraphController controller: NoteGraphController ? NoteGraphController : null

    // 基础配置参数
    property real scaleFactor: 1.0
    property real offsetX: 0
    property real offsetY: 0
    property bool isDragging: false  // 背景拖动状态（全局）
    property real startDragX: 0
    property real startDragY: 0
    property int lastPaintTime: 0

    // 模型变化监听
    onControllerChanged: {
        if (controller) {
            controller.modelChanged.connect(refreshView);
            controller.model.rowsRemoved.connect(function(parent, first, last) {
                refreshView();
            });
            controller.model.rowsInserted.connect(function(parent, first, last) {
                refreshView();
            });
        }
    }

    // 背景拖动区域（使用全局isDragging）
    MouseArea {
        id: backgroundMouseArea
        anchors.fill: parent

        onPressed: function(mouse) {
            noteGraphView.isDragging = true;  // 明确指定作用域
            startDragX = mouse.x - offsetX;
            startDragY = mouse.y - offsetY;
        }

        onMouseXChanged: function(mouse) {
            if (noteGraphView.isDragging) {  // 限定作用域
                offsetX = mouse.x - startDragX;
                offsetY = mouse.y - startDragY;
                requestThrottledPaint();
            }
        }

        onMouseYChanged: function(mouse) {
            if (noteGraphView.isDragging) {  // 限定作用域
                offsetX = mouse.x - startDragX;
                offsetY = mouse.y - startDragY;
                requestThrottledPaint();
            }
        }

        onReleased: function(mouse) {
            noteGraphView.isDragging = false;  // 明确指定作用域
            connectionLines.requestPaint();
        }
        propagateComposedEvents: true
    }

    // 绘制连接线（强化可见性）
    Canvas {
        id: connectionLines
        anchors.fill: parent
        antialiasing: true

        onPaint: {
            const ctx = getContext("2d");
            ctx.resetTransform();
            ctx.clearRect(0, 0, width, height);

            if (!controller || !controller.model) {
                ctx.fillStyle = "red";
                ctx.fillText("模型未加载", 50, 50);
                return;
            }

            // 调试信息（确认关系数）
            const relationCount = controller.model.getRelations ? controller.model.getRelations().length : 0;
            ctx.fillStyle = "red";
            ctx.fillText("节点数: " + controller.model.rowCount(), 50, 30);
            ctx.fillText("关系数: " + relationCount, 50, 50);
            if (relationCount === 0) {
                ctx.fillText("无关系数据", 50, 70);
                return;
            }

            // 应用视图偏移（居中显示）
            ctx.translate(noteGraphView.offsetX + width/2, noteGraphView.offsetY + height/2);

            // 绘制红色粗线
            const relations = controller.model.getRelations();
            ctx.strokeStyle = "#FF0000";  // 纯红色
            ctx.lineWidth = 4;  // 固定粗度，确保可见
            ctx.lineCap = "round";

            for (let i = 0; i < relations.length; i++) {
                const rel = relations[i];
                const sourceIdx = controller.model.index(rel.source, 0);
                const targetIdx = controller.model.index(rel.target, 0);

                // 获取节点位置（强制转换为QPointF）
                const sourcePos = controller.model.data(sourceIdx, NoteGraphModel.PositionRole).value;
                const targetPos = controller.model.data(targetIdx, NoteGraphModel.PositionRole).value;

                // 调试单个关系
                if (!sourcePos || !targetPos) {
                    ctx.fillText("节点位置无效: " + i, 50, 90 + i*20);
                    continue;
                }

                // 绘制连线
                ctx.beginPath();
                ctx.moveTo(sourcePos.x * scaleFactor, sourcePos.y * scaleFactor);
                ctx.lineTo(targetPos.x * scaleFactor, targetPos.y * scaleFactor);
                ctx.stroke();

                // 绘制箭头
                drawArrow(ctx, targetPos.x * scaleFactor, targetPos.y * scaleFactor,
                         sourcePos.x * scaleFactor, sourcePos.y * scaleFactor, 10);
            }
        }

        function drawArrow(ctx, x, y, fromX, fromY, size) {
            const angle = Math.atan2(y - fromY, x - fromX);
            ctx.save();
            ctx.translate(x, y);
            ctx.rotate(angle);
            ctx.beginPath();
            ctx.moveTo(0, 0);
            ctx.lineTo(-size, size/2);
            ctx.lineTo(-size, -size/2);
            ctx.closePath();
            ctx.fillStyle = "#FF0000";
            ctx.fill();
            ctx.restore();
        }
    }

    // 节点视图（修复dragging作用域）
    Repeater {
        model: controller && controller.model ? controller.model : null

        delegate: Rectangle {
            id: nodeItem
            property string nodeName: model.name || "未知节点"
            property bool isCurrentNote: model.isCurrentNote || false
            property string filePath: model.filePath || ""
            property bool nodeDragging: false  // 节点拖动状态（独立变量，避免冲突）
            property real nodeStartX: 0
            property real nodeStartY: 0
            signal positionChanged()
            signal doubleClicked()

            // 节点位置（居中显示）
            x: noteGraphView.offsetX + width/2 + (model.position.x || 0) * scaleFactor
            y: noteGraphView.offsetY + height/2 + (model.position.y || 0) * scaleFactor
            width: Math.max(textItem.implicitWidth + 16, 80)
            height: Math.max(textItem.implicitHeight + 12, 40)
            radius: 8
            color: isCurrentNote ? "#4285F4" : "#F0F0F0"
            border.color: isCurrentNote ? "#2D62D3" : "#CCCCCC"
            border.width: 2

            MouseArea {
                anchors.fill: parent

                onPressed: function(mouse) {
                    nodeItem.nodeDragging = true;  // 使用节点自身的拖动变量
                    nodeItem.nodeStartX = mouse.x;
                    nodeItem.nodeStartY = mouse.y;
                    nodeItem.z = 10;
                }

                onMouseXChanged: function(mouse) {
                    if (nodeItem.nodeDragging) {  // 限定作用域
                        nodeItem.x += mouse.x - nodeItem.nodeStartX;
                        nodeItem.y += mouse.y - nodeItem.nodeStartY;
                        nodeItem.nodeStartX = mouse.x;
                        nodeItem.nodeStartY = mouse.y;
                        nodeItem.positionChanged();
                        requestThrottledPaint();
                    }
                }

                onMouseYChanged: function(mouse) {
                    if (nodeItem.nodeDragging) {  // 限定作用域
                        nodeItem.x += mouse.x - nodeItem.nodeStartX;
                        nodeItem.y += mouse.y - nodeItem.nodeStartY;
                        nodeItem.nodeStartX = mouse.x;
                        nodeItem.nodeStartY = mouse.y;
                        nodeItem.positionChanged();
                        requestThrottledPaint();
                    }
                }

                onReleased: function(mouse) {
                    nodeItem.nodeDragging = false;
                    nodeItem.z = 0;
                    connectionLines.requestPaint();
                }

                onDoubleClicked: function(mouse) {
                    nodeItem.doubleClicked();
                }
                propagateComposedEvents: false
            }

            Text {
                id: textItem
                anchors.centerIn: parent
                text: nodeName
                color: isCurrentNote ? "white" : "black"
                font.pixelSize: 14
                font.family: "SimHei"
                wrapMode: Text.WordWrap
                width: parent.width - 16
            }

            onPositionChanged: {
                if (controller && controller.model) {
                    controller.model.setNodePosition(
                        index,
                        (x - noteGraphView.offsetX - width/2) / scaleFactor,
                        (y - noteGraphView.offsetY - height/2) / scaleFactor
                    );
                }
            }

            onDoubleClicked: {
                if (controller) {
                    controller.handleNodeDoubleClick(filePath);
                }
            }
        }
    }

    // 重绘频率控制
    function requestThrottledPaint() {
        const now = Date.now();
        if (now - lastPaintTime > 16) {
            connectionLines.requestPaint();
            lastPaintTime = now;
        }
    }

    // 视图刷新函数
    function refreshView() {
        connectionLines.requestPaint();
    }
}
