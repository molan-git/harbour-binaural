import QtQuick 2.0
import Sailfish.Silica 1.0

Rectangle {
    property Item target

    anchors.fill: target

    radius: Theme.paddingSmall

    color: target && target.highlighted
           ? Theme.highlightColor
           : "transparent"

    opacity: target && target.highlighted ? 0.15 : 0.0

    Behavior on opacity {
        NumberAnimation {
            duration: 150
        }
    }
}
