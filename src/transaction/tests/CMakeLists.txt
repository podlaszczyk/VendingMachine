find_package(Catch2 REQUIRED)

set(TEST_CASES_TARGET transaction-test-cases)
add_library(${TEST_CASES_TARGET} INTERFACE)
target_sources(${TEST_CASES_TARGET} INTERFACE
        TransactionTests.cpp
)
add_library(Tests::TransactionCases ALIAS ${TEST_CASES_TARGET})

target_link_libraries(${TEST_CASES_TARGET} INTERFACE
        Model::Transaction
        Catch2::Catch2
        Project::Coverage
)

set(TEST_NAME TransactionTests)

add_executable(${TEST_NAME})

target_link_libraries(${TEST_NAME} PRIVATE
        Tests::TransactionCases
        Catch2::Catch2WithMain
)

add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

set_tests_properties(${TEST_NAME} PROPERTIES LABELS "Unit Test")
