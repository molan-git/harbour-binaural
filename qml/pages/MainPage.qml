import QtQuick 2.0
import Sailfish.Silica 1.0
import Binaural 1.0

Page {
    id: page

    property string activeBand: ""
    property string activeAmbience: ""

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

            MenuItem {
                text: "Instructions"

                onClicked: {
                    pageStack.push(Qt.resolvedUrl("InstructionsPage.qml"))
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

            SectionHeader {
                text: "Frequency Range"
            }

            Button {
                text: "Delta"
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 2 * Theme.horizontalPageMargin
                highlighted: page.activeBand === "Delta"

                onClicked: {
                    if (page.activeBand === "Delta") {
                        page.activeBand = ""
                        audioEngine.stop()
                    } else {
                        page.activeBand = "Delta"
                        audioEngine.setFrequencyBand("Delta")
                        audioEngine.start()
                    }
                }
            }

            Button {
                text: "Theta"
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 2 * Theme.horizontalPageMargin
                highlighted: page.activeBand === "Theta"

                onClicked: {
                    if (page.activeBand === "Theta") {
                        page.activeBand = ""
                        audioEngine.stop()
                    } else {
                        page.activeBand = "Theta"
                        audioEngine.setFrequencyBand("Theta")
                        audioEngine.start()
                    }
                }
            }

            Button {
                text: "Alpha"
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 2 * Theme.horizontalPageMargin
                highlighted: page.activeBand === "Alpha"

                onClicked: {
                    if (page.activeBand === "Alpha") {
                        page.activeBand = ""
                        audioEngine.stop()
                    } else {
                        page.activeBand = "Alpha"
                        audioEngine.setFrequencyBand("Alpha")
                        audioEngine.start()
                    }
                }
            }

            Button {
                text: "Beta"
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 2 * Theme.horizontalPageMargin
                highlighted: page.activeBand === "Beta"

                onClicked: {
                    if (page.activeBand === "Beta") {
                        page.activeBand = ""
                        audioEngine.stop()
                    } else {
                        page.activeBand = "Beta"
                        audioEngine.setFrequencyBand("Beta")
                        audioEngine.start()
                    }
                }
            }

            Button {
                text: "Gamma"
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 2 * Theme.horizontalPageMargin
                highlighted: page.activeBand === "Gamma"

                onClicked: {
                    if (page.activeBand === "Gamma") {
                        page.activeBand = ""
                        audioEngine.stop()
                    } else {
                        page.activeBand = "Gamma"
                        audioEngine.setFrequencyBand("Gamma")
                        audioEngine.start()
                    }
                }
            }

            Label {
                text: page.activeBand === ""
                      ? "No frequency selected"
                      : "Active: " + page.activeBand

                anchors.horizontalCenter: parent.horizontalCenter
                color: Theme.secondaryColor
            }

            Slider {
                width: parent.width
                minimumValue: 0
                maximumValue: 100
                value: 50

                label: "Binaural volume"

                onValueChanged: {
                    audioEngine.setBinauralVolume(value)
                }
            }

            SectionHeader {
                text: "Ambiences"
            }

            Button {
                text: "Wind"
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 2 * Theme.horizontalPageMargin
                highlighted: page.activeAmbience === "Wind"

                onClicked: {
                    if (page.activeAmbience === "Wind") {
                        page.activeAmbience = ""
                        audioEngine.stopAmbience()
                    } else {
                        page.activeAmbience = "Wind"
                        audioEngine.setAmbience("Wind")
                    }
                }
            }

            Button {
                text: "Sea Waves"
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 2 * Theme.horizontalPageMargin
                highlighted: page.activeAmbience === "Sea Waves"

                onClicked: {
                    if (page.activeAmbience === "Sea Waves") {
                        page.activeAmbience = ""
                        audioEngine.stopAmbience()
                    } else {
                        page.activeAmbience = "Sea Waves"
                        audioEngine.setAmbience("Sea Waves")
                    }
                }
            }

            Button {
                text: "Crickets"
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 2 * Theme.horizontalPageMargin
                highlighted: page.activeAmbience === "Crickets"

                onClicked: {
                    if (page.activeAmbience === "Crickets") {
                        page.activeAmbience = ""
                        audioEngine.stopAmbience()
                    } else {
                        page.activeAmbience = "Crickets"
                        audioEngine.setAmbience("Crickets")
                    }
                }
            }

            Button {
                text: "Stream"
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 2 * Theme.horizontalPageMargin
                highlighted: page.activeAmbience === "Stream"

                onClicked: {
                    if (page.activeAmbience === "Stream") {
                        page.activeAmbience = ""
                        audioEngine.stopAmbience()
                    } else {
                        page.activeAmbience = "Stream"
                        audioEngine.setAmbience("Stream")
                    }
                }
            }

            Button {
                text: "Rain"
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 2 * Theme.horizontalPageMargin
                highlighted: page.activeAmbience === "Rain"

                onClicked: {
                    if (page.activeAmbience === "Rain") {
                        page.activeAmbience = ""
                        audioEngine.stopAmbience()
                    } else {
                        page.activeAmbience = "Rain"
                        audioEngine.setAmbience("Rain")
                    }
                }
            }

            Label {
                text: page.activeAmbience === ""
                      ? "No ambience selected"
                      : "Active: " + page.activeAmbience

                anchors.horizontalCenter: parent.horizontalCenter
                color: Theme.secondaryColor
            }

            Slider {
                width: parent.width
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

    RemorsePopup {
        id: remorsePopup
    }

    AudioEngine {
        id: audioEngine
    }
}
