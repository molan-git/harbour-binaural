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
    property real bottomSpacing: Theme.paddingSmall

    width: parent ? parent.width : 0

    // Keep the actual spacing below the header consistent.
    height: header.height + bottomSpacing -
            (parent ? parent.spacing : 0)

    SectionHeader {
        id: header

        x: Theme.horizontalPageMargin
        width: parent.width - 2 * Theme.horizontalPageMargin

        anchors.top: parent.top
        height: implicitHeight
    }
}
