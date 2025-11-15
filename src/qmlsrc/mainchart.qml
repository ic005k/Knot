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

    // 从两个数组中计算最大值
    property real yAxisMax: {
        var maxVal = 0;
        // 遍历频率数组
        for (var i = 0; i < chartFreqValues.length; i++) {
            maxVal = Math.max(maxVal, chartFreqValues[i] || 0);
        }
        // 遍历金额数组
        for (var j = 0; j < chartAmountValues.length; j++) {
            maxVal = Math.max(maxVal, chartAmountValues[j] || 0);
        }
        return Math.ceil(maxVal * 1.2); // 增加20%的余量
    }

    // 预计算内容宽度
    readonly property real calculatedContentWidth: {
        var parentWidth = parent ? parent.width : 0;
        return Math.max(parentWidth, chartCategories.length * 50);
    }

    Flickable {
        id: flickable
        anchors.fill: parent
        clip: true
        contentWidth: calculatedContentWidth
        contentHeight: height

        flickableDirection: Flickable.HorizontalFlick
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.horizontal: ScrollBar {
            policy: ScrollBar.AlwaysOn
        }

        // 简化的点检测函数
        function pointInRect(rect, pointX, pointY) {
            return pointX >= rect.x && pointX <= rect.x + rect.width &&
                   pointY >= rect.y && pointY <= rect.y + rect.height;
        }

        // 预计算点击区域
        property var xAxisClickArea: Qt.rect(0, 0, 0, 0)
        property var plotArea: Qt.rect(0, 0, 0, 0)

        onWidthChanged: updateClickAreas()
        onHeightChanged: updateClickAreas()

        function updateClickAreas() {
            if (!chartView.plotArea) return;

            plotArea = Qt.rect(
                chartView.plotArea.x,
                chartView.plotArea.y,
                chartView.plotArea.width,
                chartView.plotArea.height
            );

            xAxisClickArea = Qt.rect(
                plotArea.x,
                plotArea.y + plotArea.height - 30,
                plotArea.width,
                30
            );
        }

        ChartView {
            id: chartView
            title: qsTr("Frequency + Amount Bar Chart")
            width: calculatedContentWidth
            height: flickable.height
            legend.alignment: Qt.AlignBottom
            antialiasing: true
            theme: isDark ? ChartView.ChartThemeDark : ChartView.ChartThemeLight
            backgroundColor: isDark ? "#1e1e1e" : "#ffffff"
            titleColor: isDark ? "#ffffff" : "#000000"
            animationOptions: ChartView.NoAnimation // 禁用动画提高性能

            margins {
                top: 10
                bottom: 10
                left: 10
                right: 10
            }

            onPlotAreaChanged: flickable.updateClickAreas()

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
                max: yAxisMax // 使用计算出的最大值
                color: isDark ? "#ffffff" : "#000000"
                labelsColor: isDark ? "#ffffff" : "#000000"
                gridLineColor: isDark ? "#404040" : "#d0d0d0"

                labelsFont.pixelSize: 10
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

        // 优化后的MouseArea
        MouseArea {
            id: chartMouseArea
            anchors.fill: chartView
            hoverEnabled: false // 禁用hover提高性能

            onClicked: function(mouse) {
                if (!flickable.pointInRect(flickable.xAxisClickArea, mouse.x, mouse.y) &&
                    !flickable.pointInRect(flickable.plotArea, mouse.x, mouse.y)) {
                    return;
                }

                var plotArea = flickable.plotArea;
                var categoryIndex = Math.floor(
                    (mouse.x - plotArea.x) / (plotArea.width / Math.max(1, chartCategories.length))
                );

                categoryIndex = Math.max(0, Math.min(categoryIndex, chartCategories.length - 1));

                if (categoryIndex >= 0 && categoryIndex < chartCategories.length) {
                    showTooltip(mouse.x, mouse.y, categoryIndex);
                }
            }

            function showTooltip(mouseX, mouseY, index) {
                var category = chartCategories[index];
                var freqValue = chartFreqValues[index] || 0;
                var amountValue = chartAmountValues[index] || 0;

                // 预构建文本
                tooltipText.text = qsTr("Date: ") + category + "\n"
                                  + qsTr("Freq: ") + freqValue.toFixed(2) + "\n"
                                  + qsTr("Amount: ") + amountValue.toFixed(2);

                // 计算位置
                var tooltipX = Math.max(10, Math.min(mouseX - tooltip.width / 2,
                    chartView.width - tooltip.width - 10));
                var tooltipY = Math.max(10, Math.min(mouseY - tooltip.height - 10,
                    chartView.height - tooltip.height - 10));

                tooltip.x = tooltipX;
                tooltip.y = tooltipY;
                tooltip.opacity = 1;
                hideTimer.restart();
            }
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
            z: 10

            Text {
                id: tooltipText
                anchors.centerIn: parent
                font.pixelSize: 15
                color: isDark ? "#ffffff" : "#333333"
            }

            Timer {
                id: hideTimer
                interval: 10000
                onTriggered: {
                    tooltip.opacity = 0
                }
            }

            Behavior on opacity {
                NumberAnimation { duration: 300 }
            }
        }
    }
}
