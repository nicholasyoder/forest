add_library(windowlist MODULE
    imagepopup.cpp
    settingswidget.cpp
    windowbutton.cpp
    windowlist.cpp
    iconresolver.cpp
    closebutton.h
    imagepopup.h
    settingswidget.h
    windowbutton.h
    windowlist.h
    iconresolver.h
    settingswidget.ui
)

target_compile_definitions(windowlist PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)

target_include_directories(windowlist PRIVATE ${CMAKE_SOURCE_DIR}/library)

target_link_libraries(windowlist PRIVATE
    Qt6::Widgets Qt6::DBus
)
forest_link_panel_library(windowlist)
forest_link_miscutills(windowlist)
forest_link_toplevels(windowlist)

install(TARGETS windowlist LIBRARY DESTINATION lib/forest/panel)
