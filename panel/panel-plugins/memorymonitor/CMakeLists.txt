add_library(memorymonitor MODULE
    memmon.cpp
    settingswidget.cpp
    memmon.h
    settingswidget.h
    settingswidget.ui
)

target_compile_definitions(memorymonitor PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)

target_link_libraries(memorymonitor PRIVATE Qt6::Widgets Qt6::DBus)
forest_link_panel_library(memorymonitor)
forest_link_miscutills(memorymonitor)

install(TARGETS memorymonitor LIBRARY DESTINATION lib/forest/panel)
