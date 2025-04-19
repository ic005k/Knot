import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Window {
    width: 400
    height: 600
    visible: true
    title: "翻书效果阅读器"

    property int currentIndex: 0
    property var files: [
        "file:///path/to/file1.txt",
        "file:///path/to/file2.txt",
        "file:///path/to/file3.txt"
    ]
    property string currentContent: "正在加载..."
    property string nextContent: ""
    property bool isAnimating: false

    function loadFile(index, callback) {
        var xhr = new XMLHttpRequest();
        xhr.open("GET", files[index]);
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 200) callback(xhr.responseText);
                else callback("无法加载文件\n" + files[index]);
            }
        };
        xhr.send();
    }

    Component.onCompleted: loadFile(currentIndex, t => currentContent = t);

    Flipable {
        id: flipable
        anchors.fill: parent
        anchors.margins: 10

        property bool flipped: false
        property int direction: 1

        front: Rectangle {
            anchors.fill: parent
            clip: true
            border.width: 1
            border.color: "#ccc"

            TextArea {
                anchors.fill: parent
                anchors.margins: 10
                text: currentContent
                wrapMode: Text.Wrap
                readOnly: true
                background: null
            }
        }

        back: Rectangle {
            anchors.fill: parent
            clip: true
            border.width: 1
            border.color: "#ccc"

            TextArea {
                anchors.fill: parent
                anchors.margins: 10
                text: nextContent
                wrapMode: Text.Wrap
                readOnly: true
                background: null
                transform: [
                    Rotation { angle: 180; axis.y: 1; origin.x: width/2 },
                    Scale { xScale: -1 }
                ]
            }
        }

        transform: Rotation {
            id: rotation
            origin.x: flipable.width/2
            axis.y: 1
            angle: 0
        }

        states: State {
            name: "flipped"
            PropertyChanges { target: rotation; angle: 180 * flipable.direction }
            when: flipable.flipped
        }

        transitions: Transition {
            SequentialAnimation {
                ScriptAction { script: isAnimating = true }
                NumberAnimation {
                    target: rotation
                    property: "angle"
                    duration: 600
                    easing.type: Easing.InOutQuad
                }
                ScriptAction {
                    script: {
                        currentContent = nextContent;
                        flipable.flipped = false;
                        rotation.angle = 0;
                        isAnimating = false;
                    }
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            enabled: !isAnimating
            property real startX: 0
            property real startY: 0  // 新增Y坐标记录
            property real minSwipe: 50

            onPressed: {
                startX = mouse.x
                startY = mouse.y  // 初始化Y坐标
            }

            onPositionChanged: {
                if (isAnimating) return;
                const deltaX = mouse.x - startX;
                rotation.angle = Math.max(-180, Math.min(180,
                    (deltaX / width) * 180 * (deltaX > 0 ? 1 : -1)));
            }

            onReleased: {
                if (isAnimating) return;
                const deltaX = mouse.x - startX;
                const deltaY = mouse.y - startY;  // 使用已定义的startY
                const absDeltaX = Math.abs(deltaX);
                const absDeltaY = Math.abs(deltaY);

                // 确保水平滑动距离大于垂直滑动距离
                if (absDeltaX > minSwipe && absDeltaX > absDeltaY) {
                    const direction = deltaX > 0 ? -1 : 1;
                    const newIndex = currentIndex + direction;

                    if (newIndex < 0 || newIndex >= files.length) {
                        rotation.angle = 0;
                        return;
                    }

                    flipable.direction = direction;
                    loadFile(newIndex, text => {
                        nextContent = text;
                        flipable.flipped = true;
                        currentIndex = newIndex;
                    });
                } else {
                    rotation.angle = 0;
                }
            }
        }
    }

    Text {
        anchors {
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
            margins: 10
        }
        text: "第 " + (currentIndex + 1) + " 页 / 共 " + files.length + " 页"
        font.pixelSize: 14
        color: "#666"
    }
}
