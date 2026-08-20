#include "TransactionJson.h"

#include <stdexcept>

namespace
{
std::string escapeJson(const std::string& input)
{
    std::string escaped;
    escaped.reserve(input.size());

    for (const char character : input) {
        switch (character) {
            case '"':
                escaped += "\\\"";
                break;
            case '\\':
                escaped += "\\\\";
                break;
            case '\n':
                escaped += "\\n";
                break;
            case '\r':
                escaped += "\\r";
                break;
            case '\t':
                escaped += "\\t";
                break;
            default:
                escaped += character;
        }
    }

    return escaped;
}

const char* statusToString(Status status)
{
    switch (status) {
        case Status::Dispensing:
            return "Dispensing";
        case Status::Completed:
            return "Completed";
        case Status::Failed:
            return "Failed";
    }

    throw std::invalid_argument("Unknown transaction status");
}
} // namespace

// could be changed to qjon or nlohmanjson
std::string transactionToJson(const Transaction& transaction)
{
    return "{\"id\":\"" + escapeJson(transaction.id) + "\",\"timestamp\":" + std::to_string(transaction.timestamp)
         + ",\"status\":\"" + statusToString(transaction.status) + "\"}";
}
