import QtQuick 2.0
import Sailfish.Silica 1.0
import Binaural 1.0

SilicaFlickable {
    id: root

    property var mainPage
    property var engine

    signal openCustomBinauralRequested()
    signal openNoiseWaveRequested()
    signal openAmbienceRequested()
    signal openSoundPickerRequested()

    signal binauralInfoRequested()
    signal coloredNoiseInfoRequested()

    contentHeight: standardColumn.height + Theme.paddingLarge
    clip: true

    Column {
        id: standardColumn

        width: parent.width
        spacing: Theme.paddingSmall

        // ── Binaural Beats ──

        CompactSectionHeader {
            text: qsTr("Binaural beats")
        }

        Item {
            width: parent.width

            height: headsetHintBinaural.height
                    + Theme.paddingSmall
                    + frequencyGrid.height

            HeadsetHint {
                id: headsetHintBinaural
            }

            SilicaGridView {
                id: frequencyGrid

                y: headsetHintBinaural.height + Theme.paddingSmall

                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                height: contentHeight

                cellWidth: width / 3
                cellHeight: Theme.itemSizeMedium

                model: ListModel {
                    ListElement { bandName: "Delta"; symbol: "δ" }
                    ListElement { bandName: "Delta/Theta"; symbol: "δ / θ" }
                    ListElement { bandName: "Theta"; symbol: "θ" }
                    ListElement { bandName: "Alpha"; symbol: "α" }
                    ListElement { bandName: "Beta"; symbol: "β" }
                    ListElement { bandName: "Gamma"; symbol: "γ" }
                }

                delegate: Item {
                    width: frequencyGrid.cellWidth
                    height: frequencyGrid.cellHeight

                    SoundButton {
                        anchors.centerIn: parent

                        preferredWidth: frequencyGrid.cellWidth -
                                        Theme.paddingMedium

                        text: symbol

                        selected: root.mainPage.activeBand === bandName
                        playing: root.mainPage.bandPlaying

                        onClicked: {
                            if (selected) {
                                root.mainPage.activeBand = ""
                                root.mainPage.isCustomBinaural = false
                                root.engine.stop()

                            } else if (!root.mainPage.pausedMode) {
                                root.mainPage.activeBand = bandName
                                root.mainPage.isCustomBinaural = false
                                root.mainPage.currentCarrier = 200
                                root.mainPage.currentBeat =
                                        root.mainPage.bandBeat(bandName)

                                root.engine.setFrequencyBand(bandName)
                                root.engine.start()

                            } else {
                                root.engine.stop()

                                root.mainPage.activeBand = bandName
                                root.mainPage.isCustomBinaural = false
                                root.mainPage.currentCarrier = 200
                                root.mainPage.currentBeat =
                                        root.mainPage.bandBeat(bandName)

                                root.engine.setFrequencyBand(bandName)
                            }
                        }
                    }
                }
            }
        }

        // Custom binaural button

        Item {
            width: parent.width
            height: customFrequenciesButton.height

            SoundButton {
                id: customFrequenciesButton

                anchors.centerIn: parent

                preferredWidth: frequencyGrid.width -
                                Theme.paddingMedium

                secondary: true

                text: root.mainPage.activeBand !== ""
                      ? Math.round(root.mainPage.currentCarrier) + " Hz / " +
                        root.mainPage.currentBeat.toFixed(1) + " Hz" +
                        (root.mainPage.beatBand(
                             root.mainPage.currentBeat) !== ""
                         ? " - " + SoundNames.label(
                             root.mainPage.beatBand(
                                 root.mainPage.currentBeat))
                         : "")
                      : ""

                iconSource: root.mainPage.activeBand === ""
                            ? "../../images/icon-m-soundsettings.svg"
                            : ""

                selected: root.mainPage.isCustomBinaural &&
                          root.mainPage.activeBand !== ""

                playing: root.mainPage.bandPlaying

                onClicked: root.openCustomBinauralRequested()

                // Enable InfoBinauralPage-Button
                showInfoButton: true
                onInfoClicked: root.binauralInfoRequested()
            }
        }

        VolumeSliderRow {
            width: parent.width - 2 * Theme.horizontalPageMargin
            x: Theme.horizontalPageMargin

            enabled: root.mainPage.activeBand !== ""

            onValueChanged:
                root.engine.setBinauralVolume(value)
        }

        // ── Colored Noise ──

        CompactSectionHeader {
            text: qsTr("Colored noise")
        }

        SilicaGridView {
            id: coloredNoiseGrid

            x: Theme.horizontalPageMargin
            width: parent.width - 2 * Theme.horizontalPageMargin
            height: contentHeight

            cellWidth: width / 2
            cellHeight: Theme.itemSizeMedium

            model: ["White", "Pink", "Brown", "Grey"]

            delegate: Item {
                width: coloredNoiseGrid.cellWidth
                height: coloredNoiseGrid.cellHeight

                SoundButton {
                    anchors.centerIn: parent

                    preferredWidth: coloredNoiseGrid.cellWidth -
                                    Theme.paddingMedium

                    text: SoundNames.label(modelData)

                    selected: root.mainPage.activeNoise === modelData
                    playing: root.mainPage.noisePlaying

                    onClicked: {
                        if (selected) {
                            root.mainPage.activeNoise = ""
                            root.engine.stopColoredNoise()

                        } else if (!root.mainPage.pausedMode) {
                            root.mainPage.activeNoise = modelData
                            root.engine.setColoredNoise(modelData)

                        } else {
                            root.engine.stopColoredNoise()
                            root.mainPage.activeNoise = modelData
                        }
                    }
                }
            }
        }

        // Wave motion button

        Item {
            width: parent.width
            height: waveMotionButton.height

            SoundButton {
                id: waveMotionButton

                anchors.centerIn: parent

                preferredWidth: frequencyGrid.width -
                                Theme.paddingMedium

                secondary: true
                text: ""

                iconSource: "../../images/icon-m-waves.svg"

                selected: root.mainPage.noiseWavesEnabled
                playing: root.mainPage.noisePlaying

                onClicked: root.openNoiseWaveRequested()

                // Enable InfoColoredPage-Button
                showInfoButton: true
                onInfoClicked: root.coloredNoiseInfoRequested()
            }
        }

        VolumeSliderRow {
            width: parent.width - 2 * Theme.horizontalPageMargin
            x: Theme.horizontalPageMargin

            enabled: root.mainPage.activeNoise !== ""

            onValueChanged:
                root.engine.setColoredNoiseVolume(value)
        }

        // ── Ambient Sound ──

        CompactSectionHeader {
            text: qsTr("Ambient sounds")
        }

        // Nature sounds button
        Item {
            width: parent.width
            height: ambienceButton.height

            SoundButton {
                id: ambienceButton

                topPadding: Theme.paddingSmall * 1.5

                anchors.centerIn: parent

                preferredWidth: frequencyGrid.width -
                                Theme.paddingMedium

                text: root.mainPage.activeAmbiences.length === 0
                      ? qsTr("Nature sounds")
                      : root.mainPage.activeAmbiences.map(function(a) {
                            return SoundNames.label(a)
                        }).join(", ")

                iconSource: root.mainPage.activeAmbiences.length === 0
                            ? "image://theme/icon-m-add"
                            : ""

                selected: root.mainPage.activeAmbiences.length > 0
                playing: root.mainPage.ambiencePlaying

                onClicked: root.openAmbienceRequested()
            }
        }

        Item {
            width: parent.width
            height: Theme.paddingSmall / 2
        }

        // Custom sound button
        Item {
            width: parent.width
            height: customSoundButton.height

            SoundButton {
                id: customSoundButton

                topPadding: Theme.paddingSmall

                anchors.centerIn: parent

                preferredWidth: frequencyGrid.width -
                                Theme.paddingMedium

                text: root.engine.customSoundName !== ""
                      ? root.engine.customSoundName
                      : SoundNames.label("Custom sound")

                iconSource: root.engine.customSoundName === ""
                            ? "image://theme/icon-m-add"
                            : ""

                enabled: root.mainPage.customSoundSelected ||
                         root.mainPage.activeAmbiences.length < 3

                selected: root.mainPage.customSoundSelected
                playing: root.mainPage.customSoundPlaying

                onClicked: root.openSoundPickerRequested()
            }
        }
    }

    VerticalScrollDecorator {}
}
