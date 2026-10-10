add_library(panel-app MODULE
    autohidemanager.cpp
    geometrymanager.cpp
    panel.cpp
    autohidemanager.h
    geometrymanager.h
    panel.h
)

target_compile_definitions(panel-app PRIVATE PLUG_LIBRARY QT_DEPRECATED_WARNINGS)

target_link_libraries(panel-app PRIVATE
    Qt6::Core Qt6::Gui Qt6::DBus Qt6::Widgets
)
forest_link_panel_library(panel-app)
forest_link_miscutills(panel-app)
forest_link_layershellqt(panel-app)
forest_link_activation(panel-app)

install(TARGETS panel-app LIBRARY DESTINATION lib/forest)
