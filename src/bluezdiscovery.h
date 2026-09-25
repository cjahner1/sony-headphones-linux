#pragma once

#include <QObject>
#include <QDBusObjectPath>
#include <QMap>
#include <QString>
#include <QVariantMap>

using BluezInterfaceMap = QMap<QString, QVariantMap>;
using BluezManagedObjects = QMap<QDBusObjectPath, BluezInterfaceMap>;
Q_DECLARE_METATYPE(BluezInterfaceMap)
Q_DECLARE_METATYPE(BluezManagedObjects)

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
    static bool isSonyHeadphones(const QString &name);
    QString m_deviceName;
    bool m_connected = false;
};
