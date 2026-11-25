import QtQuick 2.0
import Sailfish.Silica 1.0

/*
 * FocusDrawer: content-only drawer for a custom three-voice
 * binaural focus mix (same pattern as CustomIsochronicDrawer).
 * The mix is committed on slider release because starting a
 * sequence restarts the renderer.
 */

ActionDrawer {
    id: focusPanel

    signal mixChanged(var voices)

    function setVoices(voices) {
        for (var i = 0; i < voiceRepeater.count && i < voices.length; ++i) {
            var row = voiceRepeater.itemAt(i)
            row.carrier = voices[i].carrier
            row.beat = voices[i].beat
        }
    }

    function commitMix() {
        var voices = []

        for (var i = 0; i < voiceRepeater.count; ++i) {
            var row = voiceRepeater.itemAt(i)
            voices.push({
                carrier: row.carrier,
                beat: row.beat
            })
        }

        focusPanel.mixChanged(voices)
    }

    Column {
        width: parent.width
        spacing: Theme.paddingSmall

        Repeater {
            id: voiceRepeater
            model: 3

            delegate: Column {
                id: voiceRow

                property real carrier: index === 0 ? 100
                                       : index === 1 ? 200 : 300
                property real beat: index === 0 ? 1.5
                                    : index === 1 ? 4.0 : 4.0

                width: parent.width

                Slider {
                    id: carrierSlider

                    width: parent.width * 0.9
                    anchors.horizontalCenter: parent.horizontalCenter

                    label: qsTr("Voice %1 carrier").arg(index + 1)

                    minimumValue: 40
                    maximumValue: 600
                    value: index === 0 ? 100
                           : index === 1 ? 200 : 300
                    stepSize: 1

                    onValueChanged:
                        voiceRow.carrier = value

                    onReleased:
                        focusPanel.commitMix()
                }

                Label {
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.horizontalPageMargin

                    text: Math.round(carrierSlider.value) + " Hz"
                    color: Theme.secondaryHighlightColor
                    font.pixelSize: Theme.fontSizeExtraSmall
                }

                Slider {
                    id: beatSlider

                    width: parent.width * 0.9
                    anchors.horizontalCenter: parent.horizontalCenter

                    label: qsTr("Voice %1 beat").arg(index + 1)

                    minimumValue: 0.5
                    maximumValue: 40
                    value: index === 0 ? 1.5
                           : index === 1 ? 4.0 : 4.0
                    stepSize: 0.1

                    onValueChanged:
                        voiceRow.beat = value

                    onReleased:
                        focusPanel.commitMix()
                }

                Label {
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.horizontalPageMargin

                    text: beatSlider.value.toFixed(1) + " Hz"
                    color: Theme.secondaryHighlightColor
                    font.pixelSize: Theme.fontSizeExtraSmall
                }
            }
        }
    }
}
