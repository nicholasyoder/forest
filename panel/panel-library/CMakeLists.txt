add_library(panel-library STATIC
    panelbutton.cpp
    graphwidget.cpp
    extworkspacemanager.cpp
    extworkspacehandle.cpp
    panelpluginterface.h
    popup.h
    popupmenu.h
    panelbutton.h
    graphwidget.h
    extworkspacemanager.h
    extworkspacehandle.h
)

set_target_properties(panel-library PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(panel-library PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(panel-library PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets Qt6Xdg Qt6::WaylandClient)
forest_link_miscutills(panel-library)

# ext-workspace-v1 (extworkspacemanager.cpp/extworkspacehandle.cpp) is part
# of the wayland-protocols package's staging/ directory, unlike
# wlr-foreign-toplevel-management-unstable-v1.xml (vendored in
# protocol/, since it originates in wlroots' own protocol/ dir and isn't
# installed by any system package) - so it's referenced from the system
# path directly rather than vendored, per this project's decoupling-goal
# preference for standard protocols/paths over vendoring. Shared here
# (rather than duplicated in deskswitch/ and windowlist/, both of which
# need a live workspace list) since panel-library is exactly the "shared
# across panel plugins" spot other panel-wide code already lives in.
pkg_get_variable(WAYLAND_PROTOCOLS_DATADIR wayland-protocols pkgdatadir)
qt6_generate_wayland_protocol_client_sources(panel-library FILES
    "${WAYLAND_PROTOCOLS_DATADIR}/staging/ext-workspace/ext-workspace-v1.xml")
# qt6_generate_wayland_protocol_client_sources() only adds the generated
# header to this target's own (PRIVATE) include path - PUBLIC here so
# deskswitch/windowlist, which include extworkspacemanager.h and thus the
# generated qwayland-ext-workspace-v1.h transitively, can find it too.
target_include_directories(panel-library PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

install(TARGETS panel-library ARCHIVE DESTINATION lib)
