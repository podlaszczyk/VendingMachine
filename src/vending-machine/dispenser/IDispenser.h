#pragma once

#include "VendingTypes.h"

class IDispenser
{
public:
    virtual ~IDispenser() = default;

    virtual void dispense(const ProductId& productId) = 0;
};
