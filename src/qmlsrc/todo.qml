import QtQuick 6.7
import QtQuick.Controls 6.7
import QtQuick.Layouts 6.7

import QtQuick.Window 2.15
import QtQml 2.15

Rectangle {
    id: root

    width: 400
    height: 500

    color: isDark ? "#19232D" : "white"

    property int iconW: 12
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

    function insertItem(strTime, type, strText, height, curIndex) {
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

    function getText1FontColor() {

        if (isDark)
            return "#bbbbbb"
        else
            return "#777777"
    }

    function getFontColor() {

        if (isDark)
            return "white"
        else
            return "black"
    }

    ListView {
        id: view
        width: parent.width
        height: parent.height
        boundsBehavior: Flickable.StopAtBounds // 禁止滚动到边界外的弹性效果

        anchors {
            fill: parent
            margins: 0
        }
        spacing: 4
        anchors.rightMargin: 0 // 确保右侧无留白
        cacheBuffer: 50
        model: TodoModel {}

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

        delegate: Flickable {
            id: flack
            property int myw: m_width - 18
            width: myw
            height: rectan.getItemHeight()
            contentWidth: myw + donebtn.width + flagColor.width
            contentHeight: rectan.getItemHeight()
            boundsBehavior: Flickable.StopAtBounds //该属性设置过后，边界不会被拉出
            interactive: true

            Rectangle {
                id: rectan

                anchors.fill: parent
                width: parent.width
                height: getItemHeight()
                border.width: isDark ? 0 : 1
                border.color: "lightgray"
                radius: 0
                //选中颜色设置
                color: view.currentIndex === index ? "lightblue" : getColor()

                //color: view.currentIndex === index ? "#DCDCDC" : "#ffffff"
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

                Rectangle {
                    id: flagColor
                    height: rectan.getItemHeight() - 0
                    width: 6
                    radius: 2
                    anchors.leftMargin: 1
                    color: getPriorityColor(type)
                    Text {
                        anchors.centerIn: parent
                    }
                    visible: true
                }

                RowLayout {
                    id: idlistElemnet
                    height: parent.height
                    width: parent.width
                    spacing: 2
                    Layout.fillWidth: true

                    ColumnLayout {
                        id: m_col
                        height: parent.height
                        width: parent.width - flagColor.width - donebtn.width - 4
                        spacing: 2
                        Layout.fillWidth: false
                        anchors.leftMargin: 0
                        anchors.rightMargin: 0

                        RowLayout {

                            id: row1

                            function showImg() {

                                var str1 = text1.text.substring(0, 5)
                                var str2 = text1.text.substring(0, 4)
                                if (str1 === "Alarm" || str2 === "定时提醒")
                                    return true
                                else
                                    return false
                            }

                            Rectangle {
                                height: parent.height
                                width: 5
                                visible: text1Img.visible
                            }

                            Image {
                                id: text1Img

                                width: itemheight - 2
                                height: text1.contentHeight
                                fillMode: Image.NoOption
                                horizontalAlignment: Image.AlignHCenter
                                verticalAlignment: Image.AlignVCenter

                                smooth: true
                                sourceSize.height: itemheight - 2
                                sourceSize.width: itemheight - 2
                                source: "/res/time.svg"

                                visible: row1.showImg()
                            }
                            Text {
                                id: text1
                                width: text1Img.visible ? parent.width - text1Img.width
                                                          - 5 : parent.width
                                Layout.preferredWidth: rectan.width - flagColor.width
                                color: view.currentIndex === index ? "black" : getText1FontColor()
                                font.pointSize: FontSize - 2
                                                > maxFontSize ? maxFontSize : FontSize - 2
                                font.bold: true
                                wrapMode: Text.WrapAnywhere
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                                text: time
                                visible: true

                                leftPadding: 10
                                rightPadding: 10
                            }
                        }

                        TextArea {
                            id: text2
                            width: parent.width
                            wrapMode: Text.Wrap
                            text: type

                            visible: false
                        }

                        RowLayout {

                            id: row3
                            height: text3.contentHeight
                            width: parent.width

                            function showImg3() {

                                var str1 = text3.text.substring(0, 5)
                                var str2 = text3.text.substring(0, 2)
                                if (str1 === "Voice" || str2 === "语音")
                                    return true
                                else
                                    return false
                            }

                            Rectangle {
                                height: 0
                                width: 5
                                visible: text3Img.visible
                            }

                            Image {
                                id: text3Img

                                width: text3.contentHeight
                                height: text3.contentHeight
                                fillMode: Image.NoOption
                                horizontalAlignment: Image.AlignHCenter
                                verticalAlignment: Image.AlignVCenter

                                smooth: true
                                sourceSize.height: text3.contentHeight - 2
                                sourceSize.width: text3.contentHeight - 2
                                source: "/res/audio.svg"

                                visible: row3.showImg3()
                            }

                            Text {
                                id: text3
                                width: text3Img.visible ? parent.width - text3Img.width
                                                          - 5 : parent.width
                                Layout.preferredWidth: rectan.width - 52
                                font.pointSize: FontSize
                                wrapMode: TextArea.WordWrap
                                color: view.currentIndex === index ? "black" : getFontColor()
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                                text: dototext

                                visible: true

                                leftPadding: 10
                                rightPadding: 10
                            }
                        }

                        TextArea {
                            id: text4
                            width: parent.width
                            wrapMode: Text.Wrap
                            text: itemheight

                            visible: false
                        }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        view.currentIndex = index //实现item切换
                        m_Todo.stopPlayVoice()
                        //console.log("index=" + index + "  c_index=" + ListView.isCurrentItem)
                    }

                    onDoubleClicked: {
                        m_Todo.reeditText()
                        var data = view.model.get(view.currentIndex)
                        console.log(data.time + "," + data.dototext + ", count=" + view.count)
                    }

                    onPressed: {
                        donebtn.visible = true
                    }
                }

                Rectangle {
                    id: donebtn
                    height: parent.height - 0
                    width: 55
                    color: "red"
                    anchors.right: parent.right
                    visible: isBtnVisible

                    Image {
                        id: doneImg

                        width: 35
                        height: 35
                        x: (donebtn.width - doneImg.width) / 2 - 1
                        y: (donebtn.height - doneImg.height) / 2
                        fillMode: Image.NoOption
                        horizontalAlignment: Image.AlignHCenter
                        verticalAlignment: Image.AlignVCenter

                        smooth: true
                        sourceSize.height: 35
                        sourceSize.width: 35
                        source: "/res/todo_done.png"

                        visible: true
                    }

                    Text {
                        anchors.centerIn: parent
                        //text: qsTr("Done")
                        color: "#ffffff"
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            view.currentIndex = index

                            m_Todo.stopPlayVoice()
                            m_Todo.addToRecycle()
                            view.model.remove(index)
                            m_Todo.refreshTableLists()
                            m_Todo.refreshAlarm()
                            m_Todo.saveTodo()
                            console.log("Done isclick")
                        }
                    }
                }
            }
        }
    }

    function getPriorityColor(ntype) {
        switch (ntype) {
        case 0:
            return "gray"
        case 1:
            return "red"
        case 2:
            return "orange"
        case 3:
            return "#3498DB"
        default:
            return "black"
        }
    }
}
