import QtQuick 2.0
import Sailfish.Silica 1.0
import QtGraphicalEffects 1.0
import Binaural 1.0

FullscreenContentPage {
    id: page
    objectName: "InfoSolfeggioPage"

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

                    text: qsTr("Solfeggio Frequencies")
                    color: Theme.highlightColor
                    font.pixelSize: Theme.fontSizeLarge
                    wrapMode: Text.WordWrap
                }

                Label {
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    x: Theme.horizontalPageMargin

                    text: qsTr("Solfeggio frequencies are a collection of specific tones commonly used in meditation, relaxation and sound-based practices. Each frequency has a traditional symbolic association, ranging from grounding and transformation to awareness and unity.")

                    color: Theme.primaryColor
                    font.pixelSize: Theme.fontSizeMedium
                    wrapMode: Text.WordWrap
                }

                Label {
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    x: Theme.horizontalPageMargin

                    text: qsTr("The nine frequencies")
                    color: Theme.highlightColor
                    font.pixelSize: Theme.fontSizeMedium
                    wrapMode: Text.WordWrap
                }

                Grid {
                    id: frequencyGrid

                    width: parent.width - 2 * Theme.horizontalPageMargin
                    x: Theme.horizontalPageMargin

                    columns: 2
                    columnSpacing: Theme.paddingMedium * 1.5
                    rowSpacing: Theme.paddingMedium

                    property real labelWidth: Math.max(
                        frequency174Label.implicitWidth,
                        frequency285Label.implicitWidth,
                        frequency396Label.implicitWidth,
                        frequency417Label.implicitWidth,
                        frequency528Label.implicitWidth,
                        frequency639Label.implicitWidth,
                        frequency741Label.implicitWidth,
                        frequency852Label.implicitWidth,
                        frequency963Label.implicitWidth
                    )

                    // 174 Hz
                    Label {
                        id: frequency174Label

                        width: frequencyGrid.labelWidth
                        text: "174 Hz"
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        width: frequencyGrid.width -
                               frequencyGrid.labelWidth -
                               frequencyGrid.columnSpacing

                        text: qsTr("Grounding, safety and relaxation")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    // 285 Hz
                    Label {
                        id: frequency285Label

                        width: frequencyGrid.labelWidth
                        text: "285 Hz"
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        width: frequencyGrid.width -
                               frequencyGrid.labelWidth -
                               frequencyGrid.columnSpacing

                        text: qsTr("Restoration and renewal")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    // 396 Hz
                    Label {
                        id: frequency396Label

                        width: frequencyGrid.labelWidth
                        text: "396 Hz"
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        width: frequencyGrid.width -
                               frequencyGrid.labelWidth -
                               frequencyGrid.columnSpacing

                        text: qsTr("Releasing fear and guilt")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    // 417 Hz
                    Label {
                        id: frequency417Label

                        width: frequencyGrid.labelWidth
                        text: "417 Hz"
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        width: frequencyGrid.width -
                               frequencyGrid.labelWidth -
                               frequencyGrid.columnSpacing

                        text: qsTr("Change and transformation")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    // 528 Hz
                    Label {
                        id: frequency528Label

                        width: frequencyGrid.labelWidth
                        text: "528 Hz"
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        width: frequencyGrid.width -
                               frequencyGrid.labelWidth -
                               frequencyGrid.columnSpacing

                        text: qsTr("Love, transformation and harmony")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    // 639 Hz
                    Label {
                        id: frequency639Label

                        width: frequencyGrid.labelWidth
                        text: "639 Hz"
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        width: frequencyGrid.width -
                               frequencyGrid.labelWidth -
                               frequencyGrid.columnSpacing

                        text: qsTr("Relationships and connection")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    // 741 Hz
                    Label {
                        id: frequency741Label

                        width: frequencyGrid.labelWidth
                        text: "741 Hz"
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        width: frequencyGrid.width -
                               frequencyGrid.labelWidth -
                               frequencyGrid.columnSpacing

                        text: qsTr("Clarity, expression and intuition")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    // 852 Hz
                    Label {
                        id: frequency852Label

                        width: frequencyGrid.labelWidth
                        text: "852 Hz"
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        width: frequencyGrid.width -
                               frequencyGrid.labelWidth -
                               frequencyGrid.columnSpacing

                        text: qsTr("Awareness and inner reflection")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    // 963 Hz
                    Label {
                        id: frequency963Label

                        width: frequencyGrid.labelWidth
                        text: "963 Hz"
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        width: frequencyGrid.width -
                               frequencyGrid.labelWidth -
                               frequencyGrid.columnSpacing

                        text: qsTr("Unity, wholeness and higher consciousness")
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

                    text: qsTr("The 3–6–9 pattern")
                    color: Theme.highlightColor
                    font.pixelSize: Theme.fontSizeMedium
                    wrapMode: Text.WordWrap
                }

                Label {
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    x: Theme.horizontalPageMargin

                    text: qsTr("The nine frequencies form three numerical groups when their digits are added together:")

                    color: Theme.primaryColor
                    font.pixelSize: Theme.fontSizeMedium
                    wrapMode: Text.WordWrap
                }

                Grid {
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    x: Theme.horizontalPageMargin

                    columns: 2
                    columnSpacing: Theme.paddingMedium * 1.5
                    rowSpacing: Theme.paddingMedium

                    Label {
                        width: frequencyGrid.labelWidth
                        text: "3"
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                    }

                    Label {
                        width: frequencyGrid.width -
                               frequencyGrid.labelWidth -
                               frequencyGrid.columnSpacing

                        text: qsTr("174, 417, 741 Hz")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        width: frequencyGrid.labelWidth
                        text: "6"
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                    }

                    Label {
                        width: frequencyGrid.width -
                               frequencyGrid.labelWidth -
                               frequencyGrid.columnSpacing

                        text: qsTr("285, 528, 852 Hz")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        width: frequencyGrid.labelWidth
                        text: "9"
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeSmall
                    }

                    Label {
                        width: frequencyGrid.width -
                               frequencyGrid.labelWidth -
                               frequencyGrid.columnSpacing

                        text: qsTr("396, 639, 963 Hz")
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }
                }

                Label {
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    x: Theme.horizontalPageMargin

                    text: qsTr("In numerological interpretations, 3 is associated with creation, 6 with harmony and 9 with completion or unity. This pattern is symbolic rather than a physical property of sound.")

                    color: Theme.primaryColor
                    font.pixelSize: Theme.fontSizeSmall
                    wrapMode: Text.WordWrap
                }

                Item {
                    width: 1
                    height: Theme.paddingSmall
                }

                Label {
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    x: Theme.horizontalPageMargin

                    text: qsTr("How to experience Solfeggio tones")
                    color: Theme.highlightColor
                    font.pixelSize: Theme.fontSizeMedium
                    wrapMode: Text.WordWrap
                }

                Label {
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    x: Theme.horizontalPageMargin

                    text: qsTr("Each frequency can be listened to individually as a pure tone or combined with other frequencies to create layered soundscapes. Experiment with different tones and volume levels to find what feels comfortable for you.")

                    color: Theme.primaryColor
                    font.pixelSize: Theme.fontSizeMedium
                    wrapMode: Text.WordWrap
                }

                Label {
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    x: Theme.horizontalPageMargin

                    text: qsTr("Unlike binaural beats, Solfeggio tones do not require different frequencies in the left and right ears. They can be played identically through both channels.")

                    color: Theme.primaryColor
                    font.pixelSize: Theme.fontSizeMedium
                    wrapMode: Text.WordWrap
                }

                Label {
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    x: Theme.horizontalPageMargin

                    text: SoundNames.label("Individual experiences may vary.")
                    color: Theme.secondaryColor
                    font.pixelSize: Theme.fontSizeSmall
                    wrapMode: Text.WordWrap
                }

                Item {
                    width: 1
                    height: Theme.paddingSmall
                }
            }
        }

        VerticalScrollDecorator {}
    }
}
