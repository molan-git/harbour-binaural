import QtQuick 2.0
import Sailfish.Silica 1.0

/*
 * Custom SectionHeader component with less bottomSpacing.
 * Use this for sound sections, example:
 * CompactSectionHeader {
 *     text: qsTr("Ambient Sound")
 * }
 */

Item {
    id: root

    property alias text: header.text
    // Adjust spacing below component here:
    property real bottomSpacing: Theme.paddingSmall

    width: parent ? parent.width : 0

    // Explicitly reserve space below the header
    height: header.height + bottomSpacing

    SectionHeader {
        id: header

        x: Theme.horizontalPageMargin
        width: parent.width - 2 * Theme.horizontalPageMargin

        height: implicitHeight
        anchors.top: parent.top
    }
}
