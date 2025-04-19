import QtQuick 2.9
import QtQuick.Window 2.2

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

        MouseArea {
            id: mapDragArea
            anchors.fill: parent
            drag.target: mapImg

            onDoubleClicked: {
                zoomin()
                zoomin()
                zoomin()
            }

            //这里使图片不管是比显示框大还是比显示框小都不会被拖拽出显示区域


            /*drag.minimumX: (mapImg.width > mapItemArea.width) ? (mapItemArea.width
                                                                 - mapImg.width) : 0
            drag.minimumY: (mapImg.height
                            > mapItemArea.height) ? (mapItemArea.height - mapImg.height) : 0
            drag.maximumX: (mapImg.width
                            > mapItemArea.width) ? 0 : (mapItemArea.width - mapImg.width)
            drag.maximumY: (mapImg.height
                            > mapItemArea.height) ? 0 : (mapItemArea.height - mapImg.height)*/
            onClicked: {

                // console.log(mouse.x + "  " + mouse.y)
            }

            //使用鼠标滚轮缩放
            onWheel: {
                //每次滚动都是120的倍数
                var datla = wheel.angleDelta.y / 120
                if (datla > 0) {
                    mapImg.scale = mapImg.scale / 0.9
                } else {
                    mapImg.scale = mapImg.scale * 0.9
                }
            }
        }

        PinchArea {
            anchors.fill: parent
            pinch.target: mapImg
            pinch.maximumScale: 1000
            pinch.minimumScale: 0.00001
            pinch.dragAxis: Pinch.XAndYAxis
        }
    }
}
