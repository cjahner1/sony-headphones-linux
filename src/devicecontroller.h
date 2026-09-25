#pragma once

#include <QObject>
#include <QString>

#include "bluezdiscovery.h"
#include "mdrtransport.h"

class DeviceController final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY stateChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY stateChanged)
    Q_PROPERTY(int batteryLeft READ batteryLeft NOTIFY stateChanged)
    Q_PROPERTY(int batteryRight READ batteryRight NOTIFY stateChanged)
    Q_PROPERTY(int batteryCase READ batteryCase NOTIFY stateChanged)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY stateChanged)
    Q_PROPERTY(QString noiseMode READ noiseMode NOTIFY stateChanged)
    Q_PROPERTY(bool speakToChat READ speakToChat NOTIFY stateChanged)
    Q_PROPERTY(bool dsee READ dsee NOTIFY stateChanged)
    Q_PROPERTY(bool playing READ playing NOTIFY stateChanged)
    Q_PROPERTY(QString playbackSource READ playbackSource NOTIFY stateChanged)
    Q_PROPERTY(bool mediaControlsAvailable READ mediaControlsAvailable NOTIFY stateChanged)
    Q_PROPERTY(QString playbackSourceSwitchStatus READ playbackSourceSwitchStatus NOTIFY stateChanged)
    Q_PROPERTY(QString protocolStatus READ protocolStatus NOTIFY stateChanged)
    Q_PROPERTY(bool mdrReady READ mdrReady NOTIFY stateChanged)
public:
    explicit DeviceController(QObject *parent = nullptr);
    QString name() const;
    bool connected() const;
    int batteryLeft() const { return m_batteryLeft; }
    int batteryRight() const { return m_batteryRight; }
    int batteryCase() const { return m_batteryCase; }
    int volume() const { return m_volume; }
    QString noiseMode() const { return m_noiseMode; }
    bool speakToChat() const { return m_speakToChat; }
    bool dsee() const { return m_dsee; }
    bool playing() const { return m_playing; }
    QString playbackSource() const { return m_playbackSource; }
    bool mediaControlsAvailable() const { return m_mediaControlsAvailable; }
    QString playbackSourceSwitchStatus() const { return m_playbackSourceSwitchStatus; }
    QString protocolStatus() const;
    bool mdrReady() const { return m_mdr.ready(); }
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setNoiseMode(const QString &mode);
    Q_INVOKABLE void setSpeakToChat(bool enabled);
    Q_INVOKABLE void setDsee(bool enabled);
    Q_INVOKABLE void playback(const QString &action);
    Q_INVOKABLE void selectLocalPlaybackSource();
    void setVolume(int volume);
signals:
    void stateChanged();
private:
    BluezDiscovery m_bluez;
    MdrTransport m_mdr;
    int m_batteryLeft = 0, m_batteryRight = 0, m_batteryCase = 0, m_volume = 50;
    QString m_noiseMode = "Noise cancelling";
    bool m_speakToChat = false, m_dsee = true, m_playing = false, m_mediaControlsAvailable = false;
    QString m_playbackSource = "Checking playback source…";
    QString m_playbackSourceSwitchStatus;
};
