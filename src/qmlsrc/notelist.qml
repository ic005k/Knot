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

    property int i: 0
    property int itemCount: 0
    property bool isHighPriority: false
    property int btnW: 32
    property int btnH: 32
    property int iconSize: 30

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
        view.positionViewAtIndex(currentIndex, ListView.Center)
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
        const text0 = noteModel.getText0(itemIndex)
        console.log("[QML] 从C++接口获取text0：", text0)
        return text0
    }

    function getText00(itemIndex) {

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
        const text3 = noteModel.getText3(itemIndex)
        console.log("[QML] 从C++接口获取text3：", text3)
        return text3
    }

    function getText33(itemIndex) {

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
        noteModel.addItem(t0, t1, t2, t3, height) // 调用C++模型的addItem
    }

    function addItem_old(t0, t1, t2, t3, height) {

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

    function delItem_old(currentIndex) {
        view.model.remove(currentIndex)
    }

    function delItem(currentIndex) {
        noteModel.removeItem(currentIndex) // 调用C++接口
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
            strColor = "#333333"
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

    Component {
        id: dragDelegate

        Rectangle {
            id: listItem
            width: ListView.view.width

            // height: colLayout.implicitHeight + 0
            height: colLayout.implicitHeight + (ListView.isCurrentItem ? itemButtons.height : 0)
            color: ListView.isCurrentItem ? "lightblue" : getColor()

            border.width: isDark ? 0 : 1
            border.color: "lightgray"

            radius: 2

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

                onClicked: function (mouse) {

                    view.currentIndex = index //实现item切换

                    for (i = 0; i < view.count; i++) {

                        //view.model.setProperty(i, "text2", "")
                    }

                    // view.model.setProperty(index, "text2", "ShowRect")
                    m_NotesList.clickNoteList()
                }

                onPressAndHold: {

                }

                onDoubleClicked: {

                    mw_one.on_btnRename_clicked()
                }
            }

            //RowLayout {
            ColumnLayout {

                id: idlistElemnet

                width: parent.width
                spacing: 2
                Layout.fillWidth: true

                Rectangle {
                    height: parent.height - 2
                    width: 6
                    radius: 2
                    anchors.leftMargin: 1
                    color: "red"
                    visible: false
                    Text {
                        anchors.centerIn: parent
                    }
                }

                ColumnLayout {
                    id: colLayout
                    height: parent.height
                    width: parent.width
                    spacing: 2
                    Layout.fillWidth: true
                    anchors.leftMargin: 0
                    anchors.rightMargin: 0

                    Rectangle {
                        width: view.width
                        height: 5 // 空白高度
                        color: "transparent"
                    }

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
                        wrapMode: TextArea.WordWrap
                        color: listItem.ListView.isCurrentItem ? "black" : getFontColor()
                        font.bold: false
                        font.italic: true
                        font.pointSize: noteTimeFontSize
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
                        font.pointSize: noteTimeFontSize
                        text: text2
                        color: listItem.ListView.isCurrentItem ? "black" : getFontColor()

                        leftPadding: 5
                        rightPadding: 5

                        visible: false
                    }

                    Text {
                        id: item3
                        anchors.rightMargin: 0
                        width: parent.width
                        wrapMode: Text.WordWrap
                        elide: Text.ElideRight
                        Layout.preferredWidth: listItem.width
                        font.bold: false
                        font.pointSize: noteTimeFontSize
                        text: text3
                        color: listItem.ListView.isCurrentItem ? "black" : getFontColor()

                        leftPadding: 5
                        rightPadding: 5

                        visible: false
                    }

                    Rectangle {
                        width: view.width
                        height: 5 // 空白高度
                        color: "transparent"
                    }
                }

                Item {
                    // 容器尺寸，可根据实际需求调整
                    id: itemButtons
                    width: parent.width
                    height: 40 // 略大于按钮高度，留出边距
                    visible: listItem.ListView.isCurrentItem ? true : false

                    // 使用Row布局实现水平排列
                    Row {
                        id: buttonRow
                        anchors.left: parent.left // 左对齐
                        anchors.top: parent.top
                        spacing: 10 // 按钮之间的间距

                        // 第一个工具按钮
                        ToolButton {
                            id: btn1
                            icon.source: "qrc:/res/view.svg"
                            text: "view"

                            width: btnW // 固定宽度
                            height: btnH // 固定高度
                            icon.width: iconSize - 13
                            icon.height: iconSize - 13
                            onClicked: {
                                console.log("View按钮被点击")
                                mw_one.on_btnOpenNote_clicked()
                            }
                        }

                        // 第二个工具按钮
                        ToolButton {
                            id: btn2
                            icon.source: "qrc:/res/edit.svg"

                            width: btnW
                            height: btnH
                            icon.width: iconSize
                            icon.height: iconSize
                            onClicked: {
                                console.log("编辑按钮被点击")
                                m_NotesList.qmlOpenEdit()
                            }
                        }

                        // 第三个工具按钮
                        ToolButton {
                            id: btn3
                            icon.source: "qrc:/res/link.svg"

                            width: btnW
                            height: btnH
                            icon.width: iconSize
                            icon.height: iconSize
                            onClicked: {
                                console.log("链接按钮被点击")
                                m_NotesList.on_actionCopyNoteLink()
                            }
                        }

                        // 第四个工具按钮
                        ToolButton {
                            id: btn4
                            icon.source: "qrc:/res/graph.svg"

                            width: btnW
                            height: btnH
                            icon.width: iconSize - 13
                            icon.height: iconSize - 13
                            onClicked: {
                                console.log("图谱按钮被点击")
                                m_NotesList.on_actionRelationshipGraph()
                            }
                        }
                    }
                }
            }
        }
    }

    ListView {
        id: view
        spacing: 4
        boundsBehavior: Flickable.StopAtBounds // 禁止滚动到边界外的弹性效果
        anchors {
            fill: parent
            margins: 4
        }

        model: noteModel


        /* model: ListModel {
            id: listmain

            // debug


            ListElement {
                text0: '<span style="background-color: #ff6600;">Hello</span>'
                text1: "123456  <b>Hello</b> <i>World!</i>  123456"
                text2: '123456 <font color="red"><b>TEST</b></font>  123456'
                text3: "str3 1234567890 1234567890  1234567890 1234567890"
                myh: 0
            }
        }*/
        delegate: dragDelegate

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
