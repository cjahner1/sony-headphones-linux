#include "mdrtransport.h"

#include <mdr-bt/ConnectionLinux.h>
#include <mdr-c/Base.h>
#include <mdr-c/Headphones.h>

#include <QDebug>
#include <QSysInfo>

#include <algorithm>
#include <vector>

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

void MdrTransport::refresh() {
    if (!m_ready || !m_headphones) return;
    qInfo() << "Requesting MDR state and multipoint refresh";
    if (mdrHeadphonesIsReady(m_headphones)) mdrHeadphonesRequestSync(m_headphones);
    updatePairedDevices();
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
    if (event == MDR_EVENT_NOISE_CONTROL_CHANGED || event == MDR_EVENT_SPEAK_TO_CHAT_CHANGED ||
        event == MDR_EVENT_PLAYBACK_CHANGED || event == MDR_EVENT_EQUALIZER_CHANGED ||
        event == MDR_EVENT_SYNC_COMPLETE) updateSoundState();
    if (event == MDR_EVENT_PAIRED_DEVICES_CHANGED || event == MDR_EVENT_SYNC_COMPLETE) {
        if (m_sourceSwitchPending && event == MDR_EVENT_PAIRED_DEVICES_CHANGED) logSourceSwitchResult();
        updatePairedDevices();
    }
    if (mdrHeadphonesIsReady(m_headphones) && !m_ready) {
        m_ready = true; m_status = "MDR connected"; updateBatteries(); updateSoundState(); updatePairedDevices(); emit stateChanged();
    }
    if (m_ready && !m_pendingPlayback.isEmpty() && mdrHeadphonesIsReady(m_headphones)) {
        const auto pending = m_pendingPlayback;
        m_pendingPlayback.clear();
        playback(pending);
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

void MdrTransport::updateSoundState() {
    if (!m_headphones) return;
    MDRPlayback playback{}; MDRNoiseControl noise{}; MDRSpeakToChat speak{}; MDREqualizer equalizer{};
    const bool hasPlayback = mdrHeadphonesGetPlayback(m_headphones, &playback) == MDR_RESULT_OK;
    const bool hasNoise = mdrHeadphonesGetNoiseControl(m_headphones, &noise) == MDR_RESULT_OK;
    const bool hasSpeak = mdrHeadphonesGetSpeakToChat(m_headphones, &speak) == MDR_RESULT_OK;
    const bool hasEqualizer = mdrHeadphonesGetEqualizer(m_headphones, &equalizer) == MDR_RESULT_OK;
    QString mode = "Off";
    if (hasNoise && noise.mode == MDR_NOISE_MODE_CANCELLING) mode = "Noise cancelling";
    else if (hasNoise && noise.mode == MDR_NOISE_MODE_AMBIENT) mode = "Ambient sound";
    emit soundStateChanged(hasPlayback ? playback.volume : 0, mode,
                           hasSpeak && speak.enabled == MDR_TRUE,
                           hasEqualizer && equalizer.dsee_enabled == MDR_TRUE);
    if (hasPlayback) emit playbackStateChanged(playback.status == MDR_PLAYBACK_PLAYING);
}

void MdrTransport::updatePairedDevices() {
    qInfo() << "Querying MDR multipoint devices";
    if (!m_headphones) return;
    uint32_t count = 0;
    const auto countResult = mdrHeadphonesGetPairedDevices(m_headphones, nullptr, &count);
    if (countResult != MDR_RESULT_OK) {
        qInfo() << "MDR multipoint device count unavailable:" << mdrResultString(countResult);
        return;
    }
    if (!count) { qInfo() << "MDR reports no multipoint devices"; return; }
    std::vector<MDRPairedDevice> devices(count);
    const auto listResult = mdrHeadphonesGetPairedDevices(m_headphones, devices.data(), &count);
    if (listResult != MDR_RESULT_OK) {
        qWarning() << "MDR multipoint device list unavailable:" << mdrResultString(listResult);
        return;
    }
    qInfo() << "MDR multipoint devices:" << count;
    QString playbackSource;
    const auto localHost = QSysInfo::machineHostName();
    bool playbackControllable = false;
    m_localPlaybackDeviceId.clear();
    for (uint32_t i = 0; i < count; ++i) {
        qInfo().noquote() << QString("MDR device: '%1' (%2), connected=%3, playback-source=%4")
            .arg(devices[i].name, devices[i].macAddress)
            .arg(devices[i].connected == MDR_TRUE ? "true" : "false")
            .arg(devices[i].playback_device == MDR_TRUE ? "true" : "false");
        if (devices[i].playback_device == MDR_TRUE) {
            playbackSource = QString::fromUtf8(devices[i].name);
            playbackControllable = playbackSource.compare(localHost, Qt::CaseInsensitive) == 0;
        }
        if (QString::fromUtf8(devices[i].name).compare(localHost, Qt::CaseInsensitive) == 0 &&
            devices[i].connected == MDR_TRUE)
            m_localPlaybackDeviceId = QString::fromUtf8(devices[i].macAddress);
    }
    if (playbackSource.isEmpty()) {
        qInfo() << "MDR reports no active playback source";
        playbackSource = "No active source";
    }
    qInfo().noquote() << QString("MDR playback source: '%1'; local host '%2'; media controls available=%3")
        .arg(playbackSource, localHost, playbackControllable ? "true" : "false");
    emit playbackSourceChanged(playbackSource, playbackControllable);
}

void MdrTransport::logSourceSwitchResult() {
    m_sourceSwitchPending = false;
    MDRSourceSwitchControlResult result{};
    if (mdrHeadphonesGetSourceSwitchControlResult(m_headphones, &result) != MDR_RESULT_OK) {
        qWarning() << "MDR playback source switch completed, but no result was available";
        return;
    }
    QString outcome = "failed";
    if (result == MDR_SOURCE_SWITCH_CONTROL_SUCCESS) outcome = "success";
    else if (result == MDR_SOURCE_SWITCH_CONTROL_FAILED_ON_CALL) outcome = "failed: call is active";
    else if (result == MDR_SOURCE_SWITCH_CONTROL_FAILED_NOT_CONNECTED) outcome = "failed: target is not connected for audio";
    else if (result == MDR_SOURCE_SWITCH_CONTROL_FAILED_VOICE_ASSISTANT) outcome = "failed: voice assistant has priority";
    qInfo() << "MDR playback source switch result:" << outcome;
}

bool MdrTransport::commit(const QString &operation) {
    if (!m_ready || !m_headphones) {
        qWarning() << operation << "ignored: MDR session is not ready";
        return false;
    }
    const auto result = mdrHeadphonesRequestCommit(m_headphones);
    if (result == MDR_RESULT_OK || result == MDR_RESULT_INPROGRESS) {
        qInfo() << operation << "commit accepted";
        return true;
    }
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

bool MdrTransport::playback(const QString &action) {
    if (!m_ready || !m_headphones) {
        qWarning() << "Playback command ignored: MDR session is not ready:" << action;
        return false;
    }
    MDRPlaybackCommand command{};
    if (action == "play") command.action = MDR_PLAYBACK_PLAY;
    else if (action == "pause") command.action = MDR_PLAYBACK_PAUSE;
    else if (action == "next") command.action = MDR_PLAYBACK_NEXT;
    else if (action == "previous") command.action = MDR_PLAYBACK_PREVIOUS;
    else return false;
    qInfo() << "Sending MDR playback command:" << action;
    const auto result = mdrHeadphonesPlayback(m_headphones, &command);
    if (result == MDR_RESULT_OK) {
        qInfo() << "MDR playback command accepted:" << action;
        return true;
    }
    if (result == MDR_RESULT_INPROGRESS) {
        m_pendingPlayback = action;
        qInfo() << "Queued playback command until MDR is ready:" << action;
        return true;
    }
    qWarning() << "MDR playback command failed:" << action << mdrResultString(result);
    return false;
}

bool MdrTransport::selectLocalPlaybackSource() {
    if (!m_ready || !m_headphones || m_localPlaybackDeviceId.isEmpty()) {
        qWarning() << "Cannot switch playback to this laptop: local multipoint device is unavailable";
        return false;
    }
    const auto deviceId = m_localPlaybackDeviceId.toUtf8();
    MDRPairedDeviceAction action{};
    action.command = MDR_PAIRED_DEVICE_SELECT_PLAYBACK;
    action.device_id = deviceId.constData();
    action.device_id_size = static_cast<uint32_t>(deviceId.size());
    qInfo() << "Requesting MDR playback source switch to local device:" << m_localPlaybackDeviceId;
    const auto result = mdrHeadphonesSetPairedDevice(m_headphones, &action);
    if (result == MDR_RESULT_OK || result == MDR_RESULT_INPROGRESS) {
        qInfo() << "MDR playback source switch staged";
        if (!commit("MDR playback source switch")) return false;
        m_sourceSwitchPending = true;
        QTimer::singleShot(5000, this, [this] {
            if (m_sourceSwitchPending) {
                m_sourceSwitchPending = false;
                qWarning() << "MDR playback source switch timed out waiting for a headphone response";
                updatePairedDevices();
            }
        });
        return true;
    }
    qWarning() << "MDR playback source switch failed:" << mdrResultString(result);
    return false;
}
