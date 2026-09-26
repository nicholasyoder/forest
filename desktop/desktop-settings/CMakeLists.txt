add_library(desktop-settings MODULE
    desktopsettings.cpp
    desktopsettings.h
    settingswidget.ui
)

target_compile_definitions(desktop-settings PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)

target_link_libraries(desktop-settings PRIVATE
    Qt6::Widgets Qt6::DBus
)
forest_link_pluginutills(desktop-settings)
forest_link_miscutills(desktop-settings)

install(TARGETS desktop-settings LIBRARY DESTINATION lib/forest)
