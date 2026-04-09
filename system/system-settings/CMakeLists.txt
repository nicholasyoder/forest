add_library(system-settings MODULE
    aboutpage.cpp
    cursorthemesettings.cpp
    forestthemesettings.cpp
    systemsettings.cpp
    aboutpage.h
    cursorthemesettings.h
    forestthemesettings.h
    systemsettings.h
)

target_compile_definitions(system-settings PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)
set_target_properties(system-settings PROPERTIES CXX_STANDARD 17)

target_link_libraries(system-settings PRIVATE
    Qt6::Widgets Qt6::DBus
    PkgConfig::X11
    PkgConfig::XCURSOR
)
forest_link_pluginutills(system-settings)
forest_link_settings_widgets(system-settings)
forest_link_miscutills(system-settings)
forest_link_xcbutills(system-settings)

install(TARGETS system-settings LIBRARY DESTINATION usr/lib/forest)
