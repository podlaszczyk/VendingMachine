#pragma once

#include <cstdint>
#include <string>

enum class Status
{
    Dispensing,
    Completed,
    Failed,
};

struct Transaction
{
    static Transaction create(std::int64_t timestamp,
                              std::string productId,
                              std::string cardId,
                              Status status);

    std::int64_t timestamp;
    std::string id;
    std::string productId;
    std::string cardId;
    Status status;
};
