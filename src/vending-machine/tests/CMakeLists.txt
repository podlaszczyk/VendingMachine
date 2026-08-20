find_package(Catch2 REQUIRED)

set(TEST_CASES_TARGET vending-machine-test-cases)
add_library(${TEST_CASES_TARGET} INTERFACE)
target_sources(${TEST_CASES_TARGET} INTERFACE
        VendingMachineTests.cpp
        FakeDispenser.cpp
        FakeCardReader.cpp
)
add_library(Tests::VendingMachineCases ALIAS ${TEST_CASES_TARGET})

target_link_libraries(${TEST_CASES_TARGET} INTERFACE
        VendingMachine::Core
        Hardware::CardReader
        Catch2::Catch2
        Project::Coverage
)

set(TEST_NAME VendingMachineCoreTests)

add_executable(${TEST_NAME})

target_link_libraries(${TEST_NAME} PRIVATE
        Tests::VendingMachineCases
        Catch2::Catch2WithMain
)

add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
set_tests_properties(${TEST_NAME} PROPERTIES LABELS "Unit Test")
