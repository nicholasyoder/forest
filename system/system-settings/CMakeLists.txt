add_library(system-settings MODULE
    aboutpage.cpp
    cursorthemesettings.cpp
    forestthemesettings.cpp
    systemsettings.cpp
    displays/arrangementcanvas.cpp
    displays/displayspage.cpp
    aboutpage.h
    cursorthemesettings.h
    forestthemesettings.h
    systemsettings.h
    displays/arrangementcanvas.h
    displays/displayspage.h
)

target_compile_definitions(system-settings PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)

target_link_libraries(system-settings PRIVATE
    Qt6::Widgets Qt6::DBus
    PkgConfig::XCURSOR
)
forest_link_pluginutills(system-settings)
forest_link_settings_widgets(system-settings)
forest_link_miscutills(system-settings)
forest_link_outputs(system-settings)

install(TARGETS system-settings LIBRARY DESTINATION lib/forest)
