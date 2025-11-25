import QtQuick
import QtQuick.Window
import QtQml
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

    function setItemHeight(h) {}

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

    function insertItem(strTime, type, strText, curIndex) {
        view.model.insert(curIndex, {
                              "time": strTime,
                              "type": type,
                              "dototext": strText
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
            height: listCol.implicitHeight + 0
            color: ListView.isCurrentItem ? "lightblue" : getColor()

            border.width: isDark ? 0 : 1
            border.color: "lightgray" //"lightsteelblue"

            radius: 0

            RowLayout {

                id: idlistElemnet

                width: parent.width
                spacing: 2
                Layout.fillWidth: true

                ColumnLayout {
                    id: listCol
                    height: parent.height
                    width: parent.width
                    spacing: 2
                    Layout.fillWidth: true
                    anchors.leftMargin: 0
                    anchors.rightMargin: 0

                    Text {
                        id: item0

                        width: parent.width
                        Layout.preferredWidth: listItem.width
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: TextArea.WordWrap
                        font.bold: true
                        text: text0
                        color: listItem.ListView.isCurrentItem ? "black" : getFontColor()

                        leftPadding: 5
                        rightPadding: 5
                    }

                    Text {
                        id: item1
                        Layout.preferredWidth: listItem.width

                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter

                        width: parent.width
                        wrapMode: TextArea.WrapAnywhere
                        color: listItem.ListView.isCurrentItem ? "black" : getFontColor()
                        font.bold: false
                        text: text1

                        leftPadding: 5
                        rightPadding: 5

                        visible: false
                    }

                    Text {
                        id: item2
                        anchors.rightMargin: 0
                        Layout.preferredWidth: listItem.width
                        Layout.alignment: Qt.AlignHCenter

                        horizontalAlignment: Text.AlignLeft
                        width: parent.width
                        wrapMode: TextArea.WrapAnywhere
                        font.bold: false
                        text: text2
                        color: listItem.ListView.isCurrentItem ? "black" : getFontColor()

                        leftPadding: 5
                        rightPadding: 5

                        visible: item2.text.length ? true : false
                    }

                    Text {
                        id: item3
                        anchors.rightMargin: 0
                        width: parent.width
                        wrapMode: Text.WordWrap
                        elide: Text.ElideRight
                        //Layout.maximumWidth: listItem.width
                        Layout.preferredWidth: listItem.width
                        font.bold: false
                        text: text3
                        color: listItem.ListView.isCurrentItem ? "black" : getFontColor()

                        leftPadding: 5
                        rightPadding: 5

                        visible: item3.text.length ? true : false
                    }

                    Rectangle {
                        id: progressBar
                        width: parent.width - 10 // 减去左右内边距
                        height: 15
                        Layout.preferredWidth: parent.width - 10
                        Layout.alignment: Qt.AlignHCenter
                        radius: 4
                        color: isDark ? "#2D3A48" : "#F0F0F0"

                        visible: item1.text.trim() !== "" && !isNaN(
                                     parseFloat(item1.text.match(
                                                    /\d+(\.\d+)?/)))

                        // 进度填充层
                        Rectangle {
                            width: progressBar.width * Math.min(
                                       1,
                                       Math.max(0,
                                                parseFloat(item1.text.match(
                                                               /\d+(\.\d+)?/)[0]
                                                           || 0) / 100))
                            height: parent.height
                            radius: 3
                            color: isDark ? "#4A90E2" : "#64B5F6" // 亮模式用更柔和的浅蓝
                        }

                        // 百分比文本叠加
                        Text {
                            anchors.centerIn: parent
                            text: item1.text.match(
                                      /\d+(\.\d+)?/)[0] ? (parseFloat(
                                                               item1.text.match(
                                                                   /\d+(\.\d+)?/)[0]).toFixed(
                                                               2) + "%") : ""

                            color: isDark ? "white" : "#444444" // 亮模式改为深灰色
                            font.pixelSize: 12
                            font.bold: true
                        }
                    }

                    Rectangle {
                        width: view.width
                        height: 5 // 空白高度
                        color: "transparent"
                    }
                }
            }

            MouseArea {

                property point clickPos: "0,0"

                anchors.fill: parent
                onPressed: function (mouse) {
                    clickPos = Qt.point(mouse.x, mouse.y)
                }
                onReleased: function (mouse) {
                    var delta = Qt.point(mouse.x - clickPos.x,
                                         mouse.y - clickPos.y)
                    console.debug("delta.x: " + delta.x)
                }

                onClicked: {

                    view.currentIndex = index //实现item切换
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
        }
        delegate: dragDelegate

        spacing: 4
        cacheBuffer: 50

        // 滚动条
        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
            width: 8
        }
    }
}
