add_library(clock MODULE
    clock.cpp
    clock.h
    clockconfig.h
)

target_compile_definitions(clock PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)

target_link_libraries(clock PRIVATE Qt6::Core Qt6::Gui Qt6::DBus Qt6::Widgets)
forest_link_panel_library(clock)
forest_link_miscutills(clock)

install(TARGETS clock LIBRARY DESTINATION lib/forest/panel)

forest_add_applet_settings(clock
    clocksettings.cpp
    clocksettings.h
    clockconfig.h
)
