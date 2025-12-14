import QtQuick 6.2
import QtQuick.Controls 6.2
import QtQuick.Layouts 6.2

Rectangle {
    id: pickerRoot
    width: parent.width
    height: parent.height
    color: isDark ? "#333333" : "#f5f5f7"
    radius: 16
    border.width: 1
    border.color: "#e0e0e0"
    clip: true

    // 内部状态属性
    property int systemCurrentYear: new Date().getFullYear()
    property int currentYear: systemCurrentYear
    property int currentMonth: new Date().getMonth() + 1
    property int currentDay: new Date().getDate()
    property int currentHour: new Date().getHours()
    property int currentMinute: new Date().getMinutes()

    // 对外暴露的选中结果
    property alias selectedYear: pickerRoot.currentYear
    property alias selectedMonth: pickerRoot.currentMonth
    property alias selectedDay: pickerRoot.currentDay
    property alias selectedHour: pickerRoot.currentHour
    property alias selectedMinute: pickerRoot.currentMinute

    // 年份范围设置
    property int startYear: 2022
    property int yearRange: 15

    function getDaysInMonth(year, month) {
        return new Date(year, month, 0).getDate()
    }

    function updateDayCount() {
        const maxDay = getDaysInMonth(currentYear, currentMonth)
        if (currentDay > maxDay)
            currentDay = maxDay
        dayTumbler.model = Array.from({
                                          "length": maxDay
                                      }, (_, i) => i + 1)
        dayTumbler.currentIndex = currentDay - 1
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 10

        // 日期标签
        Text {
            text: qsTr("Date")
            font.pixelSize: 18
            font.bold: true
            color: isDark ? "#EEEEEE" : "#000000"
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 30
        }

        // 日期选择区域
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 80
            Layout.preferredHeight: parent.height * 0.35
            spacing: 0

            // 年选择器
            Tumbler {
                id: yearTumbler
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: parent.width / 3 + 50
                wrap: true
                visibleItemCount: 3  // Tumbler的visibleItemCount属性
                model: {
                    var years = []
                    var endYear = systemCurrentYear + yearRange
                    for (var i = startYear; i <= endYear; i++) {
                        years.push(i)
                    }
                    return years
                }
                currentIndex: model.indexOf(currentYear)

                delegate: Text {
                    width: yearTumbler.width
                    // 修复：通过Tumbler.tumbler访问visibleItemCount
                    height: Tumbler.tumbler.height / Tumbler.tumbler.visibleItemCount
                    text: modelData
                    font.pixelSize: yearTumbler.height * 0.25
                    color: Tumbler.tumbler.currentIndex === index ? "#007aff" : "#999999"
                    font.bold: Tumbler.tumbler.currentIndex === index
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    opacity: Math.abs(Tumbler.displacement) < 1.5 ? 1.0 : 0.3
                }

                onCurrentIndexChanged: {
                    if (currentIndex >= 0 && currentIndex < model.length) {
                        currentYear = model[currentIndex]
                        updateDayCount()
                    }
                }
            }

            // 月选择器
            Tumbler {
                id: monthTumbler
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: parent.width / 3 - 25
                wrap: true
                visibleItemCount: 3
                model: Array.from({
                                      "length": 12
                                  }, (_, i) => i + 1)
                currentIndex: currentMonth - 1

                delegate: Text {
                    width: monthTumbler.width
                    // 修复：通过Tumbler.tumbler访问visibleItemCount
                    height: Tumbler.tumbler.height / Tumbler.tumbler.visibleItemCount
                    text: modelData
                    font.pixelSize: monthTumbler.height * 0.25
                    color: Tumbler.tumbler.currentIndex === index ? "#007aff" : "#999999"
                    font.bold: Tumbler.tumbler.currentIndex === index
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    opacity: Math.abs(Tumbler.displacement) < 1.5 ? 1.0 : 0.3
                }

                onCurrentIndexChanged: {
                    if (currentIndex >= 0 && currentIndex < model.length) {
                        currentMonth = model[currentIndex]
                        updateDayCount()
                    }
                }
            }

            // 日选择器
            Tumbler {
                id: dayTumbler
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: parent.width / 3 - 25
                wrap: true
                visibleItemCount: 3
                model: Array.from({
                                      "length": getDaysInMonth(currentYear,
                                                               currentMonth)
                                  }, (_, i) => i + 1)
                currentIndex: currentDay - 1

                delegate: Text {
                    width: dayTumbler.width
                    // 修复：通过Tumbler.tumbler访问visibleItemCount
                    height: Tumbler.tumbler.height / Tumbler.tumbler.visibleItemCount
                    text: modelData
                    font.pixelSize: dayTumbler.height * 0.25
                    color: Tumbler.tumbler.currentIndex === index ? "#007aff" : "#999999"
                    font.bold: Tumbler.tumbler.currentIndex === index
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    opacity: Math.abs(Tumbler.displacement) < 1.5 ? 1.0 : 0.3
                }

                onCurrentIndexChanged: {
                    if (currentIndex >= 0 && currentIndex < model.length) {
                        currentDay = model[currentIndex]
                    }
                }
            }
        }

        // 分隔线
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            Layout.maximumWidth: parent.width - 40
            Layout.alignment: Qt.AlignHCenter
            color: "#e0e0e0"
        }

        // 时间标签
        Text {
            text: qsTr("Time")
            font.pixelSize: 18
            font.bold: true
            color: isDark ? "#EEEEEE" : "#000000"
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 30
        }

        // 时间选择区域
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 80
            Layout.preferredHeight: parent.height * 0.35
            spacing: 0

            // 时选择器
            Tumbler {
                id: hourTumbler
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: parent.width * 0.4
                wrap: true
                visibleItemCount: 3
                model: Array.from({
                                      "length": 24
                                  }, (_, i) => i)
                currentIndex: currentHour

                delegate: Text {
                    width: hourTumbler.width
                    // 修复：通过Tumbler.tumbler访问visibleItemCount
                    height: Tumbler.tumbler.height / Tumbler.tumbler.visibleItemCount
                    text: modelData.toString().padStart(2, "0")
                    font.pixelSize: hourTumbler.height * 0.25
                    color: Tumbler.tumbler.currentIndex === index ? "#007aff" : "#999999"
                    font.bold: Tumbler.tumbler.currentIndex === index
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    opacity: Math.abs(Tumbler.displacement) < 1.5 ? 1.0 : 0.3
                }

                onCurrentIndexChanged: {
                    if (currentIndex >= 0 && currentIndex < model.length) {
                        currentHour = model[currentIndex]
                    }
                }
            }

            // 冒号分隔符
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: parent.width * 0.2

                Text {
                    anchors.centerIn: parent
                    font.pixelSize: parent.height * 0.3
                    text: ":"
                    color: "#666666"
                    font.bold: true
                }
            }

            // 分选择器
            Tumbler {
                id: minuteTumbler
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: parent.width * 0.4
                wrap: true
                visibleItemCount: 3
                model: Array.from({
                                      "length": 60
                                  }, (_, i) => i.toString().padStart(2, "0"))
                currentIndex: currentMinute

                delegate: Text {
                    width: minuteTumbler.width
                    // 修复：通过Tumbler.tumbler访问visibleItemCount
                    height: Tumbler.tumbler.height / Tumbler.tumbler.visibleItemCount
                    text: modelData
                    font.pixelSize: minuteTumbler.height * 0.25
                    color: Tumbler.tumbler.currentIndex === index ? "#007aff" : "#999999"
                    font.bold: Tumbler.tumbler.currentIndex === index
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    opacity: Math.abs(Tumbler.displacement) < 1.5 ? 1.0 : 0.3
                }

                onCurrentIndexChanged: {
                    if (currentIndex >= 0 && currentIndex < model.length) {
                        currentMinute = currentIndex
                    }
                }
            }
        }
    }

    // 监听逻辑保持不变
    onCurrentYearChanged: {
        const yearIndex = yearTumbler.model.indexOf(currentYear)
        if (yearIndex !== -1) {
            yearTumbler.currentIndex = yearIndex
        }
    }

    onCurrentMonthChanged: {
        const monthIndex = currentMonth - 1
        if (monthIndex >= 0 && monthIndex < 12) {
            monthTumbler.currentIndex = monthIndex
        }
        updateDayCount()
    }

    onCurrentDayChanged: {
        const dayIndex = currentDay - 1
        if (dayTumbler.model && dayIndex >= 0 && dayIndex < dayTumbler.model.length) {
            dayTumbler.currentIndex = dayIndex
        } else {
            console.log("dayTumbler.model未初始化或dayIndex无效，currentDay:", currentDay)
        }
    }

    onCurrentHourChanged: {
        if (currentHour >= 0 && currentHour < 24) {
            hourTumbler.currentIndex = currentHour
        }
    }

    onCurrentMinuteChanged: {
        if (currentMinute >= 0 && currentMinute < 60) {
            minuteTumbler.currentIndex = currentMinute
        }
    }

    Component.onCompleted: {
        const years = yearTumbler.model
        const yearIndex = years.indexOf(currentYear)
        if (yearIndex >= 0) {
            yearTumbler.currentIndex = yearIndex
        }

        monthTumbler.currentIndex = currentMonth - 1
        dayTumbler.currentIndex = currentDay - 1
        hourTumbler.currentIndex = currentHour
        minuteTumbler.currentIndex = currentMinute
    }
}
