import QtQuick 2.15
import QtQuick.Window 2.15
import QtQml 2.3
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.1

Rectangle {
    id: root

    width: 500
    height: 400

    property int itemCount: 0
    property bool isHighPriority: false

    function setItemHeight(h) {}

    function gotoEnd() {
        view.positionViewAtEnd()
    }

    function gotoBeginning() {
        view.positionViewAtBeginning()
    }

    function gotoIndex(index) {
        view.positionViewAtIndex(index, Tumbler.Center)
    }

    function setScrollBarPos(pos) {
        //vbar.setPosition(pos)
        if(view.contentHeight>view.height)
        view.contentY = view.contentHeight - view.height
        console.log("contentH=" + view.contentHeight + "  h=" + view.height)
    }

    function setHighPriority(isFalse) {
        isHighPriority = isFalse
    }

    function setCurrentItem(currentIndex) {
        view.currentIndex = currentIndex
    }

    function getCurrentIndex() {
        return view.currentIndex
    }

    function getItemCount() {
        itemCount = view.count
        console.log("count=" + itemCount)
        return itemCount
    }

    function getItemText(itemIndex) {
        var data = view.model.get(itemIndex)
        return data.time + "|=|" + data.dototext
    }

    function getText0(itemIndex) {
        var data = view.model.get(itemIndex)
        return data.text0
    }

    function getText1(itemIndex) {
        var data = view.model.get(itemIndex)
        return data.text1
    }

    function getText2(itemIndex) {
        var data = view.model.get(itemIndex)
        return data.text2
    }

    function getTop(itemIndex) {
        var data = view.model.get(itemIndex)
        return data.text_top
    }

    function getType(itemIndex) {
        var data = view.model.get(itemIndex)
        return data.type
    }

    function addItem(t0, t1, t2, type0, txt_top, height) {
        view.model.append({
                              "text0": t0,
                              "text1": t1,
                              "text2": t2,
                              "type": type0,
                              "text_top": txt_top,
                              "myh": height
                          })
    }

    function insertItem(strTime, type, strText, curIndex) {
        view.model.insert(curIndex, {
                              "time": strTime,
                              "type": type,
                              "dototext": strText
                          })
    }

    function delItem(currentIndex) {
        view.model.remove(currentIndex)
    }

    function modifyItem(currentIndex, strTime, strText) {

        view.model.setProperty(currentIndex, "time", strTime)
        view.model.setProperty(currentIndex, "dototext", strText)
    }

    function modifyItemTime(currentIndex, strTime) {

        view.model.setProperty(currentIndex, "time", strTime)
    }

    function modifyItemType(currentIndex, type) {

        view.model.setProperty(currentIndex, "type", type)
    }

    function modifyItemText(currentIndex, strText) {
        view.model.setProperty(currentIndex, "dototext", strText)
    }

    Component {
        id: dragDelegate

        Rectangle {
            id: listItem
            width: ListView.view.width
            height: myh
            color: ListView.isCurrentItem ? "lightblue" : "#ffffff" //选中颜色设置 #94caf7

            border.width: isDark ? 0 : 1
            border.color: "lightgray" //"lightsteelblue"

            radius: 0

            RowLayout {

                id: idlistElemnet
                height: parent.height
                width: parent.width
                spacing: 2
                Layout.fillWidth: true

                Rectangle {
                    height: parent.height - 2
                    width: 6
                    radius: 2
                    anchors.leftMargin: 1
                    color: getListEleHeadColor(type)

                    Text {
                        anchors.centerIn: parent
                    }
                }

                ColumnLayout {
                    id: idlistElemnet4
                    height: parent.height
                    width: parent.width
                    spacing: 2
                    Layout.fillWidth: true
                    anchors.leftMargin: 0
                    anchors.rightMargin: 0

                    Text {
                        id: item0

                        width: parent.width
                        Layout.preferredWidth: parent.width
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: TextArea.NoWrap
                        font.bold: type
                        text: text0
                    }

                    Text {
                        id: item1
                        Layout.preferredWidth: parent.width // / 4

                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignLeft //Text.AlignRight
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight

                        width: parent.width
                        wrapMode: TextArea.NoWrap
                        color: isHighPriority ? "#EF5B98" : "#000000"
                        font.bold: type
                        text: text1

                        visible: item1.text.length ? true : false
                    }

                    Text {
                        id: item2
                        Layout.preferredWidth: parent.width // / 3
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignLeft //Text.AlignRight
                        elide: Text.ElideRight

                        width: parent.width
                        wrapMode: TextArea.WordWrap
                        font.bold: type
                        text: text2
                        anchors.rightMargin: 10

                        visible: item2.text.length ? true : false
                    }

                    TextArea {
                        id: item_type
                        visible: false
                        width: parent.width
                        wrapMode: TextArea.NoWrap
                        text: type
                    }

                    Text {
                        id: item_top
                        visible: false
                        width: parent.width
                        wrapMode: TextArea.NoWrap
                        text: text_top
                    }
                }
            }

            MouseArea {

                property point clickPos: "0,0"

                anchors.fill: parent
                onPressed: {
                    clickPos = Qt.point(mouse.x, mouse.y)
                }
                onReleased: {
                    var delta = Qt.point(mouse.x - clickPos.x,
                                         mouse.y - clickPos.y)
                    console.debug("delta.x: " + delta.x)
                    if ((delta.x < 0) && (aBtnShow.running === false)
                            && (delBtn.width == 0)) {
                        aBtnShow.start()
                    } else if (aBtnHide.running === false
                               && (delBtn.width > 0)) {
                        aBtnHide.start()
                    }
                }

                onClicked: {
                    view.currentIndex = index //实现item切换
                    mw_one.clickData()
                }

                onDoubleClicked: {
                    mw_one.reeditData()
                    var data = view.model.get(view.currentIndex)
                    console.log(data.text0 + "," + data.type + ", count=" + view.count)
                }
            }

            Rectangle {
                color: "#AAAAAA"
                height: 0
                width: parent.width
                anchors.bottom: parent.bottom
            }

            Rectangle {
                id: delBtn
                visible: false
                height: parent.height
                width: 0
                color: "#FF0000"

                anchors.right: parent.right
                anchors.rightMargin: -30
                radius: 0

                Text {
                    width: 56
                    anchors.centerIn: parent

                    text: qsTr("Done")
                    color: "#ffffff"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        m_Todo.addToRecycle()
                        view.model.remove(index)
                    }
                }
            }

            PropertyAnimation {
                id: aBtnShow
                target: delBtn
                property: "width"
                duration: 100
                from: 0
                to: 80
            }
            PropertyAnimation {
                id: aBtnHide
                target: delBtn
                property: "width"
                duration: 100
                from: 80
                to: 0
            }
        }
    }

    ListView {
        id: view

        anchors {
            fill: parent
            margins: 4
        }

        model: MainModel {}
        delegate: dragDelegate

        spacing: 4
        cacheBuffer: 50

        ScrollBar.vertical: ScrollBar {
            width: 8
            policy: ScrollBar.AsNeeded
        }
    }

    function getListEleHeadColor(ntype) {
        switch (ntype) {
        case 0:
            return "lightgray"
        case 1:
            return "red"
        case 2:
            return "yellow"
        case 3:
            return "lightblue"
        default:
            return "black"
        }
    }
}
