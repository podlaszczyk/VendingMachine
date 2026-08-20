#pragma once

#include "ICardReader.h"

class FakeCardReader final : public ICardReader
{
public:
    void setCardTappedHandler(CardTappedHandler handler) override;
    void simulateCardTap(CardId cardId) const;

private:
    CardTappedHandler handler;
};
