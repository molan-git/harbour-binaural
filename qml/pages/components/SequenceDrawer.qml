import QtQuick 2.0
import Sailfish.Silica 1.0

ActionDrawer {
    id: sequenceDrawer

    property var sequencePresets: []
    property string activeSequence: ""

    signal sequenceSelected(string sequence)

    title: qsTr("Sequences")

    Column {
        width: parent.width
        spacing: Theme.paddingSmall

        Repeater {
            model: sequenceDrawer.sequencePresets

            delegate: Button {
                width: parent.width - 2 * Theme.horizontalPageMargin
                anchors.horizontalCenter: parent.horizontalCenter

                text: modelData
                highlighted: sequenceDrawer.activeSequence === modelData

                onClicked: {
                    sequenceDrawer.sequenceSelected(modelData)
                }
            }
        }
    }
}
