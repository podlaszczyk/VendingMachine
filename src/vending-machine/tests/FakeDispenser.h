#pragma once

#include "IDispenser.h"

#include <optional>

class FakeDispenser : public IDispenser
{
public:
    void dispense(const ProductId& productId) override;

    int dispenseCalls{0};
    std::optional<ProductId> lastProduct;
};
