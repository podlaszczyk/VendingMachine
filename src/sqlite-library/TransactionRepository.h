#pragma once

#include <sqlite3.h>
#include <vector>
#include <Transaction.h>

class TransactionRepository {
public:
    TransactionRepository();

    ~TransactionRepository();

    TransactionRepository(const TransactionRepository &) = delete;

    TransactionRepository &operator=(const TransactionRepository &) = delete;

    void initialize();

    void insert(const Transaction &transaction);

    void updateStatus(const std::string &transactionId, Status status);

    std::vector<Transaction> findByStatus(Status status) const;

private:
    sqlite3 *database_{nullptr};

    static const char *statusToString(Status status);

    static Status statusFromString(const char *status);
};
