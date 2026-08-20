#include "VendingController.h"

#include "VendingMachine.h"

#include <utility>

VendingController::VendingController(ICardReader& cardReader, VendingMachine& vendingMachine)
    : cardReader(cardReader)
{
    cardReader.setCardTappedHandler([&vendingMachine](CardId cardId) {
        vendingMachine.onCardTapped(std::move(cardId));
    });
}

VendingController::~VendingController()
{
    cardReader.setCardTappedHandler({});
}
