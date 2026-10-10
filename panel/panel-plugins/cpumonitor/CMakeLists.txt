add_library(cpumonitor MODULE
    cpumon.cpp
    cpumon.h
    cpumonitorconfig.h
)

target_compile_definitions(cpumonitor PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)

target_link_libraries(cpumonitor PRIVATE Qt6::Widgets Qt6::DBus)
forest_link_panel_library(cpumonitor)
forest_link_miscutills(cpumonitor)

install(TARGETS cpumonitor LIBRARY DESTINATION lib/forest/panel)

forest_add_applet_settings(cpumonitor
    cpumonitorsettings.cpp
    cpumonitorsettings.h
    cpumonitorconfig.h
)
