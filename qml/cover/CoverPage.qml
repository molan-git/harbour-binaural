import QtQuick 2.0
import Sailfish.Silica 1.0
import QtGraphicalEffects 1.0


CoverBackground {
    id: cover

    property string activeBand: ""
    property string activeAmbience: ""
    property string activeNoise: ""
    property bool isPlaying: false
    property bool isPaused: false

    signal playPauseClicked()

    Item {
        id: coverBackgroundIcon

        anchors.top: parent.top
        //anchors.topMargin: Theme.paddingSmall
        anchors.horizontalCenter: parent.horizontalCenter

        width: parent.width - 2 * Theme.paddingSmall
        height: parent.height * 0.45

        z: 0

        // Dim the cover art while the system has stopped the sound.
        opacity: isPlaying || (activeBand === "" &&
                              activeAmbience === "" &&
                              activeNoise === "")
                 ? 1.0
                 : 0.6

        Behavior on opacity {
            NumberAnimation {
                duration: 250
            }
        }

        Image {
            id: coverBackgroundImage

            anchors.fill: parent

            source: "../images/binaural-cover.svg"
            sourceSize: Qt.size(
                width * 2,
                height * 2
            )

            fillMode: Image.PreserveAspectFit

            visible: false
        }

        ColorOverlay {
            anchors.fill: coverBackgroundImage

            source: coverBackgroundImage
            color: Theme.primaryColor
        }
    }

    Column {
        id: coverLabels

        anchors.top: coverBackgroundIcon.bottom
        anchors.topMargin: -Theme.paddingLarge
        anchors.left: parent.left
        anchors.right: parent.right

        spacing: Theme.paddingSmall

        Label {
            width: parent.width

            horizontalAlignment: Text.AlignHCenter
            color: Theme.secondaryColor
            font.pixelSize: Theme.fontSizeMedium
            wrapMode: Text.WordWrap

            text: (activeBand === "" && activeAmbience === "" && activeNoise === "")
                  ? qsTr("Start your sound mix")
                  : isPlaying
                    ? qsTr("Playing:")
                    : qsTr("Paused:")
        }

        Label {
            width: parent.width

            horizontalAlignment: Text.AlignHCenter
            // Selected but not playing (system suspend): muted color,
            // full highlight only while actually playing.
            color: isPlaying
                   ? Theme.highlightColor
                   : Theme.secondaryHighlightColor
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
