#include "TransactionRepository.h"

#include <iostream>
#include <filesystem>
#include <stdexcept>

TransactionRepository::~TransactionRepository() {
    if (database_) {
        sqlite3_close(database_);
    }
}

TransactionRepository::TransactionRepository() {
    const int result = sqlite3_open(":memory:", &database_);

    if (result != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(database_) << '\n';

        if (database_) {
            sqlite3_close(database_);
            database_ = nullptr;
        }
        throw std::runtime_error("Failed to open database: ");
    }
}

void TransactionRepository::initialize() {
    constexpr const char *sql = R"(
        CREATE TABLE IF NOT EXISTS transactions (
            id TEXT PRIMARY KEY NOT NULL,
            timestamp INTEGER NOT NULL,
            status TEXT NOT NULL
                CHECK(status IN (
                    'Dispensing',
                    'Completed',
                    'Failed'
                ))
        );
    )";

    char *errorMessage = nullptr;

    const int result = sqlite3_exec(database_, sql, nullptr, nullptr, &errorMessage);

    if (result != SQLITE_OK) {
        const std::string error = errorMessage ? errorMessage : "Unknown SQLite error";
        sqlite3_free(errorMessage);
        throw std::runtime_error(
            "Failed to initialize database: " + error);
    }
    std::cout << "Table created\n";
}

const char *TransactionRepository::statusToString(Status status) {
    switch (status) {
        case Status::Dispensing:
            return "Dispensing";

        case Status::Completed:
            return "Completed";

        case Status::Failed:
            return "Failed";
    }

    throw std::invalid_argument(
        "Unknown transaction status");
}

Status TransactionRepository::statusFromString(const char *status) {
    if (std::string(status) == "Dispensing") {
        return Status::Dispensing;
    }

    if (std::string(status) == "Completed") {
        return Status::Completed;
    }

    if (std::string(status) == "Failed") {
        return Status::Failed;
    }

    throw std::runtime_error(
        "Unknown transaction status in database");
}

void TransactionRepository::insert(const Transaction &transaction) {
    constexpr const char *sql = R"(
        INSERT INTO transactions(id, timestamp, status) VALUES (?, ?, ?);
    )";

    sqlite3_stmt *statement = nullptr;

    int result = sqlite3_prepare_v2(database_, sql, -1, &statement, nullptr);

    if (result != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(database_));
    }

    sqlite3_bind_text(statement, 1, transaction.id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(statement, 2, transaction.timestamp);
    sqlite3_bind_text(statement, 3, statusToString(transaction.status), -1, SQLITE_TRANSIENT);

    result = sqlite3_step(statement);

    sqlite3_finalize(statement);

    if (result != SQLITE_DONE) {
        throw std::runtime_error(sqlite3_errmsg(database_));
    }
}


void TransactionRepository::updateStatus(const std::string &transactionId, Status status) {
    constexpr const char *sql = R"(
        UPDATE transactions
        SET status = ?
        WHERE id = ?;
    )";

    sqlite3_stmt *statement = nullptr;

    int result = sqlite3_prepare_v2(database_, sql, -1, &statement, nullptr);

    if (result != SQLITE_OK) {
        throw std::runtime_error(
            sqlite3_errmsg(database_));
    }

    sqlite3_bind_text(statement, 1, statusToString(status), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, transactionId.c_str(), -1, SQLITE_TRANSIENT);
    result = sqlite3_step(statement);

    sqlite3_finalize(statement);

    if (result != SQLITE_DONE) {
        throw std::runtime_error(sqlite3_errmsg(database_));
    }
}

std::vector<Transaction> TransactionRepository::findByStatus(Status status) const {
    constexpr const char *sql = R"(
        SELECT
            id,
            timestamp,
            status
        FROM transactions
        WHERE status = ?;
    )";

    sqlite3_stmt *statement = nullptr;

    int result = sqlite3_prepare_v2(database_, sql, -1, &statement, nullptr);

    if (result != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(database_));
    }

    sqlite3_bind_text(statement, 1, statusToString(status), -1, SQLITE_TRANSIENT);

    std::vector<Transaction> transactions;

    while ((result = sqlite3_step(statement)) == SQLITE_ROW) {
        Transaction transaction;

        transaction.id = reinterpret_cast<const char *>(sqlite3_column_text(statement, 0));
        transaction.timestamp = sqlite3_column_int64(statement, 1);
        transaction.status = statusFromString(reinterpret_cast<const char *>(sqlite3_column_text(statement, 2)));
        transactions.push_back(std::move(transaction));
    }

    sqlite3_finalize(statement);

    if (result != SQLITE_DONE) {
        throw std::runtime_error(sqlite3_errmsg(database_));
    }

    return transactions;
}
