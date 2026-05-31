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

    function addItem(t0, t1, t2, t3, isToday) {
        grid.model.append({
            "text0": t0,
            "text1": t1,
            "text2": t2,
            "text3": t3,
            "isFlagToday": isToday  // 放进 model！不全局
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

    // ✅ 终极兜底：强制激活 UI，用户看不见任何变化
    function forceActivateUI() {
        // 微小移动 1px 再还原 = 彻底唤醒 QML 触摸/滚动系统
        grid.contentY += 1;
        grid.contentY -= 1;

        // 如果有滚动条卡住，也一起刷新
        grid.flick(0, 0);

        console.log("【UI激活】forceActivateUI 执行成功 >>> 清除Android遮罩");
    }

    Component {
        id: gridDelegate
        Rectangle {
            width: cardWidth
            height: grid.cellHeight - 10
            radius: 10
            clip: false

            // 缩放
            property real scaleFactor: 1.0
            transform: Scale {
                origin.x: width / 2
                origin.y: height / 2
                xScale: scaleFactor
                yScale: scaleFactor
            }

            // 动画
            Behavior on scaleFactor {
                NumberAnimation {
                    duration: 80
                    easing.type: Easing.OutCubic
                }
            }

            /*MouseArea {
                anchors.fill: parent

                // ========== Qt6 专用：滚动时不触发点击 ==========
                property bool isMove: false
                property real startX: 0
                property real startY: 0

                Timer {
                    id: clickDelayTimer
                    interval: 120
                    onTriggered: {
                        if (!parent.isMove) {
                            grid.currentIndex = index;
                            mw_one.clickMainTab();
                        }
                    }
                }

                onPressed: {
                    isMove = false;
                    startX = mouseX;
                    startY = mouseY;

                    // 按下动画
                    scaleFactor = 0.95;

                    clickDelayTimer.start();
                }

                onPositionChanged: {
                    // 如果移动超过 5 像素，判定为滚动，不触发点击
                    if (Math.abs(mouseY - startY) > 5 || Math.abs(mouseX - startX) > 5) {
                        isMove = true;
                    }
                }

                onReleased: {
                    scaleFactor = 1.0;

                    // ========== 只有【不是滚动】才执行 ==========
                    //if (!isMove) {
                    //grid.currentIndex = index;
                    //mw_one.clickMainTab();
                    //}
                }
            }*/

            MouseArea {
                id: mouse
                anchors.fill: parent

                // ===== 点击缩放动画 =====
                scale: scaleFactor
                Behavior on scale {
                    NumberAnimation { duration: 100; easing.type: Easing.OutCubic }
                }

                // ===== 滚动防误触 =====
                property bool isMove: false
                property real pressX
                property real pressY

                onPressed: {
                    pressX = mouseX;
                    pressY = mouseY;
                    isMove = false;
                    scaleFactor = 0.95;
                }

                onPositionChanged: {
                    if (Math.abs(mouseX - pressX) > 5 ||
                        Math.abs(mouseY - pressY) > 5) {
                        isMove = true;
                    }
                }

                onReleased: {
                    scaleFactor = 1.0;
                    if (!isMove) {
                        grid.currentIndex = index;
                        mw_one.clickMainTab();
                    }
                }

                onCanceled: {
                    scaleFactor = 1.0;
                }
            }

            color: {
                if (isFlagToday) {
                    isDark ? (GridView.isCurrentItem ? "#4FC3F7" : "#3C2A10") : (GridView.isCurrentItem ? "#4FC3F7" : "#FFE0B2");
                } else {
                    GridView.isCurrentItem ? "#4FC3F7" : getColor();
                }
            }

            border.width: GridView.isCurrentItem ? (isFlagToday ? 3 : 2) : 1
            border.color: isFlagToday ? "#FF9800" : (isDark ? "transparent" : "#DDD")

            Text {
                anchors.fill: parent
                anchors.margins: 6
                text: text0
                color: GridView.isCurrentItem ? "black" : getFontColor()
                font.bold: isFlagToday
                font.pointSize: FontSize
                wrapMode: Text.Wrap
                elide: Text.ElideRight
                maximumLineCount: 2
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
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
        clip: false

        // 👇 Lineage 必加，触摸丝滑
        flickableDirection: Flickable.VerticalFlick
        interactive: true
        cacheBuffer: 3000

        ScrollBar.vertical: ScrollBar {
            width: 8
            policy: ScrollBar.AsNeeded
        }
    }

    ListModel {
        id: listmain
    }
}
