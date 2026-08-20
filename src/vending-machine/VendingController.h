#pragma once

#include "ICardReader.h"

class VendingMachine;

class VendingController
{
public:
    VendingController(ICardReader& cardReader, VendingMachine& vendingMachine);
    ~VendingController();

    VendingController(const VendingController&) = delete;
    VendingController& operator=(const VendingController&) = delete;

private:
    ICardReader& cardReader;
};
