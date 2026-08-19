find_package(Catch2 REQUIRED)

set(TEST_NAME TransactionTests)

add_executable(${TEST_NAME}
        TransactionRepositoryTests.cpp)


target_link_libraries(${TEST_NAME} PRIVATE
        Device::sqlite-library
        Catch2::Catch2WithMain
)

add_test(NAME ${TEST_NAME}
        COMMAND ${TEST_NAME} -d yes --order lex -v high --reporter JUnit::out=${TEST_REPORT_DIRECTORY}/${TEST_NAME}.xml --reporter console::out=-::colour-mode=ansi
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

set_tests_properties(${TEST_NAME} PROPERTIES LABELS "Unit Test")