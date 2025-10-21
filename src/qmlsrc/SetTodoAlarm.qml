import QtQuick 6.2
import QtQuick.Controls 6.2
import QtQuick.Layouts 6.2

Rectangle {
    id: mainContainer
    objectName: "setTodoAlarmComponent"
    color: "#f9f9f9"
    border.width: 1
    border.color: "#e0e0e0"
    radius: 8
    width: parent.width
    height: parent.height

    // 周选择状态（保持不变）
    property bool selectAllDays: false
    property bool week1Checked: false
    property bool week2Checked: false
    property bool week3Checked: false
    property bool week4Checked: false
    property bool week5Checked: false
    property bool week6Checked: false
    property bool week7Checked: false

    // 语音播报状态（保持不变）
    property bool voiceBroadcastEnabled: false

    // 主布局：使用ColumnLayout管理
    ColumnLayout {
        id: mainLayout
        spacing: 3
        anchors.fill: parent
        anchors.margins: 2
        Layout.maximumWidth: parent.width
        Layout.preferredWidth: parent.width

        // 1. 周选择区域（核心修改：改为3排RowLayout）
        ColumnLayout {
            spacing: 2 // 排与排之间的垂直间距
            Layout.fillWidth: true
            Layout.preferredHeight: 60 // 固定总高度，避免占用过多空间

            Text {
                text: qsTr("Select by Week")
                font.pixelSize: 16
                font.bold: true
                color: "#333"
                Layout.alignment: Qt.AlignLeft // 标题左对齐
            }

            // 第二排：1（周一）、2（周二）、3（周三）
            RowLayout {
                spacing: 20 // 三个开关均匀分布，间距适中
                Layout.fillWidth: true

                //Layout.alignment: Qt.AlignHCenter // 水平居中
                Switch {
                    text: "1"
                    checked: week1Checked
                    Layout.alignment: Qt.AlignCenter
                    onCheckedChanged: {
                        week1Checked = checked
                        updateSelectAllState()
                    }
                }

                Switch {
                    text: "2"
                    checked: week2Checked
                    Layout.alignment: Qt.AlignCenter
                    onCheckedChanged: {
                        week2Checked = checked
                        updateSelectAllState()
                    }
                }

                Switch {
                    text: "3"
                    checked: week3Checked
                    Layout.alignment: Qt.AlignCenter
                    onCheckedChanged: {
                        week3Checked = checked
                        updateSelectAllState()
                    }
                }
            }

            // 第三排：4（周四）、5（周五）、6（周六）
            RowLayout {
                spacing: 20 // 与第二排保持一致间距，视觉统一
                Layout.fillWidth: true

                //Layout.alignment: Qt.AlignHCenter // 水平居中
                Switch {
                    text: "4"
                    checked: week4Checked
                    Layout.alignment: Qt.AlignCenter
                    onCheckedChanged: {
                        week4Checked = checked
                        updateSelectAllState()
                    }
                }

                Switch {
                    text: "5"
                    checked: week5Checked
                    Layout.alignment: Qt.AlignCenter
                    onCheckedChanged: {
                        week5Checked = checked
                        updateSelectAllState()
                    }
                }

                Switch {
                    text: "6"
                    checked: week6Checked
                    Layout.alignment: Qt.AlignCenter
                    onCheckedChanged: {
                        week6Checked = checked
                        updateSelectAllState()
                    }
                }
            }

            // 第一排：每天（全选） + 7（周日）
            RowLayout {
                spacing: 20 // 两个开关之间的水平间距
                Layout.fillWidth: true // 填充宽度，方便居中

                //Layout.alignment: Qt.AlignHCenter // 水平居中
                Switch {
                    text: "7"
                    checked: week7Checked
                    Layout.alignment: Qt.AlignCenter
                    onCheckedChanged: {
                        week7Checked = checked
                        updateSelectAllState()
                    }
                }

                Switch {
                    id: selectAllSwitch
                    text: qsTr("Everyday")
                    checked: selectAllDays
                    Layout.alignment: Qt.AlignCenter
                    visible: true
                    onCheckedChanged: {
                        selectAllDays = checked
                        if (checked) {
                            week1Checked = week2Checked = week3Checked = week4Checked
                                    = week5Checked = week6Checked = week7Checked = true
                        }
                    }
                }
            }
        }

        // 2. 日期时间选择器（保持宽度约束）
        DateTimePicker {
            id: dateTimePicker
            objectName: "dateTimePicker"
            visible: true
            Layout.fillWidth: true
            Layout.maximumWidth: parent.width - 10
            Layout.preferredWidth: parent.width - 10
            Layout.preferredHeight: 300 // 固定总高度，避免占用过多空间
            Layout.alignment: Qt.AlignHCenter
        }

        // 3. 语音控制区域
        RowLayout {
            Layout.fillWidth: true
            Layout.maximumWidth: parent.width - 20
            Layout.preferredWidth: parent.width - 20
            Layout.preferredHeight: 40
            spacing: 8

            Switch {
                id: voiceSwitch
                text: qsTr("TTS Voice")
                checked: voiceBroadcastEnabled
                Layout.alignment: Qt.AlignVCenter
                onCheckedChanged: {
                    voiceBroadcastEnabled = checked
                    m_Todo.setChkVoice(checked)
                    console.log("Voice " + (checked ? "enabled" : "disabled"))
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                id: testVoiceBtn
                text: qsTr("Test")
                onClicked: {
                    console.log("Test voice clicked")
                }
                padding: 8
                Layout.alignment: Qt.AlignVCenter
            }
        }

        // 4. 操作按钮区域
        RowLayout {
            spacing: 12
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBottom

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: qsTr("Close")
                onClicked: {
                    m_Todo.isTodoAlarmShow = false
                    setTodoAlarm.close()
                }
                padding: 8
                Layout.maximumWidth: Math.min(implicitWidth,
                                              mainContainer.width * 0.3)
            }

            Button {
                text: qsTr("Del Alarm")
                onClicked: {
                    /* 删除逻辑 */
                    m_Todo.on_DelAlarm()

                    m_Todo.isTodoAlarmShow = false
                    setTodoAlarm.close()
                }
                padding: 8
                Layout.maximumWidth: Math.min(implicitWidth,
                                              mainContainer.width * 0.3)
            }

            Button {
                text: qsTr("Set Alarm")
                onClicked: {
                    /* 设置逻辑 */
                    m_Todo.on_SetAlarm(week1Checked, week2Checked,
                                       week3Checked, week4Checked,
                                       week5Checked, week6Checked,
                                       week7Checked,
                                       dateTimePicker.currentYear,
                                       dateTimePicker.currentMonth,
                                       dateTimePicker.currentDay,
                                       dateTimePicker.currentHour,
                                       dateTimePicker.currentMinute)

                    m_Todo.isTodoAlarmShow = false
                    setTodoAlarm.close()
                }
                padding: 8
                Layout.maximumWidth: Math.min(implicitWidth,
                                              mainContainer.width * 0.3)
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }

    // 联动更新"每天"开关状态
    function updateSelectAllState() {
        selectAllDays = week1Checked && week2Checked && week3Checked
                && week4Checked && week5Checked && week6Checked && week7Checked
    }
}
