import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    allowedOrientations: defaultAllowedOrientations

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: creditColumn.height + Theme.paddingLarge

        Column {
            id: creditColumn

            width: page.width
            spacing: Theme.paddingMedium

            PageHeader {
                title: qsTr("Audio Credits")
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: qsTr("Audio files have been manually edited and adjusted for use in the app. Pixabay content is used under the Pixabay Content License: ") +
                      "<a href=\"https://pixabay.com/service/license-summary/\" " +
                      "style=\"color:" + Theme.highlightColor + ";\">" +
                      qsTr("Pixabay Content License") +
                      "</a>"

                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeSmall
                horizontalAlignment: Text.AlignLeft
                wrapMode: Text.WordWrap
                textFormat: Text.RichText

                onLinkActivated: {
                    if (link.indexOf("https://") === 0 ||
                        link.indexOf("http://") === 0) {
                        Qt.openUrlExternally(link)
                    }
                }
            }

            Item {
                width: 1
                height: Theme.paddingMedium
            }

            // Wind
            Column {
                width: parent.width
                spacing: Theme.paddingSmall

                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin

                    text: qsTr("Wind")
                    color: Theme.secondaryHighlightColor
                    font.pixelSize: Theme.fontSizeMedium
                }

                Text {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin

                    textFormat: Text.RichText
                    wrapMode: Text.WordWrap

                    color: Theme.primaryColor
                    font.pixelSize: Theme.fontSizeSmall

                    text: qsTr("Sound Effect by ") +
                          "<a href=\"https://pixabay.com/users/traian1984-41907904/?utm_source=link-attribution&utm_medium=referral&utm_campaign=music&utm_content=186986\" " +
                          "style=\"color:" + Theme.highlightColor + ";\">" +
                          "Traian Mitroi</a> " +
                          qsTr("from ") +
                          "<a href=\"https://pixabay.com//?utm_source=link-attribution&utm_medium=referral&utm_campaign=music&utm_content=186986\" " +
                          "style=\"color:" + Theme.highlightColor + ";\">" +
                          "Pixabay</a>"

                    onLinkActivated: {
                        if (link.indexOf("https://") === 0 ||
                            link.indexOf("http://") === 0) {
                            Qt.openUrlExternally(link)
                        }
                    }
                }
            }

            // Rain
            Column {
                width: parent.width
                spacing: Theme.paddingSmall

                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin

                    text: qsTr("Rain")
                    color: Theme.secondaryHighlightColor
                    font.pixelSize: Theme.fontSizeMedium
                }

                Text {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin

                    textFormat: Text.RichText
                    wrapMode: Text.WordWrap

                    color: Theme.primaryColor
                    font.pixelSize: Theme.fontSizeSmall

                    text: qsTr("Sound Effect by ") +
                          "<a href=\"https://pixabay.com/users/boons_freak-39857343/?utm_source=link-attribution&utm_medium=referral&utm_campaign=music&utm_content=188158\" " +
                          "style=\"color:" + Theme.highlightColor + ";\">" +
                          "Pig Bank - Mood</a> " +
                          qsTr("from ") +
                          "<a href=\"https://pixabay.com//?utm_source=link-attribution&utm_medium=referral&utm_campaign=music&utm_content=188158\" " +
                          "style=\"color:" + Theme.highlightColor + ";\">" +
                          "Pixabay</a>"

                    onLinkActivated: {
                        if (link.indexOf("https://") === 0 ||
                            link.indexOf("http://") === 0) {
                            Qt.openUrlExternally(link)
                        }
                    }
                }
            }

            // Crickets
            Column {
                width: parent.width
                spacing: Theme.paddingSmall

                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin

                    text: qsTr("Crickets")
                    color: Theme.secondaryHighlightColor
                    font.pixelSize: Theme.fontSizeMedium
                }

                Text {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin

                    textFormat: Text.RichText
                    wrapMode: Text.WordWrap

                    color: Theme.primaryColor
                    font.pixelSize: Theme.fontSizeSmall

                    text: qsTr("Sound Effect by ") +
                          "<a href=\"https://pixabay.com/users/freesound_community-46691455/?utm_source=link-attribution&utm_medium=referral&utm_campaign=music&utm_content=7015\" " +
                          "style=\"color:" + Theme.highlightColor + ";\">" +
                          "freesound_community</a> " +
                          qsTr("from ") +
                          "<a href=\"https://pixabay.com//?utm_source=link-attribution&utm_medium=referral&utm_campaign=music&utm_content=7015\" " +
                          "style=\"color:" + Theme.highlightColor + ";\">" +
                          "Pixabay</a>"

                    onLinkActivated: {
                        if (link.indexOf("https://") === 0 ||
                            link.indexOf("http://") === 0) {
                            Qt.openUrlExternally(link)
                        }
                    }
                }
            }

            // Sea Waves
            Column {
                width: parent.width
                spacing: Theme.paddingSmall

                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin

                    text: qsTr("Sea Waves")
                    color: Theme.secondaryHighlightColor
                    font.pixelSize: Theme.fontSizeMedium
                }

                Text {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin

                    textFormat: Text.RichText
                    wrapMode: Text.WordWrap

                    color: Theme.primaryColor
                    font.pixelSize: Theme.fontSizeSmall

                    text: qsTr("Sound Effect by ") +
                          "<a href=\"https://pixabay.com/users/soundreality-31074404/?utm_source=link-attribution&utm_medium=referral&utm_campaign=music&utm_content=611954\" " +
                          "style=\"color:" + Theme.highlightColor + ";\">" +
                          "Jurij</a> " +
                          qsTr("from ") +
                          "<a href=\"https://pixabay.com/sound-effects//?utm_source=link-attribution&utm_medium=referral&utm_campaign=music&utm_content=611954\" " +
                          "style=\"color:" + Theme.highlightColor + ";\">" +
                          "Pixabay</a>"

                    onLinkActivated: {
                        if (link.indexOf("https://") === 0 ||
                            link.indexOf("http://") === 0) {
                            Qt.openUrlExternally(link)
                        }
                    }
                }
            }

            // Birds
            Column {
                width: parent.width
                spacing: Theme.paddingSmall

                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin

                    text: qsTr("Birds")
                    color: Theme.secondaryHighlightColor
                    font.pixelSize: Theme.fontSizeMedium
                }

                Text {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin

                    textFormat: Text.RichText
                    wrapMode: Text.WordWrap

                    color: Theme.primaryColor
                    font.pixelSize: Theme.fontSizeSmall

                    text: qsTr("Sound Effect by ") +
                          "<a href=\"https://pixabay.com/users/soundreality-31074404/?utm_source=link-attribution&utm_medium=referral&utm_campaign=music&utm_content=576540\" " +
                          "style=\"color:" + Theme.highlightColor + ";\">" +
                          "Jurij</a> " +
                          qsTr("from ") +
                          "<a href=\"https://pixabay.com//?utm_source=link-attribution&utm_medium=referral&utm_campaign=music&utm_content=576540\" " +
                          "style=\"color:" + Theme.highlightColor + ";\">" +
                          "Pixabay</a>"

                    onLinkActivated: {
                        if (link.indexOf("https://") === 0 ||
                            link.indexOf("http://") === 0) {
                            Qt.openUrlExternally(link)
                        }
                    }
                }
            }

            // Fire
            Column {
                width: parent.width
                spacing: Theme.paddingSmall

                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin

                    text: qsTr("Fire")
                    color: Theme.secondaryHighlightColor
                    font.pixelSize: Theme.fontSizeMedium
                }

                Text {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin

                    textFormat: Text.RichText
                    wrapMode: Text.WordWrap

                    color: Theme.primaryColor
                    font.pixelSize: Theme.fontSizeSmall

                    text: qsTr("Sound Effect by ") +
                          "<a href=\"https://pixabay.com/users/soundreality-31074404/?utm_source=link-attribution&utm_medium=referral&utm_campaign=music&utm_content=528618\" " +
                          "style=\"color:" + Theme.highlightColor + ";\">" +
                          "Jurij</a> " +
                          qsTr("from ") +
                          "<a href=\"https://pixabay.com/sound-effects//?utm_source=link-attribution&utm_medium=referral&utm_campaign=music&utm_content=528618\" " +
                          "style=\"color:" + Theme.highlightColor + ";\">" +
                          "Pixabay</a>"

                    onLinkActivated: {
                        if (link.indexOf("https://") === 0 ||
                            link.indexOf("http://") === 0) {
                            Qt.openUrlExternally(link)
                        }
                    }
                }
            }

            // Chimes
            Column {
                width: parent.width
                spacing: Theme.paddingSmall

                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin

                    text: qsTr("Chimes")
                    color: Theme.secondaryHighlightColor
                    font.pixelSize: Theme.fontSizeMedium
                }

                Text {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin

                    textFormat: Text.RichText
                    wrapMode: Text.WordWrap

                    color: Theme.primaryColor
                    font.pixelSize: Theme.fontSizeSmall

                    text: qsTr("Sound Effect by ") +
                          "<a href=\"https://pixabay.com/users/freesound_community-46691455/?utm_source=link-attribution&utm_medium=referral&utm_campaign=music&utm_content=57238\" " +
                          "style=\"color:" + Theme.highlightColor + ";\">" +
                          "freesound_community</a> " +
                          qsTr("from ") +
                          "<a href=\"https://pixabay.com/sound-effects//?utm_source=link-attribution&utm_medium=referral&utm_campaign=music&utm_content=57238\" " +
                          "style=\"color:" + Theme.highlightColor + ";\">" +
                          "Pixabay</a>"

                    onLinkActivated: {
                        if (link.indexOf("https://") === 0 ||
                            link.indexOf("http://") === 0) {
                            Qt.openUrlExternally(link)
                        }
                    }
                }
            }

            // Stream
            Column {
                width: parent.width
                spacing: Theme.paddingSmall

                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin

                    text: qsTr("Stream")
                    color: Theme.secondaryHighlightColor
                    font.pixelSize: Theme.fontSizeMedium
                }

                Text {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin

                    textFormat: Text.RichText
                    wrapMode: Text.WordWrap

                    color: Theme.primaryColor
                    font.pixelSize: Theme.fontSizeSmall

                    text: qsTr("Sound Effect by ") +
                          "<a href=\"https://pixabay.com/users/soundreality-31074404/?utm_source=link-attribution&utm_medium=referral&utm_campaign=music&utm_content=360596\" " +
                          "style=\"color:" + Theme.highlightColor + ";\">" +
                          "Jurij</a> " +
                          qsTr("from ") +
                          "<a href=\"https://pixabay.com//?utm_source=link-attribution&utm_medium=referral&utm_campaign=music&utm_content=360596\" " +
                          "style=\"color:" + Theme.highlightColor + ";\">" +
                          "Pixabay</a>"

                    onLinkActivated: {
                        if (link.indexOf("https://") === 0 ||
                            link.indexOf("http://") === 0) {
                            Qt.openUrlExternally(link)
                        }
                    }
                }
            }
        }

        VerticalScrollDecorator {}
    }
}
