import QtQuick 2.0
import Sailfish.Silica 1.0

/*
 * CustomBinauralDrawer: content-only drawer for live-editing
 * the binaural carrier / beat frequencies.
 * Uses ActionDrawer and PageDrawer.
 *
 * Moving a slider changes the running generator immediately (or stores
 * the values for the next start when nothing is playing).
 */

ActionDrawer {
    id: customBinauralPanel

    property real carrierHz: carrierFrequencySlider.value
    property real beatHz: beatFrequencySlider.value

    signal frequencyChanged(real carrier, real beat)

    // title: "Custom Binaural"

    // Called by MainPage before opening, so the sliders
    // always show the currently playing values.
    function setValues(carrier, beat) {
        carrierFrequencySlider.value = carrier
        beatFrequencySlider.value = beat
    }

    Column {
        width: parent.width
        spacing: Theme.paddingSmall

        // Base frequency
        Item {
            width: parent.width
            height: baseFrequencySlider.height +
                    baseValueLabel.height - Theme.paddingLarge

            Slider {
                id: baseFrequencySlider

                width: parent.width * 0.9
                anchors.horizontalCenter: parent.horizontalCenter

                label: qsTr("Base frequency")

                minimumValue: 50
                maximumValue: 500
                value: 200
                stepSize: 1

                onValueChanged:
                    customBinauralPanel.frequencyChanged(
                        value, beatFrequencySlider.value)
            }

            Label {
                id: baseValueLabel

                anchors.top: baseFrequencySlider.bottom
                anchors.topMargin: -Theme.paddingLarge * 1.8
                anchors.right: parent.right
                anchors.rightMargin: Theme.horizontalPageMargin

                text: Math.round(baseFrequencySlider.value) + " Hz"
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeExtraSmall
            }
        }

        // Beat frequency
        Item {
            width: parent.width
            height: beatFrequencySlider.height +
                    beatValueLabel.height - Theme.paddingLarge

            Slider {
                id: beatFrequencySlider

                width: parent.width * 0.9
                anchors.horizontalCenter: parent.horizontalCenter

                label: qsTr("Beat frequency")

                minimumValue: 0.5
                maximumValue: 40
                value: 10
                stepSize: 0.5

                onValueChanged:
                    customBinauralPanel.frequencyChanged(
                        baseFrequencySlider.value, value)
            }

            Label {
                id: beatValueLabel

                anchors.top: beatFrequencySlider.bottom
                anchors.topMargin: -Theme.paddingLarge * 1.8
                anchors.right: parent.right
                anchors.rightMargin: Theme.horizontalPageMargin

                text: beatFrequencySlider.value.toFixed(1) + " Hz"
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeExtraSmall
            }
        }
    }
}
