import QtQuick 6.2
import QtQuick.Controls 6.2
import QtQuick.Layouts 6.2

Rectangle {
    id: pickerRoot
    width: 340
    height: 320  // 高度减小以适应移除按钮后的布局
    color: "#f5f5f7"
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
    property int startYear: 2022  // 起始年份（可自定义）
    property int yearRange: 25    // 前后年份范围（共显示50年）

    function getDaysInMonth(year, month) {
        return new Date(year, month, 0).getDate()
    }

    // 更新天数（当年份或月份变化时调用）
    function updateDayCount() {
        const maxDay = getDaysInMonth(currentYear, currentMonth)
        if (currentDay > maxDay) currentDay = maxDay
        dayTumbler.model = Array.from({length: maxDay}, (_, i) => i + 1)
        dayTumbler.currentIndex = currentDay - 1
    }

    // 日期标签
    Text {
        id: dateLabel
        text: "日期"
        font.pixelSize: 18  // 增大字体
        font.bold: true
        color: "#000000"
        anchors.top: parent.top
        anchors.topMargin: 15
        anchors.horizontalCenter: parent.horizontalCenter
    }

    // 时间标签
    Text {
        id: timeLabel
        text: "时间"
        font.pixelSize: 18  // 增大字体
        font.bold: true
        color: "#000000"
        anchors.top: datePickerArea.bottom
        anchors.topMargin: 15
        anchors.horizontalCenter: parent.horizontalCenter
    }

    // 日期选择区域
    Item {
        id: datePickerArea
        width: parent.width - 40
        height: 100
        anchors.top: dateLabel.bottom
        anchors.topMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter

        // 选中指示器
        Rectangle {
            width: parent.width
            height: 45  // 增大高度以适应更大字体
            color: "#f0f5ff"
            radius: 6
            anchors.verticalCenter: parent.verticalCenter
        }

        RowLayout {
            anchors.fill: parent
            spacing: 0

            // 年选择器
            Tumbler {
                id: yearTumbler
                Layout.fillHeight: true
                Layout.preferredWidth: parent.width / 3
                wrap: true // 启用循环滚动
                visibleItemCount: 3

                // 模型：从startYear-yearRange到startYear+yearRange
                model: {
                    var years = [];
                    for (var i = startYear - yearRange; i <= startYear + yearRange; i++) {
                        years.push(i);
                    }
                    return years;
                }

                // 初始位置设置为当前年份
                currentIndex: model.indexOf(currentYear)

                delegate: Text {
                    width: yearTumbler.width
                    height: 45  // 增大高度以适应更大字体
                    text: modelData
                    font.pixelSize: 22  // 增大字体
                    color: Tumbler.tumbler.currentIndex === index ? "#007aff" : "#999999"
                    font.bold: Tumbler.tumbler.currentIndex === index
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    opacity: Math.abs(Tumbler.displacement) < 1.5 ? 1.0 : 0.3
                }

                // 更新当前年份
                onCurrentIndexChanged: {
                    if (currentIndex >= 0 && currentIndex < model.length) {
                        currentYear = model[currentIndex];
                        updateDayCount();
                    }
                }
            }

            // 月选择器
            Tumbler {
                id: monthTumbler
                Layout.fillHeight: true
                Layout.preferredWidth: parent.width / 3
                wrap: true // 启用循环滚动
                visibleItemCount: 3

                model: Array.from({length: 12}, (_, i) => i + 1)
                currentIndex: currentMonth - 1

                delegate: Text {
                    width: monthTumbler.width
                    height: 45  // 增大高度以适应更大字体
                    text: modelData
                    font.pixelSize: 22  // 增大字体
                    color: Tumbler.tumbler.currentIndex === index ? "#007aff" : "#999999"
                    font.bold: Tumbler.tumbler.currentIndex === index
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    opacity: Math.abs(Tumbler.displacement) < 1.5 ? 1.0 : 0.3
                }

                onCurrentIndexChanged: {
                    if (currentIndex >= 0 && currentIndex < model.length) {
                        currentMonth = model[currentIndex];
                        updateDayCount();
                    }
                }
            }

            // 日选择器
            Tumbler {
                id: dayTumbler
                Layout.fillHeight: true
                Layout.preferredWidth: parent.width / 3
                wrap: true // 启用循环滚动
                visibleItemCount: 3

                model: Array.from({length: getDaysInMonth(currentYear, currentMonth)}, (_, i) => i + 1)
                currentIndex: currentDay - 1

                delegate: Text {
                    width: dayTumbler.width
                    height: 45  // 增大高度以适应更大字体
                    text: modelData
                    font.pixelSize: 22  // 增大字体
                    color: Tumbler.tumbler.currentIndex === index ? "#007aff" : "#999999"
                    font.bold: Tumbler.tumbler.currentIndex === index
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    opacity: Math.abs(Tumbler.displacement) < 1.5 ? 1.0 : 0.3
                }

                onCurrentIndexChanged: {
                    if (currentIndex >= 0 && currentIndex < model.length) {
                        currentDay = model[currentIndex];
                    }
                }
            }
        }
    }

    // 分隔线
    Rectangle {
        width: parent.width - 40
        height: 1
        color: "#e0e0e0"
        anchors.top: datePickerArea.bottom
        anchors.topMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter
    }

    // 时间选择区域
    Item {
        id: timePickerArea
        width: parent.width - 40
        height: 100
        anchors.top: timeLabel.bottom
        anchors.topMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter

        // 选中指示器
        Rectangle {
            width: parent.width
            height: 45  // 增大高度以适应更大字体
            color: "#f0f5ff"
            radius: 6
            anchors.verticalCenter: parent.verticalCenter
        }

        RowLayout {
            anchors.fill: parent
            spacing: 0

            // 时选择器
            Tumbler {
                id: hourTumbler
                Layout.fillHeight: true
                Layout.preferredWidth: parent.width / 2.5
                wrap: true // 启用循环滚动
                visibleItemCount: 3

                model: Array.from({length: 24}, (_, i) => i)
                currentIndex: currentHour

                delegate: Text {
                    width: hourTumbler.width
                    height: 45  // 增大高度以适应更大字体
                    text: modelData
                    font.pixelSize: 22  // 增大字体
                    color: Tumbler.tumbler.currentIndex === index ? "#007aff" : "#999999"
                    font.bold: Tumbler.tumbler.currentIndex === index
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    opacity: Math.abs(Tumbler.displacement) < 1.5 ? 1.0 : 0.3
                }

                onCurrentIndexChanged: {
                    if (currentIndex >= 0 && currentIndex < model.length) {
                        currentHour = model[currentIndex];
                    }
                }
            }

            Text {
                Layout.alignment: Qt.AlignCenter
                text: ":"
                font.pixelSize: 26  // 增大字体
                color: "#666666"
                font.bold: true
            }

            // 分选择器
            Tumbler {
                id: minuteTumbler
                Layout.fillHeight: true
                Layout.preferredWidth: parent.width / 2.5
                wrap: true // 启用循环滚动
                visibleItemCount: 3

                model: Array.from({length: 60}, (_, i) => i.toString().padStart(2, "0"))
                currentIndex: currentMinute

                delegate: Text {
                    width: minuteTumbler.width
                    height: 45  // 增大高度以适应更大字体
                    text: modelData
                    font.pixelSize: 22  // 增大字体
                    color: Tumbler.tumbler.currentIndex === index ? "#007aff" : "#999999"
                    font.bold: Tumbler.tumbler.currentIndex === index
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    opacity: Math.abs(Tumbler.displacement) < 1.5 ? 1.0 : 0.3
                }

                onCurrentIndexChanged: {
                    if (currentIndex >= 0 && currentIndex < model.length) {
                        currentMinute = currentIndex;
                    }
                }
            }
        }
    }

    // 监听年份和月份变化，更新天数
    onCurrentYearChanged: updateDayCount()
    onCurrentMonthChanged: updateDayCount()

    // 组件完成时初始化
    Component.onCompleted: {
        // 设置初始年份位置
        const years = yearTumbler.model;
        const yearIndex = years.indexOf(currentYear);
        if (yearIndex >= 0) {
            yearTumbler.currentIndex = yearIndex;
        }

        // 设置初始月份位置
        monthTumbler.currentIndex = currentMonth - 1;

        // 设置初始日期位置
        dayTumbler.currentIndex = currentDay - 1;

        // 设置初始小时位置
        hourTumbler.currentIndex = currentHour;

        // 设置初始分钟位置
        minuteTumbler.currentIndex = currentMinute;
    }
}
