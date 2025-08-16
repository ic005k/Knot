import QtQuick 2.15
import QtQuick.Controls 2.15
import NoteGraph 1.0

Flickable {
    id: flickable
    anchors.fill: parent
    contentWidth: 3000
    contentHeight: 3000
    interactive: true
    // 初始透明度设为0，等待首次数据加载完成后再淡入
    opacity: 0

    // --- 数据存储 ---
    property var currentNode: null
    property var references: []
    property var referencedBy: []
    property real centerX: flickable.contentWidth / 2
    property real centerY: flickable.contentHeight / 2

    // --- 节点样式 ---
    property real nodeWidth: 120
    property real nodeHeight: 80
    property real nodeRadius: 8
    property real centerNodeScale: 1.2

    // --- 布局参数 ---
    property real minRingRadius: 220
    property real maxRingRadius: Math.min(centerX, centerY) - Math.max(
                                     nodeWidth, nodeHeight) / 2 - 60

    // --- UI 元素 ---
    Rectangle {
        id: centerNode
        x: flickable.centerX - (nodeWidth * centerNodeScale) / 2
        y: flickable.centerY - (nodeHeight * centerNodeScale) / 2
        width: nodeWidth * centerNodeScale
        height: nodeHeight * centerNodeScale
        radius: nodeRadius
        color: "#66bb6a"
        border.width: 2
        border.color: "#2e7d32"

        Text {
            anchors.fill: parent // 让文本填充整个矩形框
            anchors.margins: 8 // 留出内边距，避免文本贴边
            anchors.centerIn: parent
            text: currentNode ? currentNode.name : ""
            color: "white"
            font.bold: true
            font.pixelSize: 14
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter // 垂直居中
            maximumLineCount: Math.floor(
                                  parent.height / font.pixelSize) // 动态计算最大行数
            elide: Text.ElideRight // 超出时尾部显示省略号
        }
    }

    Canvas {
        id: connectionCanvas
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

            references.forEach(node => {
                                   if (node.x !== undefined
                                       && node.y !== undefined) {
                                       const targetX = node.x + nodeWidth / 2
                                       const targetY = node.y + nodeHeight / 2
                                       const startIntersect = calculateRectLineIntersection(
                                           centerNodeX, centerNodeY,
                                           targetX, targetY,
                                           centerNodeX - centerNodeW / 2,
                                           centerNodeY - centerNodeH / 2,
                                           centerNodeW, centerNodeH)
                                       const endIntersect = calculateRectLineIntersection(
                                           targetX, targetY, centerNodeX,
                                           centerNodeY, node.x, node.y,
                                           nodeWidth, nodeHeight)

                                       if (startIntersect && endIntersect) {
                                           drawCurvedArrow(ctx,
                                                           startIntersect.x,
                                                           startIntersect.y,
                                                           endIntersect.x,
                                                           endIntersect.y,
                                                           "#1e88e5")
                                       }
                                   }
                               })

            referencedBy.forEach(node => {
                                     if (node.x !== undefined
                                         && node.y !== undefined) {
                                         const sourceX = node.x + nodeWidth / 2
                                         const sourceY = node.y + nodeHeight / 2
                                         const startIntersect = calculateRectLineIntersection(
                                             sourceX, sourceY, centerNodeX,
                                             centerNodeY, node.x, node.y,
                                             nodeWidth, nodeHeight)
                                         const endIntersect = calculateRectLineIntersection(
                                             centerNodeX, centerNodeY,
                                             sourceX, sourceY,
                                             centerNodeX - centerNodeW / 2,
                                             centerNodeY - centerNodeH / 2,
                                             centerNodeW, centerNodeH)

                                         if (startIntersect && endIntersect) {
                                             drawCurvedArrow(ctx,
                                                             startIntersect.x,
                                                             startIntersect.y,
                                                             endIntersect.x,
                                                             endIntersect.y,
                                                             "#8e24aa")
                                         }
                                     }
                                 })
        }
    }

    Item {
        id: nodesContainer
        x: 0
        y: 0
        width: flickable.contentWidth
        height: flickable.contentHeight
    }

    // --- 绘图函数 ---
    function calculateRectLineIntersection(px, py, rx, ry, rectX, rectY, rectW, rectH) {
        const rcx = rectX + rectW / 2
        const rcy = rectY + rectH / 2
        const dx = rx - px
        const dy = ry - py

        if (dx === 0 && dy === 0)
            return null

        let tMin = Number.MAX_VALUE
        let intersectX, intersectY

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
        return {
            "x": rcx,
            "y": rcy
        }
    }

    function drawCurvedArrow(ctx, fromX, fromY, toX, toY, color) {
        const headLength = 12
        const dx = toX - fromX
        const dy = toY - fromY

        const midX = (fromX + toX) / 2
        const midY = (fromY + toY) / 2
        const distance = Math.sqrt(dx * dx + dy * dy)
        const offsetDistance = Math.min(100, distance / 3)
        const len = Math.sqrt(dy * dy + dx * dx)
        const unitPerpX = (dy / len)
        const unitPerpY = (-dx / len)

        let controlPointOffsetX, controlPointOffsetY
        if (color === "#1e88e5") {
            controlPointOffsetX = unitPerpX * offsetDistance
            controlPointOffsetY = unitPerpY * offsetDistance
        } else {
            controlPointOffsetX = -unitPerpX * offsetDistance
            controlPointOffsetY = -unitPerpY * offsetDistance
        }

        const controlX = midX + controlPointOffsetX
        const controlY = midY + controlPointOffsetY

        ctx.beginPath()
        ctx.moveTo(fromX, fromY)
        ctx.quadraticCurveTo(controlX, controlY, toX, toY)
        ctx.strokeStyle = color
        ctx.lineWidth = 2
        ctx.stroke()

        const tangentX = 2 * (toX - controlX)
        const tangentY = 2 * (toY - controlY)
        const tangentLength = Math.sqrt(
                                tangentX * tangentX + tangentY * tangentY)
        if (tangentLength > 0.001) {
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
        clearNodes()

        if (!currentNode) {
            centerOnCurrentNode()
            return
        }

        const refCount = references.length
        const refByCount = referencedBy.length
        const totalNodes = refCount + refByCount

        if (totalNodes === 0) {
            connectionCanvas.requestPaint()
            centerOnCurrentNode()
            return
        }

        const nodeArcLengthEstimate = nodeWidth + 35
        const nodesPerFullCircle = (2 * Math.PI * minRingRadius) / nodeArcLengthEstimate

        let currentRingRadius = minRingRadius
        if (totalNodes > nodesPerFullCircle) {
            currentRingRadius = (totalNodes * nodeArcLengthEstimate) / (2 * Math.PI)
        }
        currentRingRadius = Math.max(minRingRadius, Math.min(maxRingRadius,
                                                             currentRingRadius))

        let index = 0
        const angleStep = totalNodes > 1 ? (2 * Math.PI) / totalNodes : 0
        const startAngle = -Math.PI / 2

        references.forEach(node => {
                               const angle = startAngle + angleStep * index
                               const x = flickable.centerX + currentRingRadius * Math.cos(
                                   angle) - nodeWidth / 2
                               const y = flickable.centerY + currentRingRadius * Math.sin(
                                   angle) - nodeHeight / 2
                               createNode(node, x, y, "#42a5f5", "#1976d2")
                               index++
                           })

        referencedBy.forEach(node => {
                                 const angle = startAngle + angleStep * index
                                 const x = flickable.centerX + currentRingRadius * Math.cos(
                                     angle) - nodeWidth / 2
                                 const y = flickable.centerY + currentRingRadius * Math.sin(
                                     angle) - nodeHeight / 2
                                 createNode(node, x, y, "#ba68c8", "#6a1b9a")
                                 index++
                             })

        connectionCanvas.requestPaint()
        centerOnCurrentNode() // 布局和绘制指令已发出

        // 延迟一小段时间后淡入，确保绘制完成
        // 使用 Qt.callLater 确保在当前 JS 执行栈清空后（即绘制指令被处理后）再执行
        Qt.callLater(function () {
            // 可以再加一个极短的 Timer 以确保更大概率渲染完成
            let fadeInTimer = Qt.createQmlObject(
                    'import QtQuick 2.15; Timer { interval: 10; repeat: false; running: true; onTriggered: { flickable.opacity = 1; destroy(); } }',
                    flickable)
        })
    }

    // 在开始更新数据时，先将视图移出并设为透明
    function prepareForUpdate() {
        // 移出视图区域（移动到左上角之外很远的地方）
        flickable.contentX = -10000
        flickable.contentY = -10000
        // 设为透明
        flickable.opacity = 0
        // 强制刷新一次画布，清除可能的残留 (可选，但有助于确保干净)
        connectionCanvas.requestPaint()
    }

    function centerOnCurrentNode() {
        const targetContentX = flickable.centerX - flickable.width / 2
        const targetContentY = flickable.centerY - flickable.height / 2

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
                                        font.pixelSize: 13
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
        // 在开始任何数据处理前，先准备视图
        prepareForUpdate()

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
            calculateNodePositions()
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

    // --- 动画行为 ---
    Behavior on opacity {
        NumberAnimation {
            duration: 300
        } // 300ms 淡入淡出动画
    }

    Component.onCompleted: {
        // 首次加载数据
        initData()
        // 注意：首次加载时，initData 结束后会调用 calculateNodePositions，
        // 进而调用 centerOnCurrentNode 并触发淡入。
    }
}
