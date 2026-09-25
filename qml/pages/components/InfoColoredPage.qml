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

                text: qsTr("Color Noise")

                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeLarge
                wrapMode: Text.WordWrap
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: qsTr("Color noises are different sound profiles with varying frequency distributions. They are commonly used for relaxation, focus, sleep, meditation, and masking unwanted sounds.")

                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeMedium
                wrapMode: Text.WordWrap
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: qsTr("<b>White:</b> masks distractions.<br><br>") +
                      qsTr("<b>Pink:</b> helps with sleep and focus.<br><br>") +
                      qsTr("<b>Brown:</b> helps relaxation and focus.<br><br>") +
                      qsTr("<b>Grey:</b> masks sounds evenly.")

                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeMedium
                textFormat: Text.RichText
                wrapMode: Text.WordWrap
            }
        }
    }
}
