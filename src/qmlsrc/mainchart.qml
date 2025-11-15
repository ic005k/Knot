import QtQuick
import QtCharts
import QtQuick.Controls

Flickable {
    id: flickable
    anchors.fill: parent
    clip: true
    contentWidth: Math.max(width, chartCategories.length * 50)
    contentHeight: height

    // 只允许水平滚动
    flickableDirection: Flickable.HorizontalFlick
    boundsBehavior: Flickable.StopAtBounds

    // 添加水平滚动条
    ScrollBar.horizontal: ScrollBar {
        policy: ScrollBar.AlwaysOn
    }

    ChartView {
        id: chartView
        title: qsTr("Frequency + Amount Bar Chart")
        width: flickable.contentWidth
        height: flickable.height
        legend.alignment: Qt.AlignBottom
        antialiasing: true
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
        }

        ValueAxis {
            id: yAxis
            min: 0
            max: {
                var maxVal = 0;
                for (var i = 0; i < chartFreqValues.length; i++) {
                    maxVal = Math.max(maxVal, chartFreqValues[i] || 0);
                }
                for (var j = 0; j < chartAmountValues.length; j++) {
                    maxVal = Math.max(maxVal, chartAmountValues[j] || 0);
                }
                return Math.ceil(maxVal * 1.2);
            }
        }

        BarSeries {
            axisX: xAxis
            axisY: yAxis
            BarSet { label: qsTr("Freq"); values: chartFreqValues }
            BarSet { label: qsTr("Amount"); values: chartAmountValues }
        }
    }
}
