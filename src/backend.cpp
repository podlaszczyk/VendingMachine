#include "backend.h"

Backend::Backend(QObject *parent)
    : QObject(parent)
{
}

void Backend::sayHello()
{
    emit messageChanged("Hello from C++!");
}