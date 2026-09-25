#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "devicecontroller.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName("Sony Headphones");
    app.setOrganizationName("Chris Jahner");
    QQuickStyle::setStyle("Basic");

    DeviceController device;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("device", &device);
    engine.loadFromModule("Sony.Headphones", "Main");
    if (engine.rootObjects().isEmpty()) return 1;
    return app.exec();
}
