import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Fusion

Rectangle {
    id: speedometer
    width: 300
    height: 150
    color: "transparent"

    // 速度表属性
    property double myMaxSpeed: 0
    property int maxSpeed: myMaxSpeed <= 10 ? 10 : myMaxSpeed // 适合骑行的最大速度
    property double currentSpeed: 0
    property int minSpeed: 0
    property color backgroundColor: "#222222"
    property color foregroundColor: "#ffffff"
    property color accentColor: "#4CAF50"
    property color needleColor: "#FF5722"

    // 计算进度百分比
    property real progress: (currentSpeed - minSpeed) / (maxSpeed - minSpeed)

    // 速度表背景
    Rectangle {
        id: background
        width: parent.width
        height: parent.height
        radius: 0
        color: backgroundColor
        border.color: foregroundColor
        border.width: 2

        // 速度数字显示
        Text {
            id: speedText
            text: currentSpeed + " km/h"
            color: foregroundColor
            font.pixelSize: 40
            font.bold: true
            width: parent.width

            y: 0

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        // 进度条
        Rectangle {
            id: progressBar
            width: (background.width - 20) * progress
            height: 20
            radius: 2
            color: accentColor
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20
            anchors.leftMargin: 10

            // 进度条动画
            Behavior on width {
                NumberAnimation {
                    duration: 500
                    easing.type: Easing.InOutQuad
                }
            }
        }

        // 刻度线
        Repeater {
            model: 11 // 0到10共11个刻度

            Rectangle {
                id: tickLine
                width: 2
                height: 15
                color: foregroundColor
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 15
                x: 10 + (index * (parent.width - 20) / 10)
            }
        }

        // 刻度文本
        Repeater {
            model: 11 // 0到10共11个刻度

            Text {
                id: tickText
                text: Math.round(
                          minSpeed + (index * (maxSpeed - minSpeed) / 10))
                color: foregroundColor
                font.pixelSize: 12
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 2

                // 使用TextMetrics计算文本宽度
                TextMetrics {
                    id: metrics
                    font.family: tickText.font.family
                    font.pixelSize: tickText.font.pixelSize
                    text: tickText.text
                }

                // 改进文本位置计算，避免被边界遮挡
                x: {
                    // 使用TextMetrics获取文本宽度
                    var textWidth = metrics.width
                    // 计算理论位置
                    var pos = 10 + (index * (parent.width - 20) / 10)

                    // 特殊处理：如果是最后一个刻度，确保它不会被右侧边界遮挡
                    if (index === 10) {
                        return Math.max(10, pos - textWidth / 2 - 0)
                    }

                    // 特殊处理：如果是第一个刻度，确保它不会被左侧边界遮挡
                    if (index === 0) {
                        return Math.min(parent.width - 10 - textWidth,
                                        pos - textWidth / 2 + 0)
                    }

                    // 确保文本不会超出边界
                    return Math.max(10, Math.min(parent.width - 10 - textWidth,
                                                 pos - textWidth / 2))
                }
            }
        }

        // 指针
        Rectangle {
            id: needle
            width: 4
            height: 30
            color: needleColor
            radius: 2
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 15
            x: 10 + (progress * (parent.width - 20))

            // 指针动画
            Behavior on x {
                NumberAnimation {
                    duration: 500
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }
}
