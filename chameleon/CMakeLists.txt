set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTORCC ON)

find_package(DtkCMake REQUIRED)
find_package(DtkGui REQUIRED)
find_package(DtkCore REQUIRED)
find_package(DtkWidget REQUIRED)

add_library(chameleonstyleplugin MODULE
  resources.qrc
  chameleonstyle.h
  chameleonstyle.cpp
  common.h 
  dstyleanimation.h
  dstyleanimation.cpp

  chameleonstyleplugin.h
  chameleonstyleplugin.cpp
)

set_target_properties(chameleonstyleplugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/stylewindow/styles"
)

target_link_libraries(chameleonstyleplugin PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    ${DtkGui_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${DtkWidget_LIBRARIES}
)
target_include_directories(chameleonstyleplugin PUBLIC
  ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
  ${Qt5Widgets_PRIVATE_INCLUDE_DIRS}
)
install(TARGETS chameleonstyleplugin
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
