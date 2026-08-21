#pragma once

#include <sqlite3.h>
#include <Transaction.h>

#include <string>
#include <vector>

class TransactionRepository
{
public:
    explicit TransactionRepository(const std::string& databasePath = ":memory:");
    ~TransactionRepository();

    TransactionRepository(const TransactionRepository&) = delete;
    TransactionRepository& operator=(const TransactionRepository&) = delete;

    void initialize();
    void insert(const Transaction& transaction);
    void updateStatus(const std::string& transactionId, Status status);
    std::vector<Transaction> findByStatus(Status status) const;
    [[nodiscard]] std::vector<Transaction> findUnsynchronized() const;
    void markSynchronized(const std::string& transactionId);

private:
    sqlite3* database{nullptr};
    const static char* statusToString(Status status);
    static Status statusFromString(const char* status);
};
