add_library(session-settings MODULE
    autostart/autostartsettings.cpp
    autostart/autostartsettings.h
    general/generalsettings.cpp
    general/generalsettings.h
    sessionsettings.cpp
    sessionsettings.h
)

target_compile_definitions(session-settings PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)
set_target_properties(session-settings PROPERTIES CXX_STANDARD 17)

target_link_libraries(session-settings PRIVATE Qt6::Widgets)
forest_link_pluginutills(session-settings)
forest_link_miscutills(session-settings)

install(TARGETS session-settings LIBRARY DESTINATION usr/lib/forest)
