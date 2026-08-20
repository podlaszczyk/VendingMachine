#pragma once

#include <string>

struct CardId
{
    std::string value;

    bool operator==(const CardId&) const = default;
};

struct ProductId
{
    std::string value;

    bool operator==(const ProductId&) const = default;
};

enum class VendingState
{
    Idle,
    CardRead,
    ProductSelected,
    Dispensing,
    Completed,
    Failed,
};

enum class DispenseResult
{
    Success,
    Failure,
};

enum class EventResult
{
    Accepted,
    Ignored,
};
