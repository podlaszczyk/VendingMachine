#include "FakeCardReader.h"

#include <utility>

void FakeCardReader::setCardTappedHandler(CardTappedHandler handler)
{
    this->handler = std::move(handler);
}

void FakeCardReader::simulateCardTap(CardId cardId) const
{
    if (handler) {
        handler(std::move(cardId));
    }
}
