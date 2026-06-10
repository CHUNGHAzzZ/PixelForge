import QtQuick
import QtQuick.Window
import PixelForge.Canvas 1.0

Window {
    id: root
    width: 1440
    height: 900
    minimumWidth: 960
    minimumHeight: 640
    visible: true
    color: "#202225"
    title: appController.documentName + " - PixelForge"

    function importDroppedFile(urls, canvasView) {
        if (!urls || urls.length === 0) {
            return false
        }

        if (!canvasView.loadImage(urls[0])) {
            return false
        }

        appController.importFile(urls[0])
        return true
    }

    Rectangle {
        id: menuBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 36
        color: "#2C2F33"

        Row {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 12
            spacing: 20

            Repeater {
                model: ["File", "Edit", "View", "Layer", "Select", "Filter"]

                Text {
                    text: modelData
                    color: "#E6E8EB"
                    font.pixelSize: 13
                }
            }
        }
    }

    Rectangle {
        id: toolBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: menuBar.bottom
        height: 48
        color: "#25282C"

        Row {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 12
            spacing: 8

            Repeater {
                model: ["B", "E", "F", "S", "T"]

                Rectangle {
                    width: 32
                    height: 32
                    radius: 4
                    color: index === 0 ? "#4B7BEC" : "#34383D"
                    border.color: "#4A4F57"

                    Text {
                        anchors.centerIn: parent
                        text: modelData
                        color: "#FFFFFF"
                        font.bold: true
                        font.pixelSize: 13
                    }
                }
            }
        }
    }

    Rectangle {
        id: leftPanel
        anchors.left: parent.left
        anchors.top: toolBar.bottom
        anchors.bottom: statusBar.top
        width: 56
        color: "#2B2E33"
        border.color: "#1D1F22"

        Column {
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: 12
            spacing: 8

            Repeater {
                model: 8

                Rectangle {
                    width: 34
                    height: 34
                    radius: 4
                    color: index === 0 ? "#4B7BEC" : "#363A40"
                    border.color: "#50565F"
                }
            }
        }
    }

    Rectangle {
        id: rightPanel
        anchors.right: parent.right
        anchors.top: toolBar.bottom
        anchors.bottom: statusBar.top
        width: 280
        color: "#2B2E33"
        border.color: "#1D1F22"

        Column {
            anchors.fill: parent
            anchors.margins: 14
            spacing: 16

            Text {
                text: "Layers"
                color: "#F0F2F5"
                font.pixelSize: 14
                font.bold: true
            }

            Rectangle {
                width: parent.width
                height: 42
                radius: 4
                color: "#393E46"
                border.color: "#515862"

                Text {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 12
                    text: "Background"
                    color: "#ECEFF4"
                    font.pixelSize: 13
                }
            }

            Text {
                text: "Brush"
                color: "#F0F2F5"
                font.pixelSize: 14
                font.bold: true
            }

            Rectangle {
                width: parent.width
                height: 6
                radius: 3
                color: "#555C66"

                Rectangle {
                    width: parent.width * 0.45
                    height: parent.height
                    radius: 3
                    color: "#4B7BEC"
                }
            }
        }
    }

    Rectangle {
        id: workspace
        anchors.left: leftPanel.right
        anchors.right: rightPanel.left
        anchors.top: toolBar.bottom
        anchors.bottom: statusBar.top
        color: "#1B1D20"

        Rectangle {
            anchors.fill: parent
            anchors.margins: 24
            color: "#111315"
            border.color: "#30343A"

            CanvasView {
                id: canvas
                anchors.centerIn: parent
                width: Math.min(parent.width - 48, 960)
                height: Math.min(parent.height - 48, 640)
            }
        }

        DropArea {
            id: fileDropArea
            anchors.fill: parent

            onEntered: function(drag) {
                drag.accepted = drag.hasUrls
            }

            onDropped: function(drop) {
                if (drop.hasUrls && root.importDroppedFile(drop.urls, canvas)) {
                    drop.acceptProposedAction()
                }
            }
        }

        Rectangle {
            anchors.fill: parent
            visible: fileDropArea.containsDrag
            color: "#66000000"
            border.color: "#6EA8FF"
            border.width: 2

            Rectangle {
                anchors.centerIn: parent
                width: Math.min(parent.width - 48, 360)
                height: 96
                radius: 6
                color: "#2F343A"
                border.color: "#6EA8FF"

                Text {
                    anchors.centerIn: parent
                    text: "Drop file to import"
                    color: "#F4F7FB"
                    font.pixelSize: 18
                    font.bold: true
                }
            }
        }
    }

    Rectangle {
        id: statusBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 28
        color: "#25282C"

        Text {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 12
            text: "Ready"
            color: "#B8BEC7"
            font.pixelSize: 12
        }
    }
}
