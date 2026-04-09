add_library(systray-sni MODULE
    systray.cpp
    trayicon.cpp
    systray.h
    trayicon.h
)

target_compile_definitions(systray-sni PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)

target_include_directories(systray-sni PRIVATE ${CMAKE_SOURCE_DIR}/library)

target_link_libraries(systray-sni PRIVATE
    Qt6::Core Qt6::Gui Qt6::DBus Qt6::Widgets
    KF6WindowSystem
    PkgConfig::XCB
)
forest_link_panel_library(systray-sni)
forest_link_miscutills(systray-sni)
forest_link_xcbutills(systray-sni)

install(TARGETS systray-sni LIBRARY DESTINATION usr/lib/forest/panel)
