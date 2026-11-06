import QtQuick 2.0
import Sailfish.Silica 1.0
import Qt.labs.folderlistmodel 2.1

Dialog {
    id: picker

    objectName: "AudioFilePickerPage"

    property string selectionMode: "file"

    property url homeFolder: StandardPaths.music !== ""
                             ? StandardPaths.music
                             : "file:///home/defaultuser/Music"

    property url currentFolder: homeFolder

    signal fileSelected(url fileUrl, string fileName)
    signal folderSelected(url folderUrl, string folderName)

    allowedOrientations: Orientation.All

    function normalizedPath(value) {
        var path = value.toString()
        if (path.indexOf("file://") === 0)
            path = path.substring(7)
        return path.replace(/\/+$/, "") || "/"
    }

    property bool atHome:
        normalizedPath(currentFolder) === normalizedPath(homeFolder)

    property bool hasAudioFiles: {
        for (var i = 0; i < folderModel.count; ++i) {
            if (!folderModel.get(i, "fileIsDir"))
                return true
        }
        return false
    }

    function displayPath() {
        var current = normalizedPath(currentFolder)
        var music = normalizedPath(homeFolder)
        if (current === music)
            return "Music"
        if (current.indexOf(music + "/") === 0)
            return "Music" + current.substring(music.length)
        return current
    }

    function selectCurrentFolder() {
        var path = normalizedPath(currentFolder)
        var parts = path.split("/")
        var name = parts.length > 0 ? parts[parts.length - 1] : path
        if (name === "")
            name = qsTr("Music")
        picker.folderSelected(currentFolder, name)
        picker.accept()
    }

    canAccept: hasAudioFiles

    onAccepted: selectCurrentFolder()

    Column {
        anchors.fill: parent
        spacing: 0

        DialogHeader {
            id: dialogHeader

            acceptText: qsTr("Use Folder")
            cancelText: qsTr("Cancel")
        }

        SilicaListView {
            id: fileList

            width: parent.width
            height: parent.height - dialogHeader.height

            model: folderModel
            clip: true

            header: Column {
                width: fileList.width
                spacing: 0

                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    text: picker.displayPath()
                    color: Theme.secondaryColor
                    font.pixelSize: Theme.fontSizeSmall
                    wrapMode: Text.WrapAnywhere
                }

                ListItem {
                    width: fileList.width
                    height: Theme.itemSizeMedium
                    visible: !picker.atHome

                    Icon {
                        anchors.left: parent.left
                        anchors.leftMargin: Theme.paddingLarge
                        anchors.verticalCenter: parent.verticalCenter
                        width: Theme.iconSizeMedium
                        height: Theme.iconSizeMedium
                        source: "../images/icon-m-file-parent.svg"
                        color: Theme.primaryColor
                    }

                    onClicked: {
                        var path = picker.normalizedPath(picker.currentFolder)
                        var parentPath = path.substring(0, path.lastIndexOf("/"))
                        picker.currentFolder = "file://" + parentPath
                    }
                }
            }

            delegate: ListItem {
                width: fileList.width
                height: Theme.itemSizeMedium

                Image {
                    id: rowIcon
                    anchors.left: parent.left
                    anchors.leftMargin: Theme.paddingLarge
                    anchors.verticalCenter: parent.verticalCenter
                    width: Theme.iconSizeMedium
                    height: Theme.iconSizeMedium
                    source: fileIsDir
                            ? "image://theme/icon-m-file-folder"
                            : "image://theme/icon-m-file-audio"
                    fillMode: Image.PreserveAspectFit
                }

                Label {
                    anchors.left: rowIcon.right
                    anchors.leftMargin: Theme.paddingMedium
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.paddingLarge
                    anchors.verticalCenter: parent.verticalCenter
                    text: fileName
                    font.pixelSize: Theme.fontSizeMedium
                    truncationMode: TruncationMode.Fade
                    color: highlighted ? Theme.highlightColor : Theme.primaryColor
                }

                onClicked: {
                    if (fileIsDir) {
                        picker.currentFolder = fileURL
                    } else if (picker.selectionMode === "file") {
                        picker.fileSelected(fileURL, fileName)
                        picker.accept()
                    }
                }
            }

            ViewPlaceholder {
                enabled: folderModel.status === FolderListModel.Ready
                         && folderModel.count === 0
                text: qsTr("No audio files or folders")
                hintText: qsTr("Try another folder or check where your audio files are stored.")
            }

            VerticalScrollDecorator {}
        }
    }

    FolderListModel {
        id: folderModel
        folder: picker.currentFolder
        showDirs: true
        showFiles: true
        showHidden: false
        showOnlyReadable: true
        sortField: FolderListModel.Name
        nameFilters: [
            "*.mp3", "*.MP3", "*.wav", "*.WAV",
            "*.ogg", "*.OGG", "*.oga", "*.OGA",
            "*.flac", "*.FLAC", "*.m4a", "*.M4A",
            "*.aac", "*.AAC", "*.opus", "*.OPUS",
            "*.wma", "*.WMA", "*.aif", "*.AIF",
            "*.aiff", "*.AIFF", "*.m4b", "*.M4B",
            "*.3gp", "*.3GP"
        ]
    }
}
