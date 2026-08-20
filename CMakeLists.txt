cmake_minimum_required(VERSION 3.27)
project(VendingMachine)

include(CTest)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_COVERAGE "Enable code coverage instrumentation" OFF)

add_library(coverage-options INTERFACE)
add_library(Project::Coverage ALIAS coverage-options)

if (ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(coverage-options INTERFACE
            --coverage
            -O0
            -g
    )
    target_link_options(coverage-options INTERFACE --coverage)
endif()

add_subdirectory(src)


if (ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    find_program(GCOVR_EXECUTABLE gcovr REQUIRED)

    set(COVERAGE_REPORT_DIRECTORY ${CMAKE_BINARY_DIR}/coverage)

    add_custom_target(coverage
            COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_REPORT_DIRECTORY}
            COMMAND ${GCOVR_EXECUTABLE}
                    --root ${CMAKE_SOURCE_DIR}
                    --object-directory ${CMAKE_BINARY_DIR}
                    --filter ${CMAKE_SOURCE_DIR}/src
                    --exclude ".*/tests/.*"
                    --html-details ${COVERAGE_REPORT_DIRECTORY}/index.html
                    --print-summary
            DEPENDS all-tests
            USES_TERMINAL
            COMMENT "Generating code coverage report"
    )
endif()
