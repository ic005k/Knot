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

    property int iconW: 16
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
        // view.positionViewAtIndex(currentIndex, ListView.Beginning)
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

    function setVPos(vpos) {

        vscrollbar.position = vpos
        console.log("qwMainDate:set " + vpos)
    }

    function getVPos() {
        var vpos = vscrollbar.position
        console.log("qwMainDate:get " + vpos)
        return vpos
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

            height: listCol.implicitHeight + 0

            //选中颜色设置 #94caf7
            color: ListView.isCurrentItem ? "lightblue" : getColor()

            border.width: isDark ? 0 : 1
            border.color: "lightgray" //"lightsteelblue"

            radius: 6

            RowLayout {

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
                    visible: false // item2.text.length ? true : false
                    Text {
                        anchors.centerIn: parent
                    }
                }

                ColumnLayout {
                    id: listCol
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

                    RowLayout {

                        id: row1
                        Layout.margins: rowSpace
                        visible: item1.text.length ? true : false

                        Image {
                            id: item1Img

                            width: iconW
                            height: parent.height - 2
                            fillMode: Image.NoOption
                            horizontalAlignment: Image.AlignHCenter
                            verticalAlignment: Image.AlignVCenter

                            smooth: true
                            sourceSize.height: iconW
                            sourceSize.width: iconW
                            source: listItem.ListView.isCurrentItem ? "/res/count.svg" : isDark ? "/res/count_l.svg" : "/res/count.svg"

                            visible: item1.text.length ? true : false
                        }

                        Rectangle {
                            height: 9
                            width: 9
                            radius: 0
                            anchors.leftMargin: 1
                            color: "gray"
                            visible: false // item1.text.length ? true : false
                        }

                        Text {
                            id: item1
                            Layout.preferredWidth: listItem.width - iconW - rowSpace - 3

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

                            visible: item1.text.length ? true : false
                        }
                    }

                    RowLayout {

                        id: row2
                        Layout.margins: rowSpace
                        visible: item2.text.length ? true : false

                        Image {
                            id: item2Img

                            width: iconW
                            height: parent.height - 2
                            fillMode: Image.NoOption
                            horizontalAlignment: Image.AlignHCenter
                            verticalAlignment: Image.AlignVCenter

                            smooth: true
                            sourceSize.height: iconW
                            sourceSize.width: iconW
                            source: listItem.ListView.isCurrentItem ? "/res/coin.svg" : isDark ? "/res/coin_l.svg" : "/res/coin.svg"

                            visible: item2.text.length ? true : false
                        }

                        Rectangle {
                            height: 9
                            width: 9
                            radius: 0
                            anchors.leftMargin: 1
                            color: "red"
                            visible: false // item2.text.length ? true : false
                        }

                        Text {
                            id: item2
                            anchors.rightMargin: 0
                            Layout.preferredWidth: listItem.width - iconW - rowSpace - 3
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

                    m_Method.clickMainDate()
                    m_Method.clickMainDateData()
                }
            }

            PropertyAnimation on x {
                easing.type: Easing.Linear
                running: false
                from: maindateWidth / 2
                to: 0
                duration: 200
                loops: 1 //Animation.Infinite
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
                    duration: 0
                } //暂停400ms
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
            ListElement {
                text0: '<span style="background-color: #ff6600;">Hello</span>'
                text1: "123456  <b>Hello</b> <i>World!</i>  123456"
                text2: '123456 <font color="red"><b>TEST</b></font>  123456'
                text3: "str3 1234567890 1234567890  1234567890 1234567890"
                myh: 0
            }
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
