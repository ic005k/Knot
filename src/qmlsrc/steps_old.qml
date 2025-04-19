import QtQuick 2.13
import QtQuick.Controls 2.13
import Qt.labs.qmlmodels 1.0

Rectangle {
    width: myW

    color: isDark ? "#455364" : "white"

    function appendTableRow(Date, Steps, KM) {
        tableModel.appendRow({
                                 "Date": Date,
                                 "Steps": Steps,
                                 "KM": KM
                             })
    }

    function setScrollBarPos(pos) {
        //vbar.setPosition(pos)
        if (tableView.contentHeight > tableView.height)
            tableView.contentY = tableView.contentHeight - tableView.height
        console.log("contentH=" + tableView.contentHeight + "  h=" + tableView.height)
    }

    function getItemCount() {
        return tableModel.rowCount
    }

    function getDate(itemIndex) {
        var data = tableModel.getRow(itemIndex)
        return data.Date
    }

    function getSteps(itemIndex) {
        var data = tableModel.getRow(itemIndex)
        return data.Steps
    }

    function getKM(itemIndex) {
        var data = tableModel.getRow(itemIndex)
        return data.KM
    }

    function setTableData(currentIndex, date, steps, km) {
        tableModel.setRow(currentIndex, {
                              "Date": date,
                              "Steps": steps,
                              "KM": km
                          })
    }

    function delItem(currentIndex) {
        tableModel.removeRow(currentIndex)
    }

    Rectangle {
        id: header
        width: parent.width
        height: fontMetrics.height + 4

        FontMetrics {
            id: fontMetrics
        }

        Row {
            spacing: 0

            Repeater {
                // Table Header
                model: [qsTr("Date"), qsTr("Steps"), qsTr("KM")]

                Rectangle {
                    width: header.width / 3
                    height: header.height
                    color: isDark ? "#3498DB" : "#3498DB"
                    border.width: 1
                    border.color: isDark ? "#19232D" : "#F3F3F3"
                    Text {
                        text: modelData
                        anchors.centerIn: parent
                        //font.pointSize: 12
                        color: "white"
                    }
                }
            }
        }
    }
    TableView {
        id: tableView
        width: parent.width
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        clip: true
        boundsBehavior: Flickable.OvershootBounds

        ScrollBar.vertical: ScrollBar {
            id: vbar
            anchors.right: parent.right
            anchors.rightMargin: 0
            policy: ScrollBar.AsNeeded
            width: 8

            visible: tableView.contentHeight > tableView.height //tableModel.rowCount > 15
            background: Rectangle {
                color: isDark ? "#455364" : "lightgray"
            }

            // Always show

            /*onActiveChanged: {
                active = true
            }
            contentItem: Rectangle {
                implicitWidth: 6
                implicitHeight: 30
                radius: 3
                color: isDark ? "#455364" : "#848484"
            }*/
        }

        model: TableModel {
            id: tableModel

            TableModelColumn {
                display: "Date"
            }
            TableModelColumn {
                display: "Steps"
            }
            TableModelColumn {
                display: "KM"
            }
        }
        delegate: Rectangle {
            color: getSteps(
                       row) >= nStepsThreshold ? "#FF6A6A" : (isDark ? "#455364" : "lightgray")
            implicitWidth: header.width / 3
            implicitHeight: mytext.contentHeight + 6
            border.width: 1
            border.color: isDark ? "#19232D" : "#F3F3F3"

            Text {
                id: mytext
                text: display
                anchors.centerIn: parent

                color: isDark ? "white" : "black"
                font.pointSize: maxFontSize
            }
        }
    }

    Component.onCompleted: {
        appendTableRow("2022-11-19", "3500", 1.65)
    }
}
