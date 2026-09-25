#include "bluezdiscovery.h"

#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusReply>
#include <QDBusVariant>
#include <QDebug>

namespace {
QVariant bluezProperty(const QVariantMap &properties, const QString &key) {
    const auto value = properties.value(key);
    // Qt may either preserve a D-Bus variant wrapper or unwrap a{sv} values
    // directly into QVariant, depending on the registered container type.
    if (value.metaType().id() == qMetaTypeId<QDBusVariant>())
        return value.value<QDBusVariant>().variant();
    return value;
}
}

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
    qInfo() << "Refreshing BlueZ device list";
    QDBusInterface manager("org.bluez", "/", "org.freedesktop.DBus.ObjectManager",
                           QDBusConnection::systemBus());
    if (!manager.isValid()) {
        qWarning() << "BlueZ system service is unavailable:" << manager.lastError().message();
        m_deviceName.clear(); m_address.clear(); m_connected = false; emit changed(); return;
    }
    const QDBusReply<BluezManagedObjects> reply = manager.call("GetManagedObjects");
    if (!reply.isValid()) {
        qWarning() << "Unable to enumerate BlueZ devices:" << reply.error().message();
        m_deviceName.clear(); m_address.clear(); m_connected = false; emit changed(); return;
    }
    const auto objects = reply.value();
    qInfo() << "BlueZ managed objects:" << objects.size();
    QString foundName;
    QString foundAddress;
    bool foundConnected = false;
    for (auto object = objects.cbegin(); object != objects.cend(); ++object) {
        const auto props = object.value().value("org.bluez.Device1");
        const auto alias = bluezProperty(props, "Alias").toString();
        const auto reportedName = bluezProperty(props, "Name").toString();
        const auto name = alias.isEmpty() ? reportedName : alias;
        const auto connected = bluezProperty(props, "Connected").toBool();
        const auto paired = bluezProperty(props, "Paired").toBool();
        const auto address = bluezProperty(props, "Address").toString();
        if (!props.isEmpty()) {
            qInfo().noquote() << QString("BlueZ device %1: alias='%2', name='%3', paired=%4, connected=%5, Sony-match=%6")
                .arg(object.key().path(),
                     alias, reportedName)
                .arg(paired ? "true" : "false")
                .arg(connected ? "true" : "false")
                .arg(isSonyHeadphones(name) ? "true" : "false");
        }
        if (isSonyHeadphones(name) && (connected || foundName.isEmpty())) {
            foundName = name;
            foundAddress = address;
            foundConnected = connected;
        }
    }
    m_deviceName = foundName;
    m_address = foundAddress;
    m_connected = foundConnected;
    qInfo() << "Selected Sony device:" << m_deviceName << "connected:" << m_connected;
    emit changed();
}
