import QtQuick
import QtQuick.Controls

Button {
    property string mode
    property bool selected: false
    implicitHeight: 72
    background: Rectangle { radius: 15; color: selected ? "#D8E2FF" : "#20222B"; border.color: selected ? "#9FB4FF" : "#30333E"; border.width: 1 }
    contentItem: Label { text: mode; color: selected ? "#1C2A5A" : "#EEF0F7"; font.pixelSize: 14; font.weight: Font.DemiBold; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; wrapMode: Text.Wrap }
}
