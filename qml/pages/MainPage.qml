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

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        PullDownMenu {
            MenuItem {
                text: "About"

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


            // Bineaural frequency section
            SectionHeader {
                text: "Frequency range"
            }

            IconButton {
                id: infoBinauralsButton

                anchors.right: parent.right
                anchors.rightMargin: Theme.horizontalPageMargin

                icon.source: "image://theme/icon-m-about?" + (pressed
                             ? Theme.highlightColor
                             : Theme.primaryColor)

                onClicked: {
                    pageStack.push(infoBinauralPageComponent)
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

            Slider {
                width: parent.width - 2 * Theme.horizontalPageMargin
                anchors.horizontalCenter: parent.horizontalCenter

                minimumValue: 0
                maximumValue: 100
                value: 50

                label: "Binaural volume"

                onValueChanged: {
                    audioEngine.setBinauralVolume(value)
                }
            }

            // Colored Noise section
            SectionHeader {
                text: "Colored Noise"
            }

            SilicaGridView {
                id: coloredNoiseGrid

                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                height: contentHeight

                cellWidth: width / 2
                cellHeight: Theme.itemSizeMedium

                model: [
                    "White",
                    "Pink",
                    "Brown",
                    "Grey"
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

            Slider {
                id: coloredNoiseVolumeSlider

                width: parent.width - 2 * Theme.horizontalPageMargin
                anchors.horizontalCenter: parent.horizontalCenter

                minimumValue: 0
                maximumValue: 100
                value: 50

                label: "Colored noise volume"

                onValueChanged: {
                    audioEngine.setColoredNoiseVolume(value)
                }
            }

            // Ambience section
            SectionHeader {
                text: "Ambiences"
            }

            SilicaGridView {
                id: ambienceGrid

                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                height: contentHeight

                cellWidth: width / 2
                cellHeight: Theme.itemSizeMedium

                model: [
                    "Wind",
                    "Sea Waves",
                    "Crickets",
                    "Stream",
                    "Rain"
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

            Slider {
                width: parent.width - 2 * Theme.horizontalPageMargin
                anchors.horizontalCenter: parent.horizontalCenter

                minimumValue: 0
                maximumValue: 100
                value: 50

                label: "Ambience volume"

                onValueChanged: {
                    audioEngine.setAmbienceVolume(value)
                }
            }
        }
    }
}
