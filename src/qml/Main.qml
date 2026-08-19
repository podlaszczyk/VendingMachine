import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    visible: true
    width: 600
    height: 400
    title: "C++ / QML Example"

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 20

        Label {
            id: messageLabel

            text: "Press the button"
            font.pixelSize: 24
        }

        Button {
            text: "Say hello"

            onClicked: {
                backend.sayHello()
            }
        }

        Connections {
            target: backend

            function onMessageChanged(message) {
                messageLabel.text = message
            }
        }
    }
}