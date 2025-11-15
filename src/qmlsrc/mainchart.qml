import QtQuick
import QtCharts
import QtQuick.Controls

Item {
    id: root
    anchors.fill: parent

    // 设置背景颜色以覆盖父窗口
    Rectangle {
        anchors.fill: parent
        color: isDark ? "#1e1e1e" : "#ffffff"
    }

    // 检查点是否在矩形内的辅助函数 - 移到根作用域
    function pointInRect(rect, pointX, pointY) {
        return pointX >= rect.x && pointX <= rect.x + rect.width
                && pointY >= rect.y && pointY <= rect.y + rect.height
    }

    Flickable {
        id: flickable
        anchors.fill: parent
        clip: true
        contentWidth: Math.max(width, chartCategories.length * 50)
        contentHeight: height

        // 只允许水平滚动
        flickableDirection: Flickable.HorizontalFlick
        boundsBehavior: Flickable.StopAtBounds

        // 简化滚动条设置，避免样式自定义问题
        ScrollBar.horizontal: ScrollBar {
            policy: ScrollBar.AlwaysOn
        }

        // 提示框组件
        Rectangle {
            id: tooltip
            width: tooltipText.contentWidth + 20
            height: tooltipText.contentHeight + 10
            color: isDark ? "#2d2d2d" : "#E0F0FF"
            border.color: isDark ? "#555555" : "#A0C0E0"
            border.width: 1
            radius: 4
            opacity: 0
            visible: opacity > 0
            z: 10 // 确保提示框在最上层

            Text {
                id: tooltipText
                anchors.centerIn: parent
                font.pixelSize: 12
                color: isDark ? "#ffffff" : "#333333"
            }

            // 自动隐藏计时器
            Timer {
                id: hideTimer
                interval: 10000 // 10秒
                onTriggered: {
                    tooltip.opacity = 0
                }
            }

            Behavior on opacity {
                NumberAnimation {
                    duration: 300
                }
            }
        }

        ChartView {
            id: chartView
            title: qsTr("Frequency + Amount Bar Chart")
            width: flickable.contentWidth
            height: flickable.height
            legend.alignment: Qt.AlignBottom
            antialiasing: true
            theme: isDark ? ChartView.ChartThemeDark : ChartView.ChartThemeLight
            backgroundColor: isDark ? "#1e1e1e" : "#ffffff"
            titleColor: isDark ? "#ffffff" : "#000000"

            // interactive: false
            margins {
                top: 10
                bottom: 40
                left: 60
                right: 20
            }

            BarCategoryAxis {
                id: xAxis
                categories: chartCategories
                labelsAngle: -45
                labelsFont.pixelSize: 10
                color: isDark ? "#ffffff" : "#000000"
                labelsColor: isDark ? "#ffffff" : "#000000"
                gridLineColor: isDark ? "#404040" : "#d0d0d0"
            }

            ValueAxis {
                id: yAxis
                min: 0
                max: {
                    var maxVal = 0
                    for (var i = 0; i < chartFreqValues.length; i++) {
                        maxVal = Math.max(maxVal, chartFreqValues[i] || 0)
                    }
                    for (var j = 0; j < chartAmountValues.length; j++) {
                        maxVal = Math.max(maxVal, chartAmountValues[j] || 0)
                    }
                    return Math.ceil(maxVal * 1.2)
                }
                color: isDark ? "#ffffff" : "#000000"
                labelsColor: isDark ? "#ffffff" : "#000000"
                gridLineColor: isDark ? "#404040" : "#d0d0d0"
            }

            BarSeries {
                id: barSeries
                axisX: xAxis
                axisY: yAxis
                BarSet {
                    id: freqBarSet
                    label: qsTr("Freq")
                    values: chartFreqValues
                    borderColor: isDark ? "#ffffff" : "#000000"
                }
                BarSet {
                    id: amountBarSet
                    label: qsTr("Amount")
                    values: chartAmountValues
                    borderColor: isDark ? "#ffffff" : "#000000"
                }
            }
        }

        // 覆盖在图表上的透明区域，用于检测x轴区域的点击
        MouseArea {
            anchors.fill: chartView
            onClicked: function (mouse) {
                // 获取图表的绘图区域（排除边距）
                var plotArea = chartView.plotArea

                // 定义x轴标签区域（图表底部区域）
                var xAxisClickArea = {
                    "x": plotArea.x,
                    "y": plotArea.y + plotArea.height - 30,
                    "width"// x轴标签区域高度约为30像素
                    : plotArea.width,
                    "height": 30
                }

                // 定义整个绘图区域（用于检测垂直区域的点击）
                var xAxisVerticalArea = {
                    "x": plotArea.x,
                    "y": plotArea.y,
                    "width": plotArea.width,
                    "height": plotArea.height
                }

                // 使用自定义函数检查点击位置
                if (root.pointInRect(xAxisClickArea, mouse.x, mouse.y)
                        || root.pointInRect(xAxisVerticalArea, mouse.x,
                                            mouse.y)) {

                    // 计算点击位置对应的类别索引
                    var categoryIndex = Math.floor(
                                (mouse.x - plotArea.x) / (plotArea.width / chartCategories.length))

                    // 确保索引在有效范围内
                    categoryIndex = Math.max(0, Math.min(
                                                 categoryIndex,
                                                 chartCategories.length - 1))

                    if (categoryIndex >= 0
                            && categoryIndex < chartCategories.length) {
                        // 获取对应类别的数据
                        var category = chartCategories[categoryIndex]
                        var freqValue = chartFreqValues[categoryIndex] || 0
                        var amountValue = chartAmountValues[categoryIndex] || 0

                        // 设置提示框文本
                        tooltipText.text = qsTr("Date: ") + category + "\n"
                                + qsTr("Freq: ") + freqValue.toFixed(2) + "\n" + qsTr(
                                    "Amount: ") + amountValue.toFixed(2)

                        // 设置提示框位置（在点击位置附近显示）
                        var tooltipX = mouse.x - tooltip.width / 2
                        var tooltipY = mouse.y - tooltip.height - 10

                        // 确保提示框不超出边界
                        tooltipX = Math.max(
                                    10, Math.min(
                                        tooltipX,
                                        chartView.width - tooltip.width - 10))
                        tooltipY = Math.max(
                                    10, Math.min(
                                        tooltipY,
                                        chartView.height - tooltip.height - 10))

                        tooltip.x = tooltipX
                        tooltip.y = tooltipY

                        // 显示提示框
                        tooltip.opacity = 1

                        // 启动自动隐藏计时器
                        hideTimer.restart()
                    }
                }
            }
        }
    }
}
