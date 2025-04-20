import QtQuick 2.15
import QtQuick.Window 2.15
import QtQml 2.3
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.1

Rectangle {
    id: root

    width: 500
    height: 400

    color: isDark ? "#19232D" : "white"

    property int itemCount: 0
    property bool isHighPriority: false

    function isAlarm(index) {

        return isHighPriority
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

    function getTime(itemIndex) {
        var data = view.model.get(itemIndex)
        return data.time
    }

    function getTodoText(itemIndex) {
        var data = view.model.get(itemIndex)
        return data.dototext
    }

    function getType(itemIndex) {
        var data = view.model.get(itemIndex)
        return data.type
    }

    function addItem(strTime, type, strText, height) {
        view.model.append({
                              "time": strTime,
                              "type": type,
                              "dototext": strText,
                              "itemheight": height
                          })
    }

    function insertRecycle(strTime, type, strText, height, curIndex) {
        view.model.insert(curIndex, {
                              "time": strTime,
                              "type": type,
                              "dototext": strText,
                              "itemheight": height
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
            height: getItemHeight()
            color: ListView.isCurrentItem ? "lightblue" : getColor()
            border.width: isDark ? 0 : 1
            border.color: "lightgray"

            radius: 0

            function getItemHeight() {
                var item0H
                var item1H
                var item2H
                var item3H

                if (text1.visible == false)
                    item0H = 0
                else
                    item0H = text1.contentHeight

                if (text2.visible == false)
                    item1H = 0
                else
                    item1H = text2.contentHeight

                if (text3.visible == false)
                    item2H = 0
                else
                    item2H = text3.contentHeight

                if (text4.visible == false)
                    item3H = 0
                else
                    item3H = text4.contentHeight

                return item0H + item1H + item2H + item3H + text2.height + 5
            }

            RowLayout {

                id: idlistElemnet
                height: parent.height
                width: parent.width
                spacing: 2
                Layout.fillWidth: true

                Rectangle {
                    height: listItem.getItemHeight() - 6
                    width: 6
                    radius: 2
                    anchors.leftMargin: 1
                    color: getListEleHeadColor(type)
                    visible: false
                    Text {
                        anchors.centerIn: parent
                    }
                }

                ColumnLayout {
                    id: m_col
                    height: parent.height
                    width: parent.width - 6
                    spacing: 2
                    Layout.fillWidth: true
                    anchors.leftMargin: 0
                    anchors.rightMargin: 0

                    Text {
                        id: text1
                        color: listItem.ListView.isCurrentItem ? "black" : getFontColor()
                        font.pointSize: FontSize - 2
                        font.bold: true
                        width: parent.width
                        Layout.preferredWidth: listItem.width
                        wrapMode: Text.Wrap
                        text: time

                        leftPadding: 10
                        rightPadding: 10
                    }

                    Text {
                        id: text2
                        visible: false
                        width: parent.width
                        wrapMode: Text.Wrap
                        text: type
                    }

                    Text {
                        id: text3
                        font.pointSize: FontSize
                        width: parent.width
                        Layout.preferredWidth: listItem.width + 8
                        wrapMode: Text.Wrap
                        color: listItem.ListView.isCurrentItem ? "black" : getFontColor()
                        text: dototext

                        leftPadding: 10
                        rightPadding: 10
                    }

                    Text {
                        id: text4
                        visible: false
                        width: parent.width
                        wrapMode: TextArea.WordWrap
                        text: itemheight
                    }
                }
            }

            MouseArea {

                property point clickPos: "0,0"

                anchors.fill: parent

                onClicked: {

                    view.currentIndex = index //实现item切换
                    console.log("ItemHeight=" + text1.contentHeight + text3.contentHeight)
                }
            }
        }
    }

    ListView {
        id: view
        boundsBehavior: Flickable.StopAtBounds // 禁止滚动到边界外的弹性效果

        anchors {
            fill: parent
            margins: 4
        }

        model: TodoModel {}
        delegate: dragDelegate

        spacing: 4
        cacheBuffer: 50

        // 核心配置：绑定滚动状态
        property bool isScrolling: false
        onMovementStarted: isScrolling = true
        onMovementEnded: isScrolling = false

        ScrollBar.vertical: ScrollBar {
            id: vbar
            policy: ScrollBar.AsNeeded
            interactive: false // 关键！禁止拖动操作
            width: 4

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
                opacity: scrollBar.active ? (isDark ? 0.8 : 0.7) : 0
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

    function getListEleHeadColor(ntype) {
        switch (ntype) {
        case 0:
            return "lightgray"
        case 1:
            return "red"
        case 2:
            return "orange"
        case 3:
            return "lightblue"
        default:
            return "black"
        }
    }
}
