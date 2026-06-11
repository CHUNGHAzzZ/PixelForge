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
    color: "#FFFFFF"
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
        color: "#FFFFFF"

        Row {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 12
            spacing: 20

            Repeater {
                model: ["File", "Edit", "View", "Layer", "Select", "Filter"]

                Text {
                    text: modelData
                    color: "#1F2328"
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
        color: "#FFFFFF"

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
                    color: index === 0 ? "#F1F0F2" : "#FFFFFF"
                    border.color: "#D8D7DB"

                    Text {
                        anchors.centerIn: parent
                        text: modelData
                        color: "#1F2328"
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
        color: "#FFFFFF"
        border.color: "#E5E4E8"

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
                    color: index === 0 ? "#F1F0F2" : "#FFFFFF"
                    border.color: "#D8D7DB"
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
        color: "#FFFFFF"
        border.color: "#E5E4E8"

        Column {
            anchors.fill: parent
            anchors.margins: 14
            spacing: 16

            Text {
                text: "Layers"
                color: "#1F2328"
                font.pixelSize: 14
                font.bold: true
            }

            Rectangle {
                width: parent.width
                height: 42
                radius: 4
                color: "#FFFFFF"
                border.color: "#D8D7DB"

                Text {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 12
                    text: "Background"
                    color: "#1F2328"
                    font.pixelSize: 13
                }
            }

            Text {
                text: "Brush"
                color: "#1F2328"
                font.pixelSize: 14
                font.bold: true
            }

            Rectangle {
                width: parent.width
                height: 6
                radius: 3
                color: "#E5E4E8"

                Rectangle {
                    width: parent.width * 0.45
                    height: parent.height
                    radius: 3
                    color: "#C7C5CC"
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
        color: "#FFFFFF"

        CanvasRender {
            id: canvas
            anchors.fill: parent
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
            color: "#55F1F0F2"
            border.color: "#C7C5CC"
            border.width: 2

            Rectangle {
                anchors.centerIn: parent
                width: Math.min(parent.width - 48, 360)
                height: 96
                radius: 6
                color: "#FFFFFF"
                border.color: "#D8D7DB"

                Text {
                    anchors.centerIn: parent
                    text: "Drop file to import"
                    color: "#1F2328"
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
        color: "#FFFFFF"

        Text {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 12
            text: "Ready"
            color: "#1F2328"
            font.pixelSize: 12
        }
    }
}
