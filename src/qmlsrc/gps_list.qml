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
        view.positionViewAtIndex(index, Tumbler.Center)
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

        //var data = view.model.get(itemIndex)
        //return data.text0
        var existingItem = view.model.get(itemIndex)
        return existingItem.text0
    }

    function getText1(itemIndex) {

        //var data = view.model.get(itemIndex)
        //return data.text1
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
                              const x = index * segmentWidth
                              ctx.fillRect(x, 0, segmentWidth, canvasHeight)
                          })
    }

    function drawSpeedSpectrum(ctx, speedData, canvasWidth, canvasHeight) {
        if (speedData.length < 2) {
            // 数据不足时，用浅灰色占位（不绘制背景，仅标记区域）
            ctx.fillStyle = "rgba(150, 150, 150, 0.3)"
            ctx.fillRect(0, 0, canvasWidth, canvasHeight)
            return
        }

        // 1. 清除画布（不绘制额外背景，直接使用父容器背景）
        ctx.clearRect(0, 0, canvasWidth, canvasHeight)

        // 2. 计算速度范围（个人基准）
        const vMin = Math.min(...speedData)
        const vMax = Math.max(...speedData)
        const vRange = vMax - vMin
        const isUniform = vRange <= 0.1

        // 3. 计算每个点的坐标
        const pointCount = speedData.length
        const points = []
        const segmentWidth = canvasWidth / (pointCount - 1)

        speedData.forEach((speed, index) => {
                              const ratio = isUniform ? 0.5 : (speed - vMin) / vRange
                              const x = index * segmentWidth
                              const y = canvasHeight - (ratio * canvasHeight)
                              // 速度越高，位置越靠上
                              points.push({
                                              "x": x,
                                              "y": y,
                                              "ratio": ratio
                                          })
                          })

        // 4. 绘制平滑山丘轮廓（二次贝塞尔曲线）
        ctx.beginPath()
        ctx.moveTo(0, canvasHeight) // 起点：左下角
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
        ctx.lineTo(canvasWidth, canvasHeight) // 闭合到右下角
        ctx.closePath()

        // 5. 蓝→红渐变填充（增强饱和度，确保无背景时清晰）
        const gradient = ctx.createLinearGradient(0, 0, canvasWidth, 0)
        points.forEach((p, i) => {
                           const pos = i / (points.length - 1)
                           const hue = 240 - (p.ratio * 240)
                           // 240°蓝 → 0°红
                           const rgb = hsvToRgb(hue, 0.85, 0.85)
                           // 轻微提高饱和度，增强无背景时的辨识度
                           gradient.addColorStop(
                               pos, `rgb(${rgb.r}, ${rgb.g}, ${rgb.b})`)
                       })
        ctx.fillStyle = gradient
        ctx.fill()

        // 6. 边缘线（半透明，适配不同背景）
        ctx.strokeStyle = "rgba(255, 255, 255, 0.6)" // 白色半透，在亮色/暗色背景上均有一定对比度
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

    function addItem(t0, t1, t2, t3, height) {
        view.model.append({
                              "text0": t0,
                              "text1": t1,
                              "text2": t2,
                              "text3": t3,
                              "myh": height
                          })
    }

    function insertItem(curIndex, t0, t1, t2, t3, t4, t5, t6, speedData) {
        var speedArray = []
        // （原有转换逻辑不变，确保speedArray是纯数字数组）
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

        console.log("insert阶段转换后的数组:", speedArray)

        // 关键：将数组转为JSON字符串存储
        var speedJson = JSON.stringify(speedArray)

        view.model.insert(curIndex, {
                              "text0": t0,
                              "text1": t1,
                              "text2": t2,
                              "text3": t3,
                              "text4": t4,
                              "text5": t5,
                              "text6": t6,
                              "speedData": speedJson // 存储JSON字符串
                          })

        // 验证模型是否正确保存
        var insertedItem = view.model.get(curIndex)
        console.log("模型中存储的JSON:", insertedItem.speedData)
    }

    function updateItem(curIndex, t0, t1, t2, t3, t4, t5, t6, height) {
        // 仅当索引有效（存在该项）时才更新，避免误插入破坏历史数据
        if (curIndex >= 0 && curIndex < view.model.count) {
            var existingItem = view.model.get(curIndex)
            // 只更新字段，不改变模型结构，保护历史数据
            existingItem.text0 = t0
            existingItem.text1 = t1
            existingItem.text2 = t2
            existingItem.text3 = t3
            existingItem.text4 = t4
            existingItem.text5 = t5
            existingItem.text6 = t6
            existingItem.myh = height
            // 通知模型更新该索引（确保UI刷新）
            view.model.set(curIndex, existingItem)
        } else {
            // 索引无效时，可选择不操作（保护历史数据）或打印警告
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
        var strColor

        if (isDark)
            strColor = "#455364"
        else
            strColor = "#ffffff"

        return strColor
    }

    function getFontColor() {

        if (isDark)
            return "white"
        else
            return "black"
    }

    Component {
        id: dragDelegate

        Rectangle {
            id: listItem
            width: ListView.view.width // 仅依赖ListView宽度，无闭环
            height: colLayout.implicitHeight + 15 // 高度由子元素内容决定

            //color: index % 2 === 0 ? "#f0f0f0" : "#e0e0e0"
            color: isDark ? "#333" : "#DDD"
            border.color: "#ccc"
            border.width: 1
            radius: 3

            // 选中状态红色竖条（带动画效果）
            Rectangle {
                width: listItem.ListView.isCurrentItem ? 4 : 0 // 选中时宽度3，未选中0
                height: parent.height
                color: isDark ? "#BBBBBB" : "#666666"
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                z: 10

                // 动画核心：宽度+透明度联动过渡
                Behavior on width {
                    NumberAnimation {
                        duration: 300 // 动画时长（毫秒）
                        easing.type: Easing.OutQuart // 缓动效果，先快后慢
                    }
                }

                opacity: listItem.ListView.isCurrentItem ? 1 : 0 // 选中时不透明，未选中透明
                Behavior on opacity {
                    NumberAnimation {
                        duration: 250 // 透明度动画稍快于宽度
                        easing.type: Easing.OutQuart
                    }
                }
            }

            // 新增：点击条目切换选中状态
            MouseArea {
                property point clickPos: "0,0"
                anchors.fill: parent
                onPressed: function (mouse) {
                    clickPos = Qt.point(mouse.x, mouse.y)
                }
                onReleased: function (mouse) {
                    var delta = Qt.point(mouse.x - clickPos.x,
                                         mouse.y - clickPos.y)
                }
                onClicked: view.currentIndex = index // 核心：点击切换选中项
                onDoubleClicked: {

                    /* 双击逻辑 */ }
            }

            ColumnLayout {
                id: colLayout
                anchors.fill: parent // 直接填充listItem
                spacing: 2
                anchors.leftMargin: 10 // 新增：左内边距10
                anchors.rightMargin: 10 // 新增：右内边距10
                anchors.topMargin: 5
                anchors.bottomMargin: 5

                Rectangle {
                    Layout.fillWidth: true // 自动填充colLayout宽度，适配内边距
                    height: item0.contentHeight
                    color: item0.text.indexOf(
                               qsTr("Cycling")) ? (item0.text.indexOf(
                                                       qsTr("Hiking")) ? (item0.text.indexOf(qsTr("Running")) ? strTitleColor : "#87CEFA") : "#98FB98") : "#FFA500"

                    Text {
                        id: item0

                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: TextArea.NoWrap
                        font.bold: true
                        text: text0

                        color: isDark ? "#333" : "#333"

                        leftPadding: 5
                        rightPadding: 5
                    }
                }

                // 速度色带（放在item0的Rectangle后面）
                Canvas {
                    id: speedRibbon
                    Layout.fillWidth: true // 与item0同宽
                    height: 20 // 固定高度
                    Layout.topMargin: 2 // 与item0保持2px间距

                    onPaint: {
                        const ctx = getContext("2d")
                        ctx.resetTransform()
                        ctx.clearRect(0, 0, width, height)

                        // 从模型获取JSON字符串
                        const speedJson = model.speedData || "[]"
                        let speedArray = []

                        // 解析JSON为数组
                        try {
                            speedArray = JSON.parse(speedJson)
                            // 确保是数组且元素为数字
                            if (!Array.isArray(speedArray)) {
                                speedArray = []
                            } else {
                                speedArray = speedArray.filter(
                                            s => typeof s === "number"
                                            && !isNaN(s))
                            }
                        } catch (e) {
                            console.error("解析speedData失败:", e)
                            speedArray = []
                        }

                        console.log("最终绘制数组:", speedArray)

                        // 绘制逻辑
                        if (speedArray.length < 2) {
                            ctx.fillStyle = "#AAAAAA"
                            ctx.fillRect(0, 0, width, height)
                            return
                        }

                        //drawColorRibbon(ctx, speedArray, width, height)
                        drawSpeedSpectrum(ctx, speedArray, width, height)
                    }
                }

                RowLayout {
                    id: weatherTextContainer
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter

                    // 显示SVG图标的组件
                    Image {
                        id: weatherIcon
                        source: item6.text
                        sourceSize.height: 42
                        sourceSize.width: 42
                        fillMode: Image.PreserveAspectFit
                        Layout.alignment: Qt.AlignVCenter
                        Layout.leftMargin: 5
                        visible: item6.text.length ? true : false
                    }

                    Text {
                        id: item1
                        Layout.fillWidth: true

                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter

                        width: parent.width
                        wrapMode: TextArea.WordWrap

                        font.pointSize: item0.font.pointSize - 1
                        font.bold: false
                        color: isDark ? "#BBB" : "#555"
                        text: text1

                        leftPadding: 5
                        rightPadding: 5

                        visible: item1.text.length ? true : false
                    }
                }

                Text {
                    id: item2
                    anchors.rightMargin: 0
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter

                    horizontalAlignment: Text.AlignLeft
                    width: parent.width
                    wrapMode: TextArea.WordWrap
                    font.bold: false
                    text: text2
                    color: isDark ? "#FF6666" : "red"

                    leftPadding: 5
                    rightPadding: 5

                    visible: item2.text.length ? true : false
                }

                Text {
                    id: item3
                    anchors.rightMargin: 0
                    Layout.fillWidth: true
                    wrapMode: Text.WrapAnywhere
                    elide: Text.ElideRight

                    Layout.preferredWidth: listItem.width
                    font.bold: false
                    text: text3

                    color: isDark ? "#DDD" : "#333"

                    leftPadding: 5
                    rightPadding: 5

                    visible: item3.text.length ? true : false
                }

                Text {
                    id: item4
                    anchors.rightMargin: 0
                    Layout.fillWidth: true
                    wrapMode: Text.WrapAnywhere
                    elide: Text.ElideRight

                    Layout.preferredWidth: listItem.width
                    font.bold: false
                    text: text4

                    color: isDark ? "#1E90FF" : "blue"

                    leftPadding: 5
                    rightPadding: 5

                    visible: item4.text.length ? true : false
                }

                Text {
                    id: item5
                    anchors.rightMargin: 0
                    Layout.fillWidth: true
                    wrapMode: Text.WrapAnywhere
                    elide: Text.ElideRight

                    Layout.preferredWidth: listItem.width
                    font.bold: false
                    text: text5

                    color: isDark ? "#DDD" : "#333"

                    leftPadding: 5
                    rightPadding: 5

                    visible: item5.text.length ? true : false
                }

                Text {
                    id: item6
                    anchors.rightMargin: 0
                    Layout.fillWidth: true
                    wrapMode: Text.WrapAnywhere
                    elide: Text.ElideRight

                    Layout.preferredWidth: listItem.width
                    font.bold: false
                    text: text6

                    color: isDark ? "#DDD" : "#333"

                    leftPadding: 5
                    rightPadding: 5

                    visible: false
                }

                RowLayout {
                    id: buttonLayout
                    Layout.alignment: Qt.AlignLeft // 左对齐
                    spacing: 8 // 按钮间距
                    Layout.leftMargin: 5 // 左边缘留白
                    Layout.bottomMargin: 8 // 底部留白，避免压边界

                    // 轨迹图标按钮
                    Button {
                        id: btnViewGpsTrack
                        width: 40 // 固定宽度
                        height: 40 // 固定高度
                        enabled: true
                        visible: listItem.ListView.isCurrentItem // 仅选中条目显示

                        // 图标根据深色模式切换
                        contentItem: Image {
                            source: isDark ? "/res/track_l.svg" : "/res/track.svg"
                            sourceSize: Qt.size(24, 24) // 图标大小
                            fillMode: Image.PreserveAspectFit
                        }

                        // 简化背景，保持点击反馈
                        background: Rectangle {
                            color: btnViewGpsTrack.down ? "#4CAF50" : (isDark ? "#444" : "#CCC")
                            radius: 4
                            border.color: "#4CAF50"
                            border.width: 1
                        }

                        onClicked: {
                            strGpsTime = item0.text + "-=-" + item1.text + "-=-"
                                    + item2.text + "-=-" + item4.text
                            m_Steps.getGpsTrack()
                        }
                    }

                    // 路线图标按钮
                    Button {
                        id: btnRoute
                        width: 40 // 固定宽度
                        height: 40 // 固定高度
                        enabled: true
                        visible: isShowRoute
                                 && listItem.ListView.isCurrentItem // 仅选中条目显示

                        // 图标根据深色模式切换
                        contentItem: Image {
                            source: isDark ? "/res/route_l.svg" : "/res/route.svg"
                            sourceSize: Qt.size(24, 24) // 图标大小
                            fillMode: Image.PreserveAspectFit
                        }

                        // 与轨迹按钮保持一致的背景样式
                        background: Rectangle {
                            color: btnRoute.down ? "#4CAF50" : (isDark ? "#444" : "#CCC")
                            radius: 4
                            border.color: "#4CAF50"
                            border.width: 1
                        }

                        onClicked: {
                            strGpsTime = item0.text + "-=-" + item1.text + "-=-"
                                    + item2.text + "-=-" + item4.text
                            m_Steps.getRouteList(strGpsTime)
                        }
                    }
                }
            }
        }
    }

    ListView {
        id: view

        anchors {
            fill: parent
            margins: 4
        }

        model: ListModel {
            id: listmain

            // debug


            /*  ListElement {
                text0: "1"
                text1: "2"
                text2: "3"
                text3: "4"
                text4: "5"
                text5: "6"
                myh: 0
            }*/
        }
        delegate: dragDelegate

        spacing: 5
        cacheBuffer: 50

        // 滚动条
        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
            width: 8
        }
    }

    // 弹出窗口：显示路由数据列表
    Dialog {
        id: routeDialog
        objectName: "routeDialog"
        title: "" // qsTr("Route Data")
        width: root.width * 1.0
        height: root.height * 1.0
        modal: true // 模态窗口（禁止背景操作）
        visible: false // 默认隐藏
        x: (root.width - width) / 2
        y: (root.height - height) / 2

        // 设置对话框背景颜色
        background: Rectangle {
            color: isDark ? "#333333" : "#F5F5F5" // 深色模式/浅色模式背景色
            radius: 0 // 圆角半径
            border.color: isDark ? "#555555" : "#CCCCCC" // 边框颜色
            border.width: 1 // 边框宽度
        }

        // 路由数据模型（接收C++传递的数据）
        ListModel {
            id: routeModel
        }

        // 窗口内容：滚动列表
        ListView {
            anchors.fill: parent
            model: routeModel
            spacing: 5
            cacheBuffer: 50
            Layout.alignment: Qt.AlignHCenter

            boundsBehavior: Flickable.StopAtBounds // 禁止滚动到边界外的弹性效果

            // 列表条目样式（每个条目分三行）
            delegate: Rectangle {
                width: routeDialog.width - 20
                height: colLayout.implicitHeight + 20

                property bool hasSamePrev: {
                    if (index <= 0)
                        return false // 第一个条目没有上一个
                    const prevItem = routeModel.get(index - 1)
                    return prevItem.latLon === latLon
                            && prevItem.address === address
                }
                property bool hasSameNext: {
                    if (index >= routeModel.count - 1)
                        return false // 最后一个条目没有下一个
                    const nextItem = routeModel.get(index + 1)
                    return nextItem.latLon === latLon
                            && nextItem.address === address
                }

                // 核心：根据连续相同组判断底色
                color: {
                    if (hasSamePrev || hasSameNext) {
                        // 休息时段的浅绿色（适配明暗模式）
                        return isDark ? "#2E7D32" : "#E8F5E9"
                    } else {
                        // 默认底色
                        return isDark ? "#333" : "#EEE"
                    }
                }

                radius: 5
                border.color: isDark ? "#555" : "#CCC"
                border.width: 1
                anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined

                ColumnLayout {
                    id: colLayout
                    anchors.fill: parent
                    anchors.margins: 5
                    spacing: 5

                    // 第一行：时间
                    Text {
                        Layout.fillWidth: true
                        text: time
                        font.bold: true
                        color: isDark ? "#FFF" : "#000"
                        horizontalAlignment: Text.AlignLeft
                    }

                    // 第二行：纬度 + 经度
                    Text {
                        Layout.fillWidth: true
                        text: latLon
                        color: isDark ? "#DDD" : "#333"
                        horizontalAlignment: Text.AlignLeft
                    }

                    // 第三行：地址
                    Text {
                        Layout.fillWidth: true
                        text: address
                        color: isDark ? "#BBB" : "#666"
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.WordWrap // 地址过长时换行
                    }
                }
            }

            // 垂直滚动条
            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
                width: 8
            }
        }

        // 窗口底部按钮（清空数据 + 关闭）
        footer: Item {
            width: parent.width
            implicitHeight: footerLayout.implicitHeight + 20

            RowLayout {
                id: footerLayout
                spacing: 10
                Layout.fillWidth: true // 新增：占满footer宽度（关键修复）
                anchors.centerIn: parent
                anchors.margins: 10

                Button {
                    text: qsTr("Clear")
                    visible: false
                    onClicked: routeModel.clear() // 清空列表数据
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
                    onClicked: routeDialog.visible = false // 关闭窗口
                    background: Rectangle {
                        //color: isDark ? "#4CAF50" : "#8BC34A"
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

        // 暴露给C++的方法：添加路由条目到模型
        function addRouteItem(timeStr, latLonStr, addressStr) {
            routeModel.append({
                                  "time": timeStr,
                                  "latLon": latLonStr,
                                  "address": addressStr
                              })
        }

        // 暴露给C++的方法：清空模型（可选）
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
