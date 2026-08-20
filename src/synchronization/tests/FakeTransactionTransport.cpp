#include "FakeTransactionTransport.h"

TransportResult FakeTransactionTransport::postTransaction(const PostTransactionRequest& request)
{
    receivedRequests.push_back(request);
    if (results.empty()) {
        return TransportResult::Success;
    }

    const auto result = results.front();
    results.pop_front();
    return result;
}

void FakeTransactionTransport::enqueueResult(TransportResult result)
{
    results.push_back(result);
}

const std::vector<PostTransactionRequest>& FakeTransactionTransport::requests() const
{
    return receivedRequests;
}
