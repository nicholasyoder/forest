add_library(deskswitch MODULE
    deskswitch.cpp
    deskbutton.cpp
    deskswitch.h
    deskbutton.h
)

target_compile_definitions(deskswitch PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)
set_target_properties(deskswitch PROPERTIES CXX_STANDARD 11)

target_include_directories(deskswitch PRIVATE ${CMAKE_SOURCE_DIR}/library)

target_link_libraries(deskswitch PRIVATE
    Qt6::Core Qt6::Gui Qt6::DBus Qt6::Widgets
    KF6WindowSystem
)
forest_link_panel_library(deskswitch)
forest_link_miscutills(deskswitch)
forest_link_xcbutills(deskswitch)

install(TARGETS deskswitch LIBRARY DESTINATION lib/forest/panel)
