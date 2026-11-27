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
    signal focusInfoRequested()

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

                cellWidth: width / 4
                cellHeight: Theme.itemSizeMedium

                model: [
                    { label: "F3",     preset: "Focus 3 (Sleep)" },
                    { label: "F10",    preset: "Focus 10 (Body Asleep)" },
                    { label: "F12",    preset: "Focus 12 (Expanded Awareness)" },
                    { label: "F15",    preset: "Focus 15 (No Time)" },
                    { label: "F21/22", preset: "Focus 21/22 (Other Energy Systems)" },
                    { label: "F23",    preset: "Focus 23" },
                    { label: "F24",    preset: "Focus 24" },
                    { label: "F25",    preset: "Focus 25" }
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
                    Theme.paddingSmall * 2 +
                    sequenceButton.height +
                    (sequenceProgress.visible
                     ? sequenceProgress.height + Theme.paddingMedium
                     : 0)

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


            // Sequence progress and remaining time — press to
            // stop / play again. Exactly below the visible part
            // of the sequence button; bar and label are primary
            // while playing and secondary while paused.
            Item {
                id: sequenceProgress

                x: sequenceButton.x
                anchors.top: sequenceButton.bottom
                anchors.topMargin: Theme.paddingMedium

                width: sequenceButton.contentWidth
                height: grooveHeight + remainingLabel.height

                property int grooveHeight: Theme.paddingSmall

                property color progressColor:
                    root.mainPage.sequencePlaying
                    ? Theme.primaryColor
                    : Theme.secondaryColor

                visible: root.mainPage.activeSequence !== ""

                // Groove.
                Rectangle {
                    id: groove

                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.verticalCenterOffset:
                        -remainingLabel.height / 2

                    height: sequenceProgress.grooveHeight
                    radius: height / 2
                    color: Theme.rgba(Theme.secondaryColor, 0.3)
                }

                // Fill.
                Rectangle {
                    anchors.left: parent.left
                    anchors.verticalCenter: groove.verticalCenter

                    width: Math.round(
                        groove.width
                        * Math.max(0.0,
                                   Math.min(1.0,
                                       root.engine.sequenceProgress)))

                    height: groove.height
                    radius: height / 2
                    color: sequenceProgress.progressColor
                    visible: width > 0
                }

                Label {
                    id: remainingLabel

                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom

                    text: {
                        var remaining =
                                root.engine.sequenceRemainingSeconds

                        var minutes = Math.floor(remaining / 60)
                        var seconds = remaining % 60

                        return minutes + ":" +
                               (seconds < 10 ? "0" : "") + seconds
                    }

                    color: sequenceProgress.progressColor
                    font.pixelSize: Theme.fontSizeExtraSmall
                }

                MouseArea {
                    anchors.fill: parent

                    onClicked: root.mainPage.toggleSequencePlayback()
                }
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
