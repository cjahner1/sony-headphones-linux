import QtQuick
import QtQuick.Controls

Rectangle {
    property string text: "Connected"
    property color dotColor: "#73D89D"
    implicitWidth: label.implicitWidth + 28
    implicitHeight: 30
    radius: 15
    color: "#1D3028"
    Row { anchors.centerIn: parent; spacing: 7
        Rectangle { width: 7; height: 7; radius: 4; color: parent.parent.dotColor; anchors.verticalCenter: parent.verticalCenter }
        Label { id: label; text: parent.parent.text; color: "#D9F8E5"; font.pixelSize: 13; font.weight: Font.DemiBold }
    }
}
