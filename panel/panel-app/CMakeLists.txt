add_library(panel-app MODULE
    autohidemanager.cpp
    geometrymanager.cpp
    hiddenpanel.cpp
    panel.cpp
    autohidemanager.h
    geometrymanager.h
    hiddenpanel.h
    panel.h
)

target_compile_definitions(panel-app PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)
set_target_properties(panel-app PROPERTIES CXX_STANDARD 11)

target_link_libraries(panel-app PRIVATE
    Qt6::Core Qt6::Gui Qt6::DBus Qt6::Widgets
    KF6WindowSystem
)
forest_link_panel_library(panel-app)
forest_link_xcbutills(panel-app)
forest_link_miscutills(panel-app)
forest_link_layershellqt(panel-app)

install(TARGETS panel-app LIBRARY DESTINATION lib/forest)
