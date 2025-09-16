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

    property int iconW: 18
    property int rowSpace: 3
    property int i: 0
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

        if (itemIndex < 0 || itemIndex >= view.count)
            return "" // ✅ 边界检查
        var data = view.model.get(itemIndex)
        if (!data)
            return ""
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

    function insertItem(text0, text1, text2, text3, curIndex) {
        view.model.insert(curIndex, {
                              "text0": text0,
                              "text1": text1,
                              "text2": text2,
                              "text3": text3
                          })
    }

    function delItem(currentIndex) {
        view.model.remove(currentIndex)
    }

    function modifyItem(currentIndex, strDate, strSteps, strKM) {

        view.model.setProperty(currentIndex, "text0", strDate)
        view.model.setProperty(currentIndex, "text1", strSteps)
        view.model.setProperty(currentIndex, "text2", strKM)
    }

    function modifyItemTime(currentIndex, strTime) {

        view.model.setProperty(currentIndex, "time", strTime)
    }

    function modifyItemType(currentIndex, type) {

        view.model.setProperty(currentIndex, "type", type)
    }

    function modifyItemText0(currentIndex, strText) {
        view.model.setProperty(currentIndex, "text0", strText)
    }

    function modifyItemText2(currentIndex, strText) {
        view.model.setProperty(currentIndex, "text2", strText)
    }

    function getColor() {
        var strColor

        if (isDark)
            strColor = "#333333"
        else
            strColor = "#ffffff"

        return strColor
    }

    function getFontColor() {
        if (isDark) {
            return "white"
        } else
            return "black"
    }

    function setScrollBarPos(pos) {
        view.ScrollBar.vertical.position = 1.0 - view.ScrollBar.vertical.size
    }

    Component {
        id: dragDelegate

        Rectangle {
            id: listItem
            width: ListView.view.width
            height: colLayout.implicitHeight + 0

            color: getText1(index) >= nStepsThreshold ? "#FFC1C1" : getColor()

            border.width: isDark ? 0 : 1
            border.color: "lightgray"

            radius: 0

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

                    RowLayout {

                        id: row0
                        Layout.fillWidth: true // 确保行占据全部宽度

                        Image {
                            id: item0Img

                            width: iconW
                            height: item0.contentHeight
                            fillMode: Image.NoOption
                            horizontalAlignment: Image.AlignHCenter
                            verticalAlignment: Image.AlignVCenter

                            smooth: true
                            sourceSize.height: iconW
                            sourceSize.width: iconW
                            source: "/res/time.svg"

                            visible: false
                        }

                        Rectangle {
                            id: item0Background
                            Layout.preferredWidth: listItem.width
                            Layout.preferredHeight: item0.implicitHeight + 10
                            color: isDark ? "#424242" : "#DCDCDC"
                            radius: 0

                            Text {
                                id: item0
                                anchors.fill: parent
                                anchors.margins: 5
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                wrapMode: Text.WordWrap
                                font.bold: true
                                text: text0
                                color: isDark ? "white" : "black"
                            }
                        }
                    }

                    RowLayout {

                        id: row1
                        Layout.margins: rowSpace

                        Image {
                            id: item1Img

                            width: iconW
                            height: item0.contentHeight
                            fillMode: Image.NoOption
                            horizontalAlignment: Image.AlignHCenter
                            verticalAlignment: Image.AlignVCenter

                            smooth: true
                            sourceSize.height: item1.contentHeight - 3
                            sourceSize.width: item1.contentHeight - 3
                            source: "/res/s0.svg"

                            visible: false
                        }

                        Text {
                            id: item1
                            Layout.preferredWidth: listItem.width

                            Layout.alignment: Qt.AlignHCenter
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter

                            width: parent.width
                            wrapMode: TextArea.WordWrap
                            // color: listItem.ListView.isCurrentItem ? "black" : (getText1(index) >= nStepsThreshold ? "black" : getFontColor())
                            color: getText1(
                                       index) >= nStepsThreshold ? "black" : getFontColor()
                            font.bold: false
                            text: text1

                            leftPadding: 5
                            rightPadding: 5

                            visible: item1.text.length ? true : false
                        }
                    }

                    RowLayout {

                        id: row2
                        Layout.margins: rowSpace

                        Image {
                            id: item2Img

                            width: iconW
                            height: item0.contentHeight
                            fillMode: Image.NoOption
                            horizontalAlignment: Image.AlignHCenter
                            verticalAlignment: Image.AlignVCenter

                            smooth: true
                            sourceSize.height: item1.contentHeight - 3
                            sourceSize.width: item1.contentHeight - 3
                            source: "/res/s1.svg"

                            visible: false
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

                            color: getText1(
                                       index) >= nStepsThreshold ? "black" : getFontColor()
                            leftPadding: 5
                            rightPadding: 5

                            visible: item2.text.length ? true : false
                        }
                    }

                    RowLayout {

                        id: row3
                        Layout.margins: rowSpace

                        Image {
                            id: item3Img

                            width: iconW
                            height: item0.contentHeight
                            fillMode: Image.NoOption
                            horizontalAlignment: Image.AlignHCenter
                            verticalAlignment: Image.AlignVCenter

                            smooth: true
                            sourceSize.height: item1.contentHeight - 3
                            sourceSize.width: item1.contentHeight - 3
                            source: "/res/s2.svg"

                            visible: false
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

                            color: getText1(
                                       index) >= nStepsThreshold ? "black" : getFontColor()
                            leftPadding: 5
                            rightPadding: 5

                            visible: item3.text.length ? true : false
                        }
                    }
                }
            }

            MouseArea {

                onClicked: {

                    view.currentIndex = index //实现item切换
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

        model: ListModel {
            id: listmain

            // debug


            /*ListElement {
                text0: '<span style="background-color: #ff6600;">Hello</span>'
                text1: "123456  <b>Hello</b> <i>World!</i>  123456"
                text2: '123456 <font color="red"><b>TEST</b></font>  123456'
                text3: "str3 1234567890 1234567890  1234567890 1234567890"
                myh: 0
            }*/
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
