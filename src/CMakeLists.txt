add_subdirectory(transaction)
add_subdirectory(sqlite-library)
add_subdirectory(vending-machine)
add_subdirectory(synchronization)
add_subdirectory(tests)

find_package(Qt6 REQUIRED COMPONENTS
        Quick
        Qml
)
qt_policy(SET QTP0001 NEW)
set(APPLICATION VendingMachine)

qt_standard_project_setup()
set(CMAKE_AUTORCC ON)

qt_add_executable(${APPLICATION}
        main.cpp
)

qt_add_qml_module(${APPLICATION}
        URI MyQmlApp
        VERSION 1.0
        QML_FILES
        qml/Main.qml
)

target_link_libraries(${APPLICATION}
        PRIVATE
        Qt6::Quick
        Qt6::Qml
        PUBLIC
        sqlite-library
)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    add_custom_target(package
            COMMAND ${CMAKE_COMMAND} -E rm -rf
            "${CMAKE_BINARY_DIR}/AppDir"

            COMMAND ${CMAKE_COMMAND} -E env
            QMAKE=/usr/local/Qt/6.5.3/gcc_64/bin/qmake
            QML_SOURCES_PATHS=/${CMAKE_CURRENT_SOURCE_DIR}/qml
            QML_MODULES_PATHS=/usr/local/Qt/6.5.3/gcc_64/qml
            LD_LIBRARY_PATH=/usr/local/Qt/6.5.3/gcc_64/lib
            linuxdeploy
            --appdir "${CMAKE_BINARY_DIR}/AppDir"
            --executable "$<TARGET_FILE:${APPLICATION}>"
            --plugin qt

            DEPENDS ${APPLICATION}
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            COMMENT "Packaging ${APPLICATION} with linuxdeploy"
    )
endif ()
