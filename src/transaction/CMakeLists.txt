set(LIB_NAME Transaction)

add_library(${LIB_NAME} INTERFACE)
add_library(Model::${LIB_NAME} ALIAS ${LIB_NAME})

target_include_directories(${LIB_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})