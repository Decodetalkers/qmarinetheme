set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(DtkCMake REQUIRED)
find_package(DtkGui REQUIRED)
find_package(DtkCore REQUIRED)
find_package(DtkWidget REQUIRED)

add_library(chameleonstyleplugin MODULE
  resources.qrc
  chameleonstyle.h
  chameleonstyle.cpp
  chameleonstyle/chameleontools.hpp
  chameleonstyle/primitive.cpp
  chameleonstyle/drawcontrol.cpp
  chameleonstyle/subelementreact.cpp
  chameleonstyle/drawcomplexcontrol.cpp
  chameleonstyle/drawhittestcomplexcontrol.cpp
  chameleonstyle/subcontrolrect.cpp
  chameleonstyle/sizefromcontents.cpp
  chameleonstyle/pixelmetric.cpp
  chameleonstyle/sizehint.cpp
  chameleonstyle/polish.cpp
  chameleonstyle/unpolish.cpp
  chameleonstyle/generatedbrush.cpp
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
