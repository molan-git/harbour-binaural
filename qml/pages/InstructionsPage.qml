import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: contentColumn.height + Theme.paddingLarge

        VerticalScrollDecorator { }

        Column {
            id: contentColumn

            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: "Instructions"
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: "Binaural beats are an auditory effect created when two slightly different frequencies are played separately into each ear. They’re commonly used for relaxation, meditation, focus, sleep, and stress reduction."

                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeMedium
                wrapMode: Text.WordWrap
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: "Common brainwave frequency ranges"

                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeLarge
                wrapMode: Text.WordWrap
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                text: "<b>Delta (0.5–4 Hz):</b> associated with deep sleep and restorative rest.<br><br>" +
                      "<b>Theta (4–8 Hz):</b> associated with drowsiness, meditation, and relaxed creativity.<br><br>" +
                      "<b>Alpha (8–12 Hz):</b> associated with relaxed wakefulness, calmness, and light meditation.<br><br>" +
                      "<b>Beta (13–30 Hz):</b> associated with alertness, concentration, and active thinking.<br><br>" +
                      "<b>Gamma (30–100 Hz):</b> associated with complex cognitive processing and perception."

                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeMedium
                textFormat: Text.RichText
                wrapMode: Text.WordWrap
            }
        }
    }
}
