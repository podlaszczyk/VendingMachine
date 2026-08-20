#include "FakeCardReader.h"
#include "FakeDispenser.h"

#include <VendingController.h>
#include <VendingMachine.h>

#include <catch2/catch.hpp>

TEST_CASE("RFID card tap is forwarded to the vending machine")
{
    FakeCardReader cardReader;
    FakeDispenser dispenser;
    VendingMachine machine{dispenser};
    VendingController controller{cardReader, machine};

    cardReader.simulateCardTap(CardId{"card-1"});

    REQUIRE(machine.getState() == VendingState::CardRead);
    REQUIRE(machine.getCardId() == CardId{"card-1"});
}

TEST_CASE("Card and product selection start dispensing")
{
    FakeDispenser dispenser;
    VendingMachine machine{dispenser};

    REQUIRE(machine.onCardTapped(CardId{"card-1"}) == EventResult::Accepted);
    REQUIRE(machine.getState() == VendingState::CardRead);

    REQUIRE(machine.onProductSelected(ProductId{"product-1"}) == EventResult::Accepted);
    REQUIRE(machine.getState() == VendingState::Dispensing);
    REQUIRE(dispenser.dispenseCalls == 1);
    REQUIRE(dispenser.lastProduct == ProductId{"product-1"});
}

TEST_CASE("Dispense result completes or fails a transaction")
{
    FakeDispenser dispenser;
    VendingMachine machine{dispenser};
    machine.onCardTapped(CardId{"card-1"});
    machine.onProductSelected(ProductId{"product-1"});

    SECTION("success")
    {
        REQUIRE(machine.onDispenseResult(DispenseResult::Success) == EventResult::Accepted);
        REQUIRE(machine.getState() == VendingState::Completed);
    }

    SECTION("failure")
    {
        REQUIRE(machine.onDispenseResult(DispenseResult::Failure) == EventResult::Accepted);
        REQUIRE(machine.getState() == VendingState::Failed);
    }
}

TEST_CASE("Selection timeout returns to idle and clears context")
{
    FakeDispenser dispenser;
    VendingMachine machine{dispenser};
    machine.onCardTapped(CardId{"card-1"});

    REQUIRE(machine.onSelectionTimeout() == EventResult::Accepted);
    REQUIRE(machine.getState() == VendingState::Idle);
    REQUIRE_FALSE(machine.getProductId().has_value());
}

TEST_CASE("Another card cannot start a transaction while dispensing")
{
    FakeDispenser dispenser;
    VendingMachine machine{dispenser};
    machine.onCardTapped(CardId{"card-1"});
    machine.onProductSelected(ProductId{"product-1"});

    REQUIRE(machine.onCardTapped(CardId{"card-2"}) == EventResult::Ignored);
    REQUIRE(machine.getState() == VendingState::Dispensing);
    REQUIRE(machine.getCardId() == CardId{"card-1"});
    REQUIRE(dispenser.dispenseCalls == 1);
}

TEST_CASE("Events invalid for the current state are ignored")
{
    FakeDispenser dispenser;
    VendingMachine machine{dispenser};

    REQUIRE(machine.onProductSelected(ProductId{"product-1"}) == EventResult::Ignored);
    REQUIRE(machine.onDispenseResult(DispenseResult::Success) == EventResult::Ignored);
    REQUIRE(machine.onSelectionTimeout() == EventResult::Ignored);
    REQUIRE(machine.getState() == VendingState::Idle);
    REQUIRE(dispenser.dispenseCalls == 0);
}

TEST_CASE("Terminal state can be reset for the next customer")
{
    FakeDispenser dispenser;
    VendingMachine machine{dispenser};
    machine.onCardTapped(CardId{"card-1"});
    machine.onProductSelected(ProductId{"product-1"});
    machine.onDispenseResult(DispenseResult::Success);

    REQUIRE(machine.reset() == EventResult::Accepted);
    REQUIRE(machine.getState() == VendingState::Idle);
    REQUIRE_FALSE(machine.getCardId().has_value());
    REQUIRE_FALSE(machine.getProductId().has_value());
}
