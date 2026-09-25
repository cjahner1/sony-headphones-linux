#pragma once

#include <QObject>
#include <QString>
#include <QTimer>

struct MDRConnectionLinux;
struct MDRHeadphones;

class MdrTransport final : public QObject {
    Q_OBJECT
public:
    explicit MdrTransport(QObject *parent = nullptr);
    ~MdrTransport() override;
    void connectTo(const QString &address);
    void disconnect();
    void refresh();
    bool ready() const { return m_ready; }
    QString status() const { return m_status; }
    bool setNoiseMode(const QString &mode);
    bool setSpeakToChat(bool enabled);
    bool setDsee(bool enabled);
    bool setVolume(int volume);
    bool playback(const QString &action);
    bool selectLocalPlaybackSource();
signals:
    void stateChanged();
    void batteriesChanged(int left, int right, int caseLevel);
    void soundStateChanged(int volume, const QString &noiseMode, bool speakToChat, bool dsee);
    void playbackStateChanged(bool playing);
    void playbackSourceChanged(const QString &source, bool controllable);
    void playbackSourceSwitchStatusChanged(const QString &status);
private slots:
    void poll();
private:
    void fail(const QString &message);
    void updateBatteries();
    void updateSoundState();
    void updatePairedDevices();
    void logSourceSwitchResult();
    bool commit(const QString &operation);
    MDRConnectionLinux *m_linux = nullptr;
    MDRHeadphones *m_headphones = nullptr;
    QTimer m_timer;
    QString m_address;
    QString m_status = "MDR transport idle";
    bool m_connecting = false;
    bool m_ready = false;
    QString m_pendingPlayback;
    QString m_localPlaybackDeviceId;
    bool m_sourceSwitchPending = false;
};
