set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(simplestyleplugin SHARED
  simplestyle.cpp simplestyle.h
  simplestyleplugin.cpp simplestyleplugin.h)

set_target_properties(simplestyleplugin PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "$<TARGET_FILE_DIR:styleplugin>/styles"
)

target_link_libraries(simplestyleplugin PRIVATE
  Qt::Core
  Qt::Gui
  Qt::Widgets)
