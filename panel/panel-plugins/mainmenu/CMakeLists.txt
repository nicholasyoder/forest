add_library(mainmenu MODULE
    apploader.cpp
    contextmenu.cpp
    itemhandler.cpp
    mainmenu.cpp
    menuitem.cpp
    menupage.cpp
    propertieswidget.cpp
    apploader.h
    contextmenu.h
    globals.h
    itemhandler.h
    mainmenu.h
    menuitem.h
    menupage.h
    propertieswidget.h
    propertieswidget.ui
)

target_compile_definitions(mainmenu PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)
set_target_properties(mainmenu PROPERTIES CXX_STANDARD 11)

target_include_directories(mainmenu PRIVATE ${CMAKE_SOURCE_DIR}/library)

target_link_libraries(mainmenu PRIVATE Qt6::Core Qt6::Gui Qt6::DBus Qt6::Widgets Qt6Xdg)
forest_link_panel_library(mainmenu)
forest_link_miscutills(mainmenu)

install(TARGETS mainmenu LIBRARY DESTINATION usr/lib/forest/panel)
