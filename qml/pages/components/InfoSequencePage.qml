import QtQuick 2.0
import Sailfish.Silica 1.0
import QtGraphicalEffects 1.0
import Binaural 1.0

FullscreenContentPage {
    id: page
    objectName: "InfoSequencePage"

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

                    text: qsTr("SBaGen Sequences")
                    color: Theme.highlightColor
                    font.pixelSize: Theme.fontSizeLarge
                    wrapMode: Text.WordWrap
                }

                Label {
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    x: Theme.horizontalPageMargin

                    text: qsTr("Pre-programmed audio sessions that use binaural beats and changing frequencies to support relaxation, meditation, concentration and sleep. Each sequence creates a different listening experience.")

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
                    id: sequenceGrid

                    width: parent.width - 2 * Theme.horizontalPageMargin
                    x: Theme.horizontalPageMargin

                    columns: 2
                    columnSpacing: Theme.paddingMedium * 1.5
                    rowSpacing: Theme.paddingMedium

                    property real labelWidth: Math.max(
                                                  meditationLabel.implicitWidth,
                                                  sleepLabel.implicitWidth,
                                                  focus10Label.implicitWidth,
                                                  focus12Label.implicitWidth,
                                                  focus15Label.implicitWidth,
                                                  focus21Label.implicitWidth,
                                                  lucidLabel.implicitWidth,
                                                  concentrationLabel.implicitWidth
                                                  )

                    Label {
                        id: meditationLabel
                        width: sequenceGrid.labelWidth
                        text: qsTr("Simple Meditation")
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        width: sequenceGrid.width - sequenceGrid.labelWidth - sequenceGrid.columnSpacing
                        text: qsTr("Calm awareness and relaxation")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        id: sleepLabel
                        width: sequenceGrid.labelWidth
                        text: qsTr("Deep Sleep Aid")
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        width: sequenceGrid.width - sequenceGrid.labelWidth - sequenceGrid.columnSpacing
                        text: qsTr("Deep relaxation and sleep preparation")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        id: focus10Label
                        width: sequenceGrid.labelWidth
                        text: qsTr("Focus 10")
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        width: sequenceGrid.width - sequenceGrid.labelWidth - sequenceGrid.columnSpacing
                        text: qsTr("Mind awake, body deeply relaxed")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        id: focus12Label
                        width: sequenceGrid.labelWidth
                        text: qsTr("Focus 12")
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        width: sequenceGrid.width - sequenceGrid.labelWidth - sequenceGrid.columnSpacing
                        text: qsTr("Expanded awareness and deeper meditation")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        id: focus15Label
                        width: sequenceGrid.labelWidth
                        text: qsTr("Focus 15")
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        width: sequenceGrid.width - sequenceGrid.labelWidth - sequenceGrid.columnSpacing
                        text: qsTr("Exploration of timeless awareness")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        id: focus21Label
                        width: sequenceGrid.labelWidth
                        text: qsTr("Focus 21")
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        width: sequenceGrid.width - sequenceGrid.labelWidth - sequenceGrid.columnSpacing
                        text: qsTr("Exploration beyond ordinary awareness")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        id: lucidLabel
                        width: sequenceGrid.labelWidth
                        text: qsTr("Lucid Dreaming")
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        width: sequenceGrid.width - sequenceGrid.labelWidth - sequenceGrid.columnSpacing
                        text: qsTr("Dream awareness and dream exploration")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        id: concentrationLabel
                        width: sequenceGrid.labelWidth
                        text: qsTr("Focus / Concentration")
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        width: sequenceGrid.width - sequenceGrid.labelWidth - sequenceGrid.columnSpacing
                        text: qsTr("Alertness and mental focus")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }
                }

                Label {
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    x: Theme.horizontalPageMargin

                    text: qsTr("About SBaGen")
                    color: Theme.highlightColor
                    font.pixelSize: Theme.fontSizeMedium
                    wrapMode: Text.WordWrap
                }

                Label {
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    x: Theme.horizontalPageMargin

                    text: qsTr("The sequences in this app come from SBaGen (Sequenced Binaural Beat Generator), an open-source project by Jim Peters. SBaGen uses text-based sequence files to define binaural tones, frequency changes, timing and background sounds. Some names, such as Focus 10 and Focus 12, use terminology associated with consciousness exploration, but the sequences are from SBaGen, not official Monroe Institute recordings or programmes.")

                    color: Theme.primaryColor
                    font.pixelSize: Theme.fontSizeMedium
                    wrapMode: Text.WordWrap
                }

                Item {
                    width: 1
                    height: Theme.paddingSmall
                }

                Label {
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    x: Theme.horizontalPageMargin

                    text: qsTr("Individual experiences may vary.")

                    color: Theme.secondaryColor
                    font.pixelSize: Theme.fontSizeSmall
                    wrapMode: Text.WordWrap
                }
            }
        }

        VerticalScrollDecorator {}
    }
}
