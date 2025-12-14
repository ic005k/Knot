import QtQuick 6.2
import QtQuick.Controls 6.2
import QtQuick.Layouts 6.2

Rectangle {
    id: mainContainer
    objectName: "setTodoAlarmComponent"
    color: isDark ? "#222222" : "#f9f9f9"
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

    // 主布局
    ColumnLayout {
        id: mainLayout
        spacing: 5
        anchors.fill: parent
        anchors.margins: 5
        Layout.maximumWidth: parent.width
        Layout.preferredWidth: parent.width

        // 1. 周选择区域（核心修改：Switch+Text组合）
        ColumnLayout {
            spacing: 3 // 排与排之间的间距略增大，避免拥挤

            Layout.fillWidth: true
            //Layout.preferredHeight: 50 // 适当增加高度，避免内容挤压
            //Layout.minimumHeight: 120  // 适配三行开关+标题的最小高度

            Text {
                text: qsTr("Select by Week")
                font.pixelSize: 16
                font.bold: true
                color: isDark ? "#EEEEEE" : "#333"
                Layout.alignment: Qt.AlignLeft
            }

            // 第一排：1（周一）、2（周二）、3（周三）
            RowLayout {
                spacing: 5 // 组与组之间的间距（均匀分布）
                Layout.fillWidth: true
                //Layout.alignment: Qt.AlignHCenter  // 整排居中

                // 周一：Switch+Text组合
                RowLayout {
                    spacing: 4 // 开关与文本的紧凑间距
                    Layout.alignment: Qt.AlignCenter

                    Switch {
                        id: weekSwitch1
                        checked: week1Checked // 无text属性
                        onCheckedChanged: {
                            week1Checked = checked
                            updateSelectAllState()
                        }
                    }
                    Text {
                        text: "1"
                        color: isDark ? "#EEEEEE" : "#333" // 文本颜色可控
                        font.pixelSize: 14
                        Layout.alignment: Qt.AlignVCenter // 与开关垂直居中
                    }
                }

                // 周二：Switch+Text组合
                RowLayout {
                    spacing: 4
                    Layout.alignment: Qt.AlignCenter

                    Switch {
                        checked: week2Checked
                        onCheckedChanged: {
                            week2Checked = checked
                            updateSelectAllState()
                        }
                    }
                    Text {
                        text: "2"
                        color: isDark ? "#EEEEEE" : "#333"
                        font.pixelSize: 14
                        Layout.alignment: Qt.AlignVCenter
                    }
                }

                // 周三：Switch+Text组合
                RowLayout {
                    spacing: 4
                    Layout.alignment: Qt.AlignCenter

                    Switch {
                        checked: week3Checked
                        onCheckedChanged: {
                            week3Checked = checked
                            updateSelectAllState()
                        }
                    }
                    Text {
                        text: "3"
                        color: isDark ? "#EEEEEE" : "#333"
                        font.pixelSize: 14
                        Layout.alignment: Qt.AlignVCenter
                    }
                }
            }

            // 第二排：4（周四）、5（周五）、6（周六）
            RowLayout {
                spacing: 5 // 与上一排保持一致间距
                Layout.fillWidth: true
                //Layout.alignment: Qt.AlignHCenter

                // 周四
                RowLayout {
                    spacing: 4
                    Layout.alignment: Qt.AlignCenter

                    Switch {
                        checked: week4Checked
                        onCheckedChanged: {
                            week4Checked = checked
                            updateSelectAllState()
                        }
                    }
                    Text {
                        text: "4"
                        color: isDark ? "#EEEEEE" : "#333"
                        font.pixelSize: 14
                        Layout.alignment: Qt.AlignVCenter
                    }
                }

                // 周五
                RowLayout {
                    spacing: 4
                    Layout.alignment: Qt.AlignCenter

                    Switch {
                        checked: week5Checked
                        onCheckedChanged: {
                            week5Checked = checked
                            updateSelectAllState()
                        }
                    }
                    Text {
                        text: "5"
                        color: isDark ? "#EEEEEE" : "#333"
                        font.pixelSize: 14
                        Layout.alignment: Qt.AlignVCenter
                    }
                }

                // 周六
                RowLayout {
                    spacing: 4
                    Layout.alignment: Qt.AlignCenter

                    Switch {
                        checked: week6Checked
                        onCheckedChanged: {
                            week6Checked = checked
                            updateSelectAllState()
                        }
                    }
                    Text {
                        text: "6"
                        color: isDark ? "#EEEEEE" : "#333"
                        font.pixelSize: 14
                        Layout.alignment: Qt.AlignVCenter
                    }
                }
            }

            // 第三排：7（周日）、Everyday（全选）
            RowLayout {
                spacing: 5 // 与前两排保持一致
                Layout.fillWidth: true
                //Layout.alignment: Qt.AlignHCenter

                // 周日
                RowLayout {
                    spacing: 4
                    Layout.alignment: Qt.AlignCenter

                    Switch {
                        checked: week7Checked
                        onCheckedChanged: {
                            week7Checked = checked
                            updateSelectAllState()
                        }
                    }
                    Text {
                        text: "7"
                        color: isDark ? "#EEEEEE" : "#333"
                        font.pixelSize: 14
                        Layout.alignment: Qt.AlignVCenter
                    }
                }

                // 全选（Everyday）
                RowLayout {
                    spacing: 4
                    Layout.alignment: Qt.AlignCenter

                    Switch {
                        id: selectAllSwitch
                        checked: selectAllDays
                        onCheckedChanged: {
                            selectAllDays = checked
                            if (checked) {
                                week1Checked = week2Checked = week3Checked = week4Checked
                                        = week5Checked = week6Checked = week7Checked = true
                            }
                        }
                    }
                    Text {
                        text: qsTr("Everyday")
                        color: isDark ? "#EEEEEE" : "#333"
                        font.pixelSize: 14
                        Layout.alignment: Qt.AlignVCenter
                    }
                }
            }
        }

        // 2. 日期时间选择器
        DateTimePicker {
            id: dateTimePicker
            objectName: "dateTimePicker"
            visible: true
            Layout.fillWidth: true
            Layout.maximumWidth: parent.width - 10
            Layout.preferredWidth: parent.width - 10
            // Layout.preferredHeight: 300
            Layout.fillHeight: true // 高度填充剩余空间（核心）
            Layout.alignment: Qt.AlignHCenter
        }

        // 3. 语音控制区域
        RowLayout {
            Layout.fillWidth: true
            Layout.maximumWidth: parent.width - 20
            Layout.preferredWidth: parent.width - 20
            Layout.preferredHeight: 40
            spacing: 8

            // 语音开关：同样改为Switch+Text组合
            RowLayout {
                spacing: 4
                Layout.alignment: Qt.AlignVCenter

                Switch {
                    id: voiceSwitch
                    checked: voiceBroadcastEnabled
                    onCheckedChanged: {
                        voiceBroadcastEnabled = checked
                        m_Todo.setChkVoice(checked)
                        console.log("Voice " + (checked ? "enabled" : "disabled"))
                    }
                }
                Text {
                    text: qsTr("TTS Voice")
                    color: isDark ? "#EEEEEE" : "#333"
                    font.pixelSize: 14
                    Layout.alignment: Qt.AlignVCenter
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                id: testVoiceBtn
                text: qsTr("Test")
                onClicked: {
                    m_Todo.on_btnTestSpeech()
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
                text: qsTr("Back")
                onClicked: {

                    closeTodoAlarm()
                }
                padding: 8
                Layout.maximumWidth: Math.min(implicitWidth,
                                              mainContainer.width * 0.3)
            }

            Button {
                text: qsTr("Del Alarm")
                onClicked: {
                    m_Todo.on_DelAlarm()

                    closeTodoAlarm()
                }
                padding: 8
                Layout.maximumWidth: Math.min(implicitWidth,
                                              mainContainer.width * 0.3)
            }

            Button {
                text: qsTr("Set Alarm")
                onClicked: {
                    m_Todo.on_SetAlarm(week1Checked, week2Checked,
                                       week3Checked, week4Checked,
                                       week5Checked, week6Checked,
                                       week7Checked,
                                       dateTimePicker.currentYear,
                                       dateTimePicker.currentMonth,
                                       dateTimePicker.currentDay,
                                       dateTimePicker.currentHour,
                                       dateTimePicker.currentMinute)

                    closeTodoAlarm()
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

    // 联动更新"每天"开关状态（保持不变）
    function updateSelectAllState() {
        selectAllDays = week1Checked && week2Checked && week3Checked
                && week4Checked && week5Checked && week6Checked && week7Checked
    }
}
