add_library(batterymonitor MODULE
    battery.cpp
    batterymonitor.cpp
    battery.h
    batterymonitor.h
)

target_compile_definitions(batterymonitor PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)
set_target_properties(batterymonitor PROPERTIES CXX_STANDARD 11)

target_link_libraries(batterymonitor PRIVATE Qt6::Widgets Qt6::DBus)
forest_link_panel_library(batterymonitor)
forest_link_miscutills(batterymonitor)

install(TARGETS batterymonitor LIBRARY DESTINATION usr/lib/forest/panel)
