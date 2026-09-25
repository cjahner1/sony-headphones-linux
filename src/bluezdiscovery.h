#pragma once

#include <QObject>
#include <QString>

// Read-only BlueZ discovery. The MDR command transport is deliberately separate:
// it must be backed by a verified libmdr/libmdr-bt session, not guessed D-Bus writes.
class BluezDiscovery final : public QObject {
    Q_OBJECT
public:
    explicit BluezDiscovery(QObject *parent = nullptr);
    Q_INVOKABLE void refresh();
    QString deviceName() const { return m_deviceName; }
    bool connected() const { return m_connected; }
signals:
    void changed();
private:
    QString m_deviceName;
    bool m_connected = false;
};
