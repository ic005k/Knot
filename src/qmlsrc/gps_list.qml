import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Fusion

Rectangle {
    id: root

    width: 500
    height: 400

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
        var data = view.model.get(itemIndex)
        return data.text0
    }

    function getText1(itemIndex) {
        var data = view.model.get(itemIndex)
        return data.text1
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

    function addItem(t0, t1, t2, t3, height) {
        view.model.append({
                              "text0": t0,
                              "text1": t1,
                              "text2": t2,
                              "text3": t3,
                              "myh": height
                          })
    }

    function insertItem(curIndex, t0, t1, t2, t3, t4, t5, t6, height) {

        view.model.insert(curIndex, {
                              "text0": t0,
                              "text1": t1,
                              "text2": t2,
                              "text3": t3,
                              "text4": t4,
                              "text5": t5,
                              "text6": t6,
                              "myh": height
                          })
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
            width: ListView.view.width

            height: colLayout.implicitHeight + 5

            //color: index % 2 === 0 ? "#f0f0f0" : "#e0e0e0"
            color: isDark ? "#333" : "#DDD"
            border.color: "#ccc"
            border.width: 1

            radius: 3

            RowLayout {

                id: idlistElemnet

                width: parent.width
                spacing: 2
                Layout.fillWidth: true

                ColumnLayout {
                    id: colLayout
                    height: parent.height
                    width: parent.width
                    spacing: 2
                    Layout.fillWidth: true
                    anchors.leftMargin: 0
                    anchors.rightMargin: 0

                    Rectangle {
                        width: parent.width
                        height: item0.contentHeight
                        color: item0.text.indexOf(
                                   qsTr("Cycling")) ? (item0.text.indexOf(
                                                           qsTr("Hiking")) ? (item0.text.indexOf(qsTr("Running")) ? strTitleColor : "#87CEFA") : "#98FB98") : "#FFA500"

                        Text {
                            id: item0

                            width: parent.width
                            Layout.preferredWidth: listItem.width
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

                    RowLayout {
                        id: weatherTextContainer
                        Layout.preferredWidth: listItem.width
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
                            Layout.preferredWidth: listItem.width

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
                        Layout.preferredWidth: listItem.width
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
                        width: parent.width
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
                        width: parent.width
                        wrapMode: Text.WrapAnywhere
                        elide: Text.ElideRight

                        Layout.preferredWidth: listItem.width
                        font.bold: false
                        text: text4

                        color: isDark ? "#6666FF" : "blue"

                        leftPadding: 5
                        rightPadding: 5

                        visible: item4.text.length ? true : false
                    }

                    Text {
                        id: item5
                        anchors.rightMargin: 0
                        width: parent.width
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
                        width: parent.width
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


                    /*Button {
                        id: btnViewGpsTrack
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("View GPS Track")
                        width: parent.width
                        height: 35
                        enabled: true

                        onClicked: {
                            strGpsTime = item0.text + "-=-" + item1.text + "-=-"
                                    + item2.text + "-=-" + item4.text

                            m_Steps.getGpsTrack()
                        }

                        // 设置按钮的背景颜色和边框
                        background: Rectangle {
                            width: parent.width
                            color: btnViewGpsTrack.down ? "#4CAF50" : "#8BC34A" // 按下时颜色变深
                            radius: 5 // 圆角
                            border.color: "#4CAF50" // 边框颜色
                            border.width: 2 // 边框宽度
                        }

                        // 设置按钮的文本样式
                        contentItem: Text {
                            text: btnViewGpsTrack.text
                            font.pixelSize: 14
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }*/

                    // 替换原单独的 btnViewGpsTrack 按钮代码，改为 RowLayout 包裹两个按钮
                    RowLayout {
                        Layout.alignment: Qt.AlignHCenter // 整体水平居中
                        width: parent.width // 占满父容器宽度
                        spacing: 10 // 两个按钮之间的间距（可调整）

                        // 原 View GPS Track 按钮（保留原有逻辑和样式）
                        Button {
                            id: btnViewGpsTrack
                            text: qsTr("View GPS Track")
                            Layout.fillWidth: true // 两个按钮平分宽度
                            height: 35
                            enabled: true

                            onClicked: {
                                strGpsTime = item0.text + "-=-" + item1.text
                                        + "-=-" + item2.text + "-=-" + item4.text
                                m_Steps.getGpsTrack()
                            }

                            background: Rectangle {
                                width: parent.width
                                color: btnViewGpsTrack.down ? "#4CAF50" : "#8BC34A"
                                radius: 5
                                border.color: "#4CAF50"
                                border.width: 2
                            }

                            contentItem: Text {
                                text: btnViewGpsTrack.text
                                font.pixelSize: 14
                                color: "white"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }

                        // 新增 Route 按钮（样式与原按钮一致）
                        Button {
                            id: btnRoute
                            text: qsTr("Route")
                            Layout.fillWidth: true // 两个按钮平分宽度
                            height: 35
                            enabled: true

                            onClicked: {

                                strGpsTime = item0.text + "-=-" + item1.text
                                        + "-=-" + item2.text + "-=-" + item4.text
                                m_Steps.getRouteList(strGpsTime)
                            }

                            // 样式与原按钮统一，保持 UI 一致性
                            background: Rectangle {
                                width: parent.width
                                color: btnRoute.down ? "#4CAF50" : "#8BC34A" // 同原按钮颜色
                                radius: 5
                                border.color: "#4CAF50"
                                border.width: 2
                            }

                            contentItem: Text {
                                text: btnRoute.text
                                font.pixelSize: 14
                                color: "white"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
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
        title: qsTr("Route Data")
        width: root.width * 0.8 // 宽度为父容器80%
        height: root.height * 0.7 // 高度为父容器70%
        modal: true // 模态窗口（禁止背景操作）
        visible: false // 默认隐藏
        x: (root.width - width) / 2 // 水平居中
        y: (root.height - height) / 2 // 垂直居中

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

            // 列表条目样式（每个条目分三行）
            delegate: Rectangle {
                width: parent.width
                height: 80 // 固定高度，适配三行文本
                color: isDark ? "#333" : "#EEE"
                radius: 5
                border.color: isDark ? "#555" : "#CCC"
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent

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
                        maximumLineCount: 2 // 最多显示2行，超出省略
                        elide: Text.ElideTail
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
        footer: RowLayout {
            spacing: 10
            Layout.alignment: Qt.AlignHCenter

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
                text: qsTr("Close")
                onClicked: routeDialog.visible = false // 关闭窗口
                background: Rectangle {
                    color: isDark ? "#4CAF50" : "#8BC34A"
                    radius: 5
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
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
    }
}
