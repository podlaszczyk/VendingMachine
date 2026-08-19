#include <TransactionRepository.h>

#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    // DummyLibrary library;

    TransactionRepository transactionRepository("path");

    QApplication app(argc, argv);
    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
