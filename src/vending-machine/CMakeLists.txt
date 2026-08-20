add_library(vending-machine-types INTERFACE)
add_library(VendingMachine::Types ALIAS vending-machine-types)
target_include_directories(vending-machine-types INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(dispenser)
add_subdirectory(card-reader)

set(LIB_NAME vending-machine-core)

add_library(${LIB_NAME} STATIC
        VendingController.cpp
        VendingMachine.cpp
)
add_library(VendingMachine::Core ALIAS ${LIB_NAME})

target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${LIB_NAME} PUBLIC
        Hardware::CardReader
        Hardware::Dispenser
        VendingMachine::Types
)

add_subdirectory(tests)
