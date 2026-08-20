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
    std::int64_t timestamp;
    std::string id;
    Status status;
};
