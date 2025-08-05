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

            color: index % 2 === 0 ? "#f0f0f0" : "#e0e0e0"
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

                            font.bold: false
                            text: text1

                            leftPadding: 5
                            rightPadding: 5

                            visible: item1.text.length ? true : false
                        }
                    }

                    Rectangle {
                        width: parent.width
                        height: item2.contentHeight
                        color: "lightgray"
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
                            color: "red"

                            leftPadding: 5
                            rightPadding: 5

                            visible: item2.text.length ? true : false
                        }
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

                        leftPadding: 5
                        rightPadding: 5

                        visible: item3.text.length ? true : false
                    }

                    Rectangle {
                        width: parent.width
                        height: item4.contentHeight
                        color: "lightgray"
                        Text {
                            id: item4
                            anchors.rightMargin: 0
                            width: parent.width
                            wrapMode: Text.WrapAnywhere
                            elide: Text.ElideRight

                            Layout.preferredWidth: listItem.width
                            font.bold: false
                            text: text4

                            color: "blue"

                            leftPadding: 5
                            rightPadding: 5

                            visible: item4.text.length ? true : false
                        }
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

                        leftPadding: 5
                        rightPadding: 5

                        visible: false
                    }

                    Button {
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

        // 核心配置：绑定滚动状态
        property bool isScrolling: false
        onMovementStarted: isScrolling = true
        onMovementEnded: isScrolling = false

        ScrollBar.vertical: ScrollBar {
            id: vbar
            policy: ScrollBar.AsNeeded
            interactive: false // 关键！禁止拖动操作
            width: 8

            // 动态显隐控制
            visible: opacity > 0
            opacity: view.isScrolling ? 1 : 0
            Behavior on opacity {
                NumberAnimation {
                    duration: 300
                }
            }

            // 极简样式
            contentItem: Rectangle {
                color: isDark ? "#3498db" : "#606060"
                opacity: vbar.active ? (isDark ? 0.8 : 0.7) : 0
                Behavior on opacity {
                    NumberAnimation {
                        duration: 200 // 更流畅的动画
                        easing.type: Easing.OutQuad
                    }
                }
                radius: 3
            }
            background: null // 彻底消除背景容器
        }
    }
}
