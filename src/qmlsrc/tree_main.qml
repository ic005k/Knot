import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 1.4

Item {
    id: window
    visible: true



    function addTopItem(strItem) {
        var topItem1 = tree.createItem(strItem, "/res/nb.png")
        topItem1.setSelectionFlag(tree.selectionCurrent)
        tree.addTopLevelItem(topItem1)

        return topItem1
    }

    function addChildItem(parentItem, strChildItem, iconFile) {
        var childItem = tree.createItem(strChildItem, iconFile)
        parentItem.appendChild(childItem)

        return childItem
    }

    function clearAll() {

        for (var i = 0; i < 5000; i++) {
            var curItem = tree.topLevelItem(i)
            if (curItem) {
                var parentItem = curItem.parent()
                if (parentItem) {
                    parentItem.removeChild(curItem)
                }
            }
        }
    }

    TreeWidget {
        id: tree
        anchors.fill: parent

        Component.onCompleted: {
            iconSize = (Qt.size(25, 25))
            font.pointSize = fontSize


            var topItem1 = createItem("Item 1", "/res/nb.png")
            topItem1.setSelectionFlag(selectionCurrent)
            addTopLevelItem(topItem1)

            topItem1.appendChild(createItem("Child 1", "/res/n.png"))
            addChildItem(topItem1, "Child 2", "/res/n.png")
            addChildItem(topItem1, "Child 3", "/res/n.png")

            addTopItem("Item 2")
            addTopItem("Item 3")
            addTopItem("Item 4")


        }

        onCurrentItemChanged: {
            var item = getCurrentItem()
            if (item)
                inputName.text = item.text()
        }
    }

    Dialog {
        id: dlgRename
        modal: true
        width: window.width - 20
        x: window.width / 2 - width / 2
        y: window.height / 2 - height / 2
        title: "Rename item ..."
        visible: false
        standardButtons: Dialog.Ok | Dialog.Cancel

        property alias initName: inputName.text

        contentItem: Rectangle {
            width: dlgRename.width
            height: inputName.implicitHeight + 6
            color: "lightGray"
            TextInput {
                id: inputName
                clip: true
                text: "input here"
                x: 10
                width: parent.width
                font.pointSize: 20
                color: "black"

                anchors.verticalCenter: parent.verticalCenter

                onAccepted: {
                    dlgRename.accept()
                    dlgRename.close()
                }
            }
        }

        onAccepted: {
            var curItem = tree.getCurrentItem()
            if (curItem) {
                curItem.setText(inputName.text)
            }
        }
    }

    ToolBar {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        visible: false

        RowLayout {
            spacing: 0
            ToolButton {
                id: buttonAdd
                text: "Add Node ..."

                onClicked: {
                    menuAdd.popup()
                }


                Menu{
                    id: menuAdd
                    title: "Add Node"

                    MenuItem {
                        text: "Child"
                        onTriggered: {
                            var curItem = tree.getCurrentItem()
                            if (curItem) {
                                curItem.appendChild(
                                            tree.createItem("Child",
                                                            "ico_item.png"))
                                curItem.setExpanded(true)
                            }
                        }
                    }

                    MenuItem {
                        text: "Sibling Before"
                        onTriggered: {
                            var curItem = tree.getCurrentItem()
                            if (curItem) {
                                var parentItem = curItem.parent()
                                if (parentItem) {
                                    var pos = parentItem.indexOfChildItem(
                                                curItem)
                                    parentItem.insertChild(pos, tree.createItem(
                                                               "Item Before",
                                                               "ico_item.png"))
                                }
                            }
                        }
                    }

                    MenuItem {
                        text: "Sibling After"
                        onTriggered: {
                            var curItem = tree.getCurrentItem()
                            if (curItem) {
                                var parentItem = curItem.parent()
                                if (parentItem) {
                                    var pos = parentItem.indexOfChildItem(
                                                curItem)
                                    if (pos < parentItem.childernCount() - 1) {
                                        parentItem.insertChild(
                                                    pos + 1, tree.createItem(
                                                        "Item After",
                                                        "ico_item.png"))
                                    } else {
                                        parentItem.appendChild(
                                                    tree.createItem(
                                                        "Item After",
                                                        "ico_item.png"))
                                    }
                                }
                            }
                        }
                    }
                }
            }

            ToolSeparator {}

            ToolButton {
                id: buttonRename
                text: "Rename ..."

                onClicked: {
                    var curItem = tree.getCurrentItem()
                    if (curItem) {
                        dlgRename.initName = curItem.text()
                        dlgRename.open()
                    }
                }
            }

            ToolSeparator {}

            ToolButton {
                id: buttonDelete
                text: "Delete"

                onClicked: {
                    var curItem = tree.getCurrentItem()
                    if (curItem) {
                        var parentItem = curItem.parent()
                        if (parentItem) {
                            parentItem.removeChild(curItem)
                        }
                    }
                }
            }
        }
    }
}
