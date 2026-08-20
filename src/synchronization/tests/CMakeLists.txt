find_package(Catch2 REQUIRED)

set(TEST_CASES_TARGET synchronization-test-cases)
add_library(${TEST_CASES_TARGET} INTERFACE)
target_sources(${TEST_CASES_TARGET} INTERFACE
        SyncWorkerTests.cpp
        FakeTransactionTransport.cpp
)
add_library(Tests::SynchronizationCases ALIAS ${TEST_CASES_TARGET})

target_link_libraries(${TEST_CASES_TARGET} INTERFACE
        Sync::Worker
        Device::sqlite-library
        Catch2::Catch2
        Project::Coverage
)

set(TEST_NAME SynchronizationTests)
add_executable(${TEST_NAME})
target_link_libraries(${TEST_NAME} PRIVATE
        Tests::SynchronizationCases
        Catch2::Catch2WithMain
)

add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
set_tests_properties(${TEST_NAME} PROPERTIES LABELS "Unit Test")
