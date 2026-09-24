import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    // The AudioEngine is passed in by MainPage
    property var audioEngine

    property string activeAmbience: ""

    SilicaFlickable {
        anchors.fill: parent

        contentHeight: contentColumn.height + Theme.paddingLarge

        VerticalScrollDecorator { }

        Column {
            id: contentColumn

            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: "Ambiences"
            }

            SectionHeader {
                text: "Ambiences"
            }

            ButtonLayout {

                Button {
                    text: "Wind"

                    highlighted: page.activeAmbience === "Wind"

                    onClicked: {
                        if (page.activeAmbience === "Wind") {
                            page.activeAmbience = ""
                            page.audioEngine.stopAmbience()
                        } else {
                            page.activeAmbience = "Wind"
                            page.audioEngine.setAmbience("Wind")
                        }
                    }
                }

                Button {
                    text: "Sea Waves"

                    highlighted: page.activeAmbience === "Sea Waves"

                    onClicked: {
                        if (page.activeAmbience === "Sea Waves") {
                            page.activeAmbience = ""
                            page.audioEngine.stopAmbience()
                        } else {
                            page.activeAmbience = "Sea Waves"
                            page.audioEngine.setAmbience("Sea Waves")
                        }
                    }
                }

                Button {
                    text: "Crickets"

                    highlighted: page.activeAmbience === "Crickets"

                    onClicked: {
                        if (page.activeAmbience === "Crickets") {
                            page.activeAmbience = ""
                            page.audioEngine.stopAmbience()
                        } else {
                            page.activeAmbience = "Crickets"
                            page.audioEngine.setAmbience("Crickets")
                        }
                    }
                }

                Button {
                    text: "Stream"

                    highlighted: page.activeAmbience === "Stream"

                    onClicked: {
                        if (page.activeAmbience === "Stream") {
                            page.activeAmbience = ""
                            page.audioEngine.stopAmbience()
                        } else {
                            page.activeAmbience = "Stream"
                            page.audioEngine.setAmbience("Stream")
                        }
                    }
                }

                Button {
                    text: "Rain"

                    highlighted: page.activeAmbience === "Rain"

                    onClicked: {
                        if (page.activeAmbience === "Rain") {
                            page.activeAmbience = ""
                            page.audioEngine.stopAmbience()
                        } else {
                            page.activeAmbience = "Rain"
                            page.audioEngine.setAmbience("Rain")
                        }
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

            SectionHeader {
                text: "Volume"
            }

            Slider {
                width: parent.width

                minimumValue: 0
                maximumValue: 100
                value: 50

                label: "Ambience volume"

                onValueChanged: {
                    if (page.audioEngine) {
                        page.audioEngine.setAmbienceVolume(value)
                    }
                }
            }
        }
    }
}
