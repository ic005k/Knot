import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Fusion

Rectangle {
    id: root

    color: isDark ? "#19232D" : "white"

    property int i: 0
    property int itemCount: 0
    property int isFlagToday: 1
    property bool isHighPriority: false
    property int maintabHeight: 50

    readonly property int columns: 3
    readonly property real spacing: 8
    readonly property real cardWidth: (width - spacing * (columns)) / columns

    function setItemHeight(h) {
    }
    function gotoEnd() {
        grid.positionViewAtEnd();
    }
    function gotoBeginning() {
        grid.positionViewAtBeginning();
    }
    function gotoIndex(index) {
        grid.currentIndex = index;
    }
    function setHighPriority(isFalse) {
        isHighPriority = isFalse;
    }
    function setCurrentItem(currentIndex) {
        grid.currentIndex = currentIndex;
    }
    function getCurrentIndex() {
        return grid.currentIndex;
    }
    function getItemCount() {
        return grid.count;
    }

    function getItemText(itemIndex) {
        var data = grid.model.get(itemIndex);
        return data.time + "|=|" + data.dototext;
    }

    function getText0(itemIndex) {
        return grid.model.get(itemIndex).text0;
    }
    function getText1(itemIndex) {
        return grid.model.get(itemIndex).text1;
    }
    function getText2(itemIndex) {
        return grid.model.get(itemIndex).text2;
    }
    function getText3(itemIndex) {
        return grid.model.get(itemIndex).text3;
    }
    function getTop(itemIndex) {
        return grid.model.get(itemIndex).text_top;
    }
    function getType(itemIndex) {
        return grid.model.get(itemIndex).type;
    }

    function addItem(t0, t1, t2, t3, height) {
        isFlagToday = height;
        grid.model.append({
            "text0": t0,
            "text1": t1,
            "text2": t2,
            "text3": t3,
            "isFlagToday": height
        });
    }

    function insertItem(text0, text1, text2, text3, curIndex) {
        grid.model.insert(curIndex, {
            "text0": text0,
            "text1": text1,
            "text2": text2,
            "text3": text3
        });
    }

    function delItem(currentIndex) {
        grid.model.remove(currentIndex);
    }

    function modifyItem(currentIndex, strTime, strText) {
        grid.model.setProperty(currentIndex, "time", strTime);
        grid.model.setProperty(currentIndex, "dototext", strText);
    }
    function modifyItemTime(currentIndex, strTime) {
        grid.model.setProperty(currentIndex, "time", strTime);
    }
    function modifyItemType(currentIndex, type) {
        grid.model.setProperty(currentIndex, "type", type);
    }
    function modifyItemText0(currentIndex, strText) {
        grid.model.setProperty(currentIndex, "text0", strText);
    }
    function modifyItemText2(currentIndex, strText) {
        grid.model.setProperty(currentIndex, "text2", strText);
    }

    function getColor() {
        return isDark ? "#455364" : "#ffffff";
    }
    function getFontColor() {
        return isDark ? "white" : "black";
    }

    Component {
        id: gridDelegate
        Rectangle {
            width: cardWidth
            height: 75
            radius: 10
            clip: true

            // ======================
            // ✅ 核心：根据 isFlagToday 高亮标识
            // ======================
            color: {
                if (isFlagToday === 1) {
                    // 今天标识：浅橙色/主题色
                    if (isDark)
                        GridView.isCurrentItem ? "#4FC3F7" : "#3C2A10";
                    else
                        // 暗黑今日 深色底
                        GridView.isCurrentItem ? "#4FC3F7" : "#FFE0B2";   // 白天今日 橙色底
                } else {
                    GridView.isCurrentItem ? "#4FC3F7" : getColor();
                }
            }

            border.width: isFlagToday === 1 && GridView.isCurrentItem ? 3 : 1
            border.color: {
                if (isFlagToday === 1) {
                    "#FF9800"; // 今日标识边框
                } else {
                    isDark ? "transparent" : "#DDD";
                }
            }

            Text {
                anchors.fill: parent
                anchors.margins: 6
                text: text0
                color: GridView.isCurrentItem ? "black" : getFontColor()
                font.bold: isFlagToday === 1 ? true : false
                font.pointSize: FontSize
                wrapMode: Text.Wrap
                elide: Text.ElideRight
                maximumLineCount: 2
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    grid.currentIndex = index;
                    mw_one.clickMainTab();
                }
            }
        }
    }

    GridView {
        id: grid
        anchors.fill: parent
        model: listmain
        delegate: gridDelegate
        cellWidth: cardWidth + spacing
        cellHeight: 85
        clip: true

        ScrollBar.vertical: ScrollBar {
            width: 8
            policy: ScrollBar.AsNeeded
        }
    }

    ListModel {
        id: listmain
    }
}
