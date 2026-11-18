import QtQuick 2.0
import Sailfish.Silica 1.0
import Binaural 1.0

SilicaFlickable {
    id: root

    property var mainPage
    property var engine

    property real customCarrierHz: 200
    property real customPulseHz: 10

    signal openSolfeggioDrawerRequested()
    signal openIsochronicDrawerRequested()

    signal solfeggioInfoRequested()
    signal isochronicInfoRequested()

    contentHeight: extendedColumn.height + Theme.paddingLarge
    clip: true

    Column {
        id: extendedColumn

        width: parent.width
        spacing: Theme.paddingMedium

        // ── Solfeggio ──

        SectionHeader {
            text: SoundNames.label("Solfeggio frequencies")
        }

        Item {
            width: parent.width
            height: solfeggioButton.height

            SoundButton {
                id: solfeggioButton

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
            }
        }

        VolumeSliderRow {
            width: parent.width - 2 * Theme.horizontalPageMargin
            x: Theme.horizontalPageMargin

            showInfoButton: true
            enabled: root.mainPage.activeSolfeggio.length > 0

            onValueChanged:
                root.engine.setSolfeggioVolume(value)

            onInfoClicked:
                root.solfeggioInfoRequested()
        }

        // ── Isochronic Tones ──

        SectionHeader {
            text: SoundNames.label("Isochronic tones")
        }

        HeadsetHint {}

        SilicaGridView {
            id: isochronicGrid

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
                            root.mainPage.activeIsochronic = modelData
                            root.mainPage.isCustomIsochronic = false

                            root.engine.setIsochronicPreset(modelData)
                            root.engine.startIsochronic()

                        } else {
                            root.engine.stopIsochronic()

                            root.mainPage.activeIsochronic = modelData
                            root.mainPage.isCustomIsochronic = false

                            root.engine.setIsochronicPreset(modelData)
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

                text: root.mainPage.isCustomIsochronic
                      ? Math.round(root.customCarrierHz) + " Hz / " +
                        root.customPulseHz.toFixed(2) + " Hz"
                      : ""

                iconSource: !root.mainPage.isCustomIsochronic
                            ? "../../images/icon-m-soundsettings.svg"
                            : ""

                selected: root.mainPage.isCustomIsochronic
                playing: root.mainPage.isochronicPlaying

                onClicked: root.openIsochronicDrawerRequested()
            }
        }

        VolumeSliderRow {
            width: parent.width - 2 * Theme.horizontalPageMargin
            x: Theme.horizontalPageMargin

            showInfoButton: true
            enabled: root.mainPage.activeIsochronic !== ""

            onValueChanged:
                root.engine.setIsochronicVolume(value)

            onInfoClicked:
                root.isochronicInfoRequested()
        }

        Item {
            width: parent.width
            height: Theme.itemSizeMedium
        }
    }

    VerticalScrollDecorator {}
}
