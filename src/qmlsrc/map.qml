import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtLocation
import QtPositioning

Rectangle {
    visible: true
    width: 800
    height: 600

    // 变量定义
    property double gpsx: 59.91032691205
    property double gpsy: 10.75763249129
    property point lastDragPos: Qt.point(0, 0)

    focus: true

    // 添加焦点控制逻辑
    Component.onCompleted: {
        forceActiveFocus()
        Qt.callLater(() => map.forceActiveFocus()) // 延迟确保焦点生效
    }

    function updateInfoText(strDistance, strSpeed) {
        infoText1.text = qsTr("Distance") + ": " + strDistance
        infoText2.text = qsTr("Speed") + ": " + strSpeed
    }

    function appendTrack(lat, lon) {
        gpsx = lat
        gpsy = lon

        var newCoordinate = QtPositioning.coordinate(lat, lon)

        //方法1（在线程中执行效率太低）
        //let pathArray = polyline.path
        //pathArray.push(newCoordinate) // 添加新的点
        //polyline.path = pathArray

        //方法2（推荐）
        polyline.addCoordinate(QtPositioning.coordinate(lat, lon))

        map.center = newCoordinate
    }

    function updateTrackData(lat, lon) {
        gpsx = lat
        gpsy = lon

        polyline.addCoordinate(QtPositioning.coordinate(lat, lon))
    }

    function updateMapTrackUi(lastLat, lastLon) {

        map.center = QtPositioning.coordinate(lastLat, lastLon)
        console.log("update track...")
    }

    function clearTrack() {
        polyline.path = []
    }

    Plugin {
        id: osmPlugin
        name: "osm"

        // 确保使用Qt6兼容的参数
        PluginParameter {
            name: "osm.mapping.input.touch.enabled"
            value: "true"
        }
        PluginParameter {
            name: "osm.mapping.input.mouse.enabled"
            value: "true"
        }

        PluginParameter {
            name: "osm.mapping.providersrepository.address"
            value: "https://maps-redirect.qt.io/osm/5.15/"
        }

        // 必须指定渲染类型
        PluginParameter {
            name: "osm.mapping.providersrepository.disabled"
            value: "false"
        }
        PluginParameter {
            name: "osm.mapping.providersrepository.address"
            value: "http://maps-redirect.qt.io/osm/5.8/"
        }

        // 明确使用矢量渲染
        PluginParameter {
            name: "osm.mapping.highdpi_tiles"
            value: true
        }
        PluginParameter {
            name: "osm.mapping.offline.directory"
            value: "./cache"
        }

        PluginParameter {
            name: "osm.mapping.highdpi_tiles"
            value: true
        }
    }

    MapView {
        id: mapView
        anchors.fill: parent

        MouseArea {
            anchors.fill: parent
            propagateComposedEvents: true
            preventStealing: false

            onPressed: mouse => {
                           console.log("Mouse pressed at:", mouse.x, mouse.y)
                           mouse.accepted = false // 关键：允许事件继续传递
                       }
            onPositionChanged: mouse => mouse.accepted = false
            onReleased: mouse => mouse.accepted = false
        }

        Map {
            id: map
            anchors.fill: parent
            plugin: osmPlugin
            center: QtPositioning.coordinate(gpsx, gpsy) // 初始中心坐标（奥斯陆）
            zoomLevel: 13
            focus: true

            activeMapType: supportedMapTypes[1] // Cycle map provided by Thunderforest

            // 确保焦点设置生效
            Component.onCompleted: {
                forceActiveFocus()
            }

            TapHandler {
                acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchScreen
                onDoubleTapped: (eventPoint, button) => {
                                    if (button === Qt.LeftButton) {
                                        zoomLevel += 0.5
                                        eventPoint.accepted = true
                                    }
                                }
            }

            // 替换原有的 DragHandler，改用手动计算偏移
            MouseArea {
                anchors.fill: parent
                preventStealing: true

                onPressed: mouse => {
                               lastDragPos = Qt.point(mouse.x, mouse.y)
                               map.forceActiveFocus()
                           }

                onPositionChanged: mouse => {
                                       if (pressed) {
                                           // 计算像素偏移量
                                           var dx = mouse.x - lastDragPos.x
                                           var dy = mouse.y - lastDragPos.y

                                           // 转换为地图坐标偏移（核心逻辑）
                                           var centerPixel = map.fromCoordinate(
                                               map.center, false)
                                           var newCenterPixel = Qt.point(
                                               centerPixel.x - dx,
                                               centerPixel.y - dy)
                                           var newCenterCoord = map.toCoordinate(
                                               newCenterPixel, false)

                                           // 更新地图中心
                                           map.center = newCenterCoord
                                           lastDragPos = Qt.point(mouse.x,
                                                                  mouse.y)
                                       }
                                   }
            }

            // 核心：确保 WheelHandler 和 DragHandler 互不干扰
            WheelHandler {
                id: wheelZoom
                acceptedDevices: PointerDevice.Mouse
                property: "zoomLevel" //绑定到缩放
                rotationScale: 0.1 //调低灵敏度
                // 仅当没有拖拽时才处理滚轮
                enabled: !dragHandler.active
            }

            DragHandler {
                id: dragHandler
                target: null
                onActiveChanged: map.forceActiveFocus()
            }

            MapPolyline {
                id: polyline
                line.color: "red"
                line.width: 3
            }

            // 可选：添加一个标记来表示当前位置
            MapQuickItem {
                coordinate: QtPositioning.coordinate(gpsx, gpsy)
                anchorPoint.x: markerImage.width / 2 + 6
                anchorPoint.y: markerImage.height / 2
                sourceItem: Image {
                    id: markerImage
                    source: "/res/marker.png"
                    width: 32
                    height: 32
                }
            }
        }
    }

    PositionSource {
        id: positionSource
        active: false

        onPositionChanged: {
            var coord = positionSource.position.coordinate
            if (coord.isValid) {
                // 更新地图中心
                map.center = coord

                // 添加新坐标到轨迹
                polyline.addCoordinate(coord)
            }
        }
    }

    // 运动信息显示区域
    Rectangle {
        id: infoArea
        anchors {
            left: parent.left
            bottom: parent.bottom
            leftMargin: 0
            bottomMargin: 0
        }
        width: parent.width
        height: 35
        color: "blue"
        radius: 0

        Row {
            anchors.centerIn: parent
            spacing: 5

            Text {
                id: infoText1
                text: qsTr("Distance") + ": 0 km"
                color: "white"
            }
            Text {
                id: infoText2
                text: qsTr("Speed") + ": 0 km/h"
                color: "white"
            }
        }
    }
}
