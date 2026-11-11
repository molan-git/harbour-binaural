import QtQuick 2.0
import Sailfish.Silica 1.0
import QtGraphicalEffects 1.0
import Binaural 1.0

FullscreenContentPage {
    id: page
    objectName: "InfoIsochronicPage"

    Rectangle {
        anchors.fill: parent
        color: Theme.overlayBackgroundColor
        z: -1
    }

    SilicaFlickable {
        id: flickable

        anchors.fill: parent
        contentHeight: scrollContent.height + Theme.paddingLarge
        clip: true

        Item {
            id: scrollContent

            width: flickable.width
            height: contentColumn.y + contentColumn.height

            IconButton {
                id: dismissButton

                y: Theme.paddingLarge

                anchors {
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                }

                icon.source: "image://theme/icon-m-dismiss?" + Theme.primaryColor

                onClicked: pageStack.pop()
            }

            Column {
                id: contentColumn

                y: 0
                width: parent.width
                spacing: Theme.paddingLarge

                Item {
                    width: parent.width
                    height: dismissButton.y + dismissButton.height +
                            Theme.paddingSmall
                }

                Label {
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    x: Theme.horizontalPageMargin

                    text: SoundNames.label("Isochronic tones")
                    color: Theme.highlightColor
                    font.pixelSize: Theme.fontSizeLarge
                    wrapMode: Text.WordWrap
                }

                Label {
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    x: Theme.horizontalPageMargin

                    text: qsTr("An isochronic tone is a sound that switches on and off at regular intervals. A continuous carrier tone is rapidly pulsed on and off; the carrier frequency is the pitch you hear, while the pulse frequency determines how quickly the sound pulses. The tones are played identically in both ears, independent of binaural beats.")

                    color: Theme.primaryColor
                    font.pixelSize: Theme.fontSizeMedium
                    wrapMode: Text.WordWrap
                }

                Label {
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    x: Theme.horizontalPageMargin

                    text: qsTr("Choose from:")
                    color: Theme.highlightColor
                    font.pixelSize: Theme.fontSizeMedium
                    wrapMode: Text.WordWrap
                }

                Grid {
                    id: isochronicGrid

                    width: parent.width - 2 * Theme.horizontalPageMargin
                    x: Theme.horizontalPageMargin

                    columns: 2
                    columnSpacing: Theme.paddingMedium * 1.5
                    rowSpacing: Theme.paddingMedium

                    Label {
                        width: isochronicGrid.width / 2 -
                               isochronicGrid.columnSpacing / 2

                        text: qsTr("1 Hz")
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        width: isochronicGrid.width / 2 -
                               isochronicGrid.columnSpacing / 2

                        text: qsTr("Deep sleep, slow breathing exercises")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        width: isochronicGrid.width / 2 -
                               isochronicGrid.columnSpacing / 2

                        text: qsTr("2 Hz")
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        width: isochronicGrid.width / 2 -
                               isochronicGrid.columnSpacing / 2

                        text: qsTr("Deep relaxation, sleep-oriented meditation")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        width: isochronicGrid.width / 2 -
                               isochronicGrid.columnSpacing / 2

                        text: qsTr("4 Hz")
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        width: isochronicGrid.width / 2 -
                               isochronicGrid.columnSpacing / 2

                        text: qsTr("Deep meditation, visualization, lucid dreaming practices")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        width: isochronicGrid.width / 2 -
                               isochronicGrid.columnSpacing / 2

                        text: qsTr("6 Hz")
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        width: isochronicGrid.width / 2 -
                               isochronicGrid.columnSpacing / 2

                        text: qsTr("Deep meditation, guided visualization, relaxation")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        width: isochronicGrid.width / 2 -
                               isochronicGrid.columnSpacing / 2

                        text: qsTr("7.83 Hz")
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        width: isochronicGrid.width / 2 -
                               isochronicGrid.columnSpacing / 2

                        text: qsTr("Deep relaxation, meditative awareness, grounding")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        width: isochronicGrid.width / 2 -
                               isochronicGrid.columnSpacing / 2

                        text: qsTr("10 Hz")
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        width: isochronicGrid.width / 2 -
                               isochronicGrid.columnSpacing / 2

                        text: qsTr("Relaxed wakefulness, stress management, calm concentration")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        width: isochronicGrid.width / 2 -
                               isochronicGrid.columnSpacing / 2

                        text: qsTr("40 Hz")
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        width: isochronicGrid.width / 2 -
                               isochronicGrid.columnSpacing / 2

                        text: qsTr("Attention and sensory processing")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }
                }

                Item {
                    width: 1
                    height: Theme.paddingSmall
                }

                Label {
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    x: Theme.horizontalPageMargin

                    text: SoundNames.label("Individual experiences may vary.")
                    color: Theme.secondaryColor
                    font.pixelSize: Theme.fontSizeSmall
                    wrapMode: Text.WordWrap
                }
            }
        }

        VerticalScrollDecorator {}
    }
}
