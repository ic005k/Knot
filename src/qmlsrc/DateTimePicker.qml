import QtQuick 6.2
import QtQuick.Controls 6.2

Rectangle {
    id: pickerRoot
    width: parent.width - 0
    height: 320
    color: isDark ? "#333333" : "#f5f5f7"
    radius: 16
    border.width: 1
    border.color: "#e0e0e0"
    clip: true

    // 内部状态属性
    property int currentYear: new Date().getFullYear()
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

    Column {
        anchors.fill: parent
        spacing: 10
        topPadding: 15
        bottomPadding: 10

        // 日期标签
        Text {
            text: qsTr("Date")
            font.pixelSize: 18
            font.bold: true
            color: isDark ? "#EEEEEE" : "#000000"
            anchors.horizontalCenter: parent.horizontalCenter
        }

        // 日期选择区域 - 简洁布局
        Row {
            width: parent.width - 40
            height: 90
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 0

            // 年选择器
            Tumbler {
                id: yearTumbler
                width: parent.width / 3
                height: parent.height
                wrap: true
                visibleItemCount: 3
                model: {
                    var years = []
                    for (var i = startYear - yearRange; i <= startYear + yearRange; i++) {
                        years.push(i)
                    }
                    return years
                }
                currentIndex: model.indexOf(currentYear)

                delegate: Text {
                    width: yearTumbler.width
                    height: 40
                    text: modelData
                    font.pixelSize: 20
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
                width: parent.width / 3
                height: parent.height
                wrap: true
                visibleItemCount: 3
                model: Array.from({
                                      "length": 12
                                  }, (_, i) => i + 1)
                currentIndex: currentMonth - 1

                delegate: Text {
                    width: monthTumbler.width
                    height: 40
                    text: modelData
                    font.pixelSize: 20
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
                width: parent.width / 3
                height: parent.height
                wrap: true
                visibleItemCount: 3
                model: Array.from({
                                      "length": getDaysInMonth(currentYear,
                                                               currentMonth)
                                  }, (_, i) => i + 1)
                currentIndex: currentDay - 1

                delegate: Text {
                    width: dayTumbler.width
                    height: 40
                    text: modelData
                    font.pixelSize: 20
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
            width: parent.width - 40
            height: 1
            color: "#e0e0e0"
            anchors.horizontalCenter: parent.horizontalCenter
        }

        // 时间标签
        Text {
            text: qsTr("Time")
            font.pixelSize: 18
            font.bold: true
            color: isDark ? "#EEEEEE" : "#000000"
            anchors.horizontalCenter: parent.horizontalCenter
        }

        // 时间选择区域 - 简洁布局
        Row {
            width: parent.width - 40
            height: 90
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 0

            // 时选择器
            Tumbler {
                id: hourTumbler
                width: parent.width * 0.4
                height: parent.height
                wrap: true
                visibleItemCount: 3
                model: Array.from({
                                      "length": 24
                                  }, (_, i) => i)
                currentIndex: currentHour

                delegate: Text {
                    width: hourTumbler.width
                    height: 40
                    text: modelData.toString().padStart(2, "0")
                    font.pixelSize: 20
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
                width: parent.width * 0.2
                height: parent.height

                Text {
                    anchors.centerIn: parent
                    text: ":"
                    font.pixelSize: 24
                    color: "#666666"
                    font.bold: true
                }
            }

            // 分选择器
            Tumbler {
                id: minuteTumbler
                width: parent.width * 0.4
                height: parent.height
                wrap: true
                visibleItemCount: 3
                model: Array.from({
                                      "length": 60
                                  }, (_, i) => i.toString().padStart(2, "0"))
                currentIndex: currentMinute

                delegate: Text {
                    width: minuteTumbler.width
                    height: 40
                    text: modelData
                    font.pixelSize: 20
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

    // 监听年份和月份变化，更新天数
    //onCurrentYearChanged: updateDayCount()
    //onCurrentMonthChanged: updateDayCount()

    // 1. 年份变化时，同步更新年滚轮
    onCurrentYearChanged: {
        const yearIndex = yearTumbler.model.indexOf(currentYear)
        if (yearIndex !== -1) {
            // 确保索引有效
            yearTumbler.currentIndex = yearIndex
        }
    }

    // 2. 月份变化时，同步更新月滚轮，并刷新天数
    onCurrentMonthChanged: {
        const monthIndex = currentMonth - 1
        // 月份是1-12，滚轮索引是0-11
        if (monthIndex >= 0 && monthIndex < 12) {
            monthTumbler.currentIndex = monthIndex
        }
        updateDayCount() // 月份变了，重新计算当月天数
    }

    // 3. 日期变化时，同步更新日滚轮
    onCurrentDayChanged: {
        const dayIndex = currentDay - 1
        // 先检查model是否存在，再判断索引有效性
        if (dayTumbler.model && dayIndex >= 0
                && dayIndex < dayTumbler.model.length) {
            dayTumbler.currentIndex = dayIndex
        } else {
            // 可选：打印调试信息，确认问题场景
            console.log("dayTumbler.model未初始化或dayIndex无效，currentDay:",
                        currentDay)
        }
    }

    // 4. 小时变化时，同步更新时滚轮
    onCurrentHourChanged: {
        if (currentHour >= 0 && currentHour < 24) {
            hourTumbler.currentIndex = currentHour
        }
    }

    // 5. 分钟变化时，同步更新分滚轮
    onCurrentMinuteChanged: {
        if (currentMinute >= 0 && currentMinute < 60) {
            minuteTumbler.currentIndex = currentMinute
        }
    }

    // 组件完成时初始化
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
