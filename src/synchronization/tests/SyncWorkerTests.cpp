#include "FakeTransactionTransport.h"

#include <SyncWorker.h>
#include <TransactionRepository.h>

#include <catch2/catch.hpp>

#include <chrono>

namespace
{
Transaction transaction()
{
    return Transaction{20'0000'0000, "transaction-uuid", Status::Completed};
}
} // namespace

TEST_CASE("Unsynchronized transaction is posted and marked as synchronized")
{
    TransactionRepository repository;
    repository.initialize();
    repository.insert(transaction());
    FakeTransactionTransport transport;
    SyncWorker worker{repository, transport};

    const auto result = worker.runOnce(SyncWorker::Clock::time_point{});

    REQUIRE(result.attempted == 1);
    REQUIRE(result.synchronized == 1);
    REQUIRE_FALSE(result.retryAfter.has_value());
    REQUIRE(repository.findUnsynchronized().empty());
    REQUIRE(transport.requests().size() == 1);
    REQUIRE(transport.requests().front().endpoint == "/transactions");
    REQUIRE(transport.requests().front().idempotencyKey == "transaction-uuid");
    REQUIRE(transport.requests().front().jsonBody
            == R"({"id":"transaction-uuid","timestamp":2000000000,"status":"Completed"})");

    const auto nextRun = worker.runOnce(SyncWorker::Clock::time_point{} + std::chrono::seconds{1});
    REQUIRE(nextRun.attempted == 0);
    REQUIRE(transport.requests().size() == 1);
}

TEST_CASE("Unavailable backend is retried with exponential backoff and the same idempotency key")
{
    using namespace std::chrono_literals;

    TransactionRepository repository;
    repository.initialize();
    repository.insert(transaction());
    FakeTransactionTransport transport;
    transport.enqueueResult(TransportResult::RetryableFailure);
    transport.enqueueResult(TransportResult::RetryableFailure);
    transport.enqueueResult(TransportResult::Success);
    SyncWorker worker{repository, transport, 1s, 10s};
    const auto start = SyncWorker::Clock::time_point{};

    const auto firstAttempt = worker.runOnce(start);
    REQUIRE(firstAttempt.retryAfter == 1s);
    REQUIRE(transport.requests().size() == 1);

    const auto whileBackingOff = worker.runOnce(start + 500ms);
    REQUIRE(whileBackingOff.attempted == 0);
    REQUIRE(transport.requests().size() == 1);

    const auto secondAttempt = worker.runOnce(start + 1s);
    REQUIRE(secondAttempt.retryAfter == 2s);
    REQUIRE(transport.requests().size() == 2);

    const auto successfulRetry = worker.runOnce(start + 3s);
    REQUIRE(successfulRetry.synchronized == 1);
    REQUIRE(repository.findUnsynchronized().empty());
    REQUIRE(transport.requests().size() == 3);
    REQUIRE(transport.requests()[0].idempotencyKey == transport.requests()[1].idempotencyKey);
    REQUIRE(transport.requests()[1].idempotencyKey == transport.requests()[2].idempotencyKey);
}
