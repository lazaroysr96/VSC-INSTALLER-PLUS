cmake_minimum_required(VERSION 3.16)
project(VSC-INSTALLER-PLUS VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Sql Network)

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/Installer.cpp
)

set(HEADERS
    src/MainWindow.h
    src/Installer.h
)

qt5_add_resources(RESOURCES assets/resources.qrc)

qt5_wrap_ui(UI_HEADERS ui/MainWindow.ui)

include_directories(${CMAKE_BINARY_DIR})

add_executable(VSC-INSTALLER-PLUS
    ${SOURCES}
    ${HEADERS}
    ${UI_HEADERS}
    ${RESOURCES}
)

target_link_libraries(VSC-INSTALLER-PLUS
    Qt5::Core
    Qt5::Widgets
    Qt5::Sql
    Qt5::Network
)

install(TARGETS VSC-INSTALLER-PLUS
    RUNTIME DESTINATION bin
)
