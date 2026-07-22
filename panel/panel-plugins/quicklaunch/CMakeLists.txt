add_library(quicklaunch MODULE
    launcher.cpp
    quicklaunch.cpp
    launcher.h
    quicklaunch.h
)

target_compile_definitions(quicklaunch PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)

target_link_libraries(quicklaunch PRIVATE Qt6::Widgets Qt6::DBus Qt6Xdg)
forest_link_panel_library(quicklaunch)
forest_link_miscutills(quicklaunch)

install(TARGETS quicklaunch LIBRARY DESTINATION lib/forest/panel)
