add_subdirectory(sqlite-library)

find_package(Qt6 EXACT 6.5.3 COMPONENTS Core REQUIRED)
find_package(Qt6 EXACT 6.5.3 COMPONENTS Widgets REQUIRED)

set(APPLICATION VendingMachine)

qt_standard_project_setup()
set(CMAKE_AUTORCC ON)

qt_add_executable(${APPLICATION}
        main.cpp
        MainWindow.cpp
)

target_link_libraries(${APPLICATION} PRIVATE
        Qt6::Widgets
        PUBLIC
        sqlite-library
)


if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    add_custom_target(package
            COMMAND ${CMAKE_COMMAND} -E env
            QMAKE=/usr/local/Qt/6.5.3/gcc_64/bin/qmake
            linuxdeploy
            --appdir "${CMAKE_BINARY_DIR}/AppDir"
            --executable "$<TARGET_FILE:${APPLICATION}>"
            --plugin qt
            --output appimage
            DEPENDS ${APPLICATION}
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            COMMENT "Packaging ${APPLICATION} with linuxdeploy"
    )
endif()
