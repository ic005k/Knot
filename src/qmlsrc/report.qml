import QtQuick 2.15
import QtQuick.Controls 2.15
import Qt.labs.qmlmodels 1.0

Rectangle {

    color: isDark ? "#455364" : "#666666"

    function appendTableRow(Date, Steps, KM) {

        tableModel.appendRow({
                                 "Date": Date,
                                 "Freq": Steps,
                                 "Amount": KM
                             })
    }

    function setScrollBarPos(pos) {

        tableView.contentY = 0 //tableView.contentHeight - tableView.height
        console.log("contentH=" + tableView.contentHeight + "  h=" + tableView.height)
    }

    function getCurrentIndex() {
        return tableView.currentIndex
    }

    function getItemCount() {

        return tableModel.rowCount
    }

    function getDate(itemIndex) {
        return tableModel.rows[itemIndex].Date
    }

    function getSteps(itemIndex) {
        var data = tableModel.getRow(itemIndex)
        return data.Steps
    }

    function getKM(itemIndex) {
        var data = tableModel.getRow(itemIndex)
        return data.KM
    }

    function setCurrentItem(currentIndex) {
        tableView.currentIndex = currentIndex
    }

    function setTableData(currentIndex, date, freq, amount) {
        tableModel.setRow(currentIndex, {
                              "Date": date,
                              "Freq": freq,
                              "Amount": amount
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
                model: [qsTr("Date"), qsTr("Freq"), qsTr("Amount")]

                Rectangle {
                    width: header.width / 3
                    height: header.height

                    //color:  isDark ?  "#3498DB" : "#3498DB"
                    color: isDark ? "#455364" : "#666666"
                    border.width: 1
                    border.color: isDark ? "#19232D" : "#848484"
                    Text {
                        id: m_text
                        text: modelData
                        anchors.centerIn: parent
                        font.pointSize: maxFontSize
                        color: "white"
                    }
                }
            }
        }
    }

    TableView {
        property int hoverIndex: -1
        property int currentIndex: -1
        id: tableView
        width: parent.width
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        clip: true
        boundsBehavior: Flickable.OvershootBounds

        // 核心配置：绑定滚动状态
        property bool isScrolling: false
        onMovementStarted: isScrolling = true
        onMovementEnded: isScrolling = false

        ScrollBar.vertical: ScrollBar {
            id: vbar
            policy: ScrollBar.AsNeeded
            interactive: false // 关键！禁止拖动操作
            width: 4

            // 动态显隐控制
            visible: opacity > 0
            opacity: tableView.isScrolling ? 1 : 0
            Behavior on opacity {
                NumberAnimation {
                    duration: 300
                }
            }

            // 极简样式
            contentItem: Rectangle {
                color: isDark ? "#3498db" : "#606060"
                opacity: vbar.active ? (isDark ? 0.8 : 0.7) : 0
                Behavior on opacity {
                    NumberAnimation {
                        duration: 200 // 更流畅的动画
                        easing.type: Easing.OutQuad
                    }
                }
                radius: 3
            }
            background: null // 彻底消除背景容器
        }

        // 自动隐藏触发器（关键）
        Timer {
            running: !tableView.isScrolling && vbar.opacity > 0
            interval: 800
            onTriggered: vbar.opacity = 0
        }

        model: TableModel {
            id: tableModel

            TableModelColumn {
                display: "Date"
            }
            TableModelColumn {
                display: "Freq"
            }
            TableModelColumn {
                display: "Amount"
            }
        }
        delegate: Rectangle {
            //color: "#666666"
            color: {
                //tableView.currentIndex === row ? "#3298FE" : (tableView.hoverIndex === row ? "#97CBFF" : (row % 2 ? "#666666" : "#666666"))
                tableView.currentIndex === row ? "#3298FE" : (isDark ? "#455364" : (row % 2 ? "#666666" : "#666666"))
            }

            implicitWidth: tableView.width / 3
            implicitHeight: mytext.contentHeight + 6
            border.width: 1
            border.color: isDark ? "#19232D" : "#848484"

            Text {
                id: mytext
                width: tableView.width / 3 - 15
                text: display
                anchors.centerIn: parent

                color: "white"
                font.pointSize: maxFontSize

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: TextArea.WrapAnywhere
            }

            MouseArea {
                id: cashierMouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    tableView.forceActiveFocus()
                    tableView.currentIndex = row

                    m_Report.loadDetailsQml()

                    //console.debug(row)
                    //console.log(tableModel.rows[row].Date)
                }
                onDoubleClicked: {
                    tableView.selected(row, header.logicIndexMap[column])
                }
                onEntered: tableView.hoverIndex = row
                onExited: tableView.hoverIndex = -1
            }
        }
    }

    Component.onCompleted: {

        //appendTableRow("2022-11-19", "3500", "test1")
    }
}
