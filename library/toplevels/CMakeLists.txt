add_library(toplevels STATIC
    topleveltracker.cpp
    foreigntoplevelmanager.cpp
    foreigntoplevelhandle.cpp
    extforeigntoplevellist.cpp
    extforeigntoplevelhandle.cpp
    extworkspacemanager.cpp
    extworkspacehandle.cpp
    biomeworkspaces.cpp
    topleveltracker.h
    foreigntoplevelmanager.h
    foreigntoplevelhandle.h
    extforeigntoplevellist.h
    extforeigntoplevelhandle.h
    extworkspacemanager.h
    extworkspacehandle.h
    biomeworkspaces.h
)

set_target_properties(toplevels PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(toplevels PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(toplevels PUBLIC Qt6::Core Qt6::Gui Qt6::DBus Qt6::WaylandClient)

pkg_get_variable(WAYLAND_PROTOCOLS_DATADIR wayland-protocols pkgdatadir)
qt6_generate_wayland_protocol_client_sources(toplevels FILES
    ${CMAKE_SOURCE_DIR}/protocol/wlr-foreign-toplevel-management-unstable-v1.xml
    "${WAYLAND_PROTOCOLS_DATADIR}/staging/ext-foreign-toplevel-list/ext-foreign-toplevel-list-v1.xml"
    "${WAYLAND_PROTOCOLS_DATADIR}/staging/ext-workspace/ext-workspace-v1.xml")
# PUBLIC so users including the handle headers find the generated qwayland-*.h.
target_include_directories(toplevels PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

install(TARGETS toplevels ARCHIVE DESTINATION lib)
