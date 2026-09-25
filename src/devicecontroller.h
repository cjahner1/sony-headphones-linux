#pragma once

#include <QObject>
#include <QString>

#include "bluezdiscovery.h"

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
    Q_PROPERTY(QString protocolStatus READ protocolStatus NOTIFY stateChanged)
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
    QString protocolStatus() const;
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setNoiseMode(const QString &mode);
    Q_INVOKABLE void setSpeakToChat(bool enabled);
    Q_INVOKABLE void setDsee(bool enabled);
    void setVolume(int volume);
signals:
    void stateChanged();
private:
    BluezDiscovery m_bluez;
    int m_batteryLeft = 0, m_batteryRight = 0, m_batteryCase = 0, m_volume = 50;
    QString m_noiseMode = "Noise cancelling";
    bool m_speakToChat = false, m_dsee = true;
};
