add_library(panel-library STATIC
    panelbutton.cpp
    graphwidget.cpp
    panelpluginterface.h
    popup.h
    popupmenu.h
    panelbutton.h
    graphwidget.h
)

set_target_properties(panel-library PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(panel-library PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(panel-library PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets Qt6Xdg)
forest_link_miscutills(panel-library)

install(TARGETS panel-library ARCHIVE DESTINATION lib)
