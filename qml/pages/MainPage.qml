import QtQuick 2.0
import Sailfish.Silica 1.0
import Binaural 1.0
import "components"

Page {
    id: page

    AudioEngine {
        id: audioEngine
    }

    property string activeBand: ""
    property string activeAmbience: ""
    property string activeNoise: ""
    property bool isPlaying: false

    onIsPlayingChanged: {
        appWindow.isPlaying = isPlaying
    }

    function togglePlayback() {
        if (isPlaying) {
            audioEngine.stop()
            audioEngine.stopAmbience()
            audioEngine.stopColoredNoise()
            isPlaying = false
        } else {
            if (activeBand !== "") {
                audioEngine.setFrequencyBand(activeBand)
                audioEngine.start()
            }

            if (activeAmbience !== "") {
                audioEngine.setAmbience(activeAmbience)
            }

            if (activeNoise !== "") {
                audioEngine.setColoredNoise(activeNoise)
            }

            isPlaying = true
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
        backgroundSize: ambienceBackground.height + Theme.itemSizeMedium

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
                    Theme.highlightBackgroundOpacity
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
                    height: Theme.paddingMedium
                }

                // Ambience selection
                SilicaGridView {
                    id: ambienceDrawerGrid

                    x: Theme.horizontalPageMargin
                    width: parent.width -
                           2 * Theme.horizontalPageMargin
                    height: Theme.itemSizeMedium * 3
                    cellWidth: width / 2
                    cellHeight: Theme.itemSizeMedium

                    model: [
                        qsTr("Wind"),
                        qsTr("Sea Waves"),
                        qsTr("Crickets"),
                        qsTr("Stream"),
                        qsTr("Rain"),
                        qsTr("Birds")
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

                            text: modelData
                            highlighted: page.activeAmbience === modelData

                            onClicked: {
                                if (ambienceDrawerButton.highlighted) {
                                    page.activeAmbience = ""
                                    audioEngine.stopAmbience()
                                    page.isPlaying = false
                                } else {
                                    page.activeAmbience = modelData
                                    audioEngine.setAmbience(modelData)
                                    page.isPlaying = true
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

                Slider {
                    id: ambienceVolumeSlider

                    width: parent.width * 0.75
                    anchors.horizontalCenter: parent.horizontalCenter

                    minimumValue: 0
                    maximumValue: 100
                    value: 50
                    label: qsTr("Ambience volume")

                    onValueChanged: {
                        audioEngine.setAmbienceVolume(value)
                    }
                }

                Item {
                    width: 1
                    height: Theme.paddingLarge
                }
            }

            VerticalScrollDecorator {}
        }

        SilicaFlickable {
            id: foregroundPage

            anchors.fill: parent

            contentHeight: column.height +
                           playbackBar.height +
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
                    text: qsTr("Binaural frequency range")
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
                            highlighted: page.activeBand === modelData

                            onClicked: {
                                if (frequencyButton.highlighted) {
                                    page.activeBand = ""
                                    audioEngine.stop()
                                    page.isPlaying = false
                                } else {
                                    page.activeBand = modelData
                                    audioEngine.setFrequencyBand(modelData)
                                    audioEngine.start()
                                    page.isPlaying = true
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
                        label: qsTr("Binaural volume")

                        onValueChanged: {
                            audioEngine.setBinauralVolume(value)
                        }
                    }

                    IconButton {
                        id: infoBinauralsButton

                        anchors.right: parent.right
                        anchors.rightMargin: Theme.horizontalPageMargin
                        anchors.verticalCenter: parent.verticalCenter

                        icon.source: "image://theme/icon-m-about?" +
                                     (pressed
                                      ? Theme.highlightColor
                                      : Theme.primaryColor)

                        onClicked: {
                            pageStack.push(infoBinauralPageComponent)
                        }
                    }
                }

                // Color Noise selection
                SectionHeader {
                    text: qsTr("Color Noise")
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
                        qsTr("White"),
                        qsTr("Pink"),
                        qsTr("Brown"),
                        qsTr("Grey")
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

                            text: modelData
                            highlighted: page.activeNoise === modelData

                            onClicked: {
                                if (coloredNoiseButton.highlighted) {
                                    page.activeNoise = ""
                                    audioEngine.stopColoredNoise()
                                    page.isPlaying = false
                                } else {
                                    page.activeNoise = modelData
                                    audioEngine.setColoredNoise(modelData)
                                    page.isPlaying = true
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
                        label: qsTr("Color noise volume")

                        onValueChanged: {
                            audioEngine.setColoredNoiseVolume(value)
                        }
                    }

                    IconButton {
                        id: infoColoredButton

                        anchors.right: parent.right
                        anchors.rightMargin: Theme.horizontalPageMargin
                        anchors.verticalCenter: parent.verticalCenter

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
                    text: qsTr("Ambience")
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

                        text: activeAmbience === ""
                              ? qsTr("Play an Ambience")
                              : qsTr("Playing %1").arg(activeAmbience)

                        highlighted: activeAmbience !== ""

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
    }

    // playbackBar at the bottom to stop and restart selected sounds
    Item {
        id: playbackBar

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: Theme.itemSizeLarge
        z: 1

        Rectangle {
            anchors.fill: parent
            color: Theme.overlayBackgroundColor

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

            icon.source: isPlaying
                        ? "image://theme/icon-m-pause"
                        : "image://theme/icon-m-play"

            opacity: (activeBand !== "" ||
                      activeNoise !== "" ||
                      activeAmbience !== "")
                     ? 1.0
                     : 0.35

            enabled: activeBand !== "" ||
                     activeNoise !== "" ||
                     activeAmbience !== ""

            onClicked: {
                togglePlayback()
            }
        }

        Label {
            anchors.left: playbackButton.right
            anchors.leftMargin: Theme.paddingLarge
            anchors.right: parent.right
            anchors.rightMargin: Theme.horizontalPageMargin
            anchors.verticalCenter: parent.verticalCenter

            color: (activeBand === "" &&
                    activeNoise === "" &&
                    activeAmbience === "")
                   ? Theme.secondaryColor
                   : Theme.highlightColor

            font.pixelSize: Theme.fontSizeMedium
            elide: Text.ElideRight

            text: (activeBand === "" &&
                   activeNoise === "" &&
                   activeAmbience === "")
                  ? qsTr("Start your sound mix")
                  : ((activeBand !== "" ? activeBand : "") +
                     (activeBand !== "" &&
                      (activeNoise !== "" || activeAmbience !== "")
                      ? " & " : "") +
                     (activeNoise !== "" ? activeNoise : "") +
                     (activeNoise !== "" &&
                      activeAmbience !== ""
                      ? " & " : "") +
                     (activeAmbience !== ""
                      ? activeAmbience : ""))
        }
    }
}
