add_library(services-settings MODULE
    hotkeys/edithotkeywidget.cpp
    hotkeys/hotkeysettings.cpp
    notifications/notificationssettings.cpp
    servicessettings.cpp
    hotkeys/edithotkeywidget.h
    hotkeys/hotkeysettings.h
    notifications/notificationssettings.h
    servicessettings.h
    hotkeys/edithotkeywidget.ui
)

target_compile_definitions(services-settings PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)
set_target_properties(services-settings PROPERTIES CXX_STANDARD 17)

target_link_libraries(services-settings PRIVATE Qt6::Widgets Qt6::DBus)
forest_link_pluginutills(services-settings)
forest_link_miscutills(services-settings)

install(TARGETS services-settings LIBRARY DESTINATION usr/lib/forest)
