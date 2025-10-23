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

    function showTodoAlarm() {

        setTodoAlarm.open()
    }

    function closeTodoAlarm() {

        setTodoAlarm.close()
    }

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

            MouseArea {
                anchors.fill: parent
                onClicked: function (mouse) {
                    if (actionButtons.visible && mouse.x > actionButtons.x
                            && mouse.x < actionButtons.x + actionButtons.width
                            && mouse.y > actionButtons.y
                            && mouse.y < actionButtons.y + actionButtons.height) {

                        mouse.accepted = false // 放行事件给按钮
                        console.log("按钮被点击...")
                    } else {
                        view.currentIndex = index //实现item切换
                        m_Todo.stopPlayVoice()
                        console.log("index=" + index + "  c_index=" + ListView.isCurrentItem)
                    }
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
                            height: 3 // 空白高度
                            color: "transparent"
                        }

                        // 新增：按钮区域 - 仅在当前条目被选中时显示
                        Row {
                            id: actionButtons
                            width: parent.width
                            z: 10 // 提升层级，高于其他元素
                            spacing: 15 // 按钮间距
                            padding: 5 // 内边距
                            visible: view.currentIndex === index // 选中时显示

                            // 高优先级按钮
                            ToolButton {
                                icon.name: "high"
                                icon.source: "qrc:/res/high.svg"
                                icon.width: 20
                                icon.height: 20

                                onClicked: {
                                    m_Todo.on_btnHigh_clicked()
                                    console.log("设置高优先级: " + index)
                                }
                                // 适配深色模式
                                background: Rectangle {
                                    color: "transparent"
                                }
                            }

                            // 低优先级按钮
                            ToolButton {
                                icon.name: "low"
                                icon.source: "qrc:/res/low.svg"
                                icon.width: 20
                                icon.height: 20

                                onClicked: {
                                    m_Todo.on_btnLow_clicked()
                                    console.log("设置低优先级: " + index)
                                }
                                background: Rectangle {
                                    color: "transparent"
                                }
                            }

                            // 修改按钮
                            ToolButton {
                                icon.name: "modify"
                                icon.source: "qrc:/res/edit.svg"
                                icon.width: 20
                                icon.height: 20

                                onClicked: {
                                    view.currentIndex = index
                                    m_Todo.reeditText() // 调用现有修改函数
                                    console.log("修改条目: " + index)
                                }
                                background: Rectangle {
                                    color: "transparent"
                                }
                            }

                            // 定时按钮
                            ToolButton {
                                icon.name: "setTime"
                                icon.source: "qrc:/res/alarm.svg"
                                icon.width: 20
                                icon.height: 20

                                onClicked: {
                                    m_Todo.on_btnSetTime_clicked()
                                    console.log("设置时间: " + index)
                                }
                                background: Rectangle {
                                    color: "transparent"
                                }
                            }

                            // 回收箱按钮
                            ToolButton {
                                icon.name: "recycle"
                                icon.source: "qrc:/res/recycle.svg"
                                icon.width: 20
                                icon.height: 20

                                onClicked: {
                                    m_Todo.on_btnRecycle_clicked()
                                    console.log("打开回收箱: " + index)
                                }
                                background: Rectangle {
                                    color: "transparent"
                                }
                            }
                        }

                        // 底部空白
                        Rectangle {
                            width: view.width
                            height: 5
                            color: "transparent"
                        }
                    }
                }

                Rectangle {
                    id: donebtn
                    height: parent.height
                    width: 55
                    color: isDark ? "#2D2D2D" : "#F5F5F5"
                    anchors.right: parent.right

                    opacity: flack.contentX !== 0 ? 1 : 0
                    Behavior on opacity {
                        NumberAnimation {
                            duration: 200
                        }
                    }
                    visible: opacity > 0

                    CheckBox {
                        id: todoCheckBox
                        anchors.centerIn: parent
                        text: ""
                        checked: false
                        padding: 0

                        Timer {
                            id: actionTimer
                            interval: 300
                            repeat: false
                            onTriggered: {
                                // 原删除逻辑不变
                                view.currentIndex = index
                                m_Todo.stopPlayVoice()
                                m_Todo.addToRecycle()
                                view.model.remove(index)
                                m_Todo.refreshTableLists()
                                m_Todo.refreshAlarm()
                                m_Todo.saveTodo()
                                springAnimation.to = 0
                                springAnimation.start()
                                todoCheckBox.checked = false
                                todoCheckBox.indicator.scale = 1
                            }
                        }

                        // 核心：用小矩形作为选中标记（无复杂属性，Qt6兼容）
                        indicator: Rectangle {
                            width: 26
                            height: 26
                            border.width: 2
                            border.color: isDark ? "#666666" : "#CCCCCC" // 边框色适配暗黑
                            // 检查盒背景：未选中时随模式变化，选中时保持中性色（突出红色标记）
                            color: todoCheckBox.checked ? (isDark ? "#333333" : "#FFFFFF") // 选中时背景不变，靠红色矩形标记
                                                        : (isDark ? "#333333" : "#FFFFFF")
                            anchors.centerIn: parent
                            antialiasing: true

                            // 小矩形（选中标记）：居中显示，大小适中
                            Rectangle {
                                id: checkMark
                                width: 14 // 矩形宽度
                                height: 14 // 矩形高度
                                color: isDark ? "#4CAF50" : "#8BC34A" // 暗黑模式深绿，亮色模式浅绿（与检查盒选中背景呼应）
                                anchors.centerIn: parent // 完全居中
                                visible: todoCheckBox.checked // 选中时显示
                                opacity: todoCheckBox.checked ? 1 : 0 // 透明度过渡
                                Behavior on opacity {
                                    NumberAnimation {
                                        duration: 100
                                    }
                                } // 淡入动画
                                radius: 2 // 轻微圆角，避免生硬
                            }

                            // 点击缩放动画（保留反馈）
                            Behavior on scale {
                                NumberAnimation {
                                    duration: 50
                                }
                            }
                        }

                        onClicked: {
                            todoCheckBox.indicator.scale = 0.9 // 按压缩放反馈
                            actionTimer.start()
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

    // 日期时间选择器弹窗
    Popup {
        id: setTodoAlarm

        width: root.width > 450 ? 450 : root.width
        height: root.height
        y: -20
        x: 0
        modal: true
        focus: true
        // 居中显示（替代x和y的手动偏移，避免超出屏幕）
        anchors.centerIn: root

        background: Rectangle {
            anchors.fill: parent // 填充整个Popup
            color: isDark ? "#222222" : "#f9f9f9"
            radius: 0
        }

        SetTodoAlarm {
            id: picker
            // 用Layout.fill而非anchors.fill，适配Popup的内边距
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
