set(LIB_NAME Transaction)

aux_source_directory(. SRC_FILES)

add_library(${LIB_NAME} STATIC ${SRC_FILES})
add_library(Model::${LIB_NAME} ALIAS ${LIB_NAME})

target_include_directories(${LIB_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(tests)