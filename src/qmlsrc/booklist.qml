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
        console.log("count=" + itemCount)
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
            return "#EEEEEE"
        else
            return "black"
    }

    function getFontColorGray() {

        if (isDark)
            return "lightgray"
        else
            return "gray"
    }

    Component {
        id: mComponent

        Rectangle {
            id: listItem
            width: ListView.view.width

            height: colLayout.implicitHeight + 0
            color: ListView.isCurrentItem ? "lightblue" : getColor()

            border.width: isDark ? 0 : 1
            border.color: "lightgray" //"lightsteelblue"

            radius: 0

            function getListPic() {
                if (item3.text === "txt")
                    return "/res/txt.svg"

                if (item3.text === "epub")
                    return "/res/epub.svg"

                if (item3.text === "pdf")
                    return "/res/pdf.svg"

                if (item3.text === "none")
                    return "/res/none.svg"
            }

            function getListColor() {
                if (item3.text === "txt")
                    return "#1E90FF"

                if (item3.text === "epub")
                    return "#00CD66"

                if (item3.text === "pdf")
                    return "red"

                if (item3.text === "none")
                    return "#FFD700"
            }

            RowLayout {

                id: idlistElemnet

                width: parent.width
                spacing: 2
                Layout.fillWidth: true

                Rectangle {
                    id: mySpace
                    width: 2
                }

                Rectangle {
                    id: myRect
                    height: 10
                    width: 10
                    x: 0
                    y: parent.height / 2 - width / 2
                    radius: 5
                    anchors.leftMargin: 1
                    color: listItem.getListColor()
                    visible: true
                    Text {
                        anchors.centerIn: parent
                    }
                }

                Image {
                    id: myimg
                    width: 32
                    height: parent.height - 2
                    fillMode: Image.NoOption
                    horizontalAlignment: Image.AlignHCenter
                    verticalAlignment: Image.AlignVCenter
                    visible: false
                    smooth: true
                    sourceSize.height: 32
                    sourceSize.width: 32
                    source: listItem.getListPic()
                }

                ColumnLayout {
                    id: colLayout
                    height: parent.height
                    width: parent.width
                    spacing: 2
                    Layout.fillWidth: true
                    anchors.leftMargin: 0
                    anchors.rightMargin: 0

                    Text {
                        id: item0

                        width: parent.width
                        Layout.preferredWidth: listItem.width - myRect.width - mySpace.width - 2
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
                        Layout.preferredWidth: listItem.width - myRect.width - mySpace.width - 2

                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter

                        width: parent.width
                        wrapMode: TextArea.WrapAnywhere
                        color: listItem.ListView.isCurrentItem ? "#4F4F4F" : getFontColorGray()
                        font.bold: false
                        font.pointSize: fontSize * 0.85
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
                        color: listItem.ListView.isCurrentItem ? "black" : getFontColor()
                        horizontalAlignment: Text.AlignLeft
                        width: parent.width
                        wrapMode: TextArea.WordWrap
                        font.bold: false
                        text: text2

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

                        visible: false // item3.text.length ? true : false
                    }
                }
            }

            MouseArea {

                property point clickPos: "0,0"

                anchors.fill: parent
                onPressed: function (mouse) {
                    clickPos = Qt.point(mouse.x, mouse.y)
                }
                onReleased: {

                }

                onClicked: {

                    view.currentIndex = index
                }

                onPressAndHold: {

                }

                onDoubleClicked: {
                    m_Reader.openBookListItem()
                }
            }

            Rectangle {
                color: "#AAAAAA"
                height: 0
                width: parent.width
                anchors.bottom: parent.bottom
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


            /*ListElement {
                text0: '<span style="background-color: #ff6600;">Hello</span>'
                text1: "123456  <b>Hello</b> <i>World!</i>  123456"
                text2: '123456 <font color="red"><b>TEST</b></font>  123456'
                text3: "str3 1234567890 1234567890  1234567890 1234567890"
                myh: 0
            }*/
        }
        delegate: mComponent

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
