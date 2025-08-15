import QtQuick 2.15
import QtQuick.Controls 2.15
import NoteGraph 1.0

// 使用 Flickable 包裹整个内容以支持拖动
Flickable {
    id: flickable
    anchors.fill: parent
    contentWidth: 3000 // 增大初始内容大小以提供更多拖动空间
    contentHeight: 3000
    interactive: true // 启用交互（拖动）

    // --- 数据存储 ---
    property var currentNode: null
    property var references: []
    property var referencedBy: []
    // 中心点现在基于 Flickable 的 content 居中
    property real centerX: flickable.contentWidth / 2
    property real centerY: flickable.contentHeight / 2

    // --- 节点样式 ---
    property real nodeWidth: 120
    property real nodeHeight: 80
    property real nodeRadius: 8
    property real centerNodeScale: 1.2 // 中心节点放大比例

    // --- 布局参数 ---
    // 环形分布的内外半径限制 (基于 content 大小)
    property real minRingRadius: 220 // 增加基础半径
    property real maxRingRadius: Math.min(centerX, centerY) - Math.max(
                                     nodeWidth, nodeHeight) / 2 - 60 // 增加边缘安全距离

    // --- UI 元素 ---
    // 中心节点 (当前笔记) - 保持绿色
    Rectangle {
        id: centerNode
        // 位置基于 Flickable 的 content 坐标系
        x: flickable.centerX - (nodeWidth * centerNodeScale) / 2
        y: flickable.centerY - (nodeHeight * centerNodeScale) / 2
        width: nodeWidth * centerNodeScale
        height: nodeHeight * centerNodeScale
        radius: nodeRadius
        color: "#66bb6a" // 更亮的绿色 (Light Green)
        border.width: 2
        border.color: "#2e7d32" // 深一点的绿色边框 (Green 800)

        Text {
            anchors.fill: parent // 让文本填充整个矩形框
            anchors.margins: 8 // 留出内边距，避免文本贴边
            anchors.centerIn: parent
            text: currentNode ? currentNode.name : ""
            color: "white"
            font.bold: true
            font.pixelSize: 13
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter // 垂直居中
            maximumLineCount: Math.floor(
                                  parent.height / font.pixelSize) // 动态计算最大行数
            elide: Text.ElideRight // 超出时尾部显示省略号
        }
    }

    // 连接线画布
    Canvas {
        id: connectionCanvas
        // Canvas 也需要覆盖整个 Flickable 的 content 区域
        x: 0
        y: 0
        width: flickable.contentWidth
        height: flickable.contentHeight
        onPaint: {
            const ctx = getContext("2d")
            ctx.resetTransform()
            ctx.clearRect(0, 0, width, height)

            const centerNodeX = flickable.centerX
            const centerNodeY = flickable.centerY
            const centerNodeW = nodeWidth * centerNodeScale
            const centerNodeH = nodeHeight * centerNodeScale

            // 绘制引用关系线 (中心 -> 引用节点) - 蓝色
            references.forEach(node => {
                                   if (node.x !== undefined
                                       && node.y !== undefined) {
                                       const targetX = node.x + nodeWidth / 2
                                       const targetY = node.y + nodeHeight / 2
                                       // 计算中心节点到目标节点中心的交点
                                       const startIntersect = calculateRectLineIntersection(
                                           centerNodeX, centerNodeY,
                                           targetX, targetY,
                                           centerNodeX - centerNodeW / 2,
                                           centerNodeY - centerNodeH / 2,
                                           centerNodeW, centerNodeH)
                                       // 计算目标节点到中心节点中心的交点
                                       const endIntersect = calculateRectLineIntersection(
                                           targetX, targetY, centerNodeX,
                                           centerNodeY, node.x, node.y,
                                           nodeWidth, nodeHeight)

                                       if (startIntersect && endIntersect) {
                                           drawCurvedArrow(
                                               ctx, startIntersect.x,
                                               startIntersect.y,
                                               endIntersect.x, endIntersect.y,
                                               "#1e88e5" // 蓝色 (Blue 600)
                                               )
                                       }
                                   }
                               })

            // 绘制被引用关系线 (被引用节点 -> 中心) - 紫色
            referencedBy.forEach(node => {
                                     if (node.x !== undefined
                                         && node.y !== undefined) {
                                         const sourceX = node.x + nodeWidth / 2
                                         const sourceY = node.y + nodeHeight / 2
                                         // 计算源节点到中心节点中心的交点
                                         const startIntersect = calculateRectLineIntersection(
                                             sourceX, sourceY, centerNodeX,
                                             centerNodeY, node.x, node.y,
                                             nodeWidth, nodeHeight)
                                         // 计算中心节点到源节点中心的交点
                                         const endIntersect = calculateRectLineIntersection(
                                             centerNodeX, centerNodeY,
                                             sourceX, sourceY,
                                             centerNodeX - centerNodeW / 2,
                                             centerNodeY - centerNodeH / 2,
                                             centerNodeW, centerNodeH)

                                         if (startIntersect && endIntersect) {
                                             drawCurvedArrow(
                                                 ctx, startIntersect.x,
                                                 startIntersect.y,
                                                 endIntersect.x,
                                                 endIntersect.y,
                                                 "#8e24aa" // 紫色 (Purple 600)
                                                 )
                                         }
                                     }
                                 })
        }
    }

    // 节点容器
    Item {
        id: nodesContainer
        // 节点容器也需要覆盖整个 Flickable 的 content 区域
        x: 0
        y: 0
        width: flickable.contentWidth
        height: flickable.contentHeight
    }

    // --- 绘图函数 ---
    // 精确计算从点 (px, py) 到矩形边缘的连线交点
    function calculateRectLineIntersection(px, py, rx, ry, rectX, rectY, rectW, rectH) {
        // 矩形中心
        const rcx = rectX + rectW / 2
        const rcy = rectY + rectH / 2

        // 线段方向向量
        const dx = rx - px
        const dy = ry - py

        if (dx === 0 && dy === 0)
            return null // 两点重合

        let tMin = Number.MAX_VALUE
        let intersectX, intersectY

        // 检查与左边缘 (x = rectX) 的交点
        if (dx !== 0) {
            const t = (rectX - px) / dx
            if (t >= 0) {
                const iy = py + t * dy
                if (iy >= rectY && iy <= rectY + rectH) {
                    if (t < tMin) {
                        tMin = t
                        intersectX = rectX
                        intersectY = iy
                    }
                }
            }
        }

        // 检查与右边缘 (x = rectX + rectW) 的交点
        if (dx !== 0) {
            const t = (rectX + rectW - px) / dx
            if (t >= 0) {
                const iy = py + t * dy
                if (iy >= rectY && iy <= rectY + rectH) {
                    if (t < tMin) {
                        tMin = t
                        intersectX = rectX + rectW
                        intersectY = iy
                    }
                }
            }
        }

        // 检查与上边缘 (y = rectY) 的交点
        if (dy !== 0) {
            const t = (rectY - py) / dy
            if (t >= 0) {
                const ix = px + t * dx
                if (ix >= rectX && ix <= rectX + rectW) {
                    if (t < tMin) {
                        tMin = t
                        intersectX = ix
                        intersectY = rectY
                    }
                }
            }
        }

        // 检查与下边缘 (y = rectY + rectH) 的交点
        if (dy !== 0) {
            const t = (rectY + rectH - py) / dy
            if (t >= 0) {
                const ix = px + t * dx
                if (ix >= rectX && ix <= rectX + rectW) {
                    if (t < tMin) {
                        tMin = t
                        intersectX = ix
                        intersectY = rectY + rectH
                    }
                }
            }
        }

        if (tMin !== Number.MAX_VALUE) {
            return {
                "x": intersectX,
                "y": intersectY
            }
        }
        // 如果没有找到交点（理论上不应该），返回矩形中心
        return {
            "x": rcx,
            "y": rcy
        }
    }

    // 绘制带箭头的曲线
    function drawCurvedArrow(ctx, fromX, fromY, toX, toY, color) {
        const headLength = 12
        // 箭头大小
        const dx = toX - fromX
        const dy = toY - fromY
        const angle = Math.atan2(dy, dx)

        // 计算控制点，使其垂直于起点到终点的连线，并根据距离调整弯曲度
        const midX = (fromX + toX) / 2
        const midY = (fromY + toY) / 2
        const distance = Math.sqrt(dx * dx + dy * dy)
        // 控制点偏移量，可以根据需要调整系数来改变弯曲程度
        const offsetDistance = Math.min(100, distance / 3)
        // 垂直方向的向量 (dx, dy) -> (-dy, dx) 或 (dy, -dx)
        // 使用 (dy, -dx) 并标准化
        const len = Math.sqrt(dy * dy + dx * dx)
        const unitPerpX = (dy / len)
        const unitPerpY = (-dx / len)
        // 负号使曲线向外弯曲

        // 根据节点类型调整控制点方向，使引用和被引用的曲线朝向不同，减少重叠
        // 这里简单地通过颜色区分（实际可以根据数据属性判断）
        let controlPointOffsetX, controlPointOffsetY
        if (color === "#1e88e5") {
            // 引用线 (蓝色)
            controlPointOffsetX = unitPerpX * offsetDistance
            controlPointOffsetY = unitPerpY * offsetDistance
        } else {
            // 被引用线 (紫色)
            controlPointOffsetX = -unitPerpX * offsetDistance
            controlPointOffsetY = -unitPerpY * offsetDistance
        }

        const controlX = midX + controlPointOffsetX
        const controlY = midY + controlPointOffsetY

        // 绘制曲线
        ctx.beginPath()
        ctx.moveTo(fromX, fromY)
        ctx.quadraticCurveTo(controlX, controlY, toX, toY) // 使用二次贝塞尔曲线
        ctx.strokeStyle = color
        ctx.lineWidth = 2
        ctx.stroke()

        // 绘制箭头 (在终点处)
        // 为了箭头方向与曲线末端切线一致，需要计算切线方向
        // 对于二次贝塞尔曲线 B(t) = (1-t)^2*P0 + 2*(1-t)*t*P1 + t^2*P2
        // 在 t=1 (终点) 的切线向量是 2*(P2 - P1)
        // P0 = (fromX, fromY), P1 = (controlX, controlY), P2 = (toX, toY)
        const tangentX = 2 * (toX - controlX)
        const tangentY = 2 * (toY - controlY)
        const tangentLength = Math.sqrt(
                                tangentX * tangentX + tangentY * tangentY)
        if (tangentLength > 0.001) {
            // 避免除以零
            const unitTangentX = tangentX / tangentLength
            const unitTangentY = tangentY / tangentLength
            const arrowAngle = Math.atan2(unitTangentY, unitTangentX)

            ctx.beginPath()
            ctx.moveTo(toX, toY)
            ctx.lineTo(toX - headLength * Math.cos(arrowAngle - Math.PI / 6),
                       toY - headLength * Math.sin(arrowAngle - Math.PI / 6))
            ctx.lineTo(toX - headLength * Math.cos(arrowAngle + Math.PI / 6),
                       toY - headLength * Math.sin(arrowAngle + Math.PI / 6))
            ctx.closePath()
            ctx.fillStyle = color
            ctx.fill()
        }
    }

    // --- 布局算法 ---
    function calculateNodePositions() {
        // 清除旧节点
        clearNodes()

        if (!currentNode) {
            centerOnCurrentNode() // 即使没有节点也尝试居中
            return
        }

        const refCount = references.length
        const refByCount = referencedBy.length
        const totalNodes = refCount + refByCount

        if (totalNodes === 0) {
            connectionCanvas.requestPaint() // 即使没有节点也重绘，清除旧线
            centerOnCurrentNode() // 居中到中心节点
            return
        }

        // --- 优化：动态调整环半径以避免节点重叠 ---
        const nodeArcLengthEstimate = nodeWidth + 35
        // 增加间距
        const nodesPerFullCircle = (2 * Math.PI * minRingRadius) / nodeArcLengthEstimate

        let currentRingRadius = minRingRadius
        if (totalNodes > nodesPerFullCircle) {
            currentRingRadius = (totalNodes * nodeArcLengthEstimate) / (2 * Math.PI)
        }
        currentRingRadius = Math.max(minRingRadius, Math.min(maxRingRadius,
                                                             currentRingRadius))

        // --- 优化结束 ---
        let index = 0
        const angleStep = totalNodes > 1 ? (2 * Math.PI) / totalNodes : 0
        const startAngle = -Math.PI / 2
        // 从顶部开始

        // 布局引用节点 (蓝色)
        references.forEach(node => {
                               const angle = startAngle + angleStep * index
                               const x = flickable.centerX + currentRingRadius * Math.cos(
                                   angle) - nodeWidth / 2
                               const y = flickable.centerY + currentRingRadius * Math.sin(
                                   angle) - nodeHeight / 2
                               createNode(node, x, y, "#42a5f5",
                                          "#1976d2") // 蓝色
                               index++
                           })

        // 布局被引用节点 (紫色)
        referencedBy.forEach(node => {
                                 const angle = startAngle + angleStep * index
                                 const x = flickable.centerX + currentRingRadius * Math.cos(
                                     angle) - nodeWidth / 2
                                 const y = flickable.centerY + currentRingRadius * Math.sin(
                                     angle) - nodeHeight / 2
                                 createNode(node, x, y, "#ba68c8",
                                            "#6a1b9a") // 紫色
                                 index++
                             })

        connectionCanvas.requestPaint()
        // 数据和布局更新完成后，将视图居中到中心节点
        centerOnCurrentNode()
    }

    // 将 Flickable 视图居中到中心节点
    function centerOnCurrentNode() {
        // 计算中心节点在 Flickable 内的中心坐标
        const targetContentX = flickable.centerX - flickable.width / 2
        const targetContentY = flickable.centerY - flickable.height / 2

        // 直接设置 contentX 和 contentY 使视图居中
        // 使用 Number.isFinite 确保值有效，避免 NaN 或 Infinity
        if (Number.isFinite(targetContentX) && Number.isFinite(
                    targetContentY)) {
            flickable.contentX = targetContentX
            flickable.contentY = targetContentY
        }
    }

    function clearNodes() {
        const children = nodesContainer.children
        for (var i = children.length - 1; i >= 0; i--) {
            children[i].destroy()
        }
        references.forEach(n => {
                               delete n.x
                               delete n.y
                           })
        referencedBy.forEach(n => {
                                 delete n.x
                                 delete n.y
                             })
    }

    // 创建节点
    function createNode(nodeData, x, y, fillColor, borderColor) {
        const node = Qt.createQmlObject(`
                                        import QtQuick 2.15
                                        Rectangle {
                                        x: ${x}
                                        y: ${y}
                                        width: ${nodeWidth}
                                        height: ${nodeHeight}
                                        radius: ${nodeRadius}
                                        color: "${fillColor}"
                                        border.width: 1
                                        border.color: "${borderColor}"

                                        Text {
                                        anchors.fill: parent  // 让文本填充整个矩形框
                                        anchors.margins: 8    // 留出内边距，避免文本贴边
                                        anchors.centerIn: parent
                                        text: "${nodeData.name}"
                                        color: "white"
                                        font.pixelSize: 12
                                        wrapMode: Text.WordWrap
                                        //width: ${nodeWidth * 0.8}
                                        horizontalAlignment: Text.AlignHCenter  // 水平居中
                                        verticalAlignment: Text.AlignVCenter    // 垂直居中
                                        maximumLineCount: Math.floor(parent.height / font.pixelSize)  // 动态计算最大行数
                                        elide: Text.ElideRight                 // 超出时尾部显示省略号
                                        }

                                        MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onDoubleClicked: {
                                        console.log("Double clicked node:", "${nodeData.name}");
                                        if (graphController && graphController.handleNodeDoubleClick) {
                                        graphController.handleNodeDoubleClick("${nodeData.filePath}");
                                        }
                                        }
                                        }
                                        }
                                        `, nodesContainer)

        nodeData.x = x
        nodeData.y = y
    }

    // --- 数据处理 ---
    function initData() {
        currentNode = null
        references = []
        referencedBy = []

        if (!graphController || !graphController.model)
            return

        const model = graphController.model
        const rowCount = model.rowCount()
        const allNodes = []

        for (var i = 0; i < rowCount; i++) {
            const idx = model.index(i, 0)
            allNodes.push({
                              "index": i,
                              "name": model.data(idx, NoteGraphModel.NameRole)
                                      || "无名笔记",
                              "filePath": model.data(
                                              idx, NoteGraphModel.FilePathRole)
                                          || "",
                              "isCurrent": model.data(
                                               idx,
                                               NoteGraphModel.IsCurrentNoteRole)
                          })
        }

        currentNode = allNodes.find(n => n.isCurrent)
                || (allNodes.length > 0 ? allNodes[0] : null)

        if (!currentNode) {
            calculateNodePositions() // 更新布局（清空）
            return
        }

        const relations = model.getRelations()
        relations.forEach(rel => {
                              if (rel.source === currentNode.index) {
                                  const targetNode = allNodes[rel.target]
                                  if (targetNode && !references.includes(
                                          targetNode)) {
                                      references.push(targetNode)
                                  }
                              } else if (rel.target === currentNode.index) {
                                  const sourceNode = allNodes[rel.source]
                                  if (sourceNode && !referencedBy.includes(
                                          sourceNode)) {
                                      referencedBy.push(sourceNode)
                                  }
                              }
                          })

        calculateNodePositions()
    }

    // --- 信号连接 ---
    Connections {
        target: graphController ? graphController.model : null
        function onModelReset() {
            initData()
        }
        function onDataChanged() {
            initData()
        }
    }

    // --- 初始化 ---
    Component.onCompleted: {
        // 不再在此处设置居中，改在 calculateNodePositions 完成后调用 centerOnCurrentNode
        initData()
    }
}
