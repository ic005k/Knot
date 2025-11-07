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

        // 禁用默认提供商
        PluginParameter {
            name: "osm.mapping.providersrepository.disabled"
            value: true
        }

        // 高清配置
        PluginParameter {
            name: "osm.mapping.highdpi_tiles"
            value: true
        }
        PluginParameter {
            name: "osm.mapping.tile.size"
            value: 512
        }

        // 直接指定URL模板（推荐）
        PluginParameter {
            name: "osm.mapping.custom.url"
            value: "https://tile.thunderforest.com/cycle/{z}/{x}/{y}@2x.png?apikey=5ad09b54d1e542909f4f20f3a01786ae"
        }

        // User-Agent
        PluginParameter {
            name: "user-agent"
            value: "YourApp/1.0"
        }

        // 可选：缓存设置
        /*PluginParameter {
            name: "osm.mapping.cache.directory"
            value: "./map_cache"
        }*/

        PluginParameter {
            name: "osm.mapping.cache.size"
            value: "102400" // 100MB
        }
    }

    Plugin {
        id: osmPlugin_test
        name: "osm"

        // 禁用远程仓库，使用本地配置
        PluginParameter {
            name: "osm.mapping.providersrepository.disabled"
            value: true
        }
        PluginParameter {
            name: "osm.mapping.providersrepository.address"
            value: "/res/providers.json" // 完整路径 qrc://
        }

        // 高清参数
        PluginParameter {
            name: "osm.mapping.highdpi_tiles"
            value: true
        }
        PluginParameter {
            name: "osm.mapping.tile.size"
            value: 512
        }
        PluginParameter {
            name: "user-agent"
            value: "Knot/1.0 (QtLocation)"
        }
        PluginParameter {
            name: "osm.mapping.offline.directory"
            value: "./map_cache"
        }
    }

    // 使用 Item 作为手势容器
    Item {
        id: mapContainer
        anchors.fill: parent

        // 核心手势处理器
        DragHandler {
            id: dragHandler
            target: null
            acceptedDevices: PointerDevice.TouchScreen | PointerDevice.Mouse
            grabPermissions: PointerHandler.ApprovesTakeOverByHandlersOfDifferentType

            onTranslationChanged: delta => {
                                      if (!map.dragging)
                                      return
                                      map.pan(-delta.x, -delta.y)
                                  }
        }

        PinchHandler {
            id: pinchHandler
            target: null
            acceptedDevices: PointerDevice.TouchScreen
            grabPermissions: PointerHandler.ApprovesTakeOverByHandlersOfDifferentType

            // 跟踪上次缩放比例
            property real lastScale: 1.0
            property bool activeZoom: false

            onActiveChanged: {
                if (active) {
                    map.dragging = false
                    activeZoom = true
                    map.startCentroid = map.toCoordinate(centroid.position)
                    lastScale = scale // 初始化上次缩放值
                } else {
                    activeZoom = false
                    map.dragging = true
                }
            }

            onScaleChanged: {
                if (!map.startCentroid.isValid)
                    return

                // 计算从上次变化以来的缩放增量
                const scaleDelta = scale / lastScale
                map.zoomLevel += Math.log2(scaleDelta)

                // 更新地图并保持中心点
                map.alignCoordinateToPoint(map.startCentroid, centroid.position)

                // 保存当前缩放值用于下次计算
                lastScale = scale
            }
        }

        // 直接使用 Map 而不是 MapView
        Map {
            id: map
            anchors.fill: parent
            plugin: osmPlugin
            center: QtPositioning.coordinate(gpsx, gpsy) // 初始中心坐标（奥斯陆）
            zoomLevel: 13
            focus: true

            activeMapType: supportedMapTypes[1] // Cycle map provided by Thunderforest

            // 确保选择正确的map type
            Component.onCompleted: {
                console.log("Available map types:", supportedMapTypes.length)
                for (var i = 0; i < supportedMapTypes.length; i++) {
                    console.log(i + ":", supportedMapTypes[i].name)
                }
            }

            // 控制状态变量
            property bool dragging: true
            property geoCoordinate startCentroid

            // 鼠标滚轮缩放 (桌面端)
            WheelHandler {
                id: wheelHandler
                acceptedDevices: PointerDevice.Mouse
                rotationScale: 1 / 120

                onWheel: event => {
                             const zoomFactor = event.angleDelta.y > 0 ? 1 : -1
                             map.zoomLevel += zoomFactor * 0.2

                             // 限制缩放范围
                             if (map.zoomLevel > map.maximumZoomLevel)
                             map.zoomLevel = map.maximumZoomLevel
                             if (map.zoomLevel < map.minimumZoomLevel)
                             map.zoomLevel = map.minimumZoomLevel
                         }
            }

            // 地图元素保持不变...
            MapPolyline {
                id: polyline
                line.color: "red"
                line.width: 3
            }

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
                font.pointSize: 18
            }
            Text {
                id: infoText2
                text: qsTr("Speed") + ": 0 km/h"
                color: "white"
                font.pointSize: 18
            }
        }
    }
}
