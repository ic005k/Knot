import QtQuick
import QtQuick.Window

Item {
    visible: true
    width: 1620
    height: 720

    function zoomin() {
        mapImg.scale = mapImg.scale / 0.9
    }

    function zoomout() {
        mapImg.scale = mapImg.scale * 0.9
    }

    Image {
        id: backimg
        width: myW
        height: myH
        fillMode: Image.Tile
        horizontalAlignment: Image.AlignLeft
        verticalAlignment: Image.AlignTop
        smooth: true
        source: "/res/b.png"
    }

    Flickable {
        id: mapItemArea
        width: imgW
        height: imgH
        anchors.centerIn: parent
        clip: true

        Image {
            id: mapImg
            fillMode: Image.PreserveAspectFit // 自适应并保持纵横比
            width: imgW / 10

            //这里使图片居中显示
            x: mapItemArea.width / 2 - mapImg.width / 2
            y: mapItemArea.height / 2 - mapImg.height / 2
            source: "file:///" + imgFile
            //图像异步加载，只对本地图像有用
            asynchronous: true
        }

        PinchArea {
            anchors.fill: parent
            pinch.target: mapImg
            pinch.maximumScale: 1000
            pinch.minimumScale: 0.00001
            pinch.dragAxis: Pinch.XAndYAxis

            // 添加 MouseArea 处理单指拖动
            MouseArea {
                id: dragArea
                anchors.fill: parent
                enabled: true

                // 拖动目标设置为图像
                drag.target: mapImg
                drag.axis: Drag.XAndYAxis

                // 动态计算拖动边界
                drag.minimumX: -mapImg.width * (mapImg.scale - 1) / 2
                drag.maximumX: parent.width - mapImg.width * (mapImg.scale + 1) / 2
                drag.minimumY: -mapImg.height * (mapImg.scale - 1) / 2
                drag.maximumY: parent.height - mapImg.height * (mapImg.scale + 1) / 2

                // 防止误触发点击事件
                onClicked: {

                }

                // 拖动时更新边界
                onPositionChanged: {
                    dragArea.drag.minimumX = -mapImg.width * (mapImg.scale - 1) / 2
                    dragArea.drag.maximumX = parent.width - mapImg.width * (mapImg.scale + 1) / 2
                    dragArea.drag.minimumY = -mapImg.height * (mapImg.scale - 1) / 2
                    dragArea.drag.maximumY = parent.height - mapImg.height * (mapImg.scale + 1) / 2
                }

                onDoubleClicked: {
                    zoomin()
                    zoomin()
                    zoomin()
                }

                //使用鼠标滚轮缩放
                onWheel: function (wheel) {
                    //每次滚动都是120的倍数
                    var datla = wheel.angleDelta.y / 120
                    if (datla > 0) {
                        mapImg.scale = mapImg.scale / 0.9
                    } else {
                        mapImg.scale = mapImg.scale * 0.9
                    }
                }
            }
        }
    }
    // 状态显示
    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 30
        color: "#80000000"

        Text {
            anchors.centerIn: parent
            color: "white"
            text: qsTr("Scale") + ": " + mapImg.scale.toFixed(2) + " | " + qsTr(
                      "Pos") + ": (" + mapImg.x.toFixed(0) + ", " + mapImg.y.toFixed(
                      0) + ")"
        }
    }
}
