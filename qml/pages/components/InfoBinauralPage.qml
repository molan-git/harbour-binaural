import QtQuick 2.0
import Sailfish.Silica 1.0

FullscreenContentPage {
    id: page

    SilicaFlickable {
        anchors.fill: parent

        contentHeight: contentColumn.height + Theme.paddingLarge

        VerticalScrollDecorator { }

        Column {
            id: contentColumn

            width: parent.width
            spacing: Theme.paddingLarge

            Item {
                width: parent.width
                height: Theme.itemSizeLarge + Theme.paddingLarge

                IconButton {
                    anchors {
                        right: parent.right
                        rightMargin: Theme.horizontalPageMargin
                        bottom: parent.bottom
                    }

                    icon.source: "image://theme/icon-splus-cancel"

                    onClicked: {
                        pageStack.pop()
                    }
                }
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: "Binaural beats are created by playing two slightly different frequencies in each ear, producing a rhythmic effect often used for relaxation, focus, sleep, and meditation. Color noises are different types of sound frequencies that can be used individually or combined to create different listening experiences."

                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeMedium
                wrapMode: Text.WordWrap
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: "Tap a frequency button to play or stop it. You can also add a color noise and/or ambient sound and adjust the volume of each sound individually."

                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeMedium
                wrapMode: Text.WordWrap
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: "Common brainwave frequency ranges"

                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeLarge
                wrapMode: Text.WordWrap
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: "<b>Delta (0.5–4 Hz):</b> associated with deep sleep and restorative rest.<br><br>" +
                      "<b>Theta (4–8 Hz):</b> associated with drowsiness, meditation, and relaxed creativity.<br><br>" +
                      "<b>Alpha (8–12 Hz):</b> associated with relaxed wakefulness, calmness, and light meditation.<br><br>" +
                      "<b>Beta (13–30 Hz):</b> associated with alertness, concentration, and active thinking.<br><br>" +
                      "<b>Gamma (30–100 Hz):</b> associated with complex cognitive processing and perception."

                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeMedium
                textFormat: Text.RichText
                wrapMode: Text.WordWrap
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: "Color Noises"

                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeLarge
                wrapMode: Text.WordWrap
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: "<b>White:</b> masks distractions.<br><br>" +
                      "<b>Pink:</b> helps with sleep and focus.<br><br>" +
                      "<b>Brown:</b> helps relaxation and focus.<br><br>" +
                      "<b>Grey:</b> masks sounds evenly."

                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeMedium
                textFormat: Text.RichText
                wrapMode: Text.WordWrap
            }
        }
    }
}
