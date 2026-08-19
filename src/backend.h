#pragma once

#include <QObject>

class Backend : public QObject
{
    Q_OBJECT

public:
    explicit Backend(QObject *parent = nullptr);

    Q_INVOKABLE void sayHello();

    signals:
        void messageChanged(const QString &message);
};