set(LIB_NAME marinePlatformTheme)

find_package(Qt${QT_VERSION} REQUIRED COMPONENTS Core Gui Widgets DBus)
find_package(tomlplusplus REQUIRED)

if(${QT_VERSION} STREQUAL "5" AND ${SUPPORT_KDE})
  add_definitions(-DSUPPORT_KDE)
  find_package(KF5Config REQUIRED)
  find_package(KF5ConfigWidgets REQUIRED)
  find_package(KF5IconThemes REQUIRED)
  find_package(Qt5ServiceSupport)
  find_package(Qt5ThemeSupport REQUIRED)

endif()

if (${QT_VERSION} STREQUAL "6" AND ${AS_KDE6_PLUGIN})
  add_definitions(-DUSE_AS_KDE_PLUGIN)
endif()

add_library(${LIB_NAME} MODULE
  marinePlatformTheme.h marinePlatformTheme.cpp
  marineThemePlugin.h marineThemePlugin.cpp
  qxdgdesktopportalfiledialog_p.h qxdgdesktopportalfiledialog.cpp
)

set_target_properties(${LIB_NAME} PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "$<TARGET_FILE_DIR:styleplugin>/platformthemes"
)

target_link_libraries(${LIB_NAME} PRIVATE
  Qt::Core
  Qt::Gui
  Qt::GuiPrivate
  Qt::Widgets
  Qt::DBus
)

if(${QT_VERSION} STREQUAL "5" AND ${SUPPORT_KDE})
  target_link_libraries(${LIB_NAME} PRIVATE
    KF5::ConfigCore
    KF5::ConfigWidgets
    KF5::IconThemes
    Qt5::ThemeSupportPrivate
    Qt5::ServiceSupportPrivate
  )
endif()

install(TARGETS ${LIB_NAME} DESTINATION ${QT_PLUGIN_DIR}/platformthemes)
