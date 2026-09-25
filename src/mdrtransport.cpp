#include "mdrtransport.h"

#include <mdr-bt/ConnectionLinux.h>
#include <mdr-c/Base.h>
#include <mdr-c/Headphones.h>

#include <QDebug>

#include <algorithm>

MdrTransport::MdrTransport(QObject *parent) : QObject(parent) {
    m_timer.setInterval(20);
    connect(&m_timer, &QTimer::timeout, this, &MdrTransport::poll);
}

MdrTransport::~MdrTransport() { disconnect(); }

void MdrTransport::connectTo(const QString &address) {
    if (address.isEmpty() || (m_address == address && (m_connecting || m_ready))) return;
    disconnect();
    m_address = address;
    m_linux = mdrConnectionLinuxCreate();
    if (!m_linux) return fail("Could not create the MDR BlueZ transport");
    const auto result = mdrConnectionConnect(mdrConnectionLinuxGet(m_linux), address.toUtf8().constData(), MDR_SERVICE_UUID_XM5);
    if (result != MDR_RESULT_OK && result != MDR_RESULT_INPROGRESS)
        return fail(QString("MDR connection failed: %1").arg(mdrResultString(result)));
    m_connecting = true;
    m_status = "Opening secure MDR connection…";
    qInfo() << m_status << address;
    m_timer.start();
    emit stateChanged();
}

void MdrTransport::disconnect() {
    m_timer.stop();
    if (m_headphones) mdrHeadphonesDestroy(m_headphones), m_headphones = nullptr;
    if (m_linux) mdrConnectionDisconnect(mdrConnectionLinuxGet(m_linux)), mdrConnectionLinuxDestroy(m_linux), m_linux = nullptr;
    m_connecting = false; m_ready = false;
}

void MdrTransport::fail(const QString &message) {
    qWarning() << message;
    disconnect();
    m_status = message;
    emit stateChanged();
}

void MdrTransport::poll() {
    auto *connection = mdrConnectionLinuxGet(m_linux);
    if (m_connecting) {
        const auto result = mdrConnectionPoll(connection, 0);
        if (result == MDR_RESULT_INPROGRESS || result == MDR_RESULT_ERROR_TIMEOUT) return;
        if (result != MDR_RESULT_OK)
            return fail(QString("MDR transport: %1").arg(mdrConnectionGetLastError(connection)));
        m_connecting = false;
        if (mdrHeadphonesCreate(MDR_ABI_VERSION, connection, MDR_PROTOCOL_V2, &m_headphones) != MDR_RESULT_OK)
            return fail("Could not initialize the MDR protocol");
        if (mdrHeadphonesRequestInit(m_headphones) != MDR_RESULT_OK)
            return fail("MDR initialization request failed");
        m_status = "Initializing WF-1000XM6…";
        emit stateChanged();
        return;
    }
    MDREvent event = MDR_EVENT_NONE;
    const auto result = mdrHeadphonesPoll(m_headphones, &event);
    if (result != MDR_RESULT_OK && result != MDR_RESULT_INPROGRESS)
        return fail(QString("MDR protocol: %1").arg(mdrResultString(result)));
    if (event == MDR_EVENT_INITIALIZE_COMPLETE) mdrHeadphonesRequestSync(m_headphones);
    if (event == MDR_EVENT_BATTERY_CHANGED || event == MDR_EVENT_SYNC_COMPLETE) updateBatteries();
    if (mdrHeadphonesIsReady(m_headphones) && !m_ready) {
        m_ready = true; m_status = "MDR connected"; updateBatteries(); emit stateChanged();
    }
}

void MdrTransport::updateBatteries() {
    MDRBattery batteries[4]{}; uint32_t count = 4;
    if (mdrHeadphonesGetBatteries(m_headphones, batteries, &count) != MDR_RESULT_OK) return;
    int left = 0, right = 0, caseLevel = 0;
    for (uint32_t i = 0; i < count; ++i) {
        if (!batteries[i].present) continue;
        if (batteries[i].part == MDR_BATTERY_LEFT) left = batteries[i].level_percent;
        if (batteries[i].part == MDR_BATTERY_RIGHT) right = batteries[i].level_percent;
        if (batteries[i].part == MDR_BATTERY_CASE) caseLevel = batteries[i].level_percent;
    }
    emit batteriesChanged(left, right, caseLevel);
}

bool MdrTransport::commit(const QString &operation) {
    if (!m_ready || !m_headphones) {
        qWarning() << operation << "ignored: MDR session is not ready";
        return false;
    }
    const auto result = mdrHeadphonesRequestCommit(m_headphones);
    if (result == MDR_RESULT_OK || result == MDR_RESULT_INPROGRESS) return true;
    qWarning() << operation << "commit failed:" << mdrResultString(result);
    return false;
}

bool MdrTransport::setNoiseMode(const QString &mode) {
    MDRNoiseControl noise{};
    if (!m_ready || mdrHeadphonesGetNoiseControl(m_headphones, &noise) != MDR_RESULT_OK) return false;
    if (mode == "Noise cancelling") noise.mode = MDR_NOISE_MODE_CANCELLING;
    else if (mode == "Ambient sound") noise.mode = MDR_NOISE_MODE_AMBIENT;
    else if (mode == "Off") noise.mode = MDR_NOISE_MODE_OFF;
    else return false;
    if (mdrHeadphonesSetNoiseControl(m_headphones, &noise) != MDR_RESULT_OK) return false;
    return commit("Noise mode");
}

bool MdrTransport::setSpeakToChat(bool enabled) {
    MDRSpeakToChat speak{};
    if (!m_ready || mdrHeadphonesGetSpeakToChat(m_headphones, &speak) != MDR_RESULT_OK) return false;
    speak.enabled = enabled ? MDR_TRUE : MDR_FALSE;
    if (mdrHeadphonesSetSpeakToChat(m_headphones, &speak) != MDR_RESULT_OK) return false;
    return commit("Speak-to-Chat");
}

bool MdrTransport::setDsee(bool enabled) {
    MDREqualizer equalizer{};
    if (!m_ready || mdrHeadphonesGetEqualizer(m_headphones, &equalizer) != MDR_RESULT_OK || !equalizer.dsee_available)
        return false;
    equalizer.dsee_enabled = enabled ? MDR_TRUE : MDR_FALSE;
    if (mdrHeadphonesSetEqualizer(m_headphones, &equalizer) != MDR_RESULT_OK) return false;
    return commit("DSEE");
}

bool MdrTransport::setVolume(int volume) {
    MDRPlayback playback{};
    if (!m_ready || mdrHeadphonesGetPlayback(m_headphones, &playback) != MDR_RESULT_OK) return false;
    playback.volume = static_cast<uint8_t>(std::clamp(volume, 0, 100));
    if (mdrHeadphonesSetPlayback(m_headphones, &playback) != MDR_RESULT_OK) return false;
    return commit("Volume");
}
