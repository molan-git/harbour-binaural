import QtQuick 2.0
import Sailfish.Silica 1.0

FullscreenContentPage {
    id: page

    Rectangle {
        anchors.fill: parent
        color: Theme.overlayBackgroundColor
    }

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
                    width: Theme.iconSizeMedium
                    height: Theme.iconSizeMedium

                    anchors {
                        right: parent.right
                        rightMargin: Theme.horizontalPageMargin
                        bottom: parent.bottom
                    }

                    icon.source: "image://theme/icon-m-cancel?" + Theme.primaryColor

                    onClicked: {
                        pageStack.pop()
                    }
                }
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: qsTr("Binaural Beats")

                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeLarge
                wrapMode: Text.WordWrap
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: qsTr("Binaural beats are created by playing two slightly different tones, one in each ear. The brain processes the difference between the tones as a rhythmic beat. Different beat frequencies may be used to support relaxation, focus, meditation, or sleep.")

                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeMedium
                wrapMode: Text.WordWrap
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: qsTr("The frequency shown below is the perceived beat frequency, measured in Hertz (Hz). Different frequency ranges are traditionally associated with different states of mental activity.")

                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeMedium
                wrapMode: Text.WordWrap
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: qsTr("Tap a frequency button to start or stop it. You can also add colored noise and/or ambient sounds and adjust the volume of each sound individually.")

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

                text: qsTr("Choose from:")

                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeMedium
                wrapMode: Text.WordWrap
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: qsTr("<b>Delta (2 Hz):</b> Deep sleep, restorative rest.<br><br>") +
                      qsTr("<b>Delta/Theta (4 Hz):</b> Deep relaxation, sleep onset.<br><br>") +
                      qsTr("<b>Theta (6 Hz):</b> Drowsiness, meditation, relaxed creativity.<br><br>") +
                      qsTr("<b>Alpha (10 Hz):</b> Relaxed wakefulness, calmness, light meditation.<br><br>") +
                      qsTr("<b>Beta (20 Hz):</b> Alertness, concentration, active thinking.<br><br>") +
                      qsTr("<b>Gamma (40 Hz):</b> Complex cognitive processing, perception.")

                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeSmall
                textFormat: Text.RichText
                wrapMode: Text.WordWrap
            }

            Item {
                width: 1
                height: Theme.paddingSmall
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: qsTr("These descriptions refer to commonly associated brainwave states. Individual experiences may vary.")

                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                wrapMode: Text.WordWrap
            }
        }
    }
}
