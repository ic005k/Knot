import QtQuick
import QtCharts

import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Fusion

ChartView {
    title: qsTr("Bar Chart")
    anchors.fill: parent
    legend.alignment: Qt.AlignBottom
    antialiasing: true

    BarSeries {
        id: mySeries
        axisX: BarCategoryAxis { categories: ["1", "2", "3", "4", "5", "6","7","8","9" ] }
        BarSet { label: qsTr("Freq"); values: [2, 2, 3, 4, 5, 6] }
        BarSet { label: qsTr("Amount"); values: [5, 1, 2, 4, 1, 7] }

    }
}

