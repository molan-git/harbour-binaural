import QtQuick 2.0
import Sailfish.Silica 1.0

ActionDrawer {
    id: sequenceDrawer

    property var audioEngine: null
    property var sequencePresets: []
    property string activeSequence: ""
    property string notice: ""

    signal sequenceSelected(string sequence)
    signal sequenceDeselected()
    signal importRequested()

    onNoticeChanged: noticeTimer.restart()

    Column {
        width: parent.width
        spacing: Theme.paddingSmall

        Repeater {
            model: sequenceDrawer.sequencePresets

            delegate: TextSwitch {
                id: sequenceSwitch

                width: parent.width - 2 * Theme.horizontalPageMargin

                text: modelData
                checked: sequenceDrawer.activeSequence === modelData

                onClicked: {
                    if (sequenceDrawer.activeSequence === modelData)
                        sequenceDrawer.sequenceDeselected()
                    else
                        sequenceDrawer.sequenceSelected(modelData)
                }

                // Imported sequences can be removed with a long
                // press; built-in presets have no context menu.
                Loader {
                    id: removeMenuLoader

                    active: sequenceDrawer.audioEngine !== null
                            && sequenceDrawer.audioEngine.isImportedSequence(modelData)

                    sourceComponent: ContextMenu {
                        MenuItem {
                            text: qsTr("Remove")

                            onClicked: {
                                if (sequenceDrawer.activeSequence
                                        === modelData)
                                    sequenceDrawer
                                        .sequenceDeselected()

                                sequenceDrawer.audioEngine
                                    .removeImportedSequence(
                                        modelData)
                            }
                        }
                    }
                }

                onPressAndHold: {
                    if (removeMenuLoader.active
                            && removeMenuLoader.item !== null)
                        removeMenuLoader.item.open(sequenceSwitch)
                }

                // Re-assert the checked state after external
                // changes (a click destroys the checked binding
                // through Silica's built-in auto-toggle).
                Connections {
                    target: sequenceDrawer

                    onActiveSequenceChanged:
                        sequenceSwitch.checked =
                            sequenceDrawer.activeSequence === modelData
                }
            }
        }

        // Wrapper Item so the button centers itself regardless
        // of its implicit width (horizontal anchors are not
        // allowed directly inside a Column).
        Item {
            width: parent.width
            height: importButton.height

            Button {
                id: importButton

                anchors.centerIn: parent

                text: qsTr("Import SBaGen File")
                icon.source: "image://theme/icon-m-add"

                onClicked: sequenceDrawer.importRequested()
            }
        }

        Label {
            width: parent.width - 2 * Theme.horizontalPageMargin

            text: sequenceDrawer.notice
            visible: sequenceDrawer.notice !== ""
            color: Theme.secondaryColor
            font.pixelSize: Theme.fontSizeExtraSmall
            wrapMode: Text.WordWrap

            Timer {
                id: noticeTimer

                interval: 5000
                repeat: false

                onTriggered: sequenceDrawer.notice = ""
            }
        }
    }
}
