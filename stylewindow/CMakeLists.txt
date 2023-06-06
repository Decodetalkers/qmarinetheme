set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(Dtk REQUIRED COMPONENTS Widget)

add_executable(
  styleplugin
  main.cpp
  norwegianwoodstyle.h
  norwegianwoodstyle.cpp
  widgetgallery.h
  widgetgallery.cpp
  styles.qrc)

target_link_libraries(
  styleplugin PRIVATE
  Qt5::Widgets
  ${DtkWidget_LIBRARIES})
