import QtQuick 2.15
import QtQuick.Controls 2.15
import NoteGraph 1.0

Rectangle {
    id: root
    width: 400
    height: 600
    color: "#ffffff"

    // QML端数组变量（直接接收C++数据）
    property var nodesArray: []    // 节点数组
    property var relationsArray: [] // 关系数组

    // 显示内容
    Text {
        id: displayText
        x: 20
        y: 20
        font.pixelSize: 14
        color: "#000000"
        width: 360
        height: 500
    }

    // 按钮：触发解析（如果需要手动触发）
    Button {
        x: 20
        y: 520
        text: "加载关系图谱"
        onClicked: {
            // 调用解析（假设当前笔记路径已知）
            NoteGraphController.parser.parseNoteRelations(
                NoteGraphController.model,
                NoteGraphController.currentNotePath
            );
        }
    }

    // 接收C++发送的数据
    Connections {
        target: NoteGraphController.parser  // 连接到解析器
        function onSendDataToQml(nodes, relations) {
            // 直接保存到QML数组
            root.nodesArray = nodes;
            root.relationsArray = relations;

            // 显示数据
            displayData();
        }
    }

    // 格式化显示数据
    function displayData() {
        let text = "节点总数: " + nodesArray.length + "\n\n";

        // 显示当前笔记
        let currentNode = nodesArray.find(n => n.isCurrent);
        if (currentNode) {
            text += "当前笔记: " + currentNode.name + "\n路径: " + currentNode.filePath + "\n\n";
        }

        // 显示引用关系
        text += "引用关系:\n";
        relationsArray.forEach(rel => {
            let sourceNode = nodesArray[rel.source];
            let targetNode = nodesArray[rel.target];
            text += sourceNode.name + " → " + targetNode.name + "\n";
        });

        displayText.text = text;
    }
}
