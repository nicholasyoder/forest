add_library(panel-library STATIC
    panelbutton.cpp
    panelanchor.cpp
    graphwidget.cpp
    extworkspacemanager.cpp
    extworkspacehandle.cpp
    biomeworkspaces.cpp
    panelpluginterface.h
    popup.h
    panelanchor.h
    popupmenu.h
    panelbutton.h
    graphwidget.h
    extworkspacemanager.h
    extworkspacehandle.h
    biomeworkspaces.h
)

set_target_properties(panel-library PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(panel-library PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(panel-library PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets Qt6::DBus Qt6Xdg Qt6::WaylandClient)
forest_link_miscutills(panel-library)
target_link_libraries(panel-library PUBLIC menuanchor)

# Shared by deskswitch and windowlist.
pkg_get_variable(WAYLAND_PROTOCOLS_DATADIR wayland-protocols pkgdatadir)
qt6_generate_wayland_protocol_client_sources(panel-library FILES
    "${WAYLAND_PROTOCOLS_DATADIR}/staging/ext-workspace/ext-workspace-v1.xml")
# PUBLIC so plugins including extworkspacemanager.h find the generated header.
target_include_directories(panel-library PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

install(TARGETS panel-library ARCHIVE DESTINATION lib)
