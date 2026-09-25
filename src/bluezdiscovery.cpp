#include "bluezdiscovery.h"

#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusReply>
#include <QDBusVariant>
#include <QDebug>

BluezDiscovery::BluezDiscovery(QObject *parent) : QObject(parent) {
    qDBusRegisterMetaType<BluezInterfaceMap>();
    qDBusRegisterMetaType<BluezManagedObjects>();
    refresh();
}

bool BluezDiscovery::isSonyHeadphones(const QString &name) {
    const auto normalized = name.toLower();
    return normalized.contains("sony") || normalized.contains("wf-") ||
           normalized.contains("wh-") || normalized.contains("linkbuds") ||
           normalized.contains("inzone");
}

void BluezDiscovery::refresh() {
    QDBusInterface manager("org.bluez", "/", "org.freedesktop.DBus.ObjectManager",
                           QDBusConnection::systemBus());
    if (!manager.isValid()) {
        qWarning() << "BlueZ system service is unavailable:" << manager.lastError().message();
        m_deviceName.clear(); m_connected = false; emit changed(); return;
    }
    const QDBusReply<BluezManagedObjects> reply = manager.call("GetManagedObjects");
    if (!reply.isValid()) {
        qWarning() << "Unable to enumerate BlueZ devices:" << reply.error().message();
        m_deviceName.clear(); m_connected = false; emit changed(); return;
    }
    const auto objects = reply.value();
    QString foundName;
    bool foundConnected = false;
    for (auto object = objects.cbegin(); object != objects.cend(); ++object) {
        const auto props = object.value().value("org.bluez.Device1");
        const auto name = props.value("Alias").value<QDBusVariant>().variant().toString().isEmpty()
            ? props.value("Name").value<QDBusVariant>().variant().toString()
            : props.value("Alias").value<QDBusVariant>().variant().toString();
        const auto connected = props.value("Connected").value<QDBusVariant>().variant().toBool();
        if (isSonyHeadphones(name) && (connected || foundName.isEmpty())) {
            foundName = name;
            foundConnected = connected;
        }
    }
    m_deviceName = foundName;
    m_connected = foundConnected;
    emit changed();
}
