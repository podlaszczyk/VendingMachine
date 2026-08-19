#include "DummyLibrary.h"

#include <iostream>

DummyLibrary::DummyLibrary() {

    sqlite3 *db = nullptr;

    auto rc = sqlite3_open(":memory:", &db);

    if (rc != SQLITE_OK) {
        std::cerr << "SQLite open failed: "
                << sqlite3_errmsg(db) << '\n';
    }

    std::cout << "SQLite works: "
            << sqlite3_libversion() << '\n';

    sqlite3_close(db);
}
