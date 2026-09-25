#include "devicecontroller.h"

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
    m_bluez.refresh();
}
QString DeviceController::name() const {
    return m_bluez.deviceName().isEmpty() ? "No Sony headphones found" : m_bluez.deviceName();
}
bool DeviceController::connected() const { return m_bluez.connected(); }
QString DeviceController::protocolStatus() const {
    return connected() ? m_mdr.status() : "Pair your WF-1000XM6 in Bluetooth Settings";
}
void DeviceController::refresh() { m_bluez.refresh(); }
void DeviceController::setNoiseMode(const QString &mode) { m_noiseMode = mode; emit stateChanged(); }
void DeviceController::setSpeakToChat(bool enabled) { m_speakToChat = enabled; emit stateChanged(); }
void DeviceController::setDsee(bool enabled) { m_dsee = enabled; emit stateChanged(); }
void DeviceController::setVolume(int volume) { m_volume = std::clamp(volume, 0, 100); emit stateChanged(); }
