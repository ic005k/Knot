import QtQuick 6.2
import QtQuick.Controls 6.2
import QtQuick.Layouts 6.2

Rectangle {
    id: mainContainer
    color: "#f9f9f9"
    border.width: 1
    border.color: "#e0e0e0"
    radius: 8
    width: parent.width
    height: parent.height

    // 周选择状态
    property bool selectAllDays: false
    property bool week1Checked: false
    property bool week2Checked: false
    property bool week3Checked: false
    property bool week4Checked: false
    property bool week5Checked: false
    property bool week6Checked: false
    property bool week7Checked: false

    // 语音播报状态
    property bool voiceBroadcastEnabled: false

    // 主布局：使用ColumnLayout管理，避免混合anchors
    ColumnLayout {
        id: mainLayout
        spacing: 10
        anchors.fill: parent
        anchors.margins: 6

        // 1. 周选择区域
        ColumnLayout {
            spacing: 6
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop

            Text {
                text: qsTr("Select by Week")
                font.pixelSize: 16
                font.bold: true
                color: "#333"
                Layout.alignment: Qt.AlignLeft // 左对齐，替代anchors
            }

            // 第一排：每天 + 周一到周三（用RowLayout替代Row，避免anchors冲突）
            RowLayout {
                spacing: 10
                Layout.alignment: Qt.AlignHCenter // 水平居中，替代anchors.horizontalCenter

                Switch {
                    id: selectAllSwitch
                    text: qsTr("Everyday")
                    checked: selectAllDays
                    onCheckedChanged: {
                        selectAllDays = checked
                        if (checked) {
                            week1Checked = week2Checked = week3Checked = week4Checked
                                    = week5Checked = week6Checked = week7Checked = true
                        }
                    }
                }
                Switch {
                    text: "1"
                    checked: week1Checked
                    onCheckedChanged: {
                        week1Checked = checked
                        updateSelectAllState()
                    }
                }
                Switch {
                    text: "2"
                    checked: week2Checked
                    onCheckedChanged: {
                        week2Checked = checked
                        updateSelectAllState()
                    }
                }
                Switch {
                    text: "3"
                    checked: week3Checked
                    onCheckedChanged: {
                        week3Checked = checked
                        updateSelectAllState()
                    }
                }
            }

            // 第二排：周四到周日
            RowLayout {
                spacing: 10
                Layout.alignment: Qt.AlignHCenter // 水平居中，替代anchors.horizontalCenter

                Switch {
                    text: "4"
                    checked: week4Checked
                    onCheckedChanged: {
                        week4Checked = checked
                        updateSelectAllState()
                    }
                }
                Switch {
                    text: "5"
                    checked: week5Checked
                    onCheckedChanged: {
                        week5Checked = checked
                        updateSelectAllState()
                    }
                }
                Switch {
                    text: "6"
                    checked: week6Checked
                    onCheckedChanged: {
                        week6Checked = checked
                        updateSelectAllState()
                    }
                }
                Switch {
                    text: "7"
                    checked: week7Checked
                    onCheckedChanged: {
                        week7Checked = checked
                        updateSelectAllState()
                    }
                }
            }
        }

        // 2. 日期时间选择器
        DateTimePicker {
            id: dateTimePicker
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredHeight: 310 // 适当减小
            Layout.maximumHeight: 320 // 限制最大高度
        }

        // 3. 语音控制区域
        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            spacing: 6

            Switch {
                id: voiceSwitch
                text: qsTr("Voice Broadcast")
                checked: voiceBroadcastEnabled
                onCheckedChanged: {
                    voiceBroadcastEnabled = checked
                    console.log("Voice broadcast " + (checked ? "enabled" : "disabled"))
                }
                Layout.alignment: Qt.AlignVCenter // 垂直居中，替代anchors
            }

            Item {
                Layout.fillWidth: true // 填充中间空间
            }

            Button {
                id: testVoiceBtn
                text: qsTr("Test Voice")
                onClicked: {
                    console.log("Test voice clicked")
                }
                width: 100
                Layout.alignment: Qt.AlignVCenter
            }
        }

        // 4. 操作按钮区域
        RowLayout {
            // 用RowLayout替代Row，避免anchors冲突
            spacing: 10
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom // 水平居中+底部对齐

            Button {
                text: qsTr("Cancel")
                onClicked: {

                    /* 取消逻辑 */ }
                width: 90
            }

            Button {
                text: qsTr("Delete Alarm")
                onClicked: {

                    /* 删除逻辑 */ }
                width: 90
            }

            Button {
                text: qsTr("Set Alarm")
                onClicked: {

                    /* 设置逻辑 */ }
                width: 90
            }
        }
    }

    // 联动更新"每天"开关状态
    function updateSelectAllState() {
        selectAllDays = week1Checked && week2Checked && week3Checked
                && week4Checked && week5Checked && week6Checked && week7Checked
    }
}
