import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window

    visible: true
    width: 700
    height: 500
    minimumWidth: 560
    minimumHeight: 420
    title: qsTr("Vending Machine")

    property bool cardPresent: false
    property bool online: false
    property int pendingTransactions: 2
    property string selectedProduct: ""

    Timer {
        id: dispensingTimer

        interval: 100
        repeat: true

        onTriggered: {
            dispensingProgress.value += 2

            if (dispensingProgress.value >= 100) {
                stop()
                pendingTransactions += 1
                cardPresent = false
                selectedProduct = ""
            }
        }
    }

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16

            Label {
                text: qsTr("Vending Machine")
                font.bold: true
                font.pixelSize: 20
                Layout.fillWidth: true
            }

            Label {
                text: online ? qsTr("Online") : qsTr("Offline")
                color: online ? "green" : "crimson"
                font.bold: true
            }

            Label {
                text: qsTr("Oczekujące: %1").arg(pendingTransactions)
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 20

        Label {
            text: cardPresent
                  ? qsTr("Wybierz produkt")
                  : qsTr("Najpierw przyłóż kartę")
            font.pixelSize: 22
            Layout.alignment: Qt.AlignHCenter
        }

        GridLayout {
            columns: 2
            columnSpacing: 12
            rowSpacing: 12
            Layout.fillWidth: true
            Layout.fillHeight: true

            Repeater {
                model: [
                    { name: qsTr("Kask ochronny"), price: "35,00 zł" },
                    { name: qsTr("Okulary ochronne"), price: "18,00 zł" },
                    { name: qsTr("Rękawice robocze"), price: "12,00 zł" },
                    { name: qsTr("Ochronniki słuchu"), price: "28,00 zł" }
                ]

                delegate: Button {
                    required property var modelData

                    text: "%1\n%2".arg(modelData.name).arg(modelData.price)
                    enabled: cardPresent && !dispensingTimer.running
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    onClicked: {
                        selectedProduct = modelData.name
                        dispensingProgress.value = 0
                        dispensingTimer.start()
                    }
                }
            }
        }

        Label {
            text: dispensingTimer.running
                  ? qsTr("Wydawanie: %1").arg(selectedProduct)
                  : qsTr("Gotowy")
            Layout.alignment: Qt.AlignHCenter
        }

        ProgressBar {
            id: dispensingProgress

            from: 0
            to: 100
            value: 0
            Layout.fillWidth: true
        }

        Button {
            text: qsTr("Symuluj przyłożenie karty")
            enabled: !cardPresent && !dispensingTimer.running
            Layout.alignment: Qt.AlignHCenter

            onClicked: cardPresent = true
        }
    }
}
