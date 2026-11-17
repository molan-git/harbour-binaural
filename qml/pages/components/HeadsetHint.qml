import QtQuick 2.0
import Sailfish.Silica 1.0
import QtGraphicalEffects 1.0

Item {
    id: root

    property alias text: hintText.text

    width: parent.width - 2 * Theme.horizontalPageMargin
    x: Theme.horizontalPageMargin
    height: Theme.fontSizeExtraSmall

    Row {
        anchors.right: parent.right
        spacing: Theme.paddingSmall

        Label {
            id: hintText
            text: qsTr("To be used with")
            color: Theme.secondaryHighlightColor
            font.pixelSize: Theme.fontSizeExtraSmall
            verticalAlignment: Text.AlignVCenter
        }

        Item {
            width: Theme.fontSizeExtraSmall
            height: Theme.fontSizeExtraSmall
            anchors.verticalCenter: parent.verticalCenter

            Image {
                id: headsetImage
                anchors.fill: parent
                source: "../../images/icon-s-headset.svg"
                fillMode: Image.PreserveAspectFit
                smooth: true
                visible: false
            }

            ColorOverlay {
                anchors.fill: headsetImage
                source: headsetImage
                color: Theme.secondaryHighlightColor
                opacity: 0.8
            }
        }
    }
}
