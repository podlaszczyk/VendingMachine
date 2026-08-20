#pragma once

#include <sqlite3.h>
#include <Transaction.h>

#include <vector>

class TransactionRepository
{
public:
    TransactionRepository();
    ~TransactionRepository();

    TransactionRepository(const TransactionRepository&) = delete;
    TransactionRepository& operator=(const TransactionRepository&) = delete;

    void initialize();
    void insert(const Transaction& transaction);
    void updateStatus(const std::string& transactionId, Status status);
    std::vector<Transaction> findByStatus(Status status) const;

private:
    sqlite3* database_{nullptr};
    const static char* statusToString(Status status);
    static Status statusFromString(const char* status);
};
