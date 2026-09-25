#include "devicecontroller.h"

#include <QDebug>

#include <algorithm>

DeviceController::DeviceController(QObject *parent) : QObject(parent), m_bluez(this) {
    connect(&m_bluez, &BluezDiscovery::changed, this, [this] {
        if (m_bluez.connected()) m_mdr.connectTo(m_bluez.address());
        else m_mdr.disconnect();
        emit stateChanged();
    });
    connect(&m_mdr, &MdrTransport::stateChanged, this, &DeviceController::stateChanged);
    connect(&m_mdr, &MdrTransport::batteriesChanged, this, [this](int left, int right, int caseLevel) {
        m_batteryLeft = left; m_batteryRight = right; m_batteryCase = caseLevel; emit stateChanged();
    });
    connect(&m_mdr, &MdrTransport::soundStateChanged, this, [this](int volume, const QString &noise, bool speak, bool dsee) {
        m_volume = volume; m_noiseMode = noise; m_speakToChat = speak; m_dsee = dsee; emit stateChanged();
    });
    connect(&m_mdr, &MdrTransport::playbackStateChanged, this, [this](bool playing) { m_playing = playing; emit stateChanged(); });
    connect(&m_mdr, &MdrTransport::playbackSourceChanged, this, [this](const QString &source, bool controllable) {
        m_playbackSource = source;
        m_mediaControlsAvailable = controllable;
        emit stateChanged();
    });
    connect(&m_mdr, &MdrTransport::playbackSourceSwitchStatusChanged, this, [this](const QString &status) {
        m_playbackSourceSwitchStatus = status;
        emit stateChanged();
    });
    connect(&m_mdr, &MdrTransport::advancedStateChanged, this, [this](int ambient, bool focus, int clearBass, int voice, bool automatic) {
        m_ambientLevel = ambient; m_focusOnVoice = focus; m_clearBass = clearBass;
        m_voiceGuidanceVolume = voice; m_automaticSourceSwitch = automatic; emit stateChanged();
    });
    m_bluez.refresh();
}
QString DeviceController::name() const {
    return m_bluez.deviceName().isEmpty() ? "No Sony headphones found" : m_bluez.deviceName();
}
bool DeviceController::connected() const { return m_bluez.connected(); }
QString DeviceController::protocolStatus() const {
    return connected() ? m_mdr.status() : "Pair your WF-1000XM6 in Bluetooth Settings";
}
void DeviceController::refresh() { m_bluez.refresh(); m_mdr.refresh(); }
void DeviceController::setNoiseMode(const QString &mode) {
    if (m_mdr.setNoiseMode(mode)) { m_noiseMode = mode; emit stateChanged(); }
}
void DeviceController::setSpeakToChat(bool enabled) {
    if (m_mdr.setSpeakToChat(enabled)) { m_speakToChat = enabled; emit stateChanged(); }
}
void DeviceController::setDsee(bool enabled) {
    if (m_mdr.setDsee(enabled)) { m_dsee = enabled; emit stateChanged(); }
}
void DeviceController::playback(const QString &action) {
    if (!m_mediaControlsAvailable) {
        qInfo() << "Playback command not sent: active source is" << m_playbackSource;
        return;
    }
    m_mdr.playback(action);
}
void DeviceController::selectLocalPlaybackSource() {
    qInfo() << "UI requested playback source switch";
    m_playbackSourceSwitchStatus = "Button click received — contacting the XM6…";
    emit stateChanged();
    m_mdr.selectLocalPlaybackSource();
}
void DeviceController::setAmbientLevel(int level) { if (m_mdr.setAmbientLevel(level)) { m_ambientLevel = std::clamp(level, 0, 20); emit stateChanged(); } }
void DeviceController::setFocusOnVoice(bool enabled) { if (m_mdr.setFocusOnVoice(enabled)) { m_focusOnVoice = enabled; m_noiseMode = "Ambient sound"; emit stateChanged(); } }
void DeviceController::setClearBass(int level) { if (m_mdr.setClearBass(level)) { m_clearBass = std::clamp(level, -10, 10); emit stateChanged(); } }
void DeviceController::setVoiceGuidanceVolume(int level) { if (m_mdr.setVoiceGuidanceVolume(level)) { m_voiceGuidanceVolume = std::clamp(level, -2, 2); emit stateChanged(); } }
void DeviceController::setAutomaticSourceSwitch(bool enabled) { if (m_mdr.setAutomaticSourceSwitch(enabled)) { m_automaticSourceSwitch = enabled; emit stateChanged(); } }
void DeviceController::setVolume(int volume) {
    if (m_mdr.setVolume(volume)) { m_volume = std::clamp(volume, 0, 100); emit stateChanged(); }
}
