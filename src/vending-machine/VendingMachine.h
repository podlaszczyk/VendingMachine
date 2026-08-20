#pragma once

#include "IDispenser.h"
#include "VendingTypes.h"

#include <optional>

class VendingMachine
{
public:
    explicit VendingMachine(IDispenser& dispenser);

    EventResult onCardTapped(CardId cardId);
    EventResult onProductSelected(ProductId productId);
    EventResult onDispenseResult(DispenseResult result);
    EventResult onSelectionTimeout();

    EventResult reset();

    [[nodiscard]] VendingState getState() const;
    [[nodiscard]] const std::optional<CardId>& getCardId() const;
    [[nodiscard]] const std::optional<ProductId>& getProductId() const;

private:
    void clearTransaction();

    IDispenser& dispenser;
    VendingState state{VendingState::Idle};
    std::optional<CardId> cardId;
    std::optional<ProductId> productId;
};
