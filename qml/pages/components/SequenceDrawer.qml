import QtQuick 2.0
import Sailfish.Silica 1.0

ActionDrawer {
    id: sequenceDrawer

    property var sequencePresets: []
    property string activeSequence: ""

    signal sequenceSelected(string sequence)
    signal sequenceDeselected()

    // title: "Sequences"

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
    }
}
