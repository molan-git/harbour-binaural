import QtQuick 2.0
import Sailfish.Silica 1.0

/*
 * CustomIsochronicDrawer: content-only drawer for a custom
 * carrier / pulse frequency combination.
 * Uses ActionDrawer and PageDrawer (same pattern as
 * CustomBinauralDrawer.qml).
 */

ActionDrawer {
    id: customIsochronicPanel

    property bool customActive: false
    property bool playing: false

    property real carrierHz: carrierFrequencySlider.value
    property real pulseHz: pulseFrequencySlider.value

    signal frequencyChanged(real carrier, real pulse)
    signal startRequested(real carrier, real pulse)
    signal stopRequested()

    // title: "Custom Isochronic"

    Column {
        width: parent.width
        spacing: Theme.paddingSmall

        // Carrier frequency
        Item {
            width: parent.width
            height: carrierFrequencySlider.height +
                    carrierValueLabel.height - Theme.paddingLarge

            Slider {
                id: carrierFrequencySlider

                width: parent.width * 0.9
                anchors.horizontalCenter: parent.horizontalCenter

                label: qsTr("Carrier frequency")

                minimumValue: 50
                maximumValue: 500
                value: 200
                stepSize: 1

                onValueChanged:
                    customIsochronicPanel.frequencyChanged(
                        value, pulseFrequencySlider.value)
            }

            Label {
                id: carrierValueLabel

                anchors.top: carrierFrequencySlider.bottom
                anchors.topMargin: -Theme.paddingLarge * 1.8
                anchors.right: parent.right
                anchors.rightMargin: Theme.horizontalPageMargin

                text: Math.round(carrierFrequencySlider.value) + " Hz"
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeExtraSmall
            }
        }

        // Pulse frequency
        Item {
            width: parent.width
            height: pulseFrequencySlider.height +
                    pulseValueLabel.height - Theme.paddingLarge

            Slider {
                id: pulseFrequencySlider

                width: parent.width * 0.9
                anchors.horizontalCenter: parent.horizontalCenter

                label: qsTr("Pulse frequency")

                minimumValue: 0.5
                maximumValue: 40
                value: 10
                stepSize: 0.01

                onValueChanged:
                    customIsochronicPanel.frequencyChanged(
                        carrierFrequencySlider.value, value)
            }

            Label {
                id: pulseValueLabel

                anchors.top: pulseFrequencySlider.bottom
                anchors.topMargin: -Theme.paddingLarge * 1.8
                anchors.right: parent.right
                anchors.rightMargin: Theme.horizontalPageMargin

                text: pulseFrequencySlider.value.toFixed(2) + " Hz"
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeExtraSmall
            }
        }

        // Play - Stop button (uses custom icon-l-stop)
        MouseArea {
            id: customPlayIcon

            anchors.horizontalCenter: parent.horizontalCenter
            width:  Theme.iconSizeLarge + Theme.paddingMedium * 2
            height: Theme.iconSizeLarge + Theme.paddingMedium * 2

            Icon {
                id: playStopIcon

                anchors.centerIn: parent
                width:  Theme.iconSizeLarge
                height: Theme.iconSizeLarge

                source: customIsochronicPanel.customActive
                        ? "../../images/icon-l-stop.svg"
                        : "image://theme/icon-l-play"

                color: customIsochronicPanel.customActive
                       ? (customIsochronicPanel.playing
                          ? Theme.highlightColor
                          : Theme.secondaryHighlightColor)
                       : Theme.primaryColor
            }

            onClicked: {
                if (customIsochronicPanel.customActive) {
                    customIsochronicPanel.stopRequested()
                } else {
                    customIsochronicPanel.startRequested(
                                carrierFrequencySlider.value,
                                pulseFrequencySlider.value)
                }
            }
        }
    }
}
