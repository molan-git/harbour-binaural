import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: contentColumn.height + Theme.paddingLarge

        VerticalScrollDecorator { }

        Column {
            id: contentColumn

            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: "About"
            }

            Image {
                anchors.horizontalCenter: parent.horizontalCenter

                width: Theme.iconSizeExtraLarge
                height: Theme.iconSizeExtraLarge

                source: "/usr/share/icons/hicolor/172x172/apps/harbour-binaural.png"
                fillMode: Image.PreserveAspectFit
            }

            Label {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter

                text: "Binaural"

                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeLarge
            }

            Label {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter

                text: "Version 0.1"

                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: "Application for binaural beats and ambience sounds"

                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeMedium
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }

            SectionHeader {
                text: "Development"
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter

                text: "Source code on Github"

                onClicked: {
                    Qt.openUrlExternally("https://github.com/molan-git/harbour-binaural")
                }
            }

            SectionHeader {
                text: "Translations"
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: "Help translate the application into your language."

                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeSmall
                wrapMode: Text.WordWrap
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter

                text: "Transifex"

                onClicked: {
                    Qt.openUrlExternally("https://www.transifex.com/")
                }
            }

            SectionHeader {
                text: "Support"
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: "Support the project via"

                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeSmall
                wrapMode: Text.WordWrap
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter

                text: "tba"

                onClicked: {
                    Qt.openUrlExternally("https://tba")
                }
            }

            SectionHeader {
                text: "Project"
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: "Binaural was inspired by the Metiq app. The ambient audio files are sourced from the same project. Many thanks to the developers of Metiq for their work and inspiration."

                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeSmall
                wrapMode: Text.WordWrap
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter

                text: "Metiq on Github"

                onClicked: {
                    Qt.openUrlExternally("https://github.com/metiq-xyz")
                }
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: "The coloured noise sound files were generated using the Metiq colored noise generator."

                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeSmall
                wrapMode: Text.WordWrap
            }

            SectionHeader {
                text: "Data protection"
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: "This application works exclusively offline and requires permission to output audio. No data is collected."

                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeSmall
                wrapMode: Text.WordWrap
            }

            SectionHeader {
                text: "License"
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: "Binaural is licensed under the GNU General Public License v3.0 or later."

                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeSmall
                wrapMode: Text.WordWrap
            }
        }
    }
}
