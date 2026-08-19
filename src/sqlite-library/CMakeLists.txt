find_package(SQLite3 REQUIRED)

set(LIB_NAME sqlite-library)

aux_source_directory(. SRC_FILES)

add_library(${LIB_NAME} STATIC ${SRC_FILES})
add_library(Device::${LIB_NAME} ALIAS ${LIB_NAME})

target_link_libraries(${LIB_NAME} PUBLIC
        SQLite::SQLite3
        Model::Transaction
)

target_include_directories(${LIB_NAME} INTERFACE .)

add_subdirectory(tests)