add_library(memorymonitor MODULE
    memmon.cpp
    memmon.h
    memorymonitorconfig.h
)

target_compile_definitions(memorymonitor PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)

target_link_libraries(memorymonitor PRIVATE Qt6::Widgets Qt6::DBus)
forest_link_panel_library(memorymonitor)
forest_link_miscutills(memorymonitor)

install(TARGETS memorymonitor LIBRARY DESTINATION lib/forest/panel)

forest_add_applet_settings(memorymonitor
    memorymonitorsettings.cpp
    memorymonitorsettings.h
    memorymonitorconfig.h
)
