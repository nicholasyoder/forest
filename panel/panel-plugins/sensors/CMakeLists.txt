add_library(sensors MODULE
    sensorwidget.cpp
    sensor/sensors.cpp
    sensor/chip.cpp
    sensor/feature.cpp
    sensorwidget.h
    sensorsconfig.h
    sensor/sensors.h
    sensor/chip.h
    sensor/feature.h
)

target_compile_definitions(sensors PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)

target_link_libraries(sensors PRIVATE Qt6::Widgets Qt6::DBus)
forest_link_panel_library(sensors)

target_include_directories(sensors PRIVATE ${SENSORS_INCLUDE_DIR})
target_link_libraries(sensors PRIVATE ${SENSORS_LIBRARY})

forest_link_miscutills(sensors)

install(TARGETS sensors LIBRARY DESTINATION lib/forest/panel)

forest_add_applet_settings(sensors
    sensorssettings.cpp
    sensorssettings.h
    sensorsconfig.h
    sensor/sensors.cpp
    sensor/chip.cpp
    sensor/feature.cpp
)
target_include_directories(sensors-settings PRIVATE ${SENSORS_INCLUDE_DIR})
target_link_libraries(sensors-settings PRIVATE ${SENSORS_LIBRARY})
