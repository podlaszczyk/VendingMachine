find_package(Catch2 REQUIRED)

set(TEST_NAME VendingMachineCoreTests)

add_executable(${TEST_NAME}
        VendingMachineTests.cpp
        FakeDispenser.cpp
        FakeCardReader.cpp
)

target_link_libraries(${TEST_NAME} PRIVATE
        VendingMachine::Core
        Hardware::CardReader
        Catch2::Catch2WithMain
)

add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
set_tests_properties(${TEST_NAME} PROPERTIES LABELS "Unit Test")
