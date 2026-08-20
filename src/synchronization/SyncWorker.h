#pragma once

#include "ITransactionTransport.h"

#include <TransactionRepository.h>

#include <chrono>
#include <cstddef>
#include <optional>

struct SyncRunResult
{
    std::size_t attempted{0};
    std::size_t synchronized{0};
    std::optional<std::chrono::milliseconds> retryAfter;
};

class SyncWorker
{
public:
    using Clock = std::chrono::steady_clock;
    using Duration = std::chrono::milliseconds;

    SyncWorker(TransactionRepository& repository,
               ITransactionTransport& transport,
               Duration initialBackoff = std::chrono::seconds{1},
               Duration maximumBackoff = std::chrono::minutes{1});

    [[nodiscard]] SyncRunResult runOnce(Clock::time_point now);

private:
    TransactionRepository& repository;
    ITransactionTransport& transport;
    Duration initialBackoff;
    Duration maximumBackoff;
    Duration currentBackoff;
    std::optional<Clock::time_point> nextAttempt;
};
