add_library(desktop-app MODULE
    desktop.cpp
    global_settings.cpp
    wallpaperwidget.cpp
    iconswidget.cpp
    desktopicon.cpp
    desktop.h
    global_settings.h
    wallpaperwidget.h
    iconswidget.h
    desktopicon.h
)

target_compile_definitions(desktop-app PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)
set_target_properties(desktop-app PROPERTIES CXX_STANDARD 11)

target_link_libraries(desktop-app PRIVATE
    Qt6::Widgets Qt6::DBus
    Qt6Xdg
)
forest_link_miscutills(desktop-app)

install(TARGETS desktop-app LIBRARY DESTINATION usr/lib/forest)
