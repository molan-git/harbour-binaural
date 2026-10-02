import QtQuick 2.0
import Sailfish.Silica 1.0
import Binaural 1.0
import "components"


Page {

    id: page


    AudioEngine {

        id: audioEngine

        property string activeBand: ""
        property string activeAmbience: ""
        property string activeNoise: ""


        property bool bandPlaying: false
        property bool ambiencePlaying: false
        property bool noisePlaying: false

        property bool isPlaying: bandPlaying ||
                                 ambiencePlaying ||
                                 noisePlaying


        onIsPlayingChanged: {
            appWindow.isPlaying = isPlaying
        }


        function togglePlayback() {
            if (isPlaying) {
                audioEngine.stop()
                audioEngine.stopAmbience()
                audioEngine.stopColoredNoise()


                bandPlaying = false
                ambiencePlaying = false
                noisePlaying = false
            } else {
                if (activeBand !== "") {
                    audioEngine.setFrequencyBand(activeBand)
                    audioEngine.start()
                    bandPlaying = true
                }


                if (activeAmbience !== "") {
                    audioEngine.setAmbience(activeAmbience)
                    ambiencePlaying = true
                }


                if (activeNoise !== "") {
                    audioEngine.setColoredNoise(activeNoise)
                    noisePlaying = true
                }
            }
        }
    }


    function ambienceLabel(key) {
        switch (key) {
        case "Wind":
            return qsTr("Wind")
        case "Sea Waves":
            return qsTr("Sea Waves")
        case "Crickets":
            return qsTr("Crickets")
        case "Stream":
            return qsTr("Stream")
        case "Rain":
            return qsTr("Rain")
        case "Birds":
            return qsTr("Birds")
        case "Fire":
            return qsTr("Fire")
        case "Chimes":
            return qsTr("Chimes")
        default:
            return ""
        }
    }


    Component {
        id: infoBinauralPageComponent
        InfoBinauralPage { }
    }


    Component {
        id: infoColoredPageComponent
        InfoColoredPage { }
    }


    // Drawer to open Ambience sounds - opens above playbackBar
    Drawer {
        id: ambienceDrawer


        anchors.fill: parent


        dock: Dock.Bottom
        z: 2


        backgroundSize: ambienceBackground.height


        background: SilicaFlickable {
            id: ambienceBackground


            width: parent.width
            height: ambienceColumn.height + 2 * Theme.paddingLarge
            contentHeight: ambienceColumn.height + 2 * Theme.paddingLarge
            contentWidth: width


            Rectangle {
                anchors.fill: parent


                color: Theme.rgba(
                    Theme.highlightBackgroundColor,
                    Theme.highlightBackgroundOpacity * 0.35
                )
            }


            // Swipe down action to close Drawer
            MouseArea {
                id: drawerSwipeArea


                anchors.fill: parent
                z: -1


                property real startY: 0


                onPressed: {
                    startY = mouse.y
                }


                onReleased: {
                    var deltaY = mouse.y - startY


                    if (deltaY > Theme.itemSizeSmall) {
                        ambienceDrawer.open = false
                    }
                }
            }


            Column {
                id: ambienceColumn


                width: parent.width
                spacing: Theme.paddingMedium


                Item {
                    width: 1
                    height: Theme.paddingLarge
                }


                // Ambience selection
                SilicaGridView {
                    id: ambienceDrawerGrid


                    x: Theme.horizontalPageMargin
                    width: parent.width -
                           2 * Theme.horizontalPageMargin
                    height: Theme.itemSizeMedium * 4
                    cellWidth: width / 2
                    cellHeight: Theme.itemSizeMedium


                    model: [
                        { key: "Wind",      label: qsTr("Wind") },
                        { key: "Sea Waves", label: qsTr("Sea Waves") },
                        { key: "Crickets",  label: qsTr("Crickets") },
                        { key: "Stream",    label: qsTr("Stream") },
                        { key: "Rain",      label: qsTr("Rain") },
                        { key: "Birds",     label: qsTr("Birds") },
                        { key: "Fire",      label: qsTr("Fire") },
                        { key: "Chimes",    label: qsTr("Chimes") }
                    ]


                    delegate: Item {
                        width: ambienceDrawerGrid.cellWidth
                        height: ambienceDrawerGrid.cellHeight


                        Rectangle {
                            anchors.fill: ambienceDrawerButton
                            anchors.margins: -Theme.paddingSmall
                            radius: Theme.paddingSmall


                            color: ambienceDrawerButton.highlighted
                                   ? Theme.highlightColor
                                   : "transparent"


                            opacity: ambienceDrawerButton.highlighted
                                     ? 0.15
                                     : 0.0


                            Behavior on opacity {
                                NumberAnimation {
                                    duration: 150
                                }
                            }
                        }


                        Button {
                            id: ambienceDrawerButton


                            anchors.centerIn: parent


                            preferredWidth: ambienceDrawerGrid.cellWidth -
                                            Theme.paddingMedium


                            text: modelData.label
                            highlighted: audioEngine.activeAmbience === modelData.key


                            onClicked: {
                                if (ambienceDrawerButton.highlighted) {
                                    audioEngine.activeAmbience = ""
                                    audioEngine.stopAmbience()
                                    audioEngine.ambiencePlaying = false
                                } else {
                                    audioEngine.activeAmbience = modelData.key
                                    audioEngine.setAmbience(modelData.key)
                                    audioEngine.ambiencePlaying = true
                                }
                            }


                            Label {
                                anchors.centerIn: parent
                                text: parent.text
                                color: ambienceDrawerButton.highlighted
                                       ? Theme.highlightColor
                                       : Theme.primaryColor
                            }
                        }
                    }
                }


                Item {
                    width: parent.width
                    height: ambienceVolumeSlider.height


                    Slider {
                        id: ambienceVolumeSlider


                        width: parent.width * 0.9
                        anchors.horizontalCenter: parent.horizontalCenter


                        minimumValue: 0
                        maximumValue: 100
                        value: 50


                        enabled: audioEngine.activeAmbience !== ""
                        opacity: enabled ? 1.0 : 0.5


                        onValueChanged: {
                            audioEngine.setAmbienceVolume(value)
                        }
                    }


                    Image {
                        id: ambienceVolumeIcon


                        source: ambienceVolumeSlider.value === 0
                                ? "../images/icon-s-volume-1.svg"
                                : ambienceVolumeSlider.value <= 33
                                  ? "../images/icon-s-volume-2.svg"
                                  : ambienceVolumeSlider.value <= 66
                                    ? "../images/icon-s-volume-3.svg"
                                    : "../images/icon-s-volume-4.svg"


                        sourceSize: Qt.size(
                                        Theme.iconSizeSmall * 1.6 * 2,
                                        Theme.iconSizeSmall * 1.6 * 2
                                        )


                        fillMode: Image.PreserveAspectFit


                        height: Theme.iconSizeSmall * 1.6
                        width: Theme.iconSizeSmall * 1.6


                        anchors.top: ambienceVolumeSlider.bottom
                        // moves icon closer to volume slider
                        anchors.topMargin: -Theme.paddingLarge
                        anchors.horizontalCenter: ambienceVolumeSlider.horizontalCenter


                        opacity: audioEngine.activeAmbience !== "" ? 1.0 : 0.5
                    }
                }
            }


            VerticalScrollDecorator {}
        }


        Item {
            id: foregroundContent


            anchors.fill: parent


            SilicaFlickable {
                id: foregroundPage


                anchors.fill: parent


                contentHeight: column.height +
                               Theme.paddingLarge


                PullDownMenu {
                    MenuItem {
                        text: qsTr("About")


                        onClicked: {
                            pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
                        }
                    }
                }


                MouseArea {
                    id: closeDrawerOnBackgroundTap


                    anchors.fill: parent
                    z: -1


                    onClicked: {
                        if (ambienceDrawer.opened) {
                            ambienceDrawer.open = false
                        }
                    }
                }


                Column {
                    id: column


                    width: parent.width
                    spacing: Theme.paddingMedium


                    Item {
                        width: 1
                        height: Theme.paddingLarge
                    }


                    // Binaural frequency selection
                    SectionHeader {
                        text: qsTr("Binaural Beats")
                    }


                    SilicaGridView {
                        id: frequencyGrid


                        x: Theme.horizontalPageMargin
                        width: parent.width -
                               2 * Theme.horizontalPageMargin
                        height: contentHeight
                        cellWidth: width / 2
                        cellHeight: Theme.itemSizeMedium


                        model: [
                            "Delta",
                            "Delta/Theta",
                            "Theta",
                            "Alpha",
                            "Beta",
                            "Gamma"
                        ]


                        delegate: Item {
                            width: frequencyGrid.cellWidth
                            height: frequencyGrid.cellHeight


                            Rectangle {
                                anchors.fill: frequencyButton
                                anchors.margins: -Theme.paddingSmall
                                radius: Theme.paddingSmall


                                color: frequencyButton.highlighted
                                       ? Theme.highlightColor
                                       : "transparent"


                                opacity: frequencyButton.highlighted
                                         ? 0.15
                                         : 0.0


                                Behavior on opacity {
                                    NumberAnimation {
                                        duration: 150
                                    }
                                }
                            }


                            Button {
                                id: frequencyButton


                                anchors.centerIn: parent


                                preferredWidth: frequencyGrid.cellWidth -
                                                Theme.paddingMedium


                                text: modelData
                                highlighted: audioEngine.activeBand === modelData


                                onClicked: {
                                    if (frequencyButton.highlighted) {
                                        audioEngine.activeBand = ""
                                        audioEngine.stop()
                                        audioEngine.bandPlaying = false
                                    } else {
                                        audioEngine.activeBand = modelData
                                        audioEngine.setFrequencyBand(modelData)
                                        audioEngine.start()
                                        audioEngine.bandPlaying = true
                                    }
                                }


                                Label {
                                    anchors.centerIn: parent
                                    text: parent.text
                                    color: frequencyButton.highlighted
                                           ? Theme.highlightColor
                                           : Theme.primaryColor
                                }
                            }
                        }
                    }


                    Item {
                        width: parent.width
                        height: binauralVolumeSlider.height


                        Slider {
                            id: binauralVolumeSlider


                            anchors.left: parent.left
                            anchors.leftMargin: Theme.horizontalPageMargin
                            anchors.right: infoBinauralsButton.left
                            anchors.rightMargin: Theme.paddingLarge


                            minimumValue: 0
                            maximumValue: 100
                            value: 50


                            enabled: audioEngine.activeBand !== ""
                            opacity: enabled ? 1.0 : 0.5


                            onValueChanged: {
                                audioEngine.setBinauralVolume(value)
                            }
                        }


                        Image {
                            id: binauralVolumeIcon


                            source: binauralVolumeSlider.value === 0
                                        ? "../images/icon-s-volume-1.svg"
                                        : binauralVolumeSlider.value <= 33
                                          ? "../images/icon-s-volume-2.svg"
                                          : binauralVolumeSlider.value <= 66
                                            ? "../images/icon-s-volume-3.svg"
                                            : "../images/icon-s-volume-4.svg"


                            sourceSize: Qt.size(
                                            Theme.iconSizeSmall * 1.6 * 2,
                                            Theme.iconSizeSmall * 1.6 * 2
                                            )


                            fillMode: Image.PreserveAspectFit


                            height: Theme.iconSizeSmall * 1.6
                            width: Theme.iconSizeSmall * 1.6


                            anchors.top: binauralVolumeSlider.bottom
                            // moves icon closer to volume slider
                            anchors.topMargin: -Theme.paddingLarge
                            anchors.horizontalCenter: binauralVolumeSlider.horizontalCenter


                            opacity: audioEngine.activeBand !== "" ? 1.0 : 0.5
                        }


                        IconButton {
                            id: infoBinauralsButton


                            anchors.right: parent.right
                            anchors.rightMargin: Theme.horizontalPageMargin
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.verticalCenterOffset: Theme.paddingSmall * 1.7


                            icon.source: "image://theme/icon-m-about?" +
                                         (pressed
                                          ? Theme.highlightColor
                                          : Theme.primaryColor)


                            onClicked: {
                                pageStack.push(infoBinauralPageComponent)
                            }
                        }
                    }


                    // Colored Noise selection
                    SectionHeader {
                        text: qsTr("Colored Noise")
                    }


                    SilicaGridView {
                        id: coloredNoiseGrid


                        x: Theme.horizontalPageMargin
                        width: parent.width -
                               2 * Theme.horizontalPageMargin
                        height: contentHeight
                        cellWidth: width / 2
                        cellHeight: Theme.itemSizeMedium


                        model: [
                            { key: "White", label: qsTr("White") },
                            { key: "Pink",  label: qsTr("Pink") },
                            { key: "Brown", label: qsTr("Brown") },
                            { key: "Grey",  label: qsTr("Grey") }
                        ]


                        delegate: Item {
                            width: coloredNoiseGrid.cellWidth
                            height: coloredNoiseGrid.cellHeight


                            Rectangle {
                                anchors.fill: coloredNoiseButton
                                anchors.margins: -Theme.paddingSmall
                                radius: Theme.paddingSmall


                                color: coloredNoiseButton.highlighted
                                       ? Theme.highlightColor
                                       : "transparent"


                                opacity: coloredNoiseButton.highlighted
                                         ? 0.15
                                         : 0.0


                                Behavior on opacity {
                                    NumberAnimation {
                                        duration: 150
                                    }
                                }
                            }


                            Button {
                                id: coloredNoiseButton


                                anchors.centerIn: parent


                                preferredWidth: coloredNoiseGrid.cellWidth -
                                                Theme.paddingMedium


                                text: modelData.label
                                highlighted: audioEngine.activeNoise === modelData.key


                                onClicked: {
                                    if (coloredNoiseButton.highlighted) {
                                        audioEngine.activeNoise = ""
                                        audioEngine.stopColoredNoise()
                                        audioEngine.noisePlaying = false
                                    } else {
                                        audioEngine.activeNoise = modelData.key
                                        audioEngine.setColoredNoise(modelData.key)
                                        audioEngine.noisePlaying = true
                                    }
                                }


                                Label {
                                    anchors.centerIn: parent
                                    text: parent.text
                                    color: coloredNoiseButton.highlighted
                                           ? Theme.highlightColor
                                           : Theme.primaryColor
                                }
                            }
                        }
                    }


                    Item {
                        width: parent.width
                        height: coloredNoiseVolumeSlider.height


                        Slider {
                            id: coloredNoiseVolumeSlider


                            anchors.left: parent.left
                            anchors.leftMargin: Theme.horizontalPageMargin
                            anchors.right: infoColoredButton.left
                            anchors.rightMargin: Theme.paddingLarge


                            minimumValue: 0
                            maximumValue: 100
                            value: 50


                            enabled: audioEngine.activeNoise !== ""
                            opacity: enabled ? 1.0 : 0.5


                            onValueChanged: {
                                audioEngine.setColoredNoiseVolume(value)
                            }
                        }


                        Image {
                            id: coloredNoiseVolumeIcon


                            source: coloredNoiseVolumeSlider.value === 0
                                    ? "../images/icon-s-volume-1.svg"
                                    : coloredNoiseVolumeSlider.value <= 33
                                      ? "../images/icon-s-volume-2.svg"
                                      : coloredNoiseVolumeSlider.value <= 66
                                        ? "../images/icon-s-volume-3.svg"
                                        : "../images/icon-s-volume-4.svg"


                            sourceSize: Qt.size(
                                            Theme.iconSizeSmall * 1.6 * 2,
                                            Theme.iconSizeSmall * 1.6 * 2
                                            )


                            fillMode: Image.PreserveAspectFit


                            height: Theme.iconSizeSmall * 1.6
                            width: Theme.iconSizeSmall * 1.6


                            anchors.top: coloredNoiseVolumeSlider.bottom
                            // moves icon closer to volume slider
                            anchors.topMargin: -Theme.paddingLarge
                            anchors.horizontalCenter: coloredNoiseVolumeSlider.horizontalCenter


                            opacity: audioEngine.activeNoise !== "" ? 1.0 : 0.5
                        }


                        IconButton {
                            id: infoColoredButton


                            anchors.right: parent.right
                            anchors.rightMargin: Theme.horizontalPageMargin
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.verticalCenterOffset: Theme.paddingSmall * 1.7


                            icon.source: "image://theme/icon-m-about?" +
                                         (pressed
                                          ? Theme.highlightColor
                                          : Theme.primaryColor)


                            onClicked: {
                                pageStack.push(infoColoredPageComponent)
                            }
                        }
                    }


                    // Ambience selection with button that opens Drawer
                    SectionHeader {
                        text: qsTr("Ambient Sound")
                    }


                    Item {
                        width: parent.width
                        height: ambienceButton.height


                        Rectangle {
                            anchors.fill: ambienceButton
                            anchors.margins: -Theme.paddingSmall
                            radius: Theme.paddingSmall


                            color: ambienceButton.highlighted
                                   ? Theme.highlightColor
                                   : "transparent"


                            opacity: ambienceButton.highlighted
                                     ? 0.15
                                     : 0.0


                            Behavior on opacity {
                                NumberAnimation {
                                    duration: 150
                                }
                            }
                        }


                        Button {
                            id: ambienceButton


                            anchors.centerIn: parent


                            preferredWidth: Theme.buttonWidthMedium


                            text: audioEngine.activeAmbience === ""
                                  ? qsTr("Choose an ambient sound")
                                  : page.ambienceLabel(audioEngine.activeAmbience)


                            highlighted: audioEngine.activeAmbience !== ""


                            onClicked: {
                                ambienceDrawer.open = true
                            }


                            Label {
                                anchors.centerIn: parent
                                text: parent.text
                                color: ambienceButton.highlighted
                                       ? Theme.highlightColor
                                       : Theme.primaryColor
                            }
                        }
                    }


                    Item {
                        width: 1
                        height: Theme.itemSizeMedium
                    }
                }


                VerticalScrollDecorator {}
            }


            Rectangle {
                id: drawerDimmer


                anchors.fill: foregroundContent


                z: 1


                color: Theme.overlayBackgroundColor
                opacity: ambienceDrawer.opened ? 0.6 : 0.0


                Behavior on opacity {
                    NumberAnimation {
                        duration: 250
                    }
                }


                MouseArea {
                    anchors.fill: parent
                    enabled: ambienceDrawer.opened


                    onClicked: {
                        ambienceDrawer.open = false
                    }
                }
            }
        }
    }
}
