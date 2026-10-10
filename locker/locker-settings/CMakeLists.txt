add_library(locker-settings MODULE
    lockersettingsplugin.cpp
    lockersettingsplugin.h
)

target_compile_definitions(locker-settings PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)

target_link_libraries(locker-settings PRIVATE Qt6::Widgets lockerconfig)
forest_link_pluginutills(locker-settings)
forest_link_settings_widgets(locker-settings)

install(TARGETS locker-settings LIBRARY DESTINATION lib/forest/settings)
