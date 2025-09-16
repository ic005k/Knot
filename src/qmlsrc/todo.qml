import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Fusion
import QtQuick.Window

Rectangle {
    id: root

    width: 400
    height: 500

    color: isDark ? "#1E1E1E" : "white"

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
            strColor = "#333333"
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

        // 条目和条目之间的间隔
        spacing: 4
        anchors.rightMargin: 0 // 确保右侧无留白

        cacheBuffer: 50
        model: TodoModel {}

        // 滚动条
        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
            width: 8
        }

        delegate: Flickable {
            id: flack
            property int myw: m_width - 18
            width: myw
            height: m_col.implicitHeight + 0
            contentWidth: myw + donebtn.width + flagColor.width

            interactive: true

            // 新增：启用水平滑动和边界限制
            flickableDirection: Flickable.HorizontalFlick
            boundsBehavior: Flickable.StopAtBounds

            // 新增：弹性动画
            SpringAnimation {
                id: springAnimation
                target: flack
                property: "contentX"
                spring: 4 // 增强弹性强度
                damping: 0.3 // 增加阻尼系数
                epsilon: 0.5 // 放宽停止阈值
            }
            // 增加状态标记
            property bool autoAnimation: false
            // 增加速度跟踪支持
            flickDeceleration: 1500 // 增加滑动减速度
            maximumFlickVelocity: 2000 // 限制最大滑动速度

            // 新增：滑动结束处理
            onMovementEnded: {
                autoAnimation = true
                const speedThreshold = 100
                const posThreshold = donebtn.width * 0.3

                // 使用正确的速度获取方式
                const currentVelocity = flack.horizontalVelocity
                console.log("当前速度:", currentVelocity) // 调试用

                if (contentX > posThreshold || Math.abs(
                            currentVelocity) > speedThreshold) {
                    springAnimation.to = currentVelocity > 0 ? donebtn.width : 0
                } else {
                    springAnimation.to = 0
                }
                springAnimation.start()
            }

            // 增加动画完成回调
            Component.onCompleted: {
                springAnimation.onStopped.connect(() => {
                                                      if (autoAnimation) {
                                                          contentX = springAnimation.to
                                                          autoAnimation = false
                                                      }
                                                  })
            }

            Rectangle {

                id: rectan

                anchors.fill: parent
                width: parent.width

                border.width: isDark ? 0 : 1
                border.color: "lightgray"
                radius: 0
                //选中颜色设置
                color: view.currentIndex === index ? "lightblue" : getColor()

                Rectangle {
                    id: flagColor
                    height: parent.height
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

                    width: parent.width
                    spacing: 0
                    Layout.fillWidth: true

                    ColumnLayout {
                        id: m_col

                        width: parent.width - flagColor.width - donebtn.width - 4
                        spacing: 0
                        Layout.fillWidth: false
                        anchors.leftMargin: 0
                        anchors.rightMargin: 0

                        Rectangle {
                            width: view.width
                            height: 5 // 空白高度
                            color: "transparent"
                        }

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
                                topPadding: 5
                                bottomPadding: 5
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
                                height: text3.contentHeight
                                width: text3Img.visible ? parent.width - text3Img.width
                                                          - 5 : parent.width
                                Layout.preferredWidth: rectan.width - 52
                                font.pointSize: FontSize
                                wrapMode: TextArea.WordWrap
                                color: view.currentIndex === index ? "black" : getFontColor()
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                                text: dototext
                                textFormat: Text.PlainText

                                visible: true

                                leftPadding: 10
                                rightPadding: 10
                                topPadding: 5
                                bottomPadding: 5
                            }
                        }

                        TextArea {
                            id: text4
                            width: parent.width
                            wrapMode: Text.Wrap
                            text: itemheight

                            visible: false
                        }

                        Rectangle {
                            width: view.width
                            height: 5 // 空白高度
                            color: "transparent"
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

                    // 修改：使用透明度动画替代直接显示/隐藏
                    opacity: flack.contentX !== 0 ? 1 : 0
                    Behavior on opacity {
                        NumberAnimation {
                            duration: 200
                        }
                    }
                    visible: opacity > 0

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
