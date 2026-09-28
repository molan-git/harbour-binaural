import QtQuick 2.0
import Sailfish.Silica 1.0

CoverBackground {
    id: cover

    property string activeBand: ""
    property string activeAmbience: ""
    property string activeNoise: ""
    property bool isPlaying: false

    signal playPauseClicked()

    Column {
        anchors.centerIn: parent
        width: parent.width - 2 * Theme.paddingLarge
        spacing: Theme.paddingSmall

        Label {
            width: parent.width

            horizontalAlignment: Text.AlignHCenter
            color: Theme.secondaryColor
            font.pixelSize: Theme.fontSizeMedium
            wrapMode: Text.WordWrap

            text: (activeBand === "" && activeAmbience === "" && activeNoise === "")
                  ? qsTr("Start your sound mix")
                  : qsTr("Playing:")
        }

        Label {
            width: parent.width

            horizontalAlignment: Text.AlignHCenter
            color: Theme.highlightColor
            font.pixelSize: Theme.fontSizeMedium
            wrapMode: Text.WordWrap

            text: (activeBand !== "" ? activeBand : "") +
                  (activeBand !== "" && (activeNoise !== "" || activeAmbience !== "") ? "\n" : "") +
                  (activeNoise !== "" ? activeNoise : "") +
                  (activeNoise !== "" && activeAmbience !== "" ? "\n" : "") +
                  (activeAmbience !== "" ? activeAmbience : "")
        }

    }

    // Something selected: show play/pause button
    CoverActionList {
        id: playbackActionList

        enabled: activeBand !== "" ||
                 activeAmbience !== "" ||
                 activeNoise !== ""

        CoverAction {
            iconSource: isPlaying
                        ? "image://theme/icon-cover-pause"
                        : "image://theme/icon-cover-play"

            onTriggered: {
                playPauseClicked()
            }
        }
    }
}
