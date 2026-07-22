add_library(sensors MODULE
    widgetsensorconf.cpp
    sensorwidget.cpp
    sensor/sensors.cpp
    sensor/chip.cpp
    sensor/feature.cpp
    widgetsensorconf.h
    sensorwidget.h
    sensor/sensors.h
    sensor/chip.h
    sensor/feature.h
    widgetsensorconf.ui
)

target_compile_definitions(sensors PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)

target_link_libraries(sensors PRIVATE Qt6::Widgets Qt6::DBus)
forest_link_panel_library(sensors)

target_include_directories(sensors PRIVATE ${SENSORS_INCLUDE_DIR})
target_link_libraries(sensors PRIVATE ${SENSORS_LIBRARY})

forest_link_miscutills(sensors)

install(TARGETS sensors LIBRARY DESTINATION lib/forest/panel)
