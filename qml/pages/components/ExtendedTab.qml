import QtQuick 2.0
import Sailfish.Silica 1.0
import Binaural 1.0

SilicaFlickable {
    id: root

    property var mainPage
    property var engine

    signal openSolfeggioDrawerRequested()
    signal openIsochronicDrawerRequested()

    signal solfeggioInfoRequested()
    signal isochronicInfoRequested()
    signal sequenceInfoRequested()

    signal openSequenceDrawerRequested()
    signal openFocusDrawerRequested()

    contentHeight: extendedColumn.height + Theme.paddingLarge
    clip: true

    Column {
        id: extendedColumn

        width: parent.width
        spacing: Theme.paddingSmall

        // ── Solfeggio ──

        CompactSectionHeader {
            text: SoundNames.label("Solfeggio frequencies")
        }

        Item {
            width: parent.width
            height: solfeggioButton.height

            SoundButton {
                id: solfeggioButton

                topPadding: Theme.paddingSmall * 1.5

                anchors.centerIn: parent

                preferredWidth: isochronicGrid.width -
                                Theme.paddingMedium

                text: root.mainPage.activeSolfeggio.length > 0
                      ? root.mainPage.activeSolfeggio.map(function(f) {
                            return f + " Hz"
                        }).join(" + ")
                      : ""

                iconSource: root.mainPage.activeSolfeggio.length === 0
                            ? "image://theme/icon-m-add"
                            : ""

                selected: root.mainPage.activeSolfeggio.length > 0
                playing: root.mainPage.solfeggioPlaying

                onClicked: root.openSolfeggioDrawerRequested()

                showInfoButton: true
                onInfoClicked: root.solfeggioInfoRequested()
            }
        }

        VolumeSliderRow {
            width: parent.width - 2 * Theme.horizontalPageMargin
            x: Theme.horizontalPageMargin

            enabled: root.mainPage.activeSolfeggio.length > 0

            onValueChanged:
                root.engine.setSolfeggioVolume(value)
        }

        // ── Isochronic Tones ──

        CompactSectionHeader {
            text: SoundNames.label("Isochronic tones")
        }

        Item {
            width: parent.width

            height: headsetHintIsochronic.height +
                    Theme.paddingSmall +
                    isochronicGrid.height

            HeadsetHint {
                id: headsetHintIsochronic
            }

            SilicaGridView {
                id: isochronicGrid

                y: headsetHintIsochronic.height + Theme.paddingSmall

                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                height: contentHeight

                cellWidth: width / 3
                cellHeight: Theme.itemSizeMedium

                model: [
                    "1 Hz", "2 Hz", "4 Hz",
                    "6 Hz", "7.83 Hz", "10 Hz"
                ]

                delegate: Item {
                    width: isochronicGrid.cellWidth
                    height: isochronicGrid.cellHeight

                    SoundButton {
                        anchors.centerIn: parent

                        preferredWidth: isochronicGrid.cellWidth -
                                        Theme.paddingMedium

                        text: SoundNames.label(modelData)

                        selected: root.mainPage.activeIsochronic === modelData
                        playing: root.mainPage.isochronicPlaying

                        onClicked: {
                            if (selected) {
                                root.mainPage.activeIsochronic = ""
                                root.mainPage.isCustomIsochronic = false
                                root.engine.stopIsochronic()

                            } else if (!root.mainPage.pausedMode) {
                                root.mainPage.selectIsochronic(modelData)

                            } else {
                                root.engine.stopIsochronic()

                                root.mainPage.selectIsochronic(modelData)
                            }
                        }
                    }
                }
            }
        }

        // Custom isochronic button

        Item {
            width: parent.width
            height: customIsochronicButton.height

            SoundButton {
                id: customIsochronicButton

                anchors.centerIn: parent

                preferredWidth: isochronicGrid.width -
                                Theme.paddingMedium

                secondary: true

                text: root.mainPage.activeIsochronic !== ""
                      ? Math.round(root.mainPage.currentIsoCarrier) + " Hz / " +
                        root.mainPage.currentIsoPulse.toFixed(2) + " Hz"
                      : ""

                iconSource: root.mainPage.activeIsochronic === ""
                            ? "../../images/icon-m-soundsettings.svg"
                            : ""

                selected: root.mainPage.isCustomIsochronic &&
                          root.mainPage.activeIsochronic !== ""

                playing: root.mainPage.isochronicPlaying

                onClicked: root.openIsochronicDrawerRequested()

                showInfoButton: true
                onInfoClicked: root.isochronicInfoRequested()
            }
        }

        VolumeSliderRow {
            width: parent.width - 2 * Theme.horizontalPageMargin
            x: Theme.horizontalPageMargin

            enabled: root.mainPage.activeIsochronic !== ""

            onValueChanged:
                root.engine.setIsochronicVolume(value)
        }

        // ── Focus ──

        CompactSectionHeader {
            text: qsTr("Focus")
        }

        Item {
            width: parent.width

            height: headsetHintFocus.height +
                    Theme.paddingSmall +
                    focusGrid.height

            HeadsetHint {
                id: headsetHintFocus
            }

            SilicaGridView {
                id: focusGrid

                y: headsetHintFocus.height + Theme.paddingSmall

                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                height: contentHeight

                cellWidth: width / 3
                cellHeight: Theme.itemSizeMedium

                model: [
                    { label: "F3",  preset: "Focus 3 (Sleep)" },
                    { label: "F10", preset: "Focus 10 (Body Asleep)" },
                    { label: "F12", preset: "Focus 12 (Expanded Awareness)" },
                    { label: "F15", preset: "Focus 15 (No Time)" },
                    { label: "F21", preset: "Focus 21 (Other Energy Systems)" },
                    { label: "F&C", preset: "Focus & Concentration (Beta)" }
                ]

                delegate: Item {
                    width: focusGrid.cellWidth
                    height: focusGrid.cellHeight

                    SoundButton {
                        anchors.centerIn: parent

                        preferredWidth: focusGrid.cellWidth -
                                        Theme.paddingMedium

                        text: modelData.label

                        selected: root.mainPage.activeFocusPreset ===
                                  modelData.preset
                        playing: root.mainPage.sequencePlaying

                        onClicked:
                            root.mainPage.selectFocus(modelData.preset)
                    }
                }
            }
        }

        // Custom focus button
        Item {
            width: parent.width
            height: customFocusButton.height

            SoundButton {
                id: customFocusButton

                anchors.centerIn: parent

                preferredWidth: focusGrid.width -
                                Theme.paddingMedium

                secondary: true

                text: {
                    if (root.mainPage.activeFocusPreset === "")
                        return ""

                    if (root.mainPage.isCustomFocus)
                        return qsTr("Custom mix")

                    return root.mainPage.activeFocusPreset.replace(
                                /\s*\(.*\)$/, "")
                }

                iconSource: root.mainPage.activeFocusPreset === ""
                            ? "../../images/icon-m-soundsettings.svg"
                            : ""

                selected: root.mainPage.isCustomFocus &&
                          root.mainPage.activeFocusPreset !== ""

                playing: root.mainPage.sequencePlaying

                onClicked: root.openFocusDrawerRequested()

                showInfoButton: true
                onInfoClicked: root.focusInfoRequested()
            }
        }

        VolumeSliderRow {
            width: parent.width - 2 * Theme.horizontalPageMargin
            x: Theme.horizontalPageMargin

            enabled: root.mainPage.activeFocusPreset !== ""

            value: root.engine.sequenceVolume

            onValueChanged:
                root.engine.setSequenceVolume(value)
        }

        // ── Sequences ──

        CompactSectionHeader {
            text: qsTr("Sequences")
        }

        // Sequence selection and playback
        Item {
            width: parent.width

            height: headsetHintSequences.height +
                    Theme.paddingSmall +
                    sequenceButton.height

            HeadsetHint {
                id: headsetHintSequences
            }

            SoundButton {
                id: sequenceButton

                y: headsetHintSequences.height + Theme.paddingSmall * 2

                anchors.horizontalCenter: parent.horizontalCenter

                preferredWidth: isochronicGrid.width - Theme.paddingMedium

                text: root.mainPage.activeSequence !== ""
                      ? root.mainPage.activeSequence
                      : ""

                iconSource: root.mainPage.activeSequence === ""
                            ? "image://theme/icon-m-add"
                            : ""

                selected: root.mainPage.activeSequence !== ""
                playing: root.mainPage.sequencePlaying

                onClicked: root.openSequenceDrawerRequested()

                showInfoButton: true
                onInfoClicked: root.sequenceInfoRequested()
            }
        }

        // Sequence progress and remaining time — press to stop / play again
        Item {
            x: Theme.horizontalPageMargin
            width: parent.width - 2 * Theme.horizontalPageMargin
            height: Theme.itemSizeMedium

            visible: root.mainPage.activeSequence !== ""

            ProgressBar {
                id: sequenceProgress

                anchors.verticalCenter: parent.verticalCenter
                width: parent.width

                minimumValue: 0
                maximumValue: 1

                value: root.engine.sequenceProgress

                valueText: {
                    var remaining = root.engine.sequenceRemainingSeconds
                    var minutes = Math.floor(remaining / 60)
                    var seconds = remaining % 60

                    return minutes + ":" +
                           (seconds < 10 ? "0" : "") + seconds
                }

                highlighted: root.mainPage.sequencePlaying

                opacity: root.mainPage.sequencePlaying ? 1.0 : 0.6
            }

            MouseArea {
                anchors.fill: parent

                onClicked: root.mainPage.toggleSequencePlayback()
            }
        }

        // Sequence volume
        VolumeSliderRow {
            width: parent.width - 2 * Theme.horizontalPageMargin
            x: Theme.horizontalPageMargin

            enabled: root.mainPage.activeSequence !== ""

            onValueChanged:
                root.engine.setSequenceVolume(value)
        }

        Item {
            width: parent.width
            height: Theme.itemSizeMedium
        }
    }

    VerticalScrollDecorator {}
}
