#include <Transaction.h>

#include <catch2/catch.hpp>

#include <set>

TEST_CASE("Device-generated transaction IDs are UUID v4 values and unique")
{
    std::set<std::string> ids;

    for (int index = 0; index < 100; ++index) {
        const auto transaction = Transaction::create(index, "product-1", "card-1", Status::Dispensing);
        REQUIRE(transaction.id.size() == 36);
        REQUIRE(transaction.id[14] == '4');
        REQUIRE((transaction.id[19] == '8' || transaction.id[19] == '9' || transaction.id[19] == 'a'
                 || transaction.id[19] == 'b'));
        REQUIRE(ids.insert(transaction.id).second);
    }
}
