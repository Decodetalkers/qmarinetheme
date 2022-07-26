set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#qt_add_plugin(simplestyleplugin
#    CLASS_NAME SimpleStylePlugin
#)

add_library(simplestyleplugin MODULE
    simplestyle.cpp simplestyle.h
    simplestyleplugin.cpp simplestyleplugin.h
)

set_target_properties(simplestyleplugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/stylewindow/styles"
)

target_link_libraries(simplestyleplugin PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
)

install(TARGETS simplestyleplugin
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
