set(LIB_NAME dispenser-interface)

add_library(${LIB_NAME} INTERFACE)
add_library(Hardware::Dispenser ALIAS ${LIB_NAME})

target_include_directories(${LIB_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${LIB_NAME} INTERFACE VendingMachine::Types)
