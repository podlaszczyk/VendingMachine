set(LIB_NAME synchronization)

add_library(${LIB_NAME} STATIC
        SyncWorker.cpp
        TransactionJson.cpp
)
add_library(Sync::Worker ALIAS ${LIB_NAME})

target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${LIB_NAME} PUBLIC
        Model::Transaction
        Device::sqlite-library
        PRIVATE
        Project::Coverage
)

add_subdirectory(tests)
