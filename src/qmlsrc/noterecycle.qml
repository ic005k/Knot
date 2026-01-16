import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Fusion

Rectangle {
    id: root

    width: 500
    height: 400

    function resetQMLToNewState() {
        console.debug("QML端：一键重置为全新状态，清空所有历史痕迹")
        // 1. 清空选中状态数据（核心，删除历史选中索引）
        selectedItems.clear()
        // 2. 清空列表数据（可选，若外部加载新数据前需清空原有列表）
        view.model.clear()
        // 3. 重置所有复选框为未勾选（清空视觉残留）
        for (var i = 0; i < view.count; i++) {
            var listItem = view.itemAtIndex(i)
            if (listItem && listItem.itemCheckBox) {
                listItem.itemCheckBox.checked = false
            }
        }
        // 4. 强制刷新ListView，确保视觉状态同步
        view.forceLayout()
    }

    // ========== QML组件加载完成回调（核心初始化逻辑） ==========
    Component.onCompleted: {
        console.debug("QML端：组件加载完成，清空所有选中状态")
        resetQMLToNewState()
    }

    // ========== 多选功能核心属性 ==========
    ListModel {
        id: selectedItems // 存储选中项索引，去重管理
        onCountChanged: {
            console.debug("QML端：selectedItems数量变化，当前count =", count)
        }
    }

    property int itemCount: 0
    property bool isHighPriority: false

    function getSelectedIndexes() {
        var indexes = []
        for (var i = 0; i < selectedItems.count; i++) {
            indexes.push(selectedItems.get(i).index)
        }
        // 关键调试：打印数组内容和长度（在Qt Creator的「应用程序输出」面板查看）
        console.debug("QML端：selectedItems.count =", selectedItems.count)
        console.debug("QML端：返回的索引数组 =", indexes)
        console.debug("QML端：索引数组长度 =", indexes.length)
        return indexes
    }

    function clearAllSelectedItems() {
        selectedItems.clear()
        console.debug("QML端：已清空所有选中索引")

        // ========== 遍历所有列表项，取消复选框勾选（清空视觉残留） ==========
        for (var i = 0; i < view.count; i++) {
            // 获取列表项的复选框组件，重置勾选状态
            var listItem = view.itemAtIndex(i)
            if (listItem && listItem.itemCheckBox) {
                listItem.itemCheckBox.checked = false
            }
        }
    }

    // ========== 原有工具方法（保持不变，直接依赖isDark变量） ==========
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
        return (data.time || "") + "|=|" + (data.dototext || "")
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
        return data.text_top || ""
    }

    function getType(itemIndex) {
        var data = view.model.get(itemIndex)
        return data.type || 0
    }

    function addItem(t0, t1, t2, t3, height) {
        view.model.append({
                              "text0": t0,
                              "text1": t1,
                              "text2": t2,
                              "text3": t3,
                              "myh": height || 0
                          })
    }

    function insertItem(strTime, type, strText, curIndex) {
        view.model.insert(curIndex, {
                              "time": strTime,
                              "type": type,
                              "dototext": strText
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

    // ========== 暗黑模式适配方法（直接依赖isDark变量，Qt端赋值后自动生效） ==========
    function getColor() {
        var strColor
        if (isDark)
            strColor = "#455364" // 暗黑模式条目背景
        else
            strColor = "#ffffff" // 浅色模式条目背景
        return strColor
    }

    function getFontColor() {
        if (isDark)
            return "white" // 暗黑模式常规字体
        else
            return "black" // 浅色模式常规字体
    }

    function getFontColor3() {
        if (isDark)
            return "#BBBBBB" // 暗黑模式次要字体（灰色）
        else
            return "#555555" // 浅色模式次要字体（深灰）
    }

    // ========== 多选功能核心方法 ==========
    function addSelectedItem(index) {
        for (var i = 0; i < selectedItems.count; i++) {
            if (selectedItems.get(i).index === index) {
                return
            }
        }
        selectedItems.append({
                                 "index": index
                             })
        console.debug("QML端：添加选中索引", index, "，当前selectedItems.count =",
                      selectedItems.count)
    }

    function removeSelectedItem(index) {
        for (var i = 0; i < selectedItems.count; i++) {
            if (selectedItems.get(i).index === index) {
                selectedItems.remove(i)
                console.debug("QML端：移除选中索引", index, "，当前selectedItems.count =",
                              selectedItems.count)
                break
            }
        }
    }

    function isItemSelected(index) {
        for (var i = 0; i < selectedItems.count; i++) {
            if (selectedItems.get(i).index === index) {
                return true
            }
        }
        return false
    }

    // 可选：清空所有选中项（如需批量重置）
    function clearAllSelected() {
        selectedItems.clear()
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

    // ========== 列表委托（修复绑定循环+直接基于isDark适配暗黑模式） ==========
    Component {
        id: dragDelegate

        Rectangle {
            id: listItem
            width: ListView.view.width
            // 修复绑定循环：依赖外层布局固有高度，避免父子双向依赖
            height: idlistElemnet.implicitHeight + 10
            // 直接基于isDark+选中状态适配背景色，Qt端赋值后自动生效
            color: isItemSelected(
                       index) ? (isDark ? "#2A4365" : "#6495ED") // 选中状态：暗黑深蓝/浅色浅蓝
                              : getColor() // 未选中状态：调用依赖isDark的方法
            // 暗黑模式隐藏边框，浅色模式显示边框，直接响应isDark
            border.width: isDark ? 0 : 1
            border.color: "lightgray"
            radius: 0

            RowLayout {
                id: idlistElemnet
                height: parent.height
                width: parent.width
                spacing: 2
                Layout.fillWidth: true

                // ========== 复选框（直接基于isDark适配样式，无额外按钮） ==========
                CheckBox {
                    id: itemCheckBox
                    Layout.alignment: Qt.AlignVCenter
                    Layout.leftMargin: 5
                    // 步骤1：移除与isItemSelected的直接绑定，初始化为false（无绑定，可手动修改）
                    checked: false

                    // ========== 复选框初始化，同步选中状态 ==========
                    Component.onCompleted: {
                        itemCheckBox.checked = isItemSelected(index)
                    }

                    // 步骤2：修改onClicked逻辑，先更新selectedItems，再同步自身checked状态（避免反转）
                    onClicked: {
                        console.debug("QML端：复选框被点击，点击后目标状态 =", checked)
                        if (checked) {
                            // 复选框勾选 → 添加选中索引
                            addSelectedItem(index)
                        } else {
                            // 复选框取消勾选 → 移除选中索引
                            removeSelectedItem(index)
                        }
                        // 可选：同步复选框状态与selectedItems（确保视觉与数据一致）
                        itemCheckBox.checked = isItemSelected(index)
                    }

                    // 保留原有复选框指示器样式，无修改
                    indicator: Rectangle {
                        width: 16
                        height: 16
                        color: "transparent"
                        border.width: 2
                        border.color: isDark ? "#3B82F6" : "#2196F3"
                        Rectangle {
                            width: 12
                            height: 12
                            anchors.centerIn: parent
                            visible: itemCheckBox.checked
                            color: isDark ? "#3B82F6" : "#2196F3"
                        }
                        Text {
                            text: "✓"
                            color: "white"
                            anchors.centerIn: parent
                            visible: itemCheckBox.checked
                            font.bold: true
                            font.pointSize: 12
                        }
                    }
                }

                ColumnLayout {
                    id: listCol
                    // 永久预留复选框空间，布局稳定
                    width: parent.width - itemCheckBox.width - 10
                    spacing: 2
                    Layout.fillWidth: true
                    anchors.leftMargin: 0
                    anchors.rightMargin: 0

                    Rectangle {
                        width: view.width - itemCheckBox.width - 10
                        height: 5
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
                        // 直接基于isDark+选中状态适配文本颜色
                        color: isItemSelected(index) ? "white" : getFontColor(
                                                           ) // 方法内部依赖isDark
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
                        color: isItemSelected(index) ? "white" : getFontColor()
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
                        color: isItemSelected(index) ? "white" : getFontColor()
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
                        Layout.preferredWidth: listItem.width
                        font.bold: false
                        font.italic: true
                        font.pointSize: item0.font.pointSize - 2
                        text: text3
                        // 次要文本直接基于isDark+选中状态适配
                        color: isItemSelected(
                                   index) ? "#E0EFFF" : getFontColor3(
                                                ) // 方法内部依赖isDark
                        leftPadding: 5
                        rightPadding: 5
                        visible: item3.text.length ? true : false
                    }

                    Rectangle {
                        width: view.width - itemCheckBox.width - 10
                        height: 5
                        color: "transparent"
                    }
                }
            }

            // ========== 简化MouseArea（保持核心交互，无冗余） ==========
            MouseArea {
                property point clickPos: Qt.point(0, 0)
                anchors.fill: parent

                onPressed: function (mouse) {
                    clickPos = Qt.point(mouse.x, mouse.y)
                }

                onReleased: function (mouse) {
                    var delta = Qt.point(mouse.x - clickPos.x,
                                         mouse.y - clickPos.y)
                    console.debug("QML端：条目释放，delta.x =", delta.x)
                }

                // 步骤1：点击条目，直接切换选中状态（不修改checked，避免反转）
                onClicked: {
                    var isCurrentlySelected = isItemSelected(index)
                    console.debug("QML端：条目被点击，当前是否选中 =", isCurrentlySelected)

                    if (isCurrentlySelected) {
                        // 已选中 → 取消选中（移除索引+同步复选框）
                        removeSelectedItem(index)
                        itemCheckBox.checked = false
                    } else {
                        // 未选中 → 选中（添加索引+同步复选框）
                        addSelectedItem(index)
                        itemCheckBox.checked = true
                    }
                }
            }
        }
    }

    // ========== 列表视图（优化滚动条：移除无效handle配置，解决竖线问题） ==========
    ListView {
        id: view
        anchors {
            fill: parent
            margins: 4
        }
        model: ListModel {
            id: listmain
            ListElement {
                text0: '<span style="background-color: #ff6600;">Hello</span>'
                text1: "123456  <b>Hello</b> <i>World!</i>  123456"
                text2: '123456 <font color="red"><b>TEST</b></font>  123456'
                text3: "str3 1234567890 1234567890  1234567890 1234567890"
                myh: 0
            }
            ListElement {
                text0: "Test Item 2"
                text1: "这是第二个测试条目（默认显示选择框）"
                text2: ""
                text3: "多选功能验证 - 条目2"
                myh: 0
            }
            ListElement {
                text0: "Test Item 3"
                text1: "点击条目或复选框，均可切换选中状态"
                text2: "附加说明：调用clearAllSelected()可清空所有选中项"
                text3: "多选功能验证 - 条目3"
                myh: 0
            }
        }
        delegate: dragDelegate
        spacing: 4
        cacheBuffer: 50

        // 滚动条优化：仅保留背景适配，移除handle自定义，解决无效竖线问题
        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
            width: 8
            // 仅保留背景色适配，滑块使用Qt默认样式，无视觉bug
            background: Rectangle {
                color: isDark ? "#2D3748" : "#F0F0F0"
            }
        }
    }
}
