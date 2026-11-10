import QtQuick 2.0
import Sailfish.Silica 1.0

/*
 * SolfeggioDrawer: content-only drawer for selecting up to
 * three of the nine solfeggio frequencies.
 * Uses ActionDrawer and PageDrawer (same pattern as
 * AmbienceDrawer.qml).
 */

ActionDrawer {
    id: solfeggioDrawer

    property var activeSolfeggio: []
    property bool solfeggioPlaying: false
    property bool pausedMode: false

    signal solfeggioSelected(double frequency)
    signal solfeggioDeselected(double frequency)
    signal solfeggioVolumeChanged(real volume)

    // Solfeggio Frequency Selection grid (3 x 3)
    SilicaGridView {
        id: solfeggioGrid

        x: Theme.horizontalPageMargin
        width: parent.width - 2 * Theme.horizontalPageMargin
        height: Math.ceil(model.length / 3) * Theme.itemSizeMedium

        cellWidth: width / 3
        cellHeight: Theme.itemSizeMedium

        model: [174, 285, 396, 417, 528, 639, 741, 852, 963]

        delegate: Item {
            width: solfeggioGrid.cellWidth
            height: solfeggioGrid.cellHeight

            Rectangle {
                anchors.fill: solfeggioButton
                z: -1
                radius: Theme.paddingSmall
                color: solfeggioButton.highlighted
                       ? Theme.highlightColor
                       : "transparent"
                opacity: solfeggioButton.highlighted ? 0.15 : 0.0

                Behavior on opacity { NumberAnimation { duration: 150 } }
            }

            Button {
                id: solfeggioButton

                anchors.centerIn: parent
                preferredWidth: solfeggioGrid.cellWidth -
                                 Theme.paddingMedium

                text: modelData + " Hz"
                highlighted: solfeggioDrawer.activeSolfeggio.indexOf(modelData) !== -1
                enabled: highlighted
                         || solfeggioDrawer.activeSolfeggio.length < 3

                color: !enabled ? Theme.secondaryColor
                       : highlighted
                         ? (solfeggioDrawer.solfeggioPlaying
                            ? Theme.highlightColor
                            : Theme.secondaryHighlightColor)
                         : Theme.primaryColor

                opacity: !enabled ? 0.35
                         : highlighted && !solfeggioDrawer.solfeggioPlaying ? 0.6
                         : 1.0

                onClicked: {
                    if (highlighted) {
                        solfeggioDrawer.solfeggioDeselected(modelData)
                    } else if (solfeggioDrawer.activeSolfeggio.length < 3) {
                        solfeggioDrawer.solfeggioSelected(modelData)
                    }
                }
            }
        }
    }

    // Solfeggio Volume Slider
    VolumeSliderRow {
        width: parent.width * 0.9
        anchors.horizontalCenter: parent.horizontalCenter

        enabled: solfeggioDrawer.activeSolfeggio.length > 0

        onValueChanged:
            solfeggioDrawer.solfeggioVolumeChanged(value)
    }
}
