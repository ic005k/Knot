import QtQuick
import QtQuick.Window
import QtQml
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Fusion

Rectangle {
    id: root

    width: 500
    height: 400

    color: isDark ? "#19232D" : "white"

    property int iconW: 20
    property int rowSpace: 3
    property int i: 0
    property int itemCount: 0
    property bool isHighPriority: false

    function setScrollBarPos(pos) {
        view.ScrollBar.vertical.position = 1.0 - view.ScrollBar.vertical.size;
    }

    function setItemHeight(h) {
    }

    function gotoEnd() {
        view.positionViewAtEnd();
    }

    function gotoBeginning() {
        view.positionViewAtBeginning();
    }

    function gotoIndex(index) {
        view.positionViewAtIndex(index, Tumbler.Center);
    }

    function setHighPriority(isFalse) {
        isHighPriority = isFalse;
    }

    function setCurrentItem(currentIndex) {
        view.currentIndex = currentIndex;
        view.positionViewAtIndex(currentIndex, ListView.Beginning);
    }

    function getCurrentIndex() {
        return view.currentIndex;
    }

    function getItemCount() {
        itemCount = view.count;

        return itemCount;
    }

    function getItemText(itemIndex) {
        var data = view.model.get(itemIndex);
        return data.time + "|=|" + data.dototext;
    }

    function getText0(itemIndex) {
        var data = view.model.get(itemIndex);
        return data.text0;
    }

    function getText1(itemIndex) {
        var data = view.model.get(itemIndex);
        return data.text1;
    }

    function getText2(itemIndex) {
        var data = view.model.get(itemIndex);
        return data.text2;
    }

    function getText3(itemIndex) {
        var data = view.model.get(itemIndex);
        return data.text3;
    }

    function getTop(itemIndex) {
        var data = view.model.get(itemIndex);
        return data.text_top;
    }

    function getType(itemIndex) {
        var data = view.model.get(itemIndex);
        return data.type;
    }

    /*function addItem(t0, t1, t2, t3, t4, f_size) {
        view.model.append({
            "text0": t0,
            "text1": t1,
            "text2": t2,
            "text3": t3,
            "text4": t4,
            "font_size": f_size
        });
    }*/

    function addItem(t0, t1, t2, t3, t4, f_size, lvl = 0, pIdx = -1, expand = true) {
        view.model.append({
            "text0": t0,
            "text1": t1,
            "text2": t2,
            "text3": t3,
            "text4": t4,
            "font_size": f_size,
            "level": lvl,
            "parentIndex": pIdx,
            "isExpand": expand
        });
    }

    function insertItem(text0, text1, text2, text3, curIndex) {
        view.model.insert(curIndex, {
            "text0": text0,
            "text1": text1,
            "text2": text2,
            "text3": text3
        });
    }

    function delItem(currentIndex) {
        view.model.remove(currentIndex);
    }

    function modifyItem(currentIndex, strTime, strText) {
        view.model.setProperty(currentIndex, "time", strTime);
        view.model.setProperty(currentIndex, "dototext", strText);
    }

    function modifyItemTime(currentIndex, strTime) {
        view.model.setProperty(currentIndex, "time", strTime);
    }

    function modifyItemType(currentIndex, type) {
        view.model.setProperty(currentIndex, "type", type);
    }

    function modifyItemText0(currentIndex, strText) {
        view.model.setProperty(currentIndex, "text0", strText);
    }

    function modifyItemText1(currentIndex, strText) {
        view.model.setProperty(currentIndex, "text1", strText);
    }

    function modifyItemText2(currentIndex, strText) {
        view.model.setProperty(currentIndex, "text2", strText);
    }

    function modifyItemText3(currentIndex, strText) {
        view.model.setProperty(currentIndex, "text3", strText);
    }

    function setVPos(vpos) {
        vbar.position = vpos;
        console.log("qwNoteBook:set " + vpos);
    }

    function getVPos() {
        var vpos = vbar.position;
        console.log("qwNoteBook:get " + vpos);
        return vpos;
    }

    function getColor() {
        var strColor;

        if (isDark)
            strColor = "#333333";
        else
            // "#455364"
            strColor = "#ffffff";

        return strColor;
    }

    function getFontColor() {
        if (isDark)
            return "#EEEEEE";
        else
            return "black";
    }

    function setColorFlag(strColor) {
        var currentIndex = view.currentIndex;
        if (currentIndex >= 0) {
            view.model.setProperty(currentIndex, "text4", strColor);
        }
    }

    Component {
        id: dragDelegate

        Rectangle {
            id: listItem
            implicitWidth: ListView.view.width
            implicitHeight: colLayout.implicitHeight
            color: getColor()
            border.width: isDark ? 0 : 1
            border.color: "lightgray"

            radius: 2

            // 缩进块
            Item {
                id: m_item
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                Layout.fillHeight: true

                x: level * 15
                width: parent.width - x - 0

                RowLayout {
                    id: idlistRow
                    anchors.fill: parent          // 关键：填满 m_item
                    spacing: 2

                    // 小矩形色块
                    Rectangle {
                        id: idrectColorFlag
                        //height: colLayout.implicitHeight - 0
                        Layout.fillHeight: true    // 随列高度变化
                        width: 6
                        radius: 2
                        anchors.leftMargin: 1
                        color: item4.text
                        visible: item2.text.length ? false : true
                        Text {
                            anchors.centerIn: parent
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        // 高亮背景
                        Rectangle {
                            anchors.fill: parent

                            color: listItem.ListView.isCurrentItem ? "lightblue" : "transparent"
                            radius: 2
                            z: -1
                        }

                        ColumnLayout {
                            id: colLayout
                            height: parent.height
                            width: parent.width          // 关键：显式绑定宽度
                            spacing: 2

                            Rectangle {
                                height: 1 // 空白高度
                                color: "transparent"
                            }

                            // 笔记本标题
                            Text {
                                id: item0
                                Layout.fillWidth: true   // 占满 ColumnLayout 宽度
                                Layout.alignment: Qt.AlignHCenter
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                                wrapMode: TextArea.WordWrap
                                font.bold: true
                                text: text0
                                //text: text0 + " | level:" + level
                                color: listItem.ListView.isCurrentItem ? "black" : getFontColor()

                                leftPadding: 5
                                rightPadding: 5
                            }

                            // 不可见
                            Text {
                                id: item1

                                text: text1

                                visible: false
                            }

                            // 不可见
                            Text {
                                id: item2

                                text: text2

                                visible: false
                            }

                            // 求和图标及文本
                            RowLayout {
                                id: row3
                                Layout.margins: rowSpace
                                visible: item3.text.length ? true : false

                                Image {
                                    id: item3Img

                                    width: item3.contentHeight - 5
                                    height: parent.height - 2
                                    fillMode: Image.NoOption
                                    horizontalAlignment: Image.AlignHCenter
                                    verticalAlignment: Image.AlignVCenter

                                    smooth: true
                                    sourceSize.height: item3.contentHeight - 5
                                    sourceSize.width: item3.contentHeight - 5
                                    source: "/res/sum.png"

                                    visible: item3.text.length ? true : false
                                }

                                Text {
                                    id: item3
                                    anchors.rightMargin: 0
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                    elide: Text.ElideRight
                                    Layout.alignment: Qt.AlignHCenter
                                    horizontalAlignment: Text.AlignLeft
                                    verticalAlignment: Text.AlignVCenter

                                    font.bold: false
                                    font.pointSize: font_size - 2
                                    color: listItem.ListView.isCurrentItem ? "black" : getFontColor()
                                    text: text3

                                    leftPadding: 5
                                    rightPadding: 5

                                    visible: item3.text.length ? true : false
                                }
                            }

                            // 记录色块的值（不可见）
                            Text {
                                id: item4

                                text: text4

                                visible: false
                            }

                            Rectangle {
                                height: 1 // 空白高度
                                color: "transparent"
                            }
                        }
                    }
                }

                TapHandler {
                    dragThreshold: 8

                    // 单击 → 和 onClicked 完全一样
                    onTapped: {
                        view.currentIndex = index;
                        m_NotesList.mouseClickNoteBook();
                    }

                    // 长按 → 和 onPressAndHold 完全一样
                    onLongPressed: {
                        menuTargetIndex = index;
                        // 在长按位置弹出菜单
                        //notebookMenu.popup();
                        m_NotesList.show_NoteBookPopMenu(index);
                    }

                    // 双击 → 和 onDoubleClicked 完全一样
                    onDoubleTapped: {
                        // mw_one.on_btnRename_clicked()
                    }
                }
            }
        }
    }

    ListView {
        id: view

        boundsBehavior: Flickable.StopAtBounds // 禁止滚动到边界外的弹性效果

        anchors {
            fill: parent
            margins: 4
        }

        model: ListModel {
            id: listmain
        }
        delegate: dragDelegate

        spacing: 4

        cacheBuffer: 50

        // 滚动条
        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
            width: 8
        }
    }

    // 全局右键/长按菜单
    // 记录当前长按的条目索引
    property int menuTargetIndex: -1

    Menu {
        id: notebookMenu
        modal: true

        MenuItem {
            text: qsTr("New Sub Notebook")
            onClicked: {
                // 把当前长按的条目索引发给 C++
                m_NotesList.slotCreateSubNotebook(menuTargetIndex);
                notebookMenu.close();
            }
        }

        // 后续可以继续加：重命名、删除等菜单项
    }
}
