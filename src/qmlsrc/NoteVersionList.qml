import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Fusion

// 主容器：适配 QuickWidget 大小，带轻微阴影
Rectangle {
    color: isDark ? "#333333" : "#f9f9f9" // 浅灰背景，避免刺眼
    radius: 4 // 轻微圆角，提升美观度
    border.color: "#eee" // 细边框，区分区域
    border.width: 1

    property int currentSelectedIndex: -1

    // 列表布局：垂直排列，带内边距
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8 // 内边距，避免内容贴边

        // 列表标题：清晰提示功能
        Text {
            text: qsTr("Modify History")
            font.family: "Microsoft YaHei"
            font.pointSize: 14
            font.bold: true
            color: isDark ? "#DDDDDD" : "#333"
            Layout.leftMargin: 4
            Layout.bottomMargin: 8
        }

        // 核心列表：QListView + 动态数据模型
        ListView {
            id: versionListView
            Layout.fillWidth: true // 占满宽度
            Layout.fillHeight: true // 占满剩余高度
            spacing: 2 // 列表项间距
            clip: true // 超出部分裁剪，避免溢出

            // 数据模型：存储从 C++ 传入的修改记录（可被 C++ 访问）
            model: ListModel {
                id: noteVersionModel
                objectName: "noteVersionModel"

                // C++ 可调用的方法：清空模型（刷新前调用）
                function clearModel() {
                    noteVersionModel.clear()
                }

                // C++ 可调用的方法：添加一条修改记录（传入修改时间字符串）
                function addRecord(modifyTime) {
                    noteVersionModel.append({
                                                "time": modifyTime,
                                                "isSelected": false // 选中状态（默认未选中）
                                            })
                }
            }

            // 列表项模板（Delegate）：每条修改记录的显示样式
            delegate: Rectangle {
                id: deleRect
                width: versionListView.width
                height: 40 // 固定高度，避免列表项高低不一
                color: model.isSelected ? "#e6f7ff" : "transparent" // 选中时浅蓝色背景
                border.color: model.isSelected ? "#1890ff" : "transparent" // 选中时蓝色边框
                border.width: 1

                // 点击交互：选中当前项，取消其他项选中
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        // 先取消所有项的选中状态
                        for (var i = 0; i < noteVersionModel.count; i++) {
                            noteVersionModel.setProperty(i, "isSelected", false)
                        }
                        // 选中当前项
                        noteVersionModel.setProperty(index, "isSelected", true)

                        currentSelectedIndex = index

                        // （可选）发送信号给 C++，告知当前选中的时间
                        m_NotesList.getNoteDiffHtml()
                    }
                }

                // 修改时间文本：左对齐显示，清晰易读
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    text: model.time

                    color: isDark?(model.isSelected ? "#333":"#DDD") : "#333"
                    elide: Text.ElideRight // 时间过长时右侧省略，避免换行
                    width: parent.width - 80 // 预留按钮空间
                }

                // "旧文本"按钮：仅在条目选中时显示，靠右对齐
                Button {
                    text: qsTr("Old Text")
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    anchors.rightMargin: 10

                    // 通过contentItem自定义文字（兼容大多数Qt版本）
                    contentItem: Text {
                        text: parent.text // 与按钮text绑定
                        color: "#ffffff" // 白色文字（强制生效）
                        font: parent.font // 继承按钮字体设置
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        width: parent.width // 文字区域占满按钮宽度
                        height: parent.height // 文字区域占满按钮高度
                    }

                    // 背景样式：柔和蓝色
                    background: Rectangle {
                        color: "#5c6bc0" // 柔和的蓝色
                        radius: 4
                    }

                    // 尺寸逻辑
                    font.pointSize: 12
                    height: Math.min(font.pointSize * 2.4, 36)
                    width: Math.min(text.length * font.pointSize * 1.8 + 20,
                                    100)

                    Component.onCompleted: {
                        if (Qt.platform.os === "android") {
                            height *= 1.1
                            width *= 1.1
                        }
                    }

                    visible: model.isSelected

                    onClicked: {
                        m_NotesList.newtextToOldtextFromDiffStr()
                    }
                }
            }

            // 列表为空时的提示
            Rectangle {
                visible: versionListView.count === 0
                anchors.fill: versionListView
                color: "transparent"

                Text {
                    anchors.centerIn: parent
                    text: qsTr("No modification records")

                    color: isDark ? "#DDD" : "#999"
                }
            }

            // 滚动条
            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
                width: 8
            }
        }
    }

    // （可选）定义信号：发送选中的修改时间给 C++（需在 C++ 中连接）
    signal onItemSelected(string selectedTime)
}
