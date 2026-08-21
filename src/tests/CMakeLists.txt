find_package(Catch2 REQUIRED)

set(TEST_NAME AllTests)

add_executable(${TEST_NAME})

target_link_libraries(${TEST_NAME} PRIVATE
        Tests::TransactionRepositoryCases
        Tests::TransactionCases
        Tests::VendingMachineCases
        Tests::SynchronizationCases
        Catch2::Catch2WithMain
)
