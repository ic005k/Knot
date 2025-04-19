import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import MyModel2 1.0
import QtQuick.Layouts 1.1

Item {
    id: textitem
    visible: true

    property string strText: ""

    function loadText(str) {
        strText = str
    }

    function loadHtml(msg) {

        document.load("file://" + msg)
    }

    function loadHtmlBuffer(strhtml) {

        document.loadBuffer(strhtml)
    }

    function setVPos(vpos) {
        flickable.contentY = vpos
        console.log(vpos)
    }

    function getVPos() {
        file.textPos = contentY
        console.log(file.textPos)
    }

    function move(x0) {
        x = x + x0
        file.curX = x
        console.log(file.curX)
    }

    function setX(x0) {
        x = 0
    }

    DocumentHandler {
        id: document
        objectName: "dochandler"
        document: textArea.textDocument
        cursorPosition: textArea.cursorPosition
        selectionStart: textArea.selectionStart
        selectionEnd: textArea.selectionEnd

        onLoaded: {
            textArea.text = text
        }
        onError: {
            errorDialog.text = message
            errorDialog.visible = true
        }
    }

    Flickable {
        id: flickable
        flickableDirection: Flickable.VerticalFlick
        anchors.fill: parent

        states: State {
            name: "autoscroll"
            PropertyChanges {
                target: flickable
            }
        }

        onMovementEnded: {
            state = "autoscroll"
        }

        TextArea.flickable: TextArea {
            id: textArea

            font.letterSpacing: 2
            renderType: Text.NativeRendering
            font.hintingPreference: Font.PreferVerticalHinting
            textFormat: Qt.AutoText

            wrapMode: TextArea.Wrap
            readOnly: true
            focus: true
            persistentSelection: false
            selectByMouse: false
            smooth: true

            color: isDark ? "#FFFFFF" : "#664E30"

            background: Rectangle {
                color: isDark ? "#455364" : "#FFFFFF"
                radius: 0
            }

            text: strText
            visible: true

            MouseArea {
                acceptedButtons: Qt.RightButton
                anchors.fill: parent
                onClicked: contextMenu.open()
            }

            onLinkActivated: {
                document.setBackDir(link)
                document.setReadPosition(link)

                console.log(link)
            }

            PropertyAnimation on x {
                easing.type: Easing.Linear
                running: false
                from: 300
                to: 0
                duration: 200
                loops: 1 //Animation.Infinite
            }
        }

        ScrollBar.vertical: ScrollBar {
            id: control
            size: 0.3
            position: 0.2
            width: 7
            active: true
            orientation: Qt.Vertical
            anchors.right: parent.right
            policy: ScrollBar.AsNeeded
        }

        Component.onCompleted: {

            console.log(textArea.lineCount)
            console.log(textArea.height)
            console.log(control.position)
        }
    }

    Menu {
        id: contextMenu

        MenuItem {
            text: qsTr("Copy")
            enabled: textArea.selectedText
            onTriggered: textArea.copy()
        }
        MenuItem {
            text: qsTr("Cut")
            enabled: textArea.selectedText
            onTriggered: textArea.cut()
        }
        MenuItem {
            text: qsTr("Paste")
            enabled: textArea.canPaste
            onTriggered: textArea.paste()
        }
    }
}
