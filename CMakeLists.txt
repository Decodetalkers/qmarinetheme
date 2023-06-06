cmake_minimum_required(VERSION 3.16)
project(styleplugin LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)

add_subdirectory(stylewindow)
add_subdirectory(styles)
add_subdirectory(marin_platformplugin)
