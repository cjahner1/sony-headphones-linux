#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QUrl>

#include "devicecontroller.h"
#include "logging.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName("Sony Headphones");
    app.setOrganizationName("Chris Jahner");
    installFileLogger();
    QQuickStyle::setStyle("Basic");

    DeviceController device;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("device", &device);
    // loadFromModule() arrived after Qt 6.4. Load the qrc resource directly so
    // the app works with the Qt version supplied by current Arch and Ubuntu.
    engine.load(QUrl(QStringLiteral("qrc:/Sony/Headphones/qml/Main.qml")));
    if (engine.rootObjects().isEmpty()) return 1;
    return app.exec();
}
