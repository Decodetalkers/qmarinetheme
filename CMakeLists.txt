cmake_minimum_required(VERSION 3.16)
project(qmarinetheme
  VERSION 0.1.6
  DESCRIPTION "A theme settting config with toml"
  LANGUAGES CXX
)

set(QT_PLUGIN_DIR "/usr/lib/qt6/plugins" CACHE STRING "PLUGIN DIR For qt")

option(SUPPORT_KDE "support for kde icon themes" ON)
option(AS_KDE6_PLUGIN "use as the qt platformplugin for kde" OFF)

set(QT_VERSION "6" CACHE STRING "Qt Version")
include(GNUInstallDirs)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 20)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DDEBUGMODE)
endif()
add_subdirectory(stylewindow)
add_subdirectory(styles)
add_subdirectory(platformplugins)
add_subdirectory(consolehelp)
