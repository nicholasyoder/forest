add_library(systray MODULE
    systray.cpp
    trayicon.cpp
    systray.h
    trayicon.h
)

target_compile_definitions(systray PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)

target_include_directories(systray PRIVATE ${CMAKE_SOURCE_DIR}/library)

target_link_libraries(systray PRIVATE
    Qt6::Core Qt6::Gui Qt6::DBus Qt6::Widgets
)
forest_link_panel_library(systray)
forest_link_miscutills(systray)
forest_link_dbusmenu(systray)

install(TARGETS systray LIBRARY DESTINATION lib/forest/panel)
