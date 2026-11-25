import QtQuick 2.0
import Sailfish.Silica 1.0
import QtGraphicalEffects 1.0
import Binaural 1.0

FullscreenContentPage {
    id: page
    objectName: "InfoFocusPage"

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

                    text: qsTr("Focus")
                    color: Theme.highlightColor
                    font.pixelSize: Theme.fontSizeLarge
                    wrapMode: Text.WordWrap
                }

                Label {
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    x: Theme.horizontalPageMargin

                    text: qsTr("The focus sessions combine several binaural-beat voices into one soundscape: each voice is a pair of slightly different tones played into the left and right earphone, and the difference between them is the beat frequency the brain follows. The presets layer delta, theta, alpha and beta beats on different carriers, following the classic 'Focus level' sessions.")

                    color: Theme.primaryColor
                    font.pixelSize: Theme.fontSizeMedium
                    wrapMode: Text.WordWrap
                }

                Label {
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    x: Theme.horizontalPageMargin

                    text: qsTr("Presets:")
                    color: Theme.highlightColor
                    font.pixelSize: Theme.fontSizeMedium
                    wrapMode: Text.WordWrap
                }

                Grid {
                    id: focusInfoGrid

                    width: parent.width - 2 * Theme.horizontalPageMargin
                    x: Theme.horizontalPageMargin

                    columns: 2
                    columnSpacing: Theme.paddingMedium * 1.5
                    rowSpacing: Theme.paddingMedium

                    Label {
                        width: focusInfoGrid.width / 2 -
                               focusInfoGrid.columnSpacing / 2

                        text: qsTr("F3 (Sleep)")
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        width: focusInfoGrid.width / 2 -
                               focusInfoGrid.columnSpacing / 2

                        text: qsTr("Deep delta beats for falling asleep")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        width: focusInfoGrid.width / 2 -
                               focusInfoGrid.columnSpacing / 2

                        text: qsTr("F10 (Body Asleep)")
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        width: focusInfoGrid.width / 2 -
                               focusInfoGrid.columnSpacing / 2

                        text: qsTr("Body asleep, mind awake")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        width: focusInfoGrid.width / 2 -
                               focusInfoGrid.columnSpacing / 2

                        text: qsTr("F12 (Expanded Awareness)")
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        width: focusInfoGrid.width / 2 -
                               focusInfoGrid.columnSpacing / 2

                        text: qsTr("F10 voices extended by alpha-band beats")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        width: focusInfoGrid.width / 2 -
                               focusInfoGrid.columnSpacing / 2

                        text: qsTr("F15 (No Time)")
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        width: focusInfoGrid.width / 2 -
                               focusInfoGrid.columnSpacing / 2

                        text: qsTr("F10 voices extended by low theta beats")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        width: focusInfoGrid.width / 2 -
                               focusInfoGrid.columnSpacing / 2

                        text: qsTr("F21 (Other Energy Systems)")
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        width: focusInfoGrid.width / 2 -
                               focusInfoGrid.columnSpacing / 2

                        text: qsTr("F10 voices extended by high beta beats")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        width: focusInfoGrid.width / 2 -
                               focusInfoGrid.columnSpacing / 2

                        text: qsTr("F&C (Concentration)")
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        width: focusInfoGrid.width / 2 -
                               focusInfoGrid.columnSpacing / 2

                        text: qsTr("Pink noise with beta beats for focus and concentration")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        width: focusInfoGrid.width / 2 -
                               focusInfoGrid.columnSpacing / 2

                        text: qsTr("Custom mix")
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        width: focusInfoGrid.width / 2 -
                               focusInfoGrid.columnSpacing / 2

                        text: qsTr("Build your own three-voice stack: set the carrier and beat frequency of each voice")
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

                    text: qsTr("The preset frequencies follow the classic SBaGen session collections; isochronic voices are approximated as binaural beats. Sessions run as timed sequences; use the progress bar below a preset to pause or stop early.")

                    color: Theme.secondaryColor
                    font.pixelSize: Theme.fontSizeSmall
                    wrapMode: Text.WordWrap
                }

                Label {
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    x: Theme.horizontalPageMargin

                    text: SoundNames.label("Use headphones for the binaural effect.")
                    color: Theme.secondaryColor
                    font.pixelSize: Theme.fontSizeSmall
                    wrapMode: Text.WordWrap
                }
            }
        }

        VerticalScrollDecorator {}
    }
}
