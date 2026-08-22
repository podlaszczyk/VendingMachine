set(LIB_NAME AppController)

aux_source_directory(. SRC_FILES)

add_library(${LIB_NAME} STATIC ${SRC_FILES})
add_library(App::Controller ALIAS ${LIB_NAME})
set_target_properties(${LIB_NAME} PROPERTIES AUTOMOC ON)

target_link_libraries(${LIB_NAME} PUBLIC
        Model::Transaction
        VendingMachine::Core
        Device::sqlite-library
        Qt6::Core
        Qt6::Network
        Sync::Worker
        PRIVATE
        Project::Coverage
)


target_include_directories(${LIB_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
