import QtQuick
import QtQuick.Window
import QtQml
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    width: 500
    height: 400

    color: isDark ? "#19232D" : "white"

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

    function modifyItemText0(currentIndex, strText) {
        view.model.setProperty(currentIndex, "text0", strText)
    }

    function modifyItemText2(currentIndex, strText) {
        view.model.setProperty(currentIndex, "text2", strText)
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

            width: item0.contentWidth + 32 < 60 ? 60 : item0.contentWidth + 32
            height: maintabHeight - 4
            color: ListView.isCurrentItem ? "lightblue" : getColor()

            border.width: isDark ? 0 : 1
            border.color: "lightgray" //"lightsteelblue"

            radius: 6

            function getItemHeight() {
                var item0H
                var item1H
                var item2H
                var item3H

                if (item0.text.length == 0)
                    item0H = 0
                else
                    item0H = item0.contentHeight

                if (item1.text.length == 0)
                    item1H = 0
                else
                    item1H = item1.contentHeight

                if (item2.text.length == 0)
                    item2H = 0
                else
                    item2H = item2.contentHeight

                if (item3.text.length == 0)
                    item3H = 0
                else
                    item3H = item3.contentHeight

                return item0H + item1H + item2H + item3H
            }

            RowLayout {

                id: myrow
                height: parent.height
                width: parent.width
                spacing: 2
                Layout.fillWidth: true

                Rectangle {
                    id: spaceRect
                    width: 3
                    height: 0
                }

                Rectangle {
                    id: flagRect
                    height: 12
                    width: 12
                    radius: 6
                    anchors.leftMargin: 0
                    color: listItem.ListView.isCurrentItem ? "red" : "gray"
                    visible: true
                    Text {
                        anchors.centerIn: parent
                    }
                }

                RowLayout {
                    id: colLayout
                    height: parent.height
                    width: parent.width
                    spacing: 2
                    Layout.fillWidth: true
                    anchors.leftMargin: 0
                    anchors.rightMargin: 0

                    Text {
                        id: item0

                        Layout.preferredWidth: listItem.width
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: TextArea.WordWrap
                        font.bold: false
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
                        wrapMode: TextArea.WordWrap
                        color: listItem.ListView.isCurrentItem ? "black" : getFontColor()
                        font.bold: false
                        text: text1

                        leftPadding: 5
                        rightPadding: 5

                        visible: item1.text.length ? true : false
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
                        color: listItem.ListView.isCurrentItem ? "black" : getFontColor()

                        leftPadding: 5
                        rightPadding: 5

                        visible: item2.text.length ? true : false
                    }

                    Text {
                        id: item3
                        anchors.rightMargin: 0
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
                }
            }

            MouseArea {

                anchors.fill: parent

                onClicked: {

                    view.currentIndex = index //实现item切换

                    mw_one.clickMainTab()
                }
            }

            SequentialAnimation on opacity {
                //应用于透明度上的序列动画
                running: true // isAniEffects
                loops: 1 //Animation.Infinite //无限循环
                NumberAnimation {
                    from: 0
                    to: 1
                    duration: 500
                } //淡出效果
                PauseAnimation {
                    //暂停400ms
                    duration: 0
                }
            }
        }
    }

    ListView {
        id: view

        orientation: ListView.Horizontal

        anchors {
            fill: parent
            margins: 4
        }

        model: ListModel {
            id: listmain

            // debug


            /*ListElement {
                text0: "Tab 1"
            }

            ListElement {
                text0: "Tab 2"
            }

            ListElement {
                text0: "Tab 3"
            }*/
        }
        delegate: dragDelegate

        spacing: 4
        cacheBuffer: 50

        ScrollBar.vertical: ScrollBar {
            width: 8
            policy: ScrollBar.AsNeeded
        }
    }

    function getListEleHeadColor(ntype) {
        switch (ntype) {
        case 0:
            return "lightgray"
        case 1:
            return "red"
        case 2:
            return "yellow"
        case 3:
            return "lightblue"
        default:
            return "black"
        }
    }
}
