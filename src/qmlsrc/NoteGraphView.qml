import QtQuick 2.15
import QtQuick.Controls 2.15
import NoteGraph 1.0

Item {
    id: root
    anchors.fill: parent

    // 主布局：垂直排列三个区域
    Column {
        id: mainLayout
        anchors.fill: parent
        spacing: 15
        padding: 10

        // 1. 总览区域
        Column {
            id: overviewSection
            width: parent.width
            spacing: 8

            Text {
                text: "总览"
                font.bold: true
                font.pixelSize: 16
                color: "#1976d2"
            }

            ScrollView {
                width: parent.width
                height: 120
                padding: 0

                TextEdit {
                    id: overviewContent
                    text: generateOverview()
                    textFormat: TextEdit.RichText
                    readOnly: true
                    font.pixelSize: 14
                    wrapMode: TextEdit.WordWrap
                    padding: 5
                }
            }
        }

        // 2. 引用区域（使用链接点击替代双击）
        Column {
            id: referencesSection
            width: parent.width
            spacing: 8

            Text {
                text: "引用 (" + referencesCount + ")"
                font.bold: true
                font.pixelSize: 16
                color: "#1976d2"
            }

            ScrollView {
                width: parent.width
                height: 180
                padding: 0

                TextEdit {
                    id: referencesContent
                    text: generateReferences()
                    textFormat: TextEdit.RichText
                    readOnly: true
                    font.pixelSize: 14
                    wrapMode: TextEdit.WordWrap
                    padding: 5
                    selectByMouse: true

                    // 点击引用项中的链接触发事件（参考你的单击实现）
                    onLinkActivated: (link) => {
                        console.log("引用项链接激活:", link);
                        if (graphController && typeof graphController.handleNodeDoubleClick === "function") {
                            graphController.handleNodeDoubleClick(link);
                        }
                    }
                }
            }
        }

        // 3. 被引用区域（使用链接点击替代双击）
        Column {
            id: referencedBySection
            width: parent.width
            spacing: 8

            Text {
                text: "被引用 (" + referencedByCount + ")"
                font.bold: true
                font.pixelSize: 16
                color: "#1976d2"
            }

            ScrollView {
                width: parent.width
                height: 180
                padding: 0

                TextEdit {
                    id: referencedByContent
                    text: generateReferencedBy()
                    textFormat: TextEdit.RichText
                    readOnly: true
                    font.pixelSize: 14
                    wrapMode: TextEdit.WordWrap
                    padding: 5
                    selectByMouse: true

                    // 点击被引用项中的链接触发事件
                    onLinkActivated: (link) => {
                        console.log("被引用项链接激活:", link);
                        if (graphController && typeof graphController.handleNodeDoubleClick === "function") {
                            graphController.handleNodeDoubleClick(link);
                        }
                    }
                }
            }
        }
    }

    // 数据存储与处理（完全保留原始结构）
    property var currentNode: null
    property var allNodes: []
    property var references: []
    property var referencedBy: []
    property int referencesCount: 0
    property var referencedByCount: 0

    function initData() {
        currentNode = null;
        allNodes = [];
        references = [];
        referencedBy = [];
        referencesCount = 0;
        referencedByCount = 0;

        if (!graphController || !graphController.model) return;

        var model = graphController.model;
        var rowCount = model.rowCount();

        // 收集所有节点
        for (var i = 0; i < rowCount; i++) {
            var idx = model.index(i, 0);
            var node = {
                index: i,
                name: model.data(idx, NoteGraphModel.NameRole) || "无名笔记",
                filePath: model.data(idx, NoteGraphModel.FilePathRole) || "",
                isCurrent: model.data(idx, NoteGraphModel.IsCurrentNoteRole)
            };
            allNodes.push(node);
            if (node.isCurrent) {
                currentNode = node;
            }
        }

        if (!currentNode && allNodes.length > 0) {
            currentNode = allNodes[0];
        }

        // 解析关系
        if (currentNode) {
            var relations = model.getRelations();
            relations.forEach(function(rel) {
                if (rel.source === currentNode.index) {
                    var targetNode = allNodes[rel.target];
                    if (targetNode) references.push(targetNode);
                } else if (rel.target === currentNode.index) {
                    var sourceNode = allNodes[rel.source];
                    if (sourceNode) referencedBy.push(sourceNode);
                }
            });

            referencesCount = references.length;
            referencedByCount = referencedBy.length;
        }
    }

    // 生成总览内容
    function generateOverview() {
        if (!currentNode) return "<p>未找到当前笔记</p>";
        return `<p><strong>当前笔记：</strong>${currentNode.name}</p>
                <p><strong>路径：</strong><a href="${currentNode.filePath}">${currentNode.filePath}</a></p>
                <p><strong>总节点数：</strong>${allNodes.length}</p>
                <p><strong>引用数：</strong>${referencesCount} | <strong>被引用数：</strong>${referencedByCount}</p>`;
    }

    // 生成引用列表（为每个项添加可点击链接）
    function generateReferences() {
        if (references.length === 0) return "<p>无引用的笔记</p>";
        var html = "<ul style='margin:0;padding-left:20px;'>";
        references.forEach(function(node) {
            // 为每个引用项添加链接（点击触发onLinkActivated）
            html += `<li style='margin:5px 0;'>
                        <strong>${node.name}</strong><br>
                        <small><a href="${node.filePath}">${node.filePath}</a></small>
                    </li>`;
        });
        html += "</ul>";
        return html;
    }

    // 生成被引用列表（为每个项添加可点击链接）
    function generateReferencedBy() {
        if (referencedBy.length === 0) return "<p>无引用当前笔记的笔记</p>";
        var html = "<ul style='margin:0;padding-left:20px;'>";
        referencedBy.forEach(function(node) {
            // 为每个被引用项添加链接
            html += `<li style='margin:5px 0;'>
                        <strong>${node.name}</strong><br>
                        <small><a href="${node.filePath}">${node.filePath}</a></small>
                    </li>`;
        });
        html += "</ul>";
        return html;
    }

    // 刷新显示
    function refreshDisplay() {
        initData();
        overviewContent.text = generateOverview();
        referencesContent.text = generateReferences();
        referencedByContent.text = generateReferencedBy();
    }

    // 数据变化监听
    Connections {
        target: graphController ? graphController.model : null
        function onModelReset() { refreshDisplay(); }
        function onDataChanged() { refreshDisplay(); }
    }

    // 初始化
    Component.onCompleted: {
        refreshDisplay();
    }
}
