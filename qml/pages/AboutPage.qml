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
                title: qsTr("About")
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

            // Version number
            Label {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter

                text: qsTr("Version") + " 0.3"

                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: qsTr("Application for playing binaural beats, color noises and ambient sounds.")

                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeMedium
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }

            SectionHeader {
                text: qsTr("Development")
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter

                text: qsTr("Source code on") + " Github"

                onClicked: {
                    Qt.openUrlExternally("https://github.com/molan-git/harbour-binaural")
                }
            }

            SectionHeader {
                text: qsTr("Translations")
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: qsTr("Help translate the application into your language.")

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

            /*
            SectionHeader {
                text: qsTr("Support")
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: qsTr("For small tips to support the project.")

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
            */

            SectionHeader {
                text: qsTr("Credits")
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                textFormat: Text.StyledText

                text: qsTr("Binaural was inspired by the <a href=\"https://github.com/metiq-xyz/android-app\"><font color=\"%1\">Metiq app</font></a>. The ambient audio files are sourced from the same project.").arg(Theme.highlightColor)

                color: Theme.primaryColor
                linkColor: Theme.highlightColor
                font.pixelSize: Theme.fontSizeSmall
                wrapMode: Text.WordWrap

                onLinkActivated: {
                    Qt.openUrlExternally(link)
                }
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                textFormat: Text.StyledText

                text: qsTr("Color noise sound files were generated using the <a href=\"https://github.com/metiq-xyz/colored-noise-generator\"><font color=\"%1\">Metiq colored noise generator</font></a>.").arg(Theme.highlightColor)

                color: Theme.primaryColor
                linkColor: Theme.highlightColor
                font.pixelSize: Theme.fontSizeSmall
                wrapMode: Text.WordWrap

                onLinkActivated: {
                    Qt.openUrlExternally(link)
                }
            }

            SectionHeader {
                text: qsTr("Data protection")
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: qsTr("This application works exclusively offline and requires permission to output audio. No data is collected.")

                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeSmall
                wrapMode: Text.WordWrap
            }

            SectionHeader {
                text: qsTr("License")
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                textFormat: Text.StyledText

                text: qsTr("<a href=\"https://www.gnu.org/licenses/gpl-3.0.html\"><font color=\"%1\">GNU General Public License v3.0 or later</font></a>.").arg(Theme.highlightColor)

                color: Theme.primaryColor
                linkColor: Theme.highlightColor
                font.pixelSize: Theme.fontSizeSmall
                wrapMode: Text.WordWrap

                onLinkActivated: {
                    Qt.openUrlExternally(link)
                }
            }
        }
    }
}
