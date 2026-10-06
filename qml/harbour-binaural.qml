import QtQuick 2.0
import Sailfish.Silica 1.0
import "pages"
import "cover"

ApplicationWindow {
    id: appWindow

    property var mainPage

    property string activeBand: mainPage
                              ? mainPage.activeBand
                              : ""

    property string activeAmbience: mainPage
                                   ? mainPage.activeAmbience
                                   : ""

    property string activeNoise: mainPage
                                ? mainPage.activeNoise
                                : ""

    property bool isPlaying: mainPage
                             ? mainPage.isPlaying
                             : false

    // Sleep Timer
    property bool sleepTimerRunning: false
    property int sleepTimerRemainingSeconds: 0
    property int sleepTimerDurationSeconds: 0

    function startSleepTimer(seconds) {
        if (seconds <= 0)
            return

        appWindow.sleepTimerDurationSeconds = seconds
        appWindow.sleepTimerRemainingSeconds = seconds
        appWindow.sleepTimerRunning = true

        sleepTimer.start()
    }

    function cancelSleepTimer() {
        sleepTimer.stop()

        appWindow.sleepTimerRunning = false
        appWindow.sleepTimerRemainingSeconds = 0
        appWindow.sleepTimerDurationSeconds = 0
    }

    Timer {
        id: sleepTimer

        interval: 1000
        repeat: true

        onTriggered: {
            if (appWindow.sleepTimerRemainingSeconds > 0)
                appWindow.sleepTimerRemainingSeconds--

            if (appWindow.sleepTimerRemainingSeconds <= 0) {
                sleepTimer.stop()

                appWindow.sleepTimerRunning = false
                appWindow.sleepTimerRemainingSeconds = 0
                appWindow.sleepTimerDurationSeconds = 0

                if (appWindow.mainPage)
                    appWindow.mainPage.deselectAllSounds()
            }
        }
    }

    // Something selected but not playing (system suspend or manual
    // stop): dimmed hint state of the playback bar.
    property bool pausedMode: !isPlaying &&
                              (activeBand !== "" ||
                               activeAmbience !== "" ||
                               activeNoise !== "")

    bottomMargin: playbackBar.height

    initialPage: Component {
        MainPage {
            id: mainPageInstance

            Component.onCompleted: {
                appWindow.mainPage = mainPageInstance
            }
        }
    }

    cover: Component {
        CoverPage {
            activeBand: appWindow.activeBand
            activeAmbience: appWindow.activeAmbience
            activeNoise: appWindow.activeNoise
            isPlaying: appWindow.isPlaying
            isPaused: appWindow.pausedMode

            onPlayPauseClicked: {
                if (appWindow.mainPage) {
                    appWindow.mainPage.togglePlayback()
                }
            }
        }
    }

    // playbackBar is a floating ApplicationWindow child.
    // ApplicationWindow.bottomMargin reserves its space.
    Item {
        id: playbackBar

        width: appWindow.width
        height: Theme.itemSizeExtraLarge

        x: 0
        y: appWindow.height - height

        z: 3

        Rectangle {
            anchors.fill: parent

            // Background always stays the dimmed pause color.
            color: Theme.rgba(Theme.highlightDimmerColor, 0.5)

            // Subtle highlight tint on top while playing. Fades in
            // and out via opacity, which avoids any color-flash.
            Rectangle {
                anchors.fill: parent

                color: Theme.rgba(Theme.highlightColor, 0.1)

                opacity: appWindow.isPlaying ? 1.0 : 0.0

                Behavior on opacity {
                    NumberAnimation {
                        duration: 300
                        easing.type: Easing.OutQuad
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                enabled: true

                onClicked: {}
            }
        }

        IconButton {
            id: playbackButton

            width: Theme.iconSizeMedium
            height: Theme.iconSizeMedium

            anchors.left: parent.left
            anchors.leftMargin: Theme.horizontalPageMargin
            anchors.verticalCenter: parent.verticalCenter

            icon.source: appWindow.isPlaying
                        ? "image://theme/icon-m-pause"
                        : "image://theme/icon-m-play"

            opacity: (appWindow.activeBand !== "" ||
                      appWindow.activeNoise !== "" ||
                      appWindow.activeAmbience !== "")
                     ? 1.0
                     : 0.5

            enabled: appWindow.activeBand !== "" ||
                     appWindow.activeNoise !== "" ||
                     appWindow.activeAmbience !== ""

            onClicked: {
                if (appWindow.mainPage) {
                    appWindow.mainPage.togglePlayback()
                }
            }
        }

        Label {
            anchors.left: playbackButton.right
            anchors.leftMargin: Theme.paddingLarge
            anchors.right: parent.right
            anchors.rightMargin: Theme.horizontalPageMargin
            anchors.verticalCenter: parent.verticalCenter

            // Full highlight only while actually playing; selected
            // but paused stays secondary.
            color: (appWindow.activeBand === "" &&
                    appWindow.activeNoise === "" &&
                    appWindow.activeAmbience === "")
                   ? Theme.secondaryColor
                   : appWindow.isPlaying
                     ? Theme.highlightColor
                     : Theme.secondaryHighlightColor

            font.pixelSize: Theme.fontSizeMedium
            elide: Text.ElideRight

            text: (appWindow.activeBand === "" &&
                   appWindow.activeNoise === "" &&
                   appWindow.activeAmbience === "")
                  ? qsTr("Start your sound mix")
                  : ((appWindow.activeBand !== ""
                      ? appWindow.activeBand
                      : "") +
                     (appWindow.activeBand !== "" &&
                      (appWindow.activeNoise !== "" ||
                       appWindow.activeAmbience !== "")
                      ? " & " : "") +
                     (appWindow.activeNoise !== ""
                      ? appWindow.activeNoise
                      : "") +
                     (appWindow.activeNoise !== "" &&
                      appWindow.activeAmbience !== ""
                      ? " & " : "") +
                     (appWindow.activeAmbience !== ""
                      ? appWindow.activeAmbience
                      : ""))
        }
    }

    allowedOrientations: defaultAllowedOrientations
}
