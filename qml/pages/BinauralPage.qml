import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    property var audioEngine
    property string activeBand: ""

    SilicaFlickable {
        anchors.fill: parent

        contentHeight: contentColumn.height + Theme.paddingLarge

        VerticalScrollDecorator { }

        Column {
            id: contentColumn

            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: "Binaural"
            }

            SectionHeader {
                text: "Frequency"
            }

            ButtonLayout {

                Button {
                    text: "Delta"

                    highlighted: page.activeBand === "Delta"

                    onClicked: {
                        if (page.activeBand === "Delta") {
                            page.activeBand = ""
                            page.audioEngine.stop()
                        } else {
                            page.activeBand = "Delta"
                            page.audioEngine.setFrequencyBand("Delta")
                            page.audioEngine.start()
                        }
                    }
                }

                Button {
                    text: "Theta"

                    highlighted: page.activeBand === "Theta"

                    onClicked: {
                        if (page.activeBand === "Theta") {
                            page.activeBand = ""
                            page.audioEngine.stop()
                        } else {
                            page.activeBand = "Theta"
                            page.audioEngine.setFrequencyBand("Theta")
                            page.audioEngine.start()
                        }
                    }
                }

                Button {
                    text: "Alpha"

                    highlighted: page.activeBand === "Alpha"

                    onClicked: {
                        if (page.activeBand === "Alpha") {
                            page.activeBand = ""
                            page.audioEngine.stop()
                        } else {
                            page.activeBand = "Alpha"
                            page.audioEngine.setFrequencyBand("Alpha")
                            page.audioEngine.start()
                        }
                    }
                }

                Button {
                    text: "Beta"

                    highlighted: page.activeBand === "Beta"

                    onClicked: {
                        if (page.activeBand === "Beta") {
                            page.activeBand = ""
                            page.audioEngine.stop()
                        } else {
                            page.activeBand = "Beta"
                            page.audioEngine.setFrequencyBand("Beta")
                            page.audioEngine.start()
                        }
                    }
                }

                Button {
                    text: "Gamma"

                    highlighted: page.activeBand === "Gamma"

                    onClicked: {
                        if (page.activeBand === "Gamma") {
                            page.activeBand = ""
                            page.audioEngine.stop()
                        } else {
                            page.activeBand = "Gamma"
                            page.audioEngine.setFrequencyBand("Gamma")
                            page.audioEngine.start()
                        }
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

            SectionHeader {
                text: "Volume"
            }

            Slider {
                width: parent.width

                minimumValue: 0
                maximumValue: 100
                value: 100

                label: "Binaural volume"

                onValueChanged: {
                    if (page.audioEngine) {
                        page.audioEngine.setBinauralVolume(value)
                    }
                }
            }
        }
    }
}
