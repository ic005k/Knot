import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
// QtQuick.Shapes 不再需要

Item {
    id: graphViewRoot

    // --- 主画布 ---
    Item {
        id: canvas
        anchors.fill: parent
        // --- 用于画布平移的属性 ---
        property real offsetX: 0
        property real offsetY: 0
        // --- 应用平移 ---
        transform: Translate { x: canvas.offsetX; y: canvas.offsetY }

        // --- 动态绘制所有边的 Canvas ---
        Canvas {
            id: edgesCanvas
            anchors.fill: parent
            z: 1 // 确保边在节点下方

            onPaint: {
                // --- 修改：直接使用上下文属性 graphController ---
                if (!graphController || !graphController.model) {
                    console.log("EdgesCanvas: graphController or model not ready.");
                    return;
                }

                const ctx = getContext("2d");
                ctx.reset();
                const model = graphController.model;
                const nodeCount = model.count;

                if (nodeCount < 2) {
                    console.log("EdgesCanvas: Not enough nodes to draw edges.");
                    return;
                }

                console.log("EdgesCanvas: Drawing edges for", nodeCount, "nodes.");

                ctx.strokeStyle = "#AD1457"; // 玫红色
                ctx.lineWidth = 2;
                ctx.fillStyle = "#AD1457"; // 箭头颜色

                // --- 示例边绘制逻辑 ---
                // 绘制简单的链状连接 (0->1, 1->2, ...)
                for (let i = 0; i < nodeCount - 1; i++) {
                    const srcIndex = i;
                    const tgtIndex = i + 1;

                    // 获取 QML 节点项
                    const srcNodeItem = nodesRepeater.itemAt(srcIndex);
                    const tgtNodeItem = nodesRepeater.itemAt(tgtIndex);

                    // 检查节点项是否存在
                    if (!srcNodeItem || !tgtNodeItem) {
                        console.log("EdgesCanvas: Skipping edge", srcIndex, "->", tgtIndex, "- Node item not found.");
                        continue;
                    }

                    // 计算节点中心点（考虑画布偏移）
                    // 注意：itemAt 返回的是节点的根 Item (Rectangle)，我们使用它的 x,y,width,height
                    const sx = srcNodeItem.x + srcNodeItem.width / 2;
                    const sy = srcNodeItem.y + srcNodeItem.height / 2;
                    const tx = tgtNodeItem.x + tgtNodeItem.width / 2;
                    const ty = tgtNodeItem.y + tgtNodeItem.height / 2;

                    const dx = tx - sx;
                    const dy = ty - sy;
                    const dist = Math.sqrt(dx * dx + dy * dy);

                    if (dist === 0) {
                        continue;
                    }

                    // 绘制贝塞尔曲线
                    const offset = Math.min(80, 20 + dist / 8);
                    const normX = -dy / dist;
                    const normY = dx / dist;
                    const midX = (sx + tx) / 2;
                    const midY = (sy + ty) / 2;
                    const cx1 = midX + normX * offset;
                    const cy1 = midY + normY * offset;
                    const cx2 = midX - normX * offset;
                    const cy2 = midY - normY * offset;

                    ctx.beginPath();
                    ctx.moveTo(sx, sy);
                    ctx.bezierCurveTo(cx1, cy1, cx2, cy2, tx, ty);
                    ctx.stroke();

                    // 绘制箭头 (在目标节点处)
                    const angle = Math.atan2(dy, dx);
                    const headLength = 10;
                    ctx.beginPath();
                    ctx.moveTo(tx, ty);
                    ctx.lineTo(
                        tx - headLength * Math.cos(angle - Math.PI / 6),
                        ty - headLength * Math.sin(angle - Math.PI / 6)
                    );
                    ctx.lineTo(
                        tx - headLength * Math.cos(angle + Math.PI / 6),
                        ty - headLength * Math.sin(angle + Math.PI / 6)
                    );
                    ctx.closePath();
                    ctx.fill();
                }
                // --- 示例逻辑结束 ---
                // --- 请在这里替换为您自己的完整边绘制逻辑 ---
                // 例如，遍历所有节点对，检查它们之间是否有关系，然后绘制边
                // for (let i = 0; i < nodeCount; i++) {
                //     for (let j = 0; j < nodeCount; j++) {
                //         if (i != j && /* 检查节点 i 和 j 是否有连接 */) {
                //             // 获取节点 i 和 j 的 Item
                //             // 计算中心点
                //             // 绘制贝塞尔曲线和箭头
                //         }
                //     }
                // }
                console.log("EdgesCanvas: Drawing complete.");
            }

            // --- 提供给外部调用的刷新函数 ---
            function requestUpdate() {
                 console.log("EdgesCanvas: Update requested.");
                 edgesCanvas.requestPaint(); // 触发 onPaint
            }
        }


        // --- 绘制节点 (使用上下文属性访问的模型) ---
        Repeater {
            id: nodesRepeater
            model: graphController && graphController.model ? graphController.model : null

            Rectangle {
                id: nodeRect
                x: model.position ? model.position.x : 0
                y: model.position ? model.position.y : 0
                width: Math.max(nodeText.implicitWidth + 20, 180)
                height: nodeText.implicitHeight + 15
                radius: 8
                color: model.isCurrentNote ? "#0D47A1" : "#BBDEFB" // 蓝色系
                border.color: model.isCurrentNote ? "#000000" : "#64B5F6"
                border.width: model.isCurrentNote ? 2 : 1
                z: 3 // 确保节点在边之上

                Text {
                    id: nodeText
                    anchors.centerIn: parent
                    text: model.name.length > 30 ? model.name.substring(0, 27) + "..." : model.name
                    font.pixelSize: 14
                    color: model.isCurrentNote ? "white" : "#000000"
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignHCenter
                }

                MouseArea {
                    anchors.fill: parent
                    drag.target: parent // 启用拖动

                    onClicked: {
                        console.log("Clicked note:", model.name, "Index:", index);
                    }
                    onDoubleClicked: {
                         console.log("Double-clicked note:", model.name, "Index:", index);
                         if (graphController) {
                             graphController.nodeDoubleClicked(model.filePath);
                         }
                    }
                    onReleased: {
                         console.log("Node", index, "drag released at:", parent.x, ",", parent.y);
                         // --- 关键：节点拖动后，通知边 Canvas 重绘 ---
                         edgesCanvas.requestUpdate();
                         // --- 可选：通知 C++ 更新节点位置 ---
                         // if (graphController) {
                         //     graphController.updateNodePosition(index, parent.x, parent.y);
                         // }
                    }
                }
            }
        } // Repeater (Nodes)

        // --- 用于画布平移的 MouseArea ---
        MouseArea {
            id: panArea
            anchors.fill: parent
            z: 0 // 确保在最底层
            acceptedButtons: Qt.MiddleButton | Qt.RightButton
            property point lastPos

            onPressed: {
                lastPos = Qt.point(mouse.x, mouse.y);
            }

            onPositionChanged: {
                if (pressedButtons & Qt.MiddleButton || pressedButtons & Qt.RightButton) {
                    const deltaX = mouse.x - lastPos.x;
                    const deltaY = mouse.y - lastPos.y;
                    canvas.offsetX += deltaX;
                    canvas.offsetY += deltaY;
                    lastPos = Qt.point(mouse.x, mouse.y);
                    // --- 关键：画布平移后，通知边 Canvas 重绘 ---
                    edgesCanvas.requestUpdate();
                }
            }
        }
    } // Item (Canvas)

    Component.onCompleted: {
        console.log("NoteGraphView QML Component Completed.");
        console.log("Initial graphController:", graphController);
        // 延迟调用更新，确保节点已创建
        Qt.callLater(function() {
            console.log("Initial edge draw triggered.");
            edgesCanvas.requestUpdate();
        });
    }

    // --- 提供给 C++ 调用的公共函数 ---
    function updateGraph() {
        console.log("NoteGraphView.updateGraph() called from C++.");
        // 当模型更新时，触发边的重绘
        edgesCanvas.requestUpdate();
    }
}



