#pragma once

#include <Transaction.h>

#include <string>

[[nodiscard]] std::string transactionToJson(const Transaction& transaction);
