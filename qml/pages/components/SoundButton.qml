import QtQuick 2.0
import Sailfish.Silica 1.0

/*
 * Component which defines the buttons
 * used for sound selection
 */

Item {
    id: root

    property string text: ""
    property real preferredWidth: -1
    property bool selected: false
    property bool playing: false
    property string iconSource: ""
    property bool secondary: false
    property bool showInfoButton: false
    property real topPadding: 0

    property real buttonHeight: Math.max(
                                    secondary
                                    ? secondaryButton.height
                                    : button.height,
                                    showInfoButton ? infoButton.height : 0)

    // Width available for the actual sound button
    property real contentWidth: preferredWidth > 0
                                ? Math.max(0, preferredWidth -
                                           (showInfoButton
                                            ? infoButton.width + Theme.paddingSmall
                                            : 0))
                                : -1

    // 'enabled' is inherited from Item

    signal clicked()
    signal infoClicked()

    // Total width includes the optional info button.
    width: preferredWidth > 0
           ? preferredWidth
           : (secondary ? secondaryButton.width : button.width)
             + (showInfoButton
                ? infoButton.width + Theme.paddingSmall
                : 0)

    // Additional space above the button
    height: buttonHeight + topPadding

    // Rounded highlight background
    Rectangle {
        anchors.left: parent.left

        y: root.topPadding + (root.buttonHeight - height) / 2

        width: root.secondary
               ? secondaryButton.width
               : button.width

        height: root.secondary
                ? secondaryButton.height
                : button.height

        radius: Theme.paddingSmall

        color: root.selected
               ? Theme.highlightColor
               : "transparent"

        opacity: root.selected ? 0.15 : 0.0

        Behavior on opacity {
            NumberAnimation {
                duration: 150
            }
        }
    }

    // Primary button (native, with glass)
    Button {
        id: button

        anchors.left: parent.left

        y: root.topPadding + (root.buttonHeight - height) / 2

        visible: !root.secondary
        text: root.text

        icon.source: root.iconSource !== "" && root.text !== ""
                     ? root.iconSource
                     : ""

        preferredWidth: root.contentWidth

        // Prevent long text from expanding the button.
        width: root.contentWidth > 0
               ? root.contentWidth
               : implicitWidth

        enabled: root.enabled
        highlighted: root.selected

        color: !enabled
               ? Theme.secondaryColor
               : highlighted
                 ? (root.playing
                    ? Theme.highlightColor
                    : Theme.secondaryHighlightColor)
                 : Theme.primaryColor

        opacity: !enabled ? 0.35
                : highlighted && !root.playing ? 0.6
                : 1.0

        onClicked: root.clicked()

        Icon {
            anchors.centerIn: parent
            width: Theme.iconSizeMedium
            height: Theme.iconSizeMedium
            sourceSize: Qt.size(width, height)
            visible: root.iconSource !== "" && root.text === ""
            source: root.iconSource
            color: button.color
        }
    }

    // Secondary button (native, no glass)
    SecondaryButton {
        id: secondaryButton

        anchors.left: parent.left

        y: root.topPadding + (root.buttonHeight - height) / 2

        visible: root.secondary
        text: root.text

        preferredWidth: root.contentWidth

        width: root.contentWidth > 0
               ? root.contentWidth
               : implicitWidth

        enabled: root.enabled
        highlighted: root.selected

        color: !enabled
               ? Theme.secondaryColor
               : highlighted
                 ? (root.playing
                    ? Theme.highlightColor
                    : Theme.secondaryHighlightColor)
                 : Theme.primaryColor

        opacity: !enabled ? 0.35
                : highlighted && !root.playing ? 0.6
                : 1.0

        onClicked: root.clicked()

        Icon {
            anchors.centerIn: parent
            width: Theme.iconSizeMedium
            height: Theme.iconSizeMedium
            sourceSize: Qt.size(width, height)
            visible: root.iconSource !== "" && root.text === ""
            source: root.iconSource
            color: secondaryButton.color
        }
    }

    // Optional info button
    IconButton {
        id: infoButton

        visible: root.showInfoButton

        anchors.right: parent.right

        y: root.topPadding + (root.buttonHeight - height) / 2

        icon.source: "image://theme/icon-m-about?" +
                     (pressed
                      ? Theme.highlightColor
                      : Theme.primaryColor)

        onClicked: root.infoClicked()
    }
}
