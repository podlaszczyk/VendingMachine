#pragma once

#include "ITransactionTransport.h"

#include <deque>
#include <vector>

class FakeTransactionTransport final : public ITransactionTransport
{
public:
    TransportResult postTransaction(const PostTransactionRequest& request) override;

    void enqueueResult(TransportResult result);
    [[nodiscard]] const std::vector<PostTransactionRequest>& requests() const;

private:
    std::deque<TransportResult> results;
    std::vector<PostTransactionRequest> receivedRequests;
};
