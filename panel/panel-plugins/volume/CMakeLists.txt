add_library(volume MODULE
    alsadevice.cpp
    alsaengine.cpp
    audiodevice.cpp
    audioengine.cpp
    fvolume.cpp
    settingswidget.cpp
    alsadevice.h
    alsaengine.h
    audiodevice.h
    audioengine.h
    fvolume.h
    settingswidget.h
    settingswidget.ui
)

target_compile_definitions(volume PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)

target_link_libraries(volume PRIVATE Qt6::Widgets Qt6::DBus PkgConfig::ALSA Qt6Xdg)
forest_link_panel_library(volume)
forest_link_miscutills(volume)

install(TARGETS volume LIBRARY DESTINATION lib/forest/panel)
