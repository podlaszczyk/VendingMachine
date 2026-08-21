#include <TransactionRepository.h>

#include <catch2/catch.hpp>

#include <chrono>
#include <filesystem>

namespace
{
Transaction createTransaction()
{
    return Transaction{111'1111, "random-id", "product-1", "card-1", Status::Dispensing};
}
} // namespace

namespace
{
std::filesystem::path temporaryDatabasePath()
{
    return std::filesystem::temp_directory_path()
         / ("vending-machine-transactions-"
            + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".sqlite");
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
    REQUIRE(rows[0].productId == transaction.productId);
    REQUIRE(rows[0].cardId == transaction.cardId);
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

TEST_CASE("Status update makes a synchronized transaction pending again")
{
    TransactionRepository repository;
    repository.initialize();
    const auto transaction = createTransaction();
    repository.insert(transaction);
    repository.markSynchronized(transaction.id);
    REQUIRE(repository.findUnsynchronized().empty());

    repository.updateStatus(transaction.id, Status::Completed);

    const auto pending = repository.findUnsynchronized();
    REQUIRE(pending.size() == 1);
    REQUIRE(pending.front().id == transaction.id);
    REQUIRE(pending.front().status == Status::Completed);
}

TEST_CASE("Inserted transactions start unsynchronized")
{
    TransactionRepository repository;
    repository.initialize();
    repository.insert(createTransaction());

    const auto pending = repository.findUnsynchronized();

    REQUIRE(pending.size() == 1);
    REQUIRE(pending.front().id == "random-id");
}

TEST_CASE("Transactions can be marked synchronized independently")
{
    TransactionRepository repository;
    repository.initialize();
    repository.insert(Transaction{2, "second", "product-2", "card-2", Status::Completed});
    repository.insert(Transaction{1, "first", "product-1", "card-1", Status::Failed});

    repository.markSynchronized("first");

    const auto pending = repository.findUnsynchronized();
    REQUIRE(pending.size() == 1);
    REQUIRE(pending.front().id == "second");
    REQUIRE(pending.front().status == Status::Completed);
}

TEST_CASE("Unsynchronized transactions are returned in timestamp order")
{
    TransactionRepository repository;
    repository.initialize();
    repository.insert(Transaction{30, "third", "product-3", "card-3", Status::Failed});
    repository.insert(Transaction{10, "second", "product-2", "card-2", Status::Dispensing});
    repository.insert(Transaction{20, "first", "product-1", "card-1", Status::Completed});

    const auto pending = repository.findUnsynchronized();

    REQUIRE(pending.size() == 3);
    REQUIRE(pending[0].id == "second");
    REQUIRE(pending[1].id == "first");
    REQUIRE(pending[2].id == "third");
}

TEST_CASE("Database can be reopened without losing transactions")
{
    const auto path = temporaryDatabasePath();
    const auto transaction = createTransaction();

    {
        TransactionRepository repository(path.string());
        repository.initialize();
        repository.insert(transaction);
    }

    {
        TransactionRepository repository(path.string());
        repository.initialize();
        const auto rows = repository.findByStatus(Status::Dispensing);
        REQUIRE(rows.size() == 1);
        REQUIRE(rows.front().id == transaction.id);
        REQUIRE(rows.front().timestamp == transaction.timestamp);
        REQUIRE(rows.front().productId == transaction.productId);
        REQUIRE(rows.front().cardId == transaction.cardId);
    }

    std::filesystem::remove(path);
}

TEST_CASE("Initializing an already initialized database is harmless")
{
    TransactionRepository repository;

    REQUIRE_NOTHROW(repository.initialize());
    REQUIRE_NOTHROW(repository.initialize());
}

TEST_CASE("Duplicate transaction IDs are rejected")
{
    TransactionRepository repository;
    repository.initialize();
    repository.insert(createTransaction());

    REQUIRE_THROWS(repository.insert(Transaction{222, "random-id", "product-2", "card-2", Status::Completed}));
}
