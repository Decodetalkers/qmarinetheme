set(LIB_NAME marinePlatformTheme)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(tomlplusplus REQUIRED)

add_library(${LIB_NAME} MODULE
  marinePlatformTheme.h marinePlatformTheme.cpp
  marineThemePlugin.h marineThemePlugin.cpp
)

set_target_properties(${LIB_NAME} PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "$<TARGET_FILE_DIR:styleplugin>/platformthemes"
)

target_link_libraries(${LIB_NAME} PRIVATE
  Qt::Core
  Qt::Gui
  Qt::GuiPrivate
  Qt::Widgets)
