#include <DummyLibrary.h>

#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    DummyLibrary library;

    QApplication app(argc, argv);
    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
