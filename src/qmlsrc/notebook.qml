import QtQuick 2.15
import QtQuick.Window 2.15
import QtQml 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root

    width: 500
    height: 400

    color: isDark ? "#19232D" : "white"

    property int iconW: 20
    property int rowSpace: 3
    property int i: 0
    property int itemCount: 0
    property bool isHighPriority: false

    function setScrollBarPos(pos) {
        view.ScrollBar.vertical.position = 1.0 - view.ScrollBar.vertical.size
    }

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
        view.positionViewAtIndex(currentIndex, ListView.Beginning)
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

    function addItem(t0, t1, t2, t3, f_size) {
        view.model.append({
                              "text0": t0,
                              "text1": t1,
                              "text2": t2,
                              "text3": t3,
                              "font_size": f_size
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

    function modifyItemText1(currentIndex, strText) {
        view.model.setProperty(currentIndex, "text1", strText)
    }

    function modifyItemText2(currentIndex, strText) {
        view.model.setProperty(currentIndex, "text2", strText)
    }

    function modifyItemText3(currentIndex, strText) {
        view.model.setProperty(currentIndex, "text3", strText)
    }

    function setVPos(vpos) {
        vscrollbar.position = vpos
        console.log("qwNoteBook:set " + vpos)
    }

    function getVPos() {
        var vpos = vscrollbar.position
        console.log("qwNoteBook:get " + vpos)
        return vpos
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
            height: item0.contentHeight + item2.contentHeight + 16
            color: ListView.isCurrentItem ? "lightblue" : getColor()

            border.width: isDark ? 0 : 1
            border.color: "lightgray" //"lightsteelblue"

            radius: 0

            RowLayout {

                id: idlistElemnet
                height: parent.height
                width: parent.width
                spacing: 2
                Layout.fillWidth: true

                Rectangle {
                    height: parent.height - 2
                    width: 6
                    radius: 2
                    anchors.leftMargin: 1
                    color: "red"
                    visible: item2.text.length ? false : true
                    Text {
                        anchors.centerIn: parent
                    }
                }

                ColumnLayout {
                    id: idlistElemnet4
                    height: parent.height
                    width: parent.width
                    spacing: 2
                    Layout.fillWidth: true
                    anchors.leftMargin: 0
                    anchors.rightMargin: 0

                    RowLayout {

                        id: row0

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

                        Text {
                            id: item0

                            width: parent.width
                            Layout.preferredWidth: listItem.width - 0
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

                        visible: false // item1.text.length ? true : false
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

                        visible: false // item2.text.length ? true : false
                    }

                    RowLayout {

                        id: row3
                        Layout.margins: rowSpace
                        visible: item3.text.length ? true : false

                        Image {
                            id: item3Img

                            width: item3.contentHeight - 5
                            height: parent.height - 2
                            fillMode: Image.NoOption
                            horizontalAlignment: Image.AlignHCenter
                            verticalAlignment: Image.AlignVCenter

                            smooth: true
                            sourceSize.height: item3.contentHeight - 5
                            sourceSize.width: item3.contentHeight - 5
                            source: "/res/sum.png"

                            visible: item3.text.length ? true : false
                        }

                        Text {
                            id: item3
                            anchors.rightMargin: 0
                            width: parent.width
                            wrapMode: Text.WordWrap
                            elide: Text.ElideRight
                            Layout.alignment: Qt.AlignHCenter
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter

                            //Layout.maximumWidth: listItem.width
                            Layout.preferredWidth: listItem.width

                            font.bold: false
                            font.pointSize: font_size - 2
                            color: listItem.ListView.isCurrentItem ? "black" : getFontColor()
                            text: text3

                            leftPadding: 5
                            rightPadding: 5

                            visible: item3.text.length ? true : false
                        }
                    }
                }
            }

            MouseArea {

                property point clickPos: "0,0"

                anchors.fill: parent
                onPressed: {
                    clickPos = Qt.point(mouse.x, mouse.y)
                }
                onReleased: {
                    var delta = Qt.point(mouse.x - clickPos.x,
                                         mouse.y - clickPos.y)

                    if ((delta.x < 0) && (aBtnShow.running === false)
                            && (delBtn.width == 0)) {
                        aBtnShow.start()
                    } else if (aBtnHide.running === false
                               && (delBtn.width > 0)) {
                        aBtnHide.start()
                    }
                }

                onClicked: {

                    view.currentIndex = index //实现item切换


                    /*for (i = 0; i < view.count; i++) {
                        view.model.setProperty(i, "text2", "")
                    }
                    view.model.setProperty(index, "text2", "ShowRect")*/
                    m_NotesList.clickNoteBook()
                }

                onPressAndHold: {

                    //m_Method.showNoteBookMenu(mouse.x, mouse.y)
                }

                onDoubleClicked: {

                    //mw_one.reeditData()
                    //var data = view.model.get(view.currentIndex)
                    //console.log(data.text0 + "," + data.type + ", count=" + view.count)
                }
            }

            Rectangle {
                color: "#AAAAAA"
                height: 0
                width: parent.width
                anchors.bottom: parent.bottom
            }

            Rectangle {
                id: delBtn
                visible: false
                height: parent.height
                width: 0
                color: "#FF0000"

                anchors.right: parent.right
                anchors.rightMargin: -30
                radius: 0

                Text {
                    width: 56
                    anchors.centerIn: parent

                    text: qsTr("Done")
                    color: "#ffffff"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        m_Todo.addToRecycle()
                        view.model.remove(index)
                    }
                }
            }

            PropertyAnimation {
                id: aBtnShow
                target: delBtn
                property: "width"
                duration: 100
                from: 0
                to: 80
            }
            PropertyAnimation {
                id: aBtnHide
                target: delBtn
                property: "width"
                duration: 100
                from: 80
                to: 0
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

        // 自动隐藏触发器（关键）
        Timer {
            running: !view.isScrolling && vbar.opacity > 0
            interval: 800
            onTriggered: vbar.opacity = 0
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
