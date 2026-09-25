import QtQuick 2.0
import Sailfish.Silica 1.0
import "pages"
import "cover"

ApplicationWindow {
    id: appWindow

    property var mainPage
    property string activeBand: ""
    property string activeAmbience: ""
    property string activeNoise: ""
    property bool isPlaying: false

    initialPage: Component {
        MainPage {
            id: mainPageInstance

            Component.onCompleted: {
                appWindow.mainPage = mainPageInstance
            }

            onActiveBandChanged: {
                appWindow.activeBand = activeBand
            }

            onActiveAmbienceChanged: {
                appWindow.activeAmbience = activeAmbience
            }

            onActiveNoiseChanged: {
                appWindow.activeNoise = activeNoise
            }

            onIsPlayingChanged: {
                appWindow.isPlaying = isPlaying
            }
        }
    }

    cover: Component {
        CoverPage {
            activeBand: appWindow.activeBand
            activeAmbience: appWindow.activeAmbience
            activeNoise: appWindow.activeNoise
            isPlaying: appWindow.isPlaying

            onPlayPauseClicked: {
                if (appWindow.mainPage) {
                    appWindow.mainPage.togglePlayback()
                }
            }
        }
    }

    allowedOrientations: defaultAllowedOrientations
}
