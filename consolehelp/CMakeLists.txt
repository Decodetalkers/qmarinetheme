set(BIN_NAME marinehelp${QT_VERSION})

find_package(Qt${QT_VERSION} REQUIRED COMPONENTS Core Gui Widgets)

add_executable(${BIN_NAME}
  main.cpp
)

target_link_libraries(${BIN_NAME} PRIVATE
  Qt::Core
  Qt::Gui
  Qt::GuiPrivate
  Qt::Widgets
)

install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
