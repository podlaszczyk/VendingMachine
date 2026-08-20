#include "SyncWorker.h"

#include "TransactionJson.h"

#include <algorithm>
#include <stdexcept>

SyncWorker::SyncWorker(TransactionRepository& repository,
                       ITransactionTransport& transport,
                       Duration initialBackoff,
                       Duration maximumBackoff)
    : repository(repository)
    , transport(transport)
    , initialBackoff(initialBackoff)
    , maximumBackoff(maximumBackoff)
    , currentBackoff(initialBackoff)
{
    if (initialBackoff <= Duration::zero() || maximumBackoff < initialBackoff) {
        throw std::invalid_argument("Invalid synchronization backoff");
    }
}

SyncRunResult SyncWorker::runOnce(Clock::time_point now)
{
    SyncRunResult result;

    if (nextAttempt && now < *nextAttempt) {
        result.retryAfter = std::chrono::duration_cast<Duration>(*nextAttempt - now);
        return result;
    }

    for (const auto& transaction : repository.findUnsynchronized()) {
        ++result.attempted;
        const PostTransactionRequest request{
            "/transactions",
            transactionToJson(transaction),
            transaction.id,
        };

        if (transport.postTransaction(request) == TransportResult::Success) {
            repository.markSynchronized(transaction.id);
            ++result.synchronized;
            currentBackoff = initialBackoff;
            nextAttempt.reset();
            continue;
        }

        nextAttempt = now + currentBackoff;
        result.retryAfter = currentBackoff;
        currentBackoff = std::min(currentBackoff * 2, maximumBackoff);
        return result;
    }

    currentBackoff = initialBackoff;
    nextAttempt.reset();
    return result;
}
