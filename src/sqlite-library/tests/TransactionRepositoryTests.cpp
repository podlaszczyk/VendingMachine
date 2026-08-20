#include <TransactionRepository.h>

#include <catch2/catch.hpp>

#include <filesystem>

namespace
{
Transaction createTransaction()
{
    return Transaction{111'1111, "random-id", Status::Dispensing};
}
} // namespace

TEST_CASE("Datebase does not throw when created")
{
    REQUIRE_NOTHROW(TransactionRepository());
}

TEST_CASE("Empty Datebase does not have any records")
{
    TransactionRepository repository;

    repository.initialize();

    const auto rowsDispensing = repository.findByStatus(Status::Dispensing);
    const auto rowsCompleted = repository.findByStatus(Status::Completed);
    const auto rowsFailed = repository.findByStatus(Status::Failed);

    REQUIRE(rowsDispensing.size() == 0);
    REQUIRE(rowsCompleted.size() == 0);
    REQUIRE(rowsFailed.size() == 0);
}

TEST_CASE("TransactionRepository opens database inserts one dummy row and selects it")
{
    // GIVEN
    TransactionRepository repository;

    repository.initialize();

    const auto transaction = createTransaction();
    repository.insert(transaction);

    const auto rows = repository.findByStatus(Status::Dispensing);

    REQUIRE(rows.size() == 1);
    REQUIRE(rows[0].timestamp == transaction.timestamp);
    REQUIRE(rows[0].id == transaction.id);
    REQUIRE(rows[0].status == transaction.status);
}

TEST_CASE("Updates status of transaction")
{
    // GIVEN
    TransactionRepository repository;

    repository.initialize();

    const auto transaction = createTransaction();
    repository.insert(transaction);

    const auto rowsDispensingAtStart = repository.findByStatus(Status::Dispensing);
    REQUIRE(rowsDispensingAtStart.size() == 1);

    repository.updateStatus(transaction.id, Status::Completed);

    const auto rowsDispensing = repository.findByStatus(Status::Dispensing);
    REQUIRE(rowsDispensing.size() == 0);

    const auto rowsCompleted = repository.findByStatus(Status::Completed);
    REQUIRE(rowsCompleted.size() == 1);
    REQUIRE(rowsCompleted[0].id == transaction.id);
    REQUIRE(rowsCompleted[0].status == Status::Completed);
}
