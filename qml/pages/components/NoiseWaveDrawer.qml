import QtQuick 2.0
import Sailfish.Silica 1.0

ActionDrawer {
    id: noiseWavePanel

    property bool wavesActive: false   // set from MainPage
    property bool playing: false       // set from MainPage

    signal waveChanged(real depth, real period)
    signal enableRequested(real depth, real period)
    signal disableRequested()

    // title: "Waves"

    Column {
        width: parent.width
        spacing: Theme.paddingSmall

        // Wave depth
        Item {
            width: parent.width
            height: waveDepthSlider.height +
                    waveDepthLabel.height - Theme.paddingLarge

            Slider {
                id: waveDepthSlider

                width: parent.width * 0.9
                anchors.horizontalCenter: parent.horizontalCenter

                label: qsTr("Wave depth")

                minimumValue: 10
                maximumValue: 100
                value: 70
                stepSize: 5

                onValueChanged:
                    noiseWavePanel.waveChanged(
                        value / 100.0, waveLengthSlider.value)
            }

            Label {
                id: waveDepthLabel

                anchors.top: waveDepthSlider.bottom
                anchors.topMargin: -Theme.paddingLarge * 1.8
                anchors.right: parent.right
                anchors.rightMargin: Theme.horizontalPageMargin

                text: Math.round(waveDepthSlider.value) + " %"
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeExtraSmall
            }
        }

        // Wave length
        Item {
            width: parent.width
            height: waveLengthSlider.height +
                    waveLengthLabel.height - Theme.paddingLarge

            Slider {
                id: waveLengthSlider

                width: parent.width * 0.9
                anchors.horizontalCenter: parent.horizontalCenter

                label: qsTr("Wave length")

                minimumValue: 2
                maximumValue: 30
                value: 8
                stepSize: 1

                onValueChanged:
                    noiseWavePanel.waveChanged(
                        waveDepthSlider.value / 100.0, value)
            }

            Label {
                id: waveLengthLabel

                anchors.top: waveLengthSlider.bottom
                anchors.topMargin: -Theme.paddingLarge * 1.8
                anchors.right: parent.right
                anchors.rightMargin: Theme.horizontalPageMargin

                text: Math.round(waveLengthSlider.value) + " s"
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeExtraSmall
            }
        }

        // Waves on / off (same icon pattern as Custom Binaural)
        MouseArea {
            id: wavesToggleIcon

            anchors.horizontalCenter: parent.horizontalCenter
            width:  Theme.iconSizeLarge + Theme.paddingMedium * 2
            height: Theme.iconSizeLarge + Theme.paddingMedium * 2

            Icon {
                id: playStopIcon

                anchors.centerIn: parent
                width:  Theme.iconSizeLarge
                height: Theme.iconSizeLarge

                source: noiseWavePanel.wavesActive
                        ? "../../images/icon-l-stop.svg"
                        : "image://theme/icon-l-play"

                color: noiseWavePanel.wavesActive
                       ? (noiseWavePanel.playing
                          ? Theme.highlightColor
                          : Theme.secondaryHighlightColor)
                       : Theme.primaryColor
            }

            onClicked: {
                if (noiseWavePanel.wavesActive) {
                    noiseWavePanel.disableRequested()
                } else {
                    noiseWavePanel.enableRequested(
                                waveDepthSlider.value / 100.0,
                                waveLengthSlider.value)
                }
            }
        }
    }
}
