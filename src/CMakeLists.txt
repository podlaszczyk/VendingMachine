add_subdirectory(sqlite-library)

add_executable(VendingMachine main.cpp)

target_link_libraries(VendingMachine
        PUBLIC
        sqlite-library
)