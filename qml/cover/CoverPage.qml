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
            color: Theme.highlightColor
            font.pixelSize: Theme.fontSizeMedium
            wrapMode: Text.WordWrap

            text: (activeBand === "" && activeAmbience === "" && activeNoise === "")
                  ? qsTr("Nothing being played")
                  : qsTr("Playing")
        }

        Label {
            width: parent.width

            horizontalAlignment: Text.AlignHCenter
            color: Theme.primaryColor
            font.pixelSize: Theme.fontSizeMedium
            wrapMode: Text.WordWrap

            text: (activeBand !== "" ? activeBand : "") +
                  (activeBand !== "" && (activeAmbience !== "" || activeNoise !== "") ? "\n" : "") +
                  (activeAmbience !== "" ? activeAmbience : "") +
                  (activeAmbience !== "" && activeNoise !== "" ? "\n" : "") +
                  (activeNoise !== "" ? activeNoise : "")
        }
    }

    // Nothing selected: show "new" button
    CoverActionList {
        id: newActionList

        enabled: activeBand === "" && activeAmbience === "" && activeNoise === ""

        CoverAction {
            iconSource: "image://theme/icon-cover-new"

            onTriggered: {
                appWindow.activate()
            }
        }
    }

    // Something selected: show play/pause button
    CoverActionList {
        id: playbackActionList

        enabled: activeBand !== "" || activeAmbience !== "" || activeNoise !== ""

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
