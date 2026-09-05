add_library(deskswitch MODULE
    deskswitch.cpp
    deskbutton.cpp
    deskswitch.h
    deskbutton.h
)

target_compile_definitions(deskswitch PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)
# QWaylandClientExtensionTemplate (via panel-library's ExtWorkspaceManager)
# needs C++17 (if constexpr) - see windowlist/CMakeLists.txt's identical note.
set_target_properties(deskswitch PROPERTIES CXX_STANDARD 17)

target_include_directories(deskswitch PRIVATE ${CMAKE_SOURCE_DIR}/library)

target_link_libraries(deskswitch PRIVATE
    Qt6::Core Qt6::Gui Qt6::DBus Qt6::Widgets
)
forest_link_panel_library(deskswitch)
forest_link_miscutills(deskswitch)

install(TARGETS deskswitch LIBRARY DESTINATION lib/forest/panel)
