cmake_minimum_required(VERSION 3.16)

project(JustinChat VERSION 0.1
        LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(JustinChat
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        Icons.qrc
        logindialog.h logindialog.cpp logindialog.ui
        Pictures.qrc
        registerdialog.h registerdialog.cpp registerdialog.ui
        Styles.qrc
        global.h
        global.cpp
        singleton.h
        httpmgr.h httpmgr.cpp
        config.ini
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET JustinChat APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(JustinChat SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(JustinChat
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(JustinChat PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network)

# 复制 config.ini 到输出目录（仅 Release 模式）
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CONFIG_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/config.ini")
    set(CONFIG_DESTINATION "$<TARGET_FILE_DIR:JustinChat>/config.ini")

    add_custom_command(
        TARGET JustinChat
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "${CONFIG_SOURCE}"
            "${CONFIG_DESTINATION}"
        COMMENT "Copying config.ini to output directory (Release)"
        VERBATIM
    )
endif()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.JustinChat)
endif()
set_target_properties(JustinChat PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS JustinChat
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(JustinChat)
endif()
