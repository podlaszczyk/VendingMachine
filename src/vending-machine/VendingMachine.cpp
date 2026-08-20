#include "VendingMachine.h"

#include <utility>

VendingMachine::VendingMachine(IDispenser& dispenser)
    : dispenser(dispenser)
{}

EventResult VendingMachine::onCardTapped(CardId cardId)
{
    if (state != VendingState::Idle) {
        return EventResult::Ignored;
    }

    this->cardId = std::move(cardId);
    state = VendingState::CardRead;
    return EventResult::Accepted;
}

EventResult VendingMachine::onProductSelected(ProductId productId)
{
    if (state != VendingState::CardRead) {
        return EventResult::Ignored;
    }

    this->productId = std::move(productId);
    state = VendingState::ProductSelected;

    // Set the state before calling external code. This also handles a fake or
    // driver which reports its result synchronously from dispense().
    state = VendingState::Dispensing;
    dispenser.dispense(*this->productId);
    return EventResult::Accepted;
}

EventResult VendingMachine::onDispenseResult(DispenseResult result)
{
    if (state != VendingState::Dispensing) {
        return EventResult::Ignored;
    }

    state = result == DispenseResult::Success ? VendingState::Completed : VendingState::Failed;
    return EventResult::Accepted;
}

EventResult VendingMachine::onSelectionTimeout()
{
    if (state != VendingState::CardRead) {
        return EventResult::Ignored;
    }

    clearTransaction();
    state = VendingState::Idle;
    return EventResult::Accepted;
}

EventResult VendingMachine::reset()
{
    if (state != VendingState::Completed && state != VendingState::Failed) {
        return EventResult::Ignored;
    }

    clearTransaction();
    state = VendingState::Idle;
    return EventResult::Accepted;
}

VendingState VendingMachine::getState() const
{
    return state;
}

const std::optional<CardId>& VendingMachine::getCardId() const
{
    return cardId;
}

const std::optional<ProductId>& VendingMachine::getProductId() const
{
    return productId;
}

void VendingMachine::clearTransaction()
{
    cardId.reset();
    productId.reset();
}
