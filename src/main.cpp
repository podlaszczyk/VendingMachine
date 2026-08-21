#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <VendingApplicationController.h>

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    VendingApplicationController controller;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("vendingController", &controller);

    engine.loadFromModule("MyQmlApp", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
