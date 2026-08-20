#pragma once

#include "VendingTypes.h"

#include <functional>

class ICardReader
{
public:
    using CardTappedHandler = std::function<void(CardId)>;

    virtual ~ICardReader() = default;
    virtual void setCardTappedHandler(CardTappedHandler handler) = 0;
};
