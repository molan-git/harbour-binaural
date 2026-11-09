import QtQuick 2.0
import Sailfish.Silica 1.0
import QtGraphicalEffects 1.0
import Qt.labs.folderlistmodel 2.1

SilicaFlickable {
    id: page

    property var audioEngine: null

    property url selectedFile: ""
    property string selectedFileName: ""

    property url selectedFolder: ""
    property string selectedFolderName: ""

    property bool loopFolder: false
    property bool startFolderWhenReady: false

    property int previousFolderCount: -1
    property int stableFolderCountTicks: 0
    property int folderWaitTicks: 0

    signal closeRequested()

    width: parent ? parent.width : Screen.width
    height: Math.min(contentColumn.height + Theme.paddingLarge,
                     Screen.height * 0.8)

    contentHeight: contentColumn.height + Theme.paddingLarge
    clip: true

    Component.onCompleted: {
        if (!audioEngine)
            return

        if (audioEngine.customSoundPlaylistActive && selectedFile.toString() === "") {
            // Nothing to restore, but keep the label consistent
            return
        }

        if (audioEngine.customSoundSource) {
            selectedFile = audioEngine.customSoundSource
            selectedFileName = audioEngine.customSoundName
                    ? audioEngine.customSoundName : ""
        }
    }

    // Open the picker as a separate PageStack entry.
    function openAudioPicker() {
        var dialog = pageStack.push(
                    Qt.resolvedUrl("../AudioFilePickerPage.qml"),
                    { "selectionMode": "file" })

        dialog.fileSelected.connect(function(fileUrl, fileName) {
            page.handleFileSelected(fileUrl, fileName)
        })

        dialog.folderSelected.connect(function(folderUrl, folderName) {
            page.handleFolderSelected(folderUrl, folderName)
        })
    }

    function handleFileSelected(fileUrl, fileName) {
        selectedFile = fileUrl
        selectedFileName = fileName.replace(/\.[^\.]+$/, "")

        selectedFolder = ""
        selectedFolderName = ""

        startFolderWhenReady = false
        folderPlaylistTimer.stop()

        if (audioEngine) {
            audioEngine.stopCustomSound()
            audioEngine.customSoundSource = fileUrl
            audioEngine.customSoundName = selectedFileName
            audioEngine.playCustomSound()
        }
    }

    function handleFolderSelected(folderUrl, folderName) {
        selectedFolder = folderUrl
        selectedFolderName = folderName

        selectedFile = ""
        selectedFileName = ""

        if (audioEngine)
            loopFolder = audioEngine.customSoundLoop

        startFolderPlayback()
    }

    function clearSelection() {
        folderPlaylistTimer.stop()
        startFolderWhenReady = false

        selectedFile = ""
        selectedFileName = ""
        selectedFolder = ""
        selectedFolderName = ""

        previousFolderCount = -1
        stableFolderCountTicks = 0
        folderWaitTicks = 0
        loopFolder = false

        if (audioEngine) {
            audioEngine.stopCustomSound()
            audioEngine.customSoundSource = ""
            audioEngine.customSoundName = ""
            audioEngine.customSoundLoop = false
        }
    }

    function startFolderPlayback() {
        if (!audioEngine || selectedFolder.toString() === "") {
            console.log("Folder playback: no audioEngine or folder selected")
            return
        }

        folderPlaylistTimer.stop()

        startFolderWhenReady = true
        previousFolderCount = -1
        stableFolderCountTicks = 0
        folderWaitTicks = 0

        folderPlaylistTimer.start()
    }

    function buildFolderPlaylist() {
        if (!startFolderWhenReady)
            return

        startFolderWhenReady = false
        folderPlaylistTimer.stop()

        var files = []

        for (var i = 0; i < folderPlaylistModel.count; ++i) {
            var fileUrl = folderPlaylistModel.get(i, "fileURL")
            var fileName = folderPlaylistModel.get(i, "fileName")

            if (fileUrl && fileName) {
                files.push({
                               "url": fileUrl,
                               "name": fileName.replace(/\.[^.]+$/, "")
                           })
            }
        }

        if (files.length === 0) {
            console.log("Folder playback: no supported audio files found in",
                        selectedFolder.toString())
            return
        }

        if (!audioEngine || !audioEngine.startCustomSoundPlaylist) {
            console.log("Folder playback: startCustomSoundPlaylist unavailable")
            return
        }

        console.log("Folder playback: starting playlist with",
                    files.length, "tracks")

        audioEngine.startCustomSoundPlaylist(files, loopFolder)

        selectedFile = files[0].url
        selectedFileName = files[0].name
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.rgba(
                   Theme.highlightBackgroundColor,
                   Theme.highlightBackgroundOpacity * 0.35)
    }

    Column {
        id: contentColumn

        width: parent.width
        spacing: Theme.paddingMedium

        // Drawer handle: tap or swipe down to close
        Item {
            width: parent.width
            height: Theme.itemSizeExtraSmall

            Rectangle {
                anchors.centerIn: parent

                width: Theme.itemSizeExtraSmall * 2
                height: Theme.paddingSmall
                radius: Theme.paddingSmall

                color: handleMouseArea.pressed
                       ? Theme.secondaryHighlightColor
                       : Theme.secondaryColor

                opacity: 0.6
            }

            MouseArea {
                id: handleMouseArea

                anchors.fill: parent

                property real startY: 0
                property real startX: 0

                onPressed: {
                    startY = mouse.y
                    startX = mouse.x
                }

                onClicked: page.closeRequested()

                onReleased: {
                    var deltaY = mouse.y - startY
                    var deltaX = mouse.x - startX

                    if (deltaY > Theme.itemSizeSmall
                            && Math.abs(deltaY) > Math.abs(deltaX)) {

                        page.closeRequested()
                    }
                }
            }
        }

        // Selected folder and current track information
        Item {
            id: trackInfoContainer

            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.7

            property bool showTrackInfo:
                selectedFolderName !== ""
                && audioEngine !== null
                && audioEngine.customSoundPlaylistActive

            property real expandedHeight:
                folderNameLabel.implicitHeight
                + Theme.paddingSmall
                + (showTrackInfo
                   ? trackNameLabel.implicitHeight + Theme.paddingSmall
                   : 0)

            height: expandedHeight

            Behavior on height {
                NumberAnimation {
                    duration: 300
                    easing.type: Easing.OutCubic
                }
            }

            Label {
                id: folderNameLabel

                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                }

                text: selectedFolderName !== ""
                      ? selectedFolderName
                      : (selectedFileName !== ""
                         ? selectedFileName
                         : qsTr("No file selected"))

                wrapMode: Text.Wrap
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeMedium
            }

            Label {
                id: trackNameLabel

                anchors {
                    top: folderNameLabel.bottom
                    topMargin: Theme.paddingSmall
                    left: parent.left
                    right: parent.right
                }

                visible: trackInfoContainer.showTrackInfo

                text: page.audioEngine
                      ? page.audioEngine.customSoundName : ""

                wrapMode: Text.Wrap
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeSmall
            }
        }

        // Playback controls buttons
        Item {
            id: playbackControlsContainer

            x: Theme.horizontalPageMargin
            width: parent.width - 2 * Theme.horizontalPageMargin
            height: playbackButtons.height + 2 * Theme.paddingMedium

            property real playbackIconSize: Theme.iconSizeMedium

            Row {
                id: playbackButtons

                anchors.centerIn: parent
                spacing: Theme.paddingLarge

                property real normalButtonSize:
                    playbackControlsContainer.playbackIconSize

                property real playButtonSize:
                    normalButtonSize * 1.5

                property bool hasAudio:
                    page.audioEngine !== null
                    && (page.selectedFile.toString() !== ""
                        || page.audioEngine.customSoundPlaylistActive)

                property bool isPlaying:
                    page.audioEngine !== null
                    && page.audioEngine.customSoundPlaying
                    && !page.audioEngine.customSoundPaused

                property bool hasPlaylist:
                    page.audioEngine !== null
                    && page.audioEngine.customSoundPlaylistActive

                // Previous track button
                Item {
                    width: playbackButtons.playButtonSize
                    height: playbackButtons.playButtonSize

                    IconButton {
                        anchors.centerIn: parent

                        width: playbackButtons.normalButtonSize
                        height: playbackButtons.normalButtonSize

                        icon.source: "image://theme/icon-m-previous?"
                                     + (pressed
                                        ? Theme.highlightColor
                                        : (playbackButtons.hasPlaylist
                                           ? Theme.primaryColor
                                           : Theme.secondaryColor))

                        enabled: playbackButtons.hasPlaylist

                        onClicked: {
                            if (enabled && page.audioEngine)
                                page.audioEngine.playPreviousCustomSound()
                        }
                    }
                }

                // Add or Clear custom sound selection button
                Item {
                    id: audioActionButton

                    width: playbackButtons.playButtonSize
                    height: playbackButtons.playButtonSize

                    property bool hasSelection:
                        page.selectedFile.toString() !== ""
                        || page.selectedFolder.toString() !== ""

                    Icon {
                        id: audioActionIcon

                        anchors.centerIn: parent
                        width: playbackButtons.playButtonSize
                        height: playbackButtons.playButtonSize

                        source: "image://theme/"
                                + (audioActionButton.hasSelection
                                   ? "icon-l-clear?"
                                   : "icon-l-add?")
                                + (audioActionMouseArea.pressed
                                   ? Theme.highlightColor
                                   : Theme.primaryColor)

                        sourceSize: Qt.size(width * 2, height * 2)

                        layer.enabled: page.audioEngine
                                       && page.audioEngine.customSoundPlaying

                        layer.effect: DropShadow {
                            transparentBorder: true
                            horizontalOffset: 0
                            verticalOffset: 0
                            radius: 0
                            samples: 1
                            color: Theme.highlightColor
                            opacity: 0.9
                        }
                    }

                    MouseArea {
                        id: audioActionMouseArea
                        anchors.fill: parent

                        onClicked: {
                            if (audioActionButton.hasSelection) {
                                if (page.audioEngine)
                                    page.audioEngine.stopCustomSound()

                                page.clearSelection()
                            } else {
                                page.openAudioPicker()
                            }
                        }
                    }
                }

                // Restart selected custom sound button
                Item {
                    id: restartButton

                    width: playbackButtons.playButtonSize
                    height: playbackButtons.playButtonSize

                    property bool hasSelection:
                        page.selectedFile.toString() !== ""
                        || page.selectedFolder.toString() !== ""

                    // Active only when a selection exists AND playback has stopped
                    property bool canRestart:
                        hasSelection
                        && page.audioEngine !== null
                        && !playbackButtons.isPlaying

                    Icon {
                        id: restartIcon

                        anchors.centerIn: parent
                        width: playbackButtons.playButtonSize
                        height: playbackButtons.playButtonSize

                        source: "../../images/icon-l-restart.svg"
                        sourceSize: Qt.size(width * 2, height * 2)

                        visible: false
                    }

                    ColorOverlay {
                        anchors.fill: restartIcon

                        source: restartIcon

                        color: restartMouseArea.pressed && restartButton.canRestart
                               ? Theme.highlightColor
                               : restartButton.canRestart
                                 ? Theme.primaryColor
                                 : Theme.secondaryColor
                    }

                    MouseArea {
                        id: restartMouseArea
                        anchors.fill: parent
                        enabled: restartButton.canRestart

                        onClicked: {
                            if (!page.audioEngine)
                                return

                            if (page.selectedFolder.toString() !== "") {
                                page.startFolderPlayback()
                            } else if (page.selectedFile.toString() !== "") {
                                page.audioEngine.customSoundSource = page.selectedFile
                                page.audioEngine.customSoundName = page.selectedFileName
                                page.audioEngine.playCustomSound()
                            }
                        }
                    }
                }

                // Next track button
                Item {
                    width: playbackButtons.playButtonSize
                    height: playbackButtons.playButtonSize

                    IconButton {
                        anchors.centerIn: parent

                        width: playbackButtons.normalButtonSize
                        height: playbackButtons.normalButtonSize

                        icon.source: "image://theme/icon-m-next?"
                                     + (pressed
                                        ? Theme.highlightColor
                                        : (playbackButtons.hasPlaylist
                                           ? Theme.primaryColor
                                           : Theme.secondaryColor))

                        enabled: playbackButtons.hasPlaylist

                        onClicked: {
                            if (enabled && page.audioEngine)
                                page.audioEngine.playNextCustomSound()
                        }
                    }
                }
            }
        }

        // Custom Sound Volume Slider
        Item {
            id: customSoundControls

            width: parent.width
            height: customSoundVolumeSlider.height
                    + customSoundVolumeIcon.height
                    - Theme.paddingLarge
                    + Theme.paddingMedium
                    + switchesRow.height

            Slider {
                id: customSoundVolumeSlider

                anchors.left: parent.left
                anchors.leftMargin: Theme.horizontalPageMargin
                anchors.right: parent.right
                anchors.rightMargin: Theme.horizontalPageMargin

                minimumValue: 0
                maximumValue: 100

                value: page.audioEngine
                       ? page.audioEngine.customSoundVolume
                       : 50

                enabled: page.audioEngine
                         && page.audioEngine.customSoundSource.toString() !== ""

                opacity: enabled ? 1.0 : 0.5

                onValueChanged: {
                    if (page.audioEngine)
                        page.audioEngine.customSoundVolume = value
                }
            }

            // Volume icon below the slider
            VolumeIcon {
                id: customSoundVolumeIcon
                value: customSoundVolumeSlider.value
                anchors.top: customSoundVolumeSlider.bottom
                anchors.topMargin: -Theme.paddingLarge
                anchors.horizontalCenter: customSoundVolumeSlider.horizontalCenter
                anchors.horizontalCenterOffset: Theme.paddingSmall
                opacity: page.audioEngine
                         && page.audioEngine.customSoundSource.toString() !== ""
                         ? 1.0 : 0.5
            }

            // Loop & Crossfade switches
            Row {
                id: switchesRow

                anchors.top: customSoundVolumeIcon.bottom
                anchors.topMargin: Theme.paddingMedium
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: Theme.paddingLarge

                Switch {
                    id: loopButton

                    icon.source: "image://theme/icon-m-repeat"

                    checked: page.audioEngine
                             ? page.audioEngine.customSoundLoop
                             : false

                    onCheckedChanged: {
                        page.loopFolder = checked
                        if (page.audioEngine)
                            page.audioEngine.customSoundLoop = checked
                    }

                    opacity: page.audioEngine
                             && page.audioEngine.customSoundSource.toString() !== ""
                             ? 1.0 : 0.5

                    enabled: page.audioEngine
                             && page.audioEngine.customSoundSource.toString() !== ""
                }

                Switch {
                    id: crossfadeButton

                    icon.source: "../../images/icon-m-crossfade.svg"
                    icon.sourceSize: Qt.size(Theme.iconSizeMedium * 0.9, Theme.iconSizeMedium * 0.9)

                    checked: page.audioEngine
                             ? page.audioEngine.customSoundCrossfade
                             : true

                    onCheckedChanged: {
                        if (page.audioEngine)
                            page.audioEngine.customSoundCrossfade = checked
                    }

                    enabled: page.audioEngine
                             && page.audioEngine.customSoundSource.toString() !== ""

                    opacity: enabled ? 1.0 : 0.5
                }
            }
        }
    }

    // Folder playlist loading timer
    Timer {
        id: folderPlaylistTimer

        interval: 200
        repeat: true

        onTriggered: {
            if (!page.startFolderWhenReady) {
                stop()
                return
            }

            page.folderWaitTicks++

            var currentCount = folderPlaylistModel.count

            if (currentCount === page.previousFolderCount) {
                page.stableFolderCountTicks++
            } else {
                page.previousFolderCount = currentCount
                page.stableFolderCountTicks = 0
            }

            // Wait for the model to populate and stabilize
            if (page.folderWaitTicks >= 5
                    && page.stableFolderCountTicks >= 3) {
                stop()
                page.buildFolderPlaylist()
            }

            // Avoid waiting forever if the folder cannot be read
            if (page.folderWaitTicks >= 50) {
                stop()
                page.buildFolderPlaylist()
            }
        }
    }

    FolderListModel {
        id: folderPlaylistModel

        folder: page.selectedFolder

        showDirs: false
        showFiles: true
        showHidden: false
        showOnlyReadable: true

        sortField: FolderListModel.Name

        nameFilters: [
            "*.mp3", "*.MP3",
            "*.wav", "*.WAV",
            "*.ogg", "*.OGG",
            "*.oga", "*.OGA",
            "*.flac", "*.FLAC",
            "*.m4a", "*.M4A",
            "*.aac", "*.AAC",
            "*.opus", "*.OPUS",
            "*.wma", "*.WMA",
            "*.aif", "*.AIF",
            "*.aiff", "*.AIFF",
            "*.m4b", "*.M4B",
            "*.3gp", "*.3GP"
        ]
    }

    VerticalScrollDecorator {}
}
