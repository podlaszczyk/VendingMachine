import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    visible: true
    width: 700
    height: 500
    minimumWidth: 560
    minimumHeight: 420
    title: qsTr("Vending Machine")

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
                text: qsTr("Pending: %1").arg(vendingController.pendingTransactions)
            }

            Label {
                text: vendingController.online ? qsTr("Online") : qsTr("Offline")
                color: vendingController.online ? "green" : "crimson"
                font.bold: true
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 20

        Label {
            text: vendingController.state === "Idle"
                  ? qsTr("Touch a card")
                  : vendingController.state === "CardRead"
                    ? qsTr("Select a product")
                    : qsTr("Status: %1").arg(vendingController.state)
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
                    {id: "helmet", name: qsTr("Helmet"), price: "35,00 zł"},
                    {id: "glasses", name: qsTr("Safety glasses"), price: "18,00 zł"},
                    {id: "gloves", name: qsTr("Safety gloves"), price: "12,00 zł"},
                    {id: "ear-protection", name: qsTr("Safety Headphones"), price: "28,00 zł"}
                ]

                delegate: Button {
                    required property var modelData

                    text: "%1\n%2".arg(modelData.name).arg(modelData.price)
                    enabled: vendingController.state === "CardRead"
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    onClicked: vendingController.selectProduct(modelData.id)
                }
            }
        }

        Label {
            text: qsTr("StateMachine: %1").arg(vendingController.state)
            Layout.alignment: Qt.AlignHCenter
        }

        ProgressBar {
            from: 0
            to: 100
            value: vendingController.dispensingProgress
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter

            Button {
                text: qsTr("Simulate Card Touching")
                enabled: vendingController.state === "Idle"
                onClicked: vendingController.simulateCardTap()
            }

            Button {
                text: qsTr("New Transaction")
                enabled: vendingController.state === "Completed" || vendingController.state === "Failed"
                onClicked: vendingController.reset()
            }

            Button {
                text: qsTr("Simulate Error")
                enabled: vendingController.state === "Dispensing"
                onClicked: vendingController.simulateDispenseFailure()
            }
        }
    }
}
