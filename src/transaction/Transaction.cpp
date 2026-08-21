#include "Transaction.h"

#include <array>
#include <random>
#include <utility>

namespace
{
std::string generateUuid()
{
    std::random_device randomDevice;
    std::array<unsigned char, 16> bytes{};
    for (auto& byte : bytes) {
        byte = static_cast<unsigned char>(randomDevice());
    }

    bytes[6] = static_cast<unsigned char>((bytes[6] & 0x0F) | 0x40);
    bytes[8] = static_cast<unsigned char>((bytes[8] & 0x3F) | 0x80);

    constexpr char hex[] = "0123456789abcdef";
    std::string uuid;
    uuid.reserve(36);
    for (std::size_t index = 0; index < bytes.size(); ++index) {
        if (index == 4 || index == 6 || index == 8 || index == 10) {
            uuid.push_back('-');
        }
        uuid.push_back(hex[bytes[index] >> 4]);
        uuid.push_back(hex[bytes[index] & 0x0F]);
    }
    return uuid;
}
} // namespace

Transaction
    Transaction::create(const std::int64_t timestamp, std::string productId, std::string cardId, const Status status)
{
    return Transaction{timestamp, generateUuid(), std::move(productId), std::move(cardId), status};
}
