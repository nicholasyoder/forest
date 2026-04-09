add_library(nmcontrol MODULE
    nmcontrol.cpp
    nmcontrol.h
)

target_compile_definitions(nmcontrol PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)
set_target_properties(nmcontrol PROPERTIES CXX_STANDARD 11)

target_link_libraries(nmcontrol PRIVATE Qt6::Core Qt6::Gui Qt6::DBus Qt6::Widgets)
forest_link_panel_library(nmcontrol)
forest_link_miscutills(nmcontrol)

install(TARGETS nmcontrol LIBRARY DESTINATION usr/lib/forest/panel)
