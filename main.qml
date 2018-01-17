import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

import App 1.0

ApplicationWindow {
    id: window
    color: "white"
    visible: true
    width: 600
    height: 400
    title: "Closest Available Tile"

    ColumnLayout {
        anchors.fill: parent

        Label {
            text: "Left click to choose the tile to start from. Right click to toggle a tile's passability."
        }

        RowLayout {
            Label {
                text: "x = start tile"
            }
            Rectangle {
                color: "lightgreen"
                Layout.preferredWidth: grid.tileSize
                Layout.preferredHeight: grid.tileSize
            }
            Label {
                text: "= available tile"
            }
            Rectangle {
                color: "salmon"
                Layout.preferredWidth: grid.tileSize
                Layout.preferredHeight: grid.tileSize
            }
            Label {
                text: "= non-passable tile"
            }
        }

        Grid {
            id: grid
            sizeInTiles: Qt.size(Math.floor(window.width / tileSize), Math.floor(window.height / tileSize))

            Layout.fillWidth: true
            Layout.fillHeight: true

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onClicked: {
                    if (mouse.button === Qt.LeftButton)
                        grid.findClosestAvailableTile(Qt.point(mouseX, mouseY))
                    else
                        grid.togglePassable(Qt.point(mouseX, mouseY))
                }
            }
        }
    }
}
