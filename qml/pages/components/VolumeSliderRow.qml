import QtQuick 2.0
import Sailfish.Silica 1.0
import QtGraphicalEffects 1.0

/*
 * Volume slider with speaker icon on the left.
 *
 * value   - current volume 0-100 (alias of the slider)
 *           -> implicit valueChanged signal, use onValueChanged
 * enabled - dims slider and speaker icon when false
 */

Item {
    id: root

    property alias value: slider.value
    property bool enabled: true

    width: parent ? parent.width : 0
    height: Math.max(slider.height, volumeIcon.height)

    // Speaker icon on the left
    Item {
        id: volumeIcon

        property real value: slider.value
        property real size: Theme.iconSizeSmall * 1.6

        width: size
        height: size

        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter

        opacity: root.enabled ? 1.0 : 0.5

        Image {
            id: img

            anchors.fill: parent

            source: volumeIcon.value === 0  ? "../../images/icon-s-volume-1.svg"
                   : volumeIcon.value <= 33 ? "../../images/icon-s-volume-2.svg"
                   : volumeIcon.value <= 66 ? "../../images/icon-s-volume-3.svg"
                   :                          "../../images/icon-s-volume-4.svg"

            sourceSize: Qt.size(volumeIcon.size * 2, volumeIcon.size * 2)
            fillMode: Image.PreserveAspectFit
            visible: false
        }

        ColorOverlay {
            anchors.fill: img
            source: img
            color: Theme.primaryColor
        }
    }

    // Volume slider
    Slider {
        id: slider

        x: volumeIcon.width + Theme.paddingSmall
        width: parent.width - volumeIcon.width - Theme.paddingSmall

        anchors.verticalCenter: parent.verticalCenter

        leftMargin: Theme.paddingSmall
        rightMargin: Theme.paddingLarge

        minimumValue: 0
        maximumValue: 100
        value: 50

        enabled: root.enabled
        opacity: enabled ? 1.0 : 0.5
    }
}
