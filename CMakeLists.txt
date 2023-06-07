cmake_minimum_required(VERSION 3.16)
project(styleplugin LANGUAGES CXX)

set(QT_PLUGIN_DIR "/usr/lib/qt6/plugins" CACHE STRING "PLUGIN DIR For qt")

set(QT_VERSION "6" CACHE STRING "Qt Version")
include(GNUInstallDirs)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(stylewindow)
add_subdirectory(styles)
add_subdirectory(platformplugins)
