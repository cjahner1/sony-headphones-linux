import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "components"

ApplicationWindow {
    visible: true; width: 1040; height: 710; minimumWidth: 860; minimumHeight: 600
    title: "Sony Headphones"
    color: "#111218"
    component Card: Rectangle { color: "#1A1C24"; radius: 22; border.color: "#292C36"; border.width: 1 }
    header: ToolBar { height: 68; background: Rectangle { color: "#111218" }
        RowLayout { anchors.fill: parent; anchors.leftMargin: 32; anchors.rightMargin: 32
            Label { text: "SONY"; color: "#F4F5FA"; font.pixelSize: 22; font.weight: Font.Bold; font.letterSpacing: 2 }
            Label { text: "Headphones"; color: "#9DA1B0"; font.pixelSize: 17; Layout.leftMargin: 10 }
            Item { Layout.fillWidth: true }
            StatusPill { text: device.connected ? "Connected" : "Looking for device"; dotColor: device.connected ? "#73D89D" : "#F6C453" }
            ToolButton { text: "↻"; font.pixelSize: 24; onClicked: device.refresh(); Layout.leftMargin: 12 }
        }
    }
    ScrollView { anchors.fill: parent; clip: true
        contentWidth: availableWidth
        ColumnLayout { width: parent.width; spacing: 20; anchors.margins: 32
            RowLayout { Layout.fillWidth: true; spacing: 20
                Card { Layout.preferredWidth: 430; Layout.fillHeight: true; Layout.minimumHeight: 270
                    Column { anchors.fill: parent; anchors.margins: 28; spacing: 12
                        Label { text: device.name; color: "#F7F8FC"; font.pixelSize: 26; font.weight: Font.DemiBold }
                        Label { text: "WF-1000XM6 • Adaptive Sound Control"; color: "#A4A8B7"; font.pixelSize: 14 }
                        Item { height: 12 }
                        Row { spacing: 28
                            Repeater { model: [{label:"LEFT", value:device.batteryLeft}, {label:"RIGHT", value:device.batteryRight}, {label:"CASE", value:device.batteryCase}]
                                Column {
                                    spacing: 5
                                    Label { text: modelData.label; color: "#9498A7"; font.pixelSize: 11; font.weight: Font.Bold }
                                    Label { text: device.connected ? modelData.value + "%" : "—"; color: "#F7F8FC"; font.pixelSize: 28; font.weight: Font.DemiBold }
                                }
                            }
                        }
                        Item { height: 2 }
                        Label { text: device.protocolStatus; color: "#AEB3C3"; font.pixelSize: 13; wrapMode: Text.WordWrap; width: parent.width }
                    }
                }
                Card { Layout.fillWidth: true; Layout.minimumHeight: 270
                    Column { anchors.fill: parent; anchors.margins: 28; spacing: 16
                        Label { text: "Listening mode"; color: "#F7F8FC"; font.pixelSize: 19; font.weight: Font.DemiBold }
                        RowLayout { width: parent.width; spacing: 10; enabled: device.mdrReady; opacity: enabled ? 1 : 0.45
                            NoiseModeButton { Layout.fillWidth: true; mode: "Noise\ncancelling"; selected: device.noiseMode === "Noise cancelling"; onClicked: device.setNoiseMode("Noise cancelling") }
                            NoiseModeButton { Layout.fillWidth: true; mode: "Ambient\nsound"; selected: device.noiseMode === "Ambient sound"; onClicked: device.setNoiseMode("Ambient sound") }
                            NoiseModeButton { Layout.fillWidth: true; mode: "Off"; selected: device.noiseMode === "Off"; onClicked: device.setNoiseMode("Off") }
                        }
                        Label { text: device.noiseMode; color: "#BAC9FF"; font.pixelSize: 14 }
                        Row {
                            visible: device.noiseMode === "Ambient sound"
                            enabled: device.mdrReady; opacity: enabled ? 1 : 0.45; spacing: 10
                            Label { text: "Ambient"; color: "#B2B6C5"; width: 62; anchors.verticalCenter: parent.verticalCenter }
                            Slider { width: 180; from: 0; to: 20; stepSize: 1; value: device.ambientLevel; onMoved: device.setAmbientLevel(value) }
                            Label { text: device.ambientLevel; color: "#F7F8FC"; anchors.verticalCenter: parent.verticalCenter }
                        }
                        Switch { visible: device.noiseMode === "Ambient sound"; text: "Focus on Voice"; enabled: device.mdrReady; checked: device.focusOnVoice; onToggled: device.setFocusOnVoice(checked) }
                    }
                }
            }
            RowLayout { Layout.fillWidth: true; spacing: 20
                Card { Layout.fillWidth: true; Layout.minimumHeight: 215
                    Column { anchors.fill: parent; anchors.margins: 28; spacing: 16
                        Label { text: "Sound"; color: "#F7F8FC"; font.pixelSize: 19; font.weight: Font.DemiBold }
                        Row {
                            enabled: device.mdrReady; opacity: enabled ? 1 : 0.45
                            spacing: 12
                            Label { text: "Volume"; color: "#B2B6C5"; width: 76; anchors.verticalCenter: parent.verticalCenter }
                            Slider { width: 240; from: 0; to: 100; value: device.volume; onMoved: device.setVolume(value) }
                            Label { text: device.volume + "%"; color: "#F7F8FC"; anchors.verticalCenter: parent.verticalCenter }
                        }
                        Label { text: "Clear Bass is applied through MDR; preset/EQ-band editing follows next."; color: "#8F94A3"; font.pixelSize: 13; wrapMode: Text.WordWrap; width: parent.width }
                        Row {
                            enabled: device.mdrReady; opacity: enabled ? 1 : 0.45; spacing: 12
                            Label { text: "Clear Bass"; color: "#B2B6C5"; width: 76; anchors.verticalCenter: parent.verticalCenter }
                            Slider { width: 190; from: -10; to: 10; stepSize: 1; value: device.clearBass; onMoved: device.setClearBass(value) }
                            Label { text: device.clearBass > 0 ? "+" + device.clearBass : device.clearBass; color: "#F7F8FC"; anchors.verticalCenter: parent.verticalCenter }
                        }
                        Label {
                            text: "Playback source: " + device.playbackSource
                            color: device.mediaControlsAvailable ? "#8F94A3" : "#F6C453"
                            font.pixelSize: 13
                            wrapMode: Text.WordWrap
                            width: parent.width
                        }
                        Label {
                            visible: device.mdrReady && !device.mediaControlsAvailable
                            text: "Media controls only work when this laptop is the active source."
                            color: "#8F94A3"
                            font.pixelSize: 13
                            wrapMode: Text.WordWrap
                            width: parent.width
                        }
                        Button {
                            visible: device.mdrReady && !device.mediaControlsAvailable
                            text: "Switch playback to this laptop"
                            onClicked: device.selectLocalPlaybackSource()
                        }
                        Label {
                            visible: device.playbackSourceSwitchStatus.length > 0
                            text: device.playbackSourceSwitchStatus
                            color: "#BAC9FF"
                            font.pixelSize: 13
                            wrapMode: Text.WordWrap
                            width: parent.width
                        }
                        Row { spacing: 8; enabled: device.mdrReady && device.mediaControlsAvailable; opacity: enabled ? 1 : 0.45
                            Button { text: "‹‹"; onClicked: device.playback("previous") }
                            Button { text: device.playing ? "Pause" : "Play"; onClicked: device.playback(device.playing ? "pause" : "play") }
                            Button { text: "››"; onClicked: device.playback("next") }
                        }
                    }
                }
                Card { Layout.fillWidth: true; Layout.minimumHeight: 215
                    Column { anchors.fill: parent; anchors.margins: 28; spacing: 10
                        Label { text: "Smart features"; color: "#F7F8FC"; font.pixelSize: 19; font.weight: Font.DemiBold }
                        Switch { text: "Speak-to-Chat"; enabled: device.mdrReady; checked: device.speakToChat; onToggled: device.setSpeakToChat(checked) }
                        Switch { text: "DSEE Extreme"; enabled: device.mdrReady; checked: device.dsee; onToggled: device.setDsee(checked) }
                        Row {
                            enabled: device.mdrReady; opacity: enabled ? 1 : 0.45; spacing: 10
                            Label { text: "Voice guidance"; color: "#B2B6C5"; width: 104; anchors.verticalCenter: parent.verticalCenter }
                            Slider { width: 145; from: -2; to: 2; stepSize: 1; value: device.voiceGuidanceVolume; onMoved: device.setVoiceGuidanceVolume(value) }
                            Label { text: device.voiceGuidanceVolume > 0 ? "+" + device.voiceGuidanceVolume : device.voiceGuidanceVolume; color: "#F7F8FC"; anchors.verticalCenter: parent.verticalCenter }
                        }
                        Switch { text: "Automatic multipoint switching"; enabled: device.mdrReady; checked: device.automaticSourceSwitch; onToggled: device.setAutomaticSourceSwitch(checked) }
                        Label { text: "Touch sensor"; color: "#F7F8FC"; font.pixelSize: 16; font.weight: Font.DemiBold }
                        Row { enabled: device.mdrReady; opacity: enabled ? 1 : 0.45; spacing: 10
                            Label { text: "Left"; color: "#B2B6C5"; width: 42; anchors.verticalCenter: parent.verticalCenter }
                            ComboBox { id: leftTouch; model: ["Playback", "Noise control", "Volume", "Quick Access"]; currentIndex: Math.max(0, model.indexOf(device.leftTouchAssignment)); onActivated: device.setTouchAssignment("left", currentText) }
                            Label { text: "Right"; color: "#B2B6C5"; width: 42; anchors.verticalCenter: parent.verticalCenter }
                            ComboBox { id: rightTouch; model: ["Playback", "Noise control", "Volume", "Quick Access"]; currentIndex: Math.max(0, model.indexOf(device.rightTouchAssignment)); onActivated: device.setTouchAssignment("right", currentText) }
                        }
                    }
                }
            }
        }
    }
}
