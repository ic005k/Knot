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
    property bool isRenderValid: true

    function checkRenderContext() {
        if (Qt.application.platformName === "android" && !isRenderValid) {
            console.log("渲染上下文无效，跳过操作");
            return false;
        }
        return true;
    }

    function showTodoAlarm() {
        if(checkRenderContext()) setTodoAlarm.open()
    }

    function closeTodoAlarm() {
        if(!checkRenderContext()) return;
        setTodoAlarm.close()
        m_Todo.setAlarmShowValue(false)
        m_Todo.showInputPanel()
    }

    function isAlarm(index) {
        return isHighPriority
    }

    function setHighPriority(isFalse) {
        isHighPriority = isFalse
    }

    function setCurrentItem(currentIndex) {
        if(checkRenderContext()) view.currentIndex = currentIndex
    }

    function getCurrentIndex() {
        return view.currentIndex
    }

    function getItemCount() {
        if(!checkRenderContext()) return 0;
        itemCount = view.count
        return itemCount
    }

    function getItemText(itemIndex) {
        if(!checkRenderContext()) return "";
        var data = view.model.get(itemIndex)
        return data.time + "|=|" + data.dototext
    }

    function getTime(itemIndex) {
        if(!checkRenderContext()) return "";
        var data = view.model.get(itemIndex)
        return data.time
    }

    function getTodoText(itemIndex) {
        if(!checkRenderContext() || itemIndex < 0 || itemIndex >= view.count) return "";
        var data = view.model.get(itemIndex)
        return data.dototext
    }

    function getType(itemIndex) {
        if(!checkRenderContext()) return 0;
        var data = view.model.get(itemIndex)
        return data.type
    }

    function addItem(strTime, type, strText, height) {
        if(!checkRenderContext()) return;
        view.model.append({
                              "time": strTime,
                              "type": type,
                              "dototext": strText,
                              "itemheight": height
                          })
    }

    function insertItem(strTime, type, strText, height, curIndex) {
        if(!checkRenderContext()) return;
        view.model.insert(curIndex, {
                              "time": strTime,
                              "type": type,
                              "dototext": strText,
                              "itemheight": height
                          })
    }

    function delItem(currentIndex) {
        if(!checkRenderContext()) return;
        view.model.remove(currentIndex)
    }

    function modifyItem(currentIndex, strTime, strText) {
        if(!checkRenderContext()) return;
        view.model.setProperty(currentIndex, "time", strTime)
        view.model.setProperty(currentIndex, "dototext", strText)
    }

    function modifyItemTime(currentIndex, strTime) {
        if(!checkRenderContext()) return;
        view.model.setProperty(currentIndex, "time", strTime)
    }

    function modifyItemType(currentIndex, type) {
        if(!checkRenderContext()) return;
        view.model.setProperty(currentIndex, "type", type)
    }

    function modifyItemText(currentIndex, strText) {
        if(!checkRenderContext()) return;
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
        boundsBehavior: Flickable.StopAtBounds

        anchors {
            fill: parent
            margins: 0
        }

        spacing: 4
        anchors.rightMargin: 0

        cacheBuffer: 50
        model: TodoModel {}

        // ========== 移除无效的renderType属性，保留其他渲染优化 ==========
        reuseItems: false


        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
            width: 8
            layer.enabled: false
        }

        delegate: Flickable {
            id: flack

            width: view.width
            height: m_col.implicitHeight > 0 ? m_col.implicitHeight : 80

            contentWidth: view.width + donebtn.width

            interactive: true

            flickableDirection: Flickable.HorizontalFlick
            boundsBehavior: Flickable.StopAtBounds

            // ========== 移除无效的renderType属性，保留layer.enabled=false ==========
            layer.enabled: false

            SpringAnimation {
                id: springAnimation
                target: flack
                property: "contentX"
                spring: 4
                damping: 0.3
                epsilon: 0.5
            }
            property bool autoAnimation: false
            flickDeceleration: 1500
            maximumFlickVelocity: 2000

            onMovementEnded: {
                if(!checkRenderContext()) return;
                autoAnimation = true
                const speedThreshold = 100
                const posThreshold = donebtn.width * 0.3

                const currentVelocity = flack.horizontalVelocity
                console.log("当前速度:", currentVelocity)

                if (contentX > posThreshold || Math.abs(
                            currentVelocity) > speedThreshold) {
                    springAnimation.to = currentVelocity > 0 ? donebtn.width : 0
                } else {
                    springAnimation.to = 0
                }
                springAnimation.start()
            }

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
                    if(!checkRenderContext()) return;
                    if (actionButtons.visible && mouse.x > actionButtons.x
                            && mouse.x < actionButtons.x + actionButtons.width
                            && mouse.y > actionButtons.y
                            && mouse.y < actionButtons.y + actionButtons.height) {

                        mouse.accepted = false
                        console.log("按钮被点击...")
                    } else {
                        view.currentIndex = index
                        m_Todo.stopPlayVoice()
                        console.log("index=" + index + "  c_index=" + ListView.isCurrentItem)
                    }
                }

                onDoubleClicked: {
                    if(!checkRenderContext()) return;
                    m_Todo.reeditText()
                    var data = view.model.get(view.currentIndex)
                    console.log(data.time + "," + data.dototext + ", count=" + view.count)
                }

                onPressed: {
                    if(checkRenderContext()) donebtn.visible = true
                }
            }

            Rectangle {
                id: rectan

                anchors.fill: parent
                width: parent.width

                border.width: isDark ? 0 : 1
                border.color: "lightgray"
                radius: 0
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
                            height: 5
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
                                layer.enabled: false
                            }
                            Text {
                                id: text1

                                width: text1Img.visible ? parent.width - text1Img.width
                                                          - 5 : parent.width
                                Layout.preferredWidth: rectan.width - flagColor.width
                                color: view.currentIndex === index ? "black" : getText1FontColor()
                                font.pointSize: FontSize - 2 > maxFontSize ? maxFontSize : FontSize - 2
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
                                renderType: Text.NativeRendering // Text的renderType有效，保留
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
                                layer.enabled: false
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
                                renderType: Text.NativeRendering // Text的renderType有效，保留
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
                            height: 3
                            color: "transparent"
                        }

                        Row {
                            id: actionButtons
                            width: parent.width
                            z: 10
                            spacing: 15
                            padding: 5
                            visible: view.currentIndex === index

                            ToolButton {
                                icon.name: "high"
                                icon.source: "qrc:/res/high.svg"
                                icon.width: 20
                                icon.height: 20

                                onClicked: {
                                    if(checkRenderContext()) {
                                        m_Todo.on_btnHigh()
                                        console.log("设置高优先级")
                                    }
                                }
                                background: Rectangle {
                                    color: "transparent"
                                }
                            }

                            ToolButton {
                                icon.name: "low"
                                icon.source: "qrc:/res/low.svg"
                                icon.width: 20
                                icon.height: 20

                                onClicked: {
                                    if(checkRenderContext()) {
                                        m_Todo.on_btnLow()
                                        console.log("设置低优先级")
                                    }
                                }
                                background: Rectangle {
                                    color: "transparent"
                                }
                            }

                            ToolButton {
                                icon.name: "modify"
                                icon.source: "qrc:/res/edit.svg"
                                icon.width: 20
                                icon.height: 20

                                onClicked: {
                                    if(checkRenderContext()) {
                                        view.currentIndex = index
                                        m_Todo.reeditText()
                                        console.log("修改条目: " + index)
                                    }
                                }
                                background: Rectangle {
                                    color: "transparent"
                                }
                            }

                            ToolButton {
                                icon.name: "setTime"
                                icon.source: "qrc:/res/alarm.svg"
                                icon.width: 20
                                icon.height: 20

                                onClicked: {
                                    if(checkRenderContext()) {
                                        m_Todo.on_btnSetTime()
                                        console.log("设置时间: " + index)
                                    }
                                }
                                background: Rectangle {
                                    color: "transparent"
                                }
                            }

                            ToolButton {
                                icon.name: "recycle"
                                icon.source: "qrc:/res/recycle.svg"
                                icon.width: 20
                                icon.height: 20

                                onClicked: {
                                    if(checkRenderContext()) {
                                        m_Todo.on_btnRecycle()
                                        console.log("打开回收箱: " + index)
                                    }
                                }
                                background: Rectangle {
                                    color: "transparent"
                                }
                            }
                        }

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
                        // 直接自定义indicator，无CheckBoxStyle（Qt6.10.2兼容）
                        indicator: Rectangle {
                            width: 26
                            height: 26
                            border.width: 2
                            border.color: isDark ? "#666666" : "#CCCCCC"
                            color: todoCheckBox.checked ? (isDark ? "#333333" : "#FFFFFF")
                                                        : (isDark ? "#333333" : "#FFFFFF")
                            anchors.centerIn: parent
                            antialiasing: true

                            Rectangle {
                                id: checkMark
                                width: 14
                                height: 14
                                color: isDark ? "#4CAF50" : "#8BC34A"
                                anchors.centerIn: parent
                                visible: todoCheckBox.checked
                                opacity: todoCheckBox.checked ? 1 : 0
                                Behavior on opacity {
                                    NumberAnimation {
                                        duration: 100
                                    }
                                }
                                radius: 2
                            }

                            Behavior on scale {
                                NumberAnimation {
                                    duration: 50
                                }
                            }
                        }

                        Timer {
                            id: actionTimer
                            interval: 300
                            repeat: false
                            onTriggered: {
                                if(!checkRenderContext()) return;
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

                        onClicked: {
                            if(!checkRenderContext()) return;
                            todoCheckBox.indicator.scale = 0.9
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

    Popup {
        id: setTodoAlarm

        width: root.width > 450 ? 450 : root.width
        height: root.height
        y: 0
        x: 0
        modal: true
        focus: true
        anchors.centerIn: root

        background: Rectangle {
            anchors.fill: parent
            color: isDark ? "#222222" : "#f9f9f9"
            radius: 0
        }

        SetTodoAlarm {
            id: picker
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }

    Connections {
        target: Qt.application
        function onAboutToQuit() { root.isRenderValid = false }
    }
}
