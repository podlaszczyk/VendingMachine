#pragma once

#include <string>

struct PostTransactionRequest
{
    std::string endpoint;
    std::string jsonBody;
    std::string idempotencyKey;
};

enum class TransportResult
{
    Success,
    RetryableFailure,
};

class ITransactionTransport
{
public:
    virtual ~ITransactionTransport() = default;
    virtual TransportResult postTransaction(const PostTransactionRequest& request) = 0;
};
