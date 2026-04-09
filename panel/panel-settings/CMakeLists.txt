add_library(panel-settings MODULE
    panelsettings.cpp
    panelsettings.h
)

target_compile_definitions(panel-settings PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)
set_target_properties(panel-settings PROPERTIES CXX_STANDARD 17)

target_link_libraries(panel-settings PRIVATE Qt6::Widgets Qt6::DBus)
forest_link_panel_library(panel-settings)
forest_link_settings_widgets(panel-settings)
forest_link_pluginutills(panel-settings)
forest_link_miscutills(panel-settings)

install(TARGETS panel-settings LIBRARY DESTINATION usr/lib/forest)
