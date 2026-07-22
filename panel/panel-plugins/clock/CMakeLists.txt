add_library(clock MODULE
    clock.cpp
    clocksettingswidget.cpp
    clock.h
    clocksettingswidget.h
    clocksettingswidget.ui
)

target_compile_definitions(clock PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)
set_target_properties(clock PROPERTIES CXX_STANDARD 11)

target_link_libraries(clock PRIVATE Qt6::Core Qt6::Gui Qt6::DBus Qt6::Widgets)
forest_link_panel_library(clock)
forest_link_miscutills(clock)

install(TARGETS clock LIBRARY DESTINATION lib/forest/panel)
