import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Fusion

Rectangle {
    id: root

    color: isDark ? "#19232D" : "white"

    property int itemCount: 0
    property bool isHighPriority: false
    property string strGpsTime: ""
    property string strTitleColor: "lightgray"
    property bool isShowRoute: true // 补充缺失的属性

    // 新增：获取设备像素比（安卓/iOS关键）
    property real pixelRatio: Screen.pixelRatio > 0 ? Screen.pixelRatio : 1
    property real baseFontSize: (Qt.platform.os
                                 === "android") ? (20 * pixelRatio) : (10 * pixelRatio)

    function showRouteDialog() {
        routeDialog.visible = true
    }

    function closeRouteDialog() {
        routeDialog.visible = false
    }

    function setItemHeight(h) {}

    function getGpsList() {
        return strGpsTime
    }

    function gotoEnd() {
        view.positionViewAtEnd()
    }

    function gotoBeginning() {
        view.positionViewAtBeginning()
    }

    function gotoIndex(index) {
        view.positionViewAtIndex(index, ListView.Center) // 改为ListView.Center
    }

    function setHighPriority(isFalse) {
        isHighPriority = isFalse
    }

    function setCurrentItem(currentIndex) {
        view.currentIndex = currentIndex
    }

    function getCurrentIndex() {
        return view.currentIndex
    }

    function getItemCount() {
        itemCount = view.count
        return itemCount
    }

    function getItemText(itemIndex) {
        var data = view.model.get(itemIndex)
        return data.time + "|=|" + data.dototext
    }

    function getText0(itemIndex) {
        var existingItem = view.model.get(itemIndex)
        return existingItem.text0
    }

    function getText1(itemIndex) {
        var existingItem = view.model.get(itemIndex)
        return existingItem.text1
    }

    function getText2(itemIndex) {
        var data = view.model.get(itemIndex)
        return data.text2
    }

    function getText3(itemIndex) {
        var data = view.model.get(itemIndex)
        return data.text3
    }

    function getTop(itemIndex) {
        var data = view.model.get(itemIndex)
        return data.text_top
    }

    function getType(itemIndex) {
        var data = view.model.get(itemIndex)
        return data.type
    }

    // 绘制绿-黄-橙-红色带（相对映射算法）
    function drawColorRibbon(ctx, speedData, canvasWidth, canvasHeight) {
        const vMin = Math.min(...speedData)
        const vMax = Math.max(...speedData)
        const vRange = vMax - vMin

        if (vRange <= 0) {
            ctx.fillStyle = "#FFFF00"
            ctx.fillRect(0, 0, canvasWidth, canvasHeight)
            return
        }

        const pointCount = speedData.length
        const segmentWidth = canvasWidth / (pointCount - 1)

        speedData.forEach((speed, index) => {
                              const ratio = (speed - vMin) / vRange
                              const hue = 120 - (ratio * 120)
                              const rgb = hsvToRgb(hue, 0.8, 0.9)
                              ctx.fillStyle = `rgb(${rgb.r}, ${rgb.g}, ${rgb.b})`
                              ctx.fillRect(x, 0, segmentWidth, canvasHeight)
                          })
    }

    function drawSpeedSpectrum(ctx, speedData, canvasWidth, canvasHeight) {
        if (speedData.length < 2) {
            ctx.fillStyle = "rgba(150, 150, 150, 0.3)"
            ctx.fillRect(0, 0, canvasWidth, canvasHeight)
            return
        }

        ctx.clearRect(0, 0, canvasWidth, canvasHeight)

        const vMin = Math.min(...speedData)
        const vMax = Math.max(...speedData)
        const vRange = vMax - vMin
        const isUniform = vRange <= 0.1

        const pointCount = speedData.length
        const points = []
        const segmentWidth = canvasWidth / (pointCount - 1)

        speedData.forEach((speed, index) => {
                              const ratio = isUniform ? 0.5 : (speed - vMin) / vRange
                              const x = index * segmentWidth
                              const y = canvasHeight - (ratio * canvasHeight)
                              points.push({
                                              "x": x,
                                              "y": y,
                                              "ratio": ratio
                                          })
                          })

        ctx.beginPath()
        ctx.moveTo(0, canvasHeight)
        for (var i = 0; i < points.length; i++) {
            const p = points[i]
            if (i === 0) {
                ctx.lineTo(p.x, p.y)
            } else {
                const prev = points[i - 1]
                const controlX = (prev.x + p.x) / 2
                ctx.quadraticCurveTo(controlX, (prev.y + p.y) / 2, p.x, p.y)
            }
        }
        ctx.lineTo(canvasWidth, canvasHeight)
        ctx.closePath()

        const gradient = ctx.createLinearGradient(0, 0, canvasWidth, 0)
        points.forEach((p, i) => {
                           const pos = i / (points.length - 1)
                           const hue = 240 - (p.ratio * 240)
                           const rgb = hsvToRgb(hue, 0.85, 0.85)
                           gradient.addColorStop(
                               pos, `rgb(${rgb.r}, ${rgb.g}, ${rgb.b})`)
                       })
        ctx.fillStyle = gradient
        ctx.fill()

        ctx.strokeStyle = "rgba(255, 255, 255, 0.6)"
        ctx.lineWidth = 1.2
        ctx.stroke()
    }

    // HSV转RGB工具函数
    function hsvToRgb(h, s, v) {
        let r, g, b
        const i = Math.floor(h / 60)
        const f = h / 60 - i
        const p = v * (1 - s)
        const q = v * (1 - f * s)
        const t = v * (1 - (1 - f) * s)

        switch (i % 6) {
        case 0:
            r = v
            g = t
            b = p
            break
        case 1:
            r = q
            g = v
            b = p
            break
        case 2:
            r = p
            g = v
            b = t
            break
        case 3:
            r = p
            g = q
            b = v
            break
        case 4:
            r = t
            g = p
            b = v
            break
        case 5:
            r = v
            g = p
            b = q
            break
        }

        return {
            "r": Math.round(r * 255),
            "g": Math.round(g * 255),
            "b": Math.round(b * 255)
        }
    }

    function drawAltitudeCurve(ctx, altitudeData, canvasWidth, canvasHeight) {
        if (altitudeData.length < 2) {
            ctx.fillStyle = "rgba(150, 150, 150, 0.3)"
            ctx.fillRect(0, 0, canvasWidth, canvasHeight)
            return
        }

        ctx.clearRect(0, 0, canvasWidth, canvasHeight)

        const altMin = Math.min(...altitudeData)
        const altMax = Math.max(...altitudeData)
        const altRange = altMax - altMin
        const isUniform = altRange <= 0.1

        const pointCount = altitudeData.length
        const points = []
        const segmentWidth = canvasWidth / (pointCount - 1)

        altitudeData.forEach((altitude, index) => {
                                 const ratio = isUniform ? 0.5 : (altitude - altMin) / altRange
                                 const x = index * segmentWidth
                                 const y = canvasHeight - (ratio * canvasHeight)
                                 points.push({
                                                 "x": x,
                                                 "y": y
                                             })
                             })

        // 填充区域（单色）
        ctx.beginPath()
        ctx.moveTo(0, canvasHeight)
        for (var i = 0; i < points.length; i++) {
            const p = points[i]
            if (i === 0) {
                ctx.lineTo(p.x, p.y)
            } else {
                const prev = points[i - 1]
                const controlX = (prev.x + p.x) / 2
                ctx.quadraticCurveTo(controlX, (prev.y + p.y) / 2, p.x, p.y)
            }
        }
        ctx.lineTo(canvasWidth, canvasHeight)
        ctx.closePath()

        // 单色填充（适配深浅色模式）
        ctx.fillStyle = isDark ? "rgba(76, 175, 255, 0.5)" : "rgba(33, 150, 243, 0.4)"
        ctx.fill()

        // 绘制轮廓线
        ctx.beginPath()
        for (var i = 0; i < points.length; i++) {
            const p = points[i]
            if (i === 0) {
                ctx.moveTo(p.x, p.y)
            } else {
                const prev = points[i - 1]
                const controlX = (prev.x + p.x) / 2
                ctx.quadraticCurveTo(controlX, (prev.y + p.y) / 2, p.x, p.y)
            }
        }
        ctx.strokeStyle = isDark ? "#2196F3" : "#1976D2"
        ctx.lineWidth = 1.5
        ctx.stroke()
    }

    function addItem(t0, t1, t2, t3, height) {
        view.model.append({
                              "text0": t0,
                              "text1": t1,
                              "text2": t2,
                              "text3": t3,
                              "myh": height
                          })
    }

    function insertItem(curIndex, t0, t1, t2, t3, t4, t5, t6, speedData, altitudeData) {
        // 处理速度数据（原有逻辑保持不变）
        var speedArray = []
        if (speedData && speedData.hasOwnProperty("count")
                && typeof speedData.get === "function") {
            for (var i = 0; i < speedData.count; i++) {
                var item = speedData.get(i)
                var value = typeof item
                        === "object" ? (item.value !== undefined ? item.value : item) : item
                value = Number(value)
                if (!isNaN(value)) {
                    speedArray.push(value)
                }
            }
        } else if (Array.isArray(speedData)) {
            for (var j = 0; j < speedData.length; j++) {
                var val = Number(speedData[j])
                if (!isNaN(val)) {
                    speedArray.push(val)
                }
            }
        }

        // 新增：处理海拔数据（完全复用速度数据的格式逻辑）
        var altitudeArray = []
        if (altitudeData && altitudeData.hasOwnProperty("count")
                && typeof altitudeData.get === "function") {
            for (var k = 0; k < altitudeData.count; k++) {
                var altItem = altitudeData.get(k)
                var altValue = typeof altItem
                        === "object" ? (altItem.value
                                        !== undefined ? altItem.value : altItem) : altItem
                altValue = Number(altValue)
                if (!isNaN(altValue)) {
                    altitudeArray.push(altValue)
                }
            }
        } else if (Array.isArray(altitudeData)) {
            for (var l = 0; l < altitudeData.length; l++) {
                var altVal = Number(altitudeData[l])
                if (!isNaN(altVal)) {
                    altitudeArray.push(altVal)
                }
            }
        }

        console.log("insert阶段转换后的速度数组:", speedArray)
        console.log("insert阶段转换后的海拔数组:", altitudeArray) // 新增日志

        var speedJson = JSON.stringify(speedArray)
        var altitudeJson = JSON.stringify(altitudeArray) // 新增：海拔数据序列化

        // 模型插入时新增altitudeData字段
        view.model.insert(curIndex, {
                              "text0": t0,
                              "text1": t1,
                              "text2": t2,
                              "text3": t3,
                              "text4": t4,
                              "text5": t5,
                              "text6": t6,
                              "speedData": speedJson,
                              "altitudeData": altitudeJson // 新增：海拔数据字段
                          })

        var insertedItem = view.model.get(curIndex)
        console.log("模型中存储的速度JSON:", insertedItem.speedData)
        console.log("模型中存储的海拔JSON:", insertedItem.altitudeData) // 新增日志
    }

    function updateItem(curIndex, t0, t1, t2, t3, t4, t5, t6, height) {
        if (curIndex >= 0 && curIndex < view.model.count) {
            var existingItem = view.model.get(curIndex)
            existingItem.text0 = t0
            existingItem.text1 = t1
            existingItem.text2 = t2
            existingItem.text3 = t3
            existingItem.text4 = t4
            existingItem.text5 = t5
            existingItem.text6 = t6
            existingItem.myh = height
            view.model.set(curIndex, existingItem)
        } else {
            console.log("updateItem: 索引" + curIndex + "无效，不更新")
        }
    }

    function delItem(currentIndex) {
        view.model.remove(currentIndex)
    }

    function modifyItem(currentIndex, strTime, strText) {
        view.model.setProperty(currentIndex, "time", strTime)
        view.model.setProperty(currentIndex, "dototext", strText)
    }

    function modifyItemTime(currentIndex, strTime) {
        view.model.setProperty(currentIndex, "time", strTime)
    }

    function modifyItemType(currentIndex, type) {
        view.model.setProperty(currentIndex, "type", type)
    }

    function modifyItemText(currentIndex, strText) {
        view.model.setProperty(currentIndex, "dototext", strText)
    }

    function getColor() {
        var strColor = isDark ? "#455364" : "#ffffff"
        return strColor
    }

    function getFontColor() {
        return isDark ? "white" : "black"
    }

    Component {
        id: dragDelegate

        Rectangle {
            id: listItem
            width: ListView.view.width // 宽度=ListView可视宽度（一屏宽）
            height: ListView.view.height // 高度=ListView可视高度（一屏高）
            color: isDark ? "#333" : "#DDD"
            border.color: "#ccc"
            border.width: 1
            radius: 3

            // 选中状态红色竖条
            Rectangle {
                width: listItem.ListView.isCurrentItem ? 4 : 0
                height: parent.height
                color: isDark ? "#BBBBBB" : "#666666"
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                z: 10
                visible: false

                Behavior on width {
                    NumberAnimation {
                        duration: 300
                        easing.type: Easing.OutQuart
                    }
                }

                opacity: listItem.ListView.isCurrentItem ? 1 : 0
                Behavior on opacity {
                    NumberAnimation {
                        duration: 250
                        easing.type: Easing.OutQuart
                    }
                }
            }

            // 点击切换选中
            MouseArea {
                property point clickPos: "0,0"
                anchors.fill: parent
                // 使用箭头函数显式接收mouse参数
                onPressed: mouse => {
                               clickPos = Qt.point(mouse.x, mouse.y)
                           }
                onReleased: mouse => {
                                var delta = Qt.point(mouse.x - clickPos.x,
                                                     mouse.y - clickPos.y)
                            }
                onClicked: view.currentIndex = index
                onDoubleClicked: {

                }
            }

            // 全屏布局容器
            ColumnLayout {
                id: colLayout
                anchors.fill: parent
                anchors.margins: 10
                spacing: 8

                // 标题行（带运动类型标记）
                Rectangle {
                    id: m_caption
                    //Layout.fillWidth: true
                    width: listItem.width - 20
                    Layout.preferredHeight: 40
                    color: isDark ? "#333" : "#DDD"
                    border.color: isDark ? "#444" : "#CCC"
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 5
                        spacing: 8

                        Rectangle {
                            id: sportTypeBlock
                            width: 24
                            height: 24
                            color: {
                                if (item0.text.indexOf(qsTr("Cycling")) !== -1)
                                    isDark ? "#5ABD5E" : "#4CAF50"
                                else if (item0.text.indexOf(
                                             qsTr("Hiking")) !== -1)
                                    isDark ? "#FFAB2C" : "#FF9800"
                                else if (item0.text.indexOf(
                                             qsTr("Running")) !== -1)
                                    isDark ? "#B746C9" : "#9C27B0"
                                else
                                    "transparent"
                            }
                            radius: 3
                        }

                        Text {
                            id: item0
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                            wrapMode: Text.NoWrap
                            font.bold: true
                            font.pointSize: baseFontSize * 1.0 // 标题字体放大1.2倍
                            text: text0
                            color: isDark ? "#FFFFFF" : "#333333"
                        }
                    }
                }

                // 速度图谱标签（新增）
                Text {
                    id: speedLabel
                    Layout.fillWidth: true
                    text: qsTr("Speed Curve")
                    font.bold: true
                    font.pointSize: baseFontSize * 0.8 // 基于基准字体缩放，适配移动端
                    color: isDark ? "#FFFFFF" : "#333333"
                    horizontalAlignment: Text.AlignLeft // 左对齐，与内容呼应
                    Layout.bottomMargin: 4 // 与下方Canvas保持间距
                }

                // 速度图谱（放大高度）
                Canvas {
                    id: speedRibbon
                    // 与标题宽度一致
                    //Layout.fillWidth: true
                    width: m_caption.width

                    Layout.preferredHeight: 80 // 适配全屏高度，增大显示区域
                    Layout.topMargin: 4

                    onPaint: {
                        const ctx = getContext("2d")
                        ctx.resetTransform()
                        ctx.clearRect(0, 0, width, height)

                        const speedJson = model.speedData || "[]"
                        let speedArray = []
                        try {
                            speedArray = JSON.parse(speedJson)
                            speedArray = speedArray.filter(
                                        s => typeof s === "number" && !isNaN(s))
                        } catch (e) {
                            console.error("解析speedData失败:", e)
                            speedArray = []
                        }

                        if (speedArray.length < 2) {
                            ctx.fillStyle = "#AAAAAA"
                            ctx.fillRect(0, 0, width, height)
                            return
                        }

                        drawSpeedSpectrum(ctx, speedArray, width, height)
                    }
                }

                // 天气+文本行
                RowLayout {
                    id: weatherTextContainer
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40

                    Image {
                        id: weatherIcon
                        source: item6.text
                        sourceSize: Qt.size(42, 42)
                        fillMode: Image.PreserveAspectFit
                        Layout.alignment: Qt.AlignVCenter
                        visible: item6.text.length > 0
                    }

                    Text {
                        id: item1
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: Text.WordWrap
                        font.pointSize: baseFontSize
                        color: isDark ? "#BBB" : "#555"
                        text: text1
                        visible: text1.length > 0
                    }
                }

                Rectangle {
                    width: view.width
                    height: 5 // 空白高度
                    color: "transparent"
                }

                // 内容文本区（自适应剩余空间）
                ColumnLayout {
                    Layout.fillWidth: true
                    //Layout.fillHeight: true // 填充剩余高度
                    spacing: 4

                    Text {
                        id: item2
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.WordWrap
                        font.pointSize: baseFontSize
                        color: isDark ? "#FF6666" : "red"
                        text: text2
                        visible: text2.length > 0
                    }

                    Rectangle {
                        width: view.width
                        height: 5 // 空白高度
                        color: "transparent"
                    }

                    Text {
                        id: item3
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.WordWrap
                        font.pointSize: baseFontSize
                        color: isDark ? "#DDD" : "#333"
                        text: text3
                        visible: text3.length > 0
                    }

                    Rectangle {
                        width: view.width
                        height: 5 // 空白高度
                        color: "transparent"
                    }

                    Text {
                        id: item4
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.WordWrap
                        font.pointSize: baseFontSize
                        color: isDark ? "#1E90FF" : "blue"
                        text: text4
                        visible: text4.length > 0
                    }

                    Rectangle {
                        width: view.width
                        height: 5 // 空白高度
                        color: "transparent"
                    }

                    Text {
                        id: item5
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.WordWrap
                        font.pointSize: baseFontSize
                        color: isDark ? "#DDD" : "#333"
                        text: text5
                        visible: text5.length > 0
                    }

                    // 海拔图谱标签（新增）
                    Text {
                        id: altiLabel
                        Layout.fillWidth: true
                        text: qsTr("Terrain Curve")
                        font.bold: true
                        font.pointSize: baseFontSize * 0.8 // 基于基准字体缩放，适配移动端
                        color: isDark ? "#FFFFFF" : "#333333"
                        horizontalAlignment: Text.AlignLeft // 左对齐，与内容呼应
                        Layout.bottomMargin: 4 // 与下方Canvas保持间距
                    }

                    // 海拔图谱Canvas（新增）
                    Canvas {
                        id: altitudeCanvas
                        width: m_caption.width
                        Layout.preferredHeight: 60
                        Layout.topMargin: 4

                        onPaint: {
                            const ctx = getContext("2d")
                            ctx.resetTransform()
                            ctx.clearRect(0, 0, width, height)

                            const altitudeJson = model.altitudeData || "[]"
                            let altitudeArray = []
                            try {
                                altitudeArray = JSON.parse(altitudeJson)
                                altitudeArray = altitudeArray.filter(
                                            a => typeof a === "number"
                                            && !isNaN(a))
                            } catch (e) {
                                console.error("解析altitudeData失败:", e)
                                altitudeArray = []
                            }

                            drawAltitudeCurve(ctx, altitudeArray, width, height)
                        }
                    }

                    Rectangle {
                        width: view.width
                        height: 5 // 空白高度
                        color: "transparent"
                    }

                    Text {
                        id: item6
                        text: text6
                        visible: false
                    }
                }

                // 按钮区（底部固定）
                RowLayout {
                    id: buttonLayout
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignBottom
                    spacing: 8
                    Layout.topMargin: 10

                    Button {
                        id: btnViewGpsTrack
                        width: 40
                        height: 40

                        //visible: listItem.ListView.isCurrentItem
                        contentItem: Image {
                            source: isDark ? "/res/track_l.svg" : "/res/track.svg"
                            sourceSize: Qt.size(24, 24)
                            fillMode: Image.PreserveAspectFit
                        }

                        background: Rectangle {
                            color: btnViewGpsTrack.down ? "#4CAF50" : (isDark ? "#444" : "#CCC")
                            radius: 4
                            border.color: "#4CAF50"
                            border.width: 1
                        }

                        onClicked: {
                            strGpsTime = item0.text + "-=-" + item1.text + "-=-"
                                    + item2.text + "-=-" + item4.text
                            m_Steps.getGpsTrack() // 假设外部对象已定义
                        }
                    }

                    Button {
                        id: btnRoute
                        width: 40
                        height: 40
                        visible: isShowRoute && listItem.ListView.isCurrentItem

                        contentItem: Image {
                            source: isDark ? "/res/route_l.svg" : "/res/route.svg"
                            sourceSize: Qt.size(24, 24)
                            fillMode: Image.PreserveAspectFit
                        }

                        background: Rectangle {
                            color: btnRoute.down ? "#4CAF50" : (isDark ? "#444" : "#CCC")
                            radius: 4
                            border.color: "#4CAF50"
                            border.width: 1
                        }

                        onClicked: {
                            strGpsTime = item0.text + "-=-" + item1.text + "-=-"
                                    + item2.text + "-=-" + item4.text
                            m_Steps.getRouteList(strGpsTime) // 假设外部对象已定义
                        }
                    }
                }
            }
        }
    }

    // 水平滚动ListView（核心修改）
    ListView {
        id: view
        anchors {
            fill: parent
            margins: 4
        }
        model: ListModel {
            id: listmain
        }
        delegate: dragDelegate
        spacing: 5
        cacheBuffer: 100 // 增大缓存，优化滑动体验
        orientation: ListView.Horizontal // 水平滚动
        snapMode: ListView.SnapOneItem // 滚动对齐到单个条目（一屏一个）
        highlightRangeMode: ListView.StrictlyEnforceRange // 严格限制滚动范围
        currentIndex: 0 // 默认选中第一个条目

        // 水平滚动条
        ScrollBar.horizontal: ScrollBar {
            policy: ScrollBar.AsNeeded
            height: 8
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
        }
    }

    // 弹出窗口：显示路由数据列表
    Dialog {
        id: routeDialog
        objectName: "routeDialog"
        title: ""
        width: root.width * 1.0
        height: root.height * 1.0
        modal: true
        visible: false
        x: (root.width - width) / 2
        y: (root.height - height) / 2

        background: Rectangle {
            color: isDark ? "#333333" : "#F5F5F5"
            radius: 0
            border.color: isDark ? "#555555" : "#CCCCCC"
            border.width: 1
        }

        ListModel {
            id: routeModel
        }

        ListView {
            anchors.fill: parent
            model: routeModel
            spacing: 5
            cacheBuffer: 50
            boundsBehavior: Flickable.StopAtBounds

            delegate: Rectangle {
                width: routeDialog.width - 20
                height: colLayout.implicitHeight + 20

                property string currentLastPart: {
                    if (!address)
                        return ''
                    const parts = address.split('\n')
                    return parts[parts.length - 1] || ''
                }

                property bool hasSamePrev: {
                    if (index <= 0)
                        return false
                    const prevItem = routeModel.get(index - 1)
                    const prevParts = prevItem.address.split('\n')
                    const prevLastPart = prevParts[prevParts.length - 1] || ''
                    return prevLastPart === currentLastPart
                }

                property bool hasSameNext: {
                    if (index >= routeModel.count - 1)
                        return false
                    const nextItem = routeModel.get(index + 1)
                    const nextParts = nextItem.address.split('\n')
                    const nextLastPart = nextParts[nextParts.length - 1] || ''
                    return nextLastPart === currentLastPart
                }

                property bool isGroupFirst: !hasSamePrev
                property int sameGroupCount: {
                    if (!currentLastPart)
                        return 1
                    let count = 1
                    let prevIdx = index - 1
                    while (prevIdx >= 0) {
                        const prevItem = routeModel.get(prevIdx)
                        const prevParts = prevItem.address.split('\n')
                        const prevLast = (prevParts[prevParts.length - 1]
                                          || '').trim()
                        if (prevLast === currentLastPart) {
                            count++
                            prevIdx--
                        } else
                            break
                    }
                    let nextIdx = index + 1
                    while (nextIdx < routeModel.count) {
                        const nextItem = routeModel.get(nextIdx)
                        const nextParts = nextItem.address.split('\n')
                        const nextLast = (nextParts[nextParts.length - 1]
                                          || '').trim()
                        if (nextLast === currentLastPart) {
                            count++
                            nextIdx++
                        } else
                            break
                    }
                    return count
                }

                color: (hasSamePrev
                        || hasSameNext) ? (isDark ? "#2E7D32" : "#E8F5E9") : (isDark ? "#333" : "#EEE")
                radius: 5
                border.color: isDark ? "#555" : "#CCC"
                border.width: 1

                //anchors.horizontalCenter: parent.horizontalCenter
                ColumnLayout {
                    id: colLayout
                    anchors.fill: parent
                    anchors.margins: 5
                    spacing: 5

                    Rectangle {
                        visible: isGroupFirst && sameGroupCount > 1
                        Layout.alignment: Qt.AlignRight
                        Layout.rightMargin: 5
                        width: textItem.implicitWidth + 4
                        height: textItem.implicitHeight + 2
                        color: isDark ? "#1B5E20" : "#4CAF50"
                        radius: 3

                        Text {
                            id: textItem
                            text: qsTr("Total %1 items").arg(sameGroupCount)
                            color: "white"
                            font.pointSize: 12
                            horizontalAlignment: Text.AlignRight
                            leftPadding: 2
                            rightPadding: 2
                            topPadding: 1
                            bottomPadding: 1
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        text: time
                        font.bold: true
                        color: isDark ? "#FFF" : "#000"
                        horizontalAlignment: Text.AlignLeft
                    }

                    Text {
                        Layout.fillWidth: true
                        text: latLon
                        color: isDark ? "#DDD" : "#333"
                        horizontalAlignment: Text.AlignLeft
                    }

                    Text {
                        Layout.fillWidth: true
                        text: address
                        color: isDark ? "#BBB" : "#666"
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.WordWrap
                    }
                }
            }

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
                width: 8
            }
        }

        footer: Item {
            width: parent.width
            implicitHeight: footerLayout.implicitHeight + 20

            RowLayout {
                id: footerLayout
                spacing: 10
                Layout.fillWidth: true
                anchors.centerIn: parent
                anchors.margins: 10

                Button {
                    text: qsTr("Clear")
                    visible: false
                    onClicked: routeModel.clear()
                    background: Rectangle {
                        color: isDark ? "#F44336" : "#FF5722"
                        radius: 5
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                    }
                }

                Button {
                    id: btnClose
                    text: qsTr("Close")
                    onClicked: routeDialog.visible = false
                    background: Rectangle {
                        color: btnClose.down ? (isDark ? "#388E3C" : "#4CAF50") : (isDark ? "#4CAF50" : "#8BC34A")
                        radius: 5
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }

        function addRouteItem(timeStr, latLonStr, addressStr) {
            routeModel.append({
                                  "time": timeStr,
                                  "latLon": latLonStr,
                                  "address": addressStr
                              })
        }

        function clearRouteModel() {
            routeModel.clear()
        }

        function setVisible(value) {
            routeDialog.visible = value
        }

        function isVisible() {
            return routeDialog.visible
        }
    }
}
