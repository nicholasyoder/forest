add_library(panel-library STATIC
    panelbutton.cpp
    panelanchor.cpp
    graphwidget.cpp
    panelpluginterface.h
    popup.h
    panelanchor.h
    panelbutton.h
    graphwidget.h
)

set_target_properties(panel-library PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(panel-library PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(panel-library PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets Qt6::DBus Qt6Xdg Qt6::WaylandClient)
forest_link_miscutills(panel-library)
target_link_libraries(panel-library PUBLIC menuanchor)

install(TARGETS panel-library ARCHIVE DESTINATION lib)
