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

                text: qsTr("Color Noise")

                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeLarge
                wrapMode: Text.WordWrap
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: qsTr("Color noise refers to sounds with different distributions of frequencies. The different colors describe how the sound's energy is distributed across the frequency spectrum. They are commonly used for relaxation, focus, sleep, meditation, and masking unwanted sounds.")

                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeMedium
                wrapMode: Text.WordWrap
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: qsTr("Each type of noise has a different sound character. White noise contains a broad range of frequencies, while pink and brown noise place more emphasis on lower frequencies.")

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

                text: qsTr("<b>White:</b> Bright, evenly distributed noise with broad range of frequencies.<br><br>") +
                      qsTr("<b>Pink:</b> Softer noise with more energy in the lower frequencies, used for relaxation and sleep.<br><br>") +
                      qsTr("<b>Brown:</b> Deeper, bass-heavy noise with even more emphasis on lower frequencies, used for relaxation and masking sounds.<br><br>") +
                      qsTr("<b>Grey:</b> Noise shaped to sound more balanced to human hearing, with frequencies adjusted according to perceived loudness.")

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

                text: qsTr("Individual experiences and preferences vary, so you may find some types of noise more comfortable or useful than others.")

                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                wrapMode: Text.WordWrap
            }
        }
    }
}
