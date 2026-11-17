import QtQuick 2.0
import Sailfish.Silica 1.0

ActionDrawer {
    id: customBinauralPanel

    property real carrierHz: baseFrequencySlider.value
    property real beatHz: beatFrequencySlider.value

    signal frequencyChanged(real carrier, real beat)

    // Sync the sliders to the current engine values
    // before the drawer is opened.
    function setValues(carrier, beat) {
        baseFrequencySlider.value = carrier
        beatFrequencySlider.value = beat
    }

    // title: "Custom Binaural"

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
