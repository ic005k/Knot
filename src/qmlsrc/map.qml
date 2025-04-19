import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtLocation 5.15
import QtPositioning 5.15

Rectangle {
    visible: true
    width: 800
    height: 600

    // 变量定义
    property double gpsx: 59.91032691205
    property double gpsy: 10.75763249129

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

        PluginParameter {
            name: "osm.mapping.highdpi_tiles"
            value: true
        }
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: osmPlugin
        center: QtPositioning.coordinate(gpsx, gpsy) // 初始中心坐标（奥斯陆）
        zoomLevel: 13

        activeMapType: supportedMapTypes[1] // Cycle map provided by Thunderforest

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
