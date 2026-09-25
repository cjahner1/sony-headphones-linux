#include "bluezdiscovery.h"

#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusVariant>
#include <QVariantMap>

BluezDiscovery::BluezDiscovery(QObject *parent) : QObject(parent) { refresh(); }

void BluezDiscovery::refresh() {
    QDBusInterface manager("org.bluez", "/", "org.freedesktop.DBus.ObjectManager",
                           QDBusConnection::systemBus());
    if (!manager.isValid()) { m_deviceName.clear(); m_connected = false; emit changed(); return; }
    const auto reply = manager.call("GetManagedObjects");
    const auto objects = qdbus_cast<QVariantMap>(reply.arguments().value(0));
    QString foundName;
    bool foundConnected = false;
    for (auto object = objects.cbegin(); object != objects.cend(); ++object) {
        const auto interfaces = qdbus_cast<QVariantMap>(object.value());
        const auto props = qdbus_cast<QVariantMap>(interfaces.value("org.bluez.Device1"));
        const auto name = props.value("Name").value<QDBusVariant>().variant().toString();
        const auto connected = props.value("Connected").value<QDBusVariant>().variant().toBool();
        if (name.contains("Sony", Qt::CaseInsensitive) && (connected || foundName.isEmpty())) {
            foundName = name;
            foundConnected = connected;
        }
    }
    m_deviceName = foundName;
    m_connected = foundConnected;
    emit changed();
}
