import QtQuick 2.0
import Sailfish.Silica 1.0
import Binaural 1.0
import "components"

Page {
    id: page

    AudioEngine {
        id: audioEngine
    }

    property string activeBand: ""
    property string activeAmbience: ""
    property string activeNoise: ""
    property bool isPlaying: false

    onIsPlayingChanged: {
        appWindow.isPlaying = isPlaying
    }

    function togglePlayback() {
        if (isPlaying) {
            audioEngine.stop()
            audioEngine.stopAmbience()
            audioEngine.stopColoredNoise()
            isPlaying = false
        } else {
            if (activeBand !== "") {
                audioEngine.setFrequencyBand(activeBand)
                audioEngine.start()
            }

            if (activeAmbience !== "") {
                audioEngine.setAmbience(activeAmbience)
            }

            if (activeNoise !== "") {
                audioEngine.setColoredNoise(activeNoise)
            }

            isPlaying = true
        }
    }

    Component {
        id: infoBinauralPageComponent

        InfoBinauralPage {
        }
    }

    Component {
        id: infoColoredPageComponent

        InfoColoredPage {
        }
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        PullDownMenu {
            MenuItem {
                text: qsTr("About")

                onClicked: {
                    pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
                }
            }
        }

        Column {
            id: column

            width: parent.width
            spacing: Theme.paddingMedium

            PageHeader {
                title: "Binaural"
            }

            // Binaural Beats
            SectionHeader {
                text: qsTr("Binaural frequency range")
            }

            // Volume slider and info button for Binaural
            Item {
                width: parent.width
                height: binauralVolumeSlider.height

                Slider {
                    id: binauralVolumeSlider

                    anchors.left: parent.left
                    anchors.leftMargin: Theme.horizontalPageMargin
                    anchors.right: infoBinauralsButton.left
                    anchors.rightMargin: Theme.paddingLarge

                    minimumValue: 0
                    maximumValue: 100
                    value: 50

                    label: qsTr("Binaural volume")

                    onValueChanged: {
                        audioEngine.setBinauralVolume(value)
                    }
                }

                IconButton {
                    id: infoBinauralsButton

                    anchors.right: parent.right
                    anchors.rightMargin: Theme.horizontalPageMargin
                    anchors.verticalCenter: parent.verticalCenter

                    icon.source: "image://theme/icon-m-about?" + (pressed
                                 ? Theme.highlightColor
                                 : Theme.primaryColor)

                    onClicked: {
                        pageStack.push(infoBinauralPageComponent)
                    }
                }
            }

            SilicaGridView {
                id: frequencyGrid

                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                height: contentHeight

                cellWidth: width / 2
                cellHeight: Theme.itemSizeMedium

                model: [
                    "Delta",
                    "Delta/Theta",
                    "Theta",
                    "Alpha",
                    "Beta",
                    "Gamma"
                ]

                delegate: Item {
                    width: frequencyGrid.cellWidth
                    height: frequencyGrid.cellHeight

                    Rectangle {
                        anchors.fill: frequencyButton
                        anchors.margins: -Theme.paddingSmall

                        radius: Theme.paddingSmall

                        color: frequencyButton.highlighted
                               ? Theme.highlightColor
                               : "transparent"

                        opacity: frequencyButton.highlighted ? 0.15 : 0.0

                        Behavior on opacity {
                            NumberAnimation {
                                duration: 150
                            }
                        }
                    }

                    Button {
                        id: frequencyButton

                        anchors.centerIn: parent

                        width: frequencyGrid.cellWidth - Theme.paddingMedium
                        height: frequencyGrid.cellHeight - Theme.paddingMedium

                        text: modelData
                        highlighted: page.activeBand === modelData

                        onClicked: {
                            if (frequencyButton.highlighted) {
                                page.activeBand = ""
                                audioEngine.stop()
                                page.isPlaying = false
                            } else {
                                page.activeBand = modelData
                                audioEngine.setFrequencyBand(modelData)
                                audioEngine.start()
                                page.isPlaying = true
                            }
                        }

                        Label {
                            anchors.centerIn: parent

                            text: parent.text
                            color: frequencyButton.highlighted
                                   ? Theme.highlightColor
                                   : Theme.primaryColor

                            font.pixelSize: Theme.fontSizeMedium
                        }
                    }
                }
            }

            // Colored Noise section
            SectionHeader {
                text: qsTr("Color Noise")
            }

            // Volume slider and info button for Colored Noise
            Item {
                width: parent.width
                height: coloredNoiseVolumeSlider.height

                Slider {
                    id: coloredNoiseVolumeSlider

                    anchors.left: parent.left
                    anchors.leftMargin: Theme.horizontalPageMargin
                    anchors.right: infoColoredButton.left
                    anchors.rightMargin: Theme.paddingLarge

                    minimumValue: 0
                    maximumValue: 100
                    value: 50

                    label: qsTr("Color noise volume")

                    onValueChanged: {
                        audioEngine.setColoredNoiseVolume(value)
                    }
                }

                IconButton {
                    id: infoColoredButton

                    anchors.right: parent.right
                    anchors.rightMargin: Theme.horizontalPageMargin
                    anchors.verticalCenter: parent.verticalCenter

                    icon.source: "image://theme/icon-m-about?" + (pressed
                                 ? Theme.highlightColor
                                 : Theme.primaryColor)

                    onClicked: {
                        pageStack.push(infoColoredPageComponent)
                    }
                }
            }

            SilicaGridView {
                id: coloredNoiseGrid

                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                height: contentHeight

                cellWidth: width / 2
                cellHeight: Theme.itemSizeMedium

                model: [
                    qsTr("White"),
                    qsTr("Pink"),
                    qsTr("Brown"),
                    qsTr("Grey")
                ]

                delegate: Item {
                    width: coloredNoiseGrid.cellWidth
                    height: coloredNoiseGrid.cellHeight

                    Rectangle {
                        anchors.fill: coloredNoiseButton
                        anchors.margins: -Theme.paddingSmall

                        radius: Theme.paddingSmall

                        color: coloredNoiseButton.highlighted
                               ? Theme.highlightColor
                               : "transparent"

                        opacity: coloredNoiseButton.highlighted ? 0.15 : 0.0

                        Behavior on opacity {
                            NumberAnimation {
                                duration: 150
                            }
                        }
                    }

                    Button {
                        id: coloredNoiseButton

                        anchors.centerIn: parent

                        width: coloredNoiseGrid.cellWidth - Theme.paddingMedium
                        height: coloredNoiseGrid.cellHeight - Theme.paddingMedium

                        text: modelData
                        highlighted: page.activeNoise === modelData

                        onClicked: {
                            if (coloredNoiseButton.highlighted) {
                                page.activeNoise = ""
                                audioEngine.stopColoredNoise()
                                page.isPlaying = false
                            } else {
                                page.activeNoise = modelData
                                audioEngine.setColoredNoise(modelData)
                                page.isPlaying = true
                            }
                        }

                        Label {
                            anchors.centerIn: parent

                            text: parent.text

                            color: coloredNoiseButton.highlighted
                                   ? Theme.highlightColor
                                   : Theme.primaryColor

                            font.pixelSize: Theme.fontSizeMedium
                        }
                    }
                }
            }

            // Ambience section
            SectionHeader {
                text: qsTr("Ambiences")
            }

            // Volume slider without info button for Ambience sounds
            Item {
                width: parent.width
                height: ambienceVolumeSlider.height

                Slider {
                    id: ambienceVolumeSlider

                    anchors.left: parent.left
                    anchors.leftMargin: Theme.horizontalPageMargin
                    anchors.right: ambienceInfoSpace.left
                    anchors.rightMargin: Theme.paddingLarge

                    minimumValue: 0
                    maximumValue: 100
                    value: 50

                    label: qsTr("Ambience volume")

                    onValueChanged: {
                        audioEngine.setAmbienceVolume(value)
                    }
                }

                Item {
                    id: ambienceInfoSpace

                    width: infoBinauralsButton.width
                    height: infoBinauralsButton.height

                    anchors.right: parent.right
                    anchors.rightMargin: Theme.horizontalPageMargin
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            SilicaGridView {
                id: ambienceGrid

                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                height: contentHeight

                cellWidth: width / 2
                cellHeight: Theme.itemSizeMedium

                model: [
                    qsTr("Wind"),
                    qsTr("Sea Waves"),
                    qsTr("Crickets"),
                    qsTr("Stream"),
                    qsTr("Rain"),
                    qsTr("Birds")
                ]

                delegate: Item {
                    width: ambienceGrid.cellWidth
                    height: ambienceGrid.cellHeight

                    Rectangle {
                        anchors.fill: ambienceButton
                        anchors.margins: -Theme.paddingSmall

                        radius: Theme.paddingSmall

                        color: ambienceButton.highlighted
                               ? Theme.highlightColor
                               : "transparent"

                        opacity: ambienceButton.highlighted ? 0.15 : 0.0

                        Behavior on opacity {
                            NumberAnimation {
                                duration: 150
                            }
                        }
                    }

                    Button {
                        id: ambienceButton

                        anchors.centerIn: parent

                        width: ambienceGrid.cellWidth - Theme.paddingMedium
                        height: ambienceGrid.cellHeight - Theme.paddingMedium

                        text: modelData
                        highlighted: page.activeAmbience === modelData

                        onClicked: {
                            if (ambienceButton.highlighted) {
                                page.activeAmbience = ""
                                audioEngine.stopAmbience()
                                page.isPlaying = false
                            } else {
                                page.activeAmbience = modelData
                                audioEngine.setAmbience(modelData)
                                page.isPlaying = true
                            }
                        }

                        Label {
                            anchors.centerIn: parent

                            text: parent.text
                            color: ambienceButton.highlighted
                                   ? Theme.highlightColor
                                   : Theme.primaryColor

                            font.pixelSize: Theme.fontSizeMedium
                        }
                    }
                }
            }
        }
    }
}
