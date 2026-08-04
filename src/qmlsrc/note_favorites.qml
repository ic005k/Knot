import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    width: 600
    height: 480
    property string filterText: ""
    property var sourceModel: []

    ListModel {
        id: filteredModel
    }

    Component.onCompleted: {
        refreshFilter()
    }

    onFilterTextChanged: refreshFilter()
    Binding on sourceModel {
        when: sourceModel !== undefined
        onValueChanged: refreshFilter()
    }

    function refreshFilter()
    {
        filteredModel.clear()
        const kw = filterText.trim().toLowerCase()
        for (const entry of sourceModel)
        {
            if (kw === "" || entry.title.toLowerCase().includes(kw))
            {
                // 核心改动：先定义临时对象，不要直接append({...})
                const rowObj = {
                    uid: entry.uid,
                    title: entry.title,
                    path: entry.path
                };
                filteredModel.append(rowObj);
            }
        }
    }

    Column {
        anchors.fill: parent
        spacing: 8
        padding: 12

        TextField {
            id: searchInput
            width: parent.width
            placeholderText: "输入标题过滤笔记..."
            text: root.filterText
            onTextChanged: root.filterText = text
        }

        ListView {
            id: noteListView
            width: parent.width
            height: parent.height - searchInput.height - 24
            model: filteredModel
            clip: true
            spacing: 4

            delegate: Item {
                width: noteListView.width
                height: layout.implicitHeight

                MouseArea {
                    anchors.fill: parent
                    onClicked: root.openNote(model.uid)
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                }

                Column {
                    id: layout
                    width: parent.width
                    padding: 8
                    spacing: 4

                    Text {
                        text: model.title
                        font.bold: true
                        font.pointSize: 14
                        color: palette.windowText
                        elide: Text.ElideRight
                        width: parent.width
                    }

                    Text {
                        text: model.path
                        font.italic: true
                        font.pointSize: 11
                        color: palette.mid
                        elide: Text.ElideRight
                        width: parent.width
                    }
                }
            }

            Label {
                anchors.centerIn: parent
                text: "没有匹配的笔记"
                visible: filteredModel.count === 0
                color: palette.mid
            }
        }
    }

    signal openNote(string noteUid)
}