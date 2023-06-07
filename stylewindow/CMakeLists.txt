set(CMAKE_AUTORCC ON)

find_package(Qt${QT_VERSION} REQUIRED COMPONENTS Widgets)

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
  Qt::Widgets
)
