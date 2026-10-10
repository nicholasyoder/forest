add_library(volume MODULE
    alsadevice.cpp
    alsaengine.cpp
    audiodevice.cpp
    audioengine.cpp
    fvolume.cpp
    alsadevice.h
    alsaengine.h
    audiodevice.h
    audioengine.h
    fvolume.h
    volumeconfig.h
)

target_compile_definitions(volume PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)

target_link_libraries(volume PRIVATE Qt6::Widgets Qt6::DBus PkgConfig::ALSA Qt6Xdg)
forest_link_panel_library(volume)
forest_link_miscutills(volume)

install(TARGETS volume LIBRARY DESTINATION lib/forest/panel)

forest_add_applet_settings(volume
    volumesettings.cpp
    volumesettings.h
    volumeconfig.h
    alsadevice.cpp
    alsaengine.cpp
    audiodevice.cpp
    audioengine.cpp
)
target_link_libraries(volume-settings PRIVATE PkgConfig::ALSA)
